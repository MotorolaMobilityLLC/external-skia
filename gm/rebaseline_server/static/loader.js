/*
 * Loader:
 * Reads GM result reports written out by results.py, and imports
 * them into $scope.extraColumnHeaders and $scope.imagePairs .
 */
var Loader = angular.module(
    'Loader',
    ['ConstantsModule', 'diff_viewer']
);

Loader.directive(
  'resultsUpdatedCallbackDirective',
  ['$timeout',
   function($timeout) {
     return function(scope, element, attrs) {
       if (scope.$last) {
         $timeout(function() {
           scope.resultsUpdatedCallback();
         });
       }
     };
   }
  ]
);

// TODO(epoger): Combine ALL of our filtering operations (including
// truncation) into this one filter, so that runs most efficiently?
// (We would have to make sure truncation still took place after
// sorting, though.)
Loader.filter(
  'removeHiddenImagePairs',
  function(constants) {
    return function(unfilteredImagePairs, hiddenResultTypes, hiddenConfigs,
                    builderSubstring, testSubstring, viewingTab) {
      var filteredImagePairs = [];
      for (var i = 0; i < unfilteredImagePairs.length; i++) {
        var imagePair = unfilteredImagePairs[i];
        var extraColumnValues = imagePair[constants.KEY__EXTRA_COLUMN_VALUES];
        // For performance, we examine the "set" objects directly rather
        // than calling $scope.isValueInSet().
        // Besides, I don't think we have access to $scope in here...
        if (!(true == hiddenResultTypes[extraColumnValues[
                  constants.KEY__EXTRACOLUMN__RESULT_TYPE]]) &&
            !(true == hiddenConfigs[extraColumnValues[
                  constants.KEY__EXTRACOLUMN__CONFIG]]) &&
            !(-1 == extraColumnValues[constants.KEY__EXTRACOLUMN__BUILDER]
                    .indexOf(builderSubstring)) &&
            !(-1 == extraColumnValues[constants.KEY__EXTRACOLUMN__TEST]
                    .indexOf(testSubstring)) &&
            (viewingTab == imagePair.tab)) {
          filteredImagePairs.push(imagePair);
        }
      }
      return filteredImagePairs;
    };
  }
);


Loader.controller(
  'Loader.Controller',
    function($scope, $http, $filter, $location, $log, $timeout, constants) {
    $scope.constants = constants;
    $scope.windowTitle = "Loading GM Results...";
    $scope.resultsToLoad = $location.search().resultsToLoad;
    $scope.loadingMessage = "Loading results of type '" + $scope.resultsToLoad +
        "', please wait...";

    /**
     * On initial page load, load a full dictionary of results.
     * Once the dictionary is loaded, unhide the page elements so they can
     * render the data.
     */
    $http.get("/results/" + $scope.resultsToLoad).success(
      function(data, status, header, config) {
        var dataHeader = data[constants.KEY__HEADER];
        if (dataHeader[constants.KEY__HEADER__IS_STILL_LOADING]) {
          // Apply the server's requested reload delay to local time,
          // so we will wait the right number of seconds regardless of clock
          // skew between client and server.
          var reloadDelayInSeconds =
              dataHeader[constants.KEY__HEADER__TIME_NEXT_UPDATE_AVAILABLE] -
              dataHeader[constants.KEY__HEADER__TIME_UPDATED];
          var timeNow = new Date().getTime();
          var timeToReload = timeNow + reloadDelayInSeconds * 1000;
          $scope.loadingMessage =
              "Server is still loading results; will retry at " +
              $scope.localTimeString(timeToReload / 1000);
          $timeout(
              function(){location.reload();},
              timeToReload - timeNow);
        } else {
          $scope.loadingMessage = "Processing data, please wait...";

          $scope.header = dataHeader;
          $scope.extraColumnHeaders = data[constants.KEY__EXTRACOLUMNHEADERS];
          $scope.imagePairs = data[constants.KEY__IMAGEPAIRS];
          $scope.imageSets = data[constants.KEY__IMAGESETS];
          $scope.sortColumnSubdict = constants.KEY__DIFFERENCE_DATA;
          $scope.sortColumnKey = constants.KEY__DIFFERENCE_DATA__WEIGHTED_DIFF;

          $scope.showSubmitAdvancedSettings = false;
          $scope.submitAdvancedSettings = {};
          $scope.submitAdvancedSettings[
              constants.KEY__EXPECTATIONS__REVIEWED] = true;
          $scope.submitAdvancedSettings[
              constants.KEY__EXPECTATIONS__IGNOREFAILURE] = false;
          $scope.submitAdvancedSettings['bug'] = '';

          // Create the list of tabs (lists into which the user can file each
          // test).  This may vary, depending on isEditable.
          $scope.tabs = [
            'Unfiled', 'Hidden'
          ];
          if (dataHeader[constants.KEY__HEADER__IS_EDITABLE]) {
            $scope.tabs = $scope.tabs.concat(
                ['Pending Approval']);
          }
          $scope.defaultTab = $scope.tabs[0];
          $scope.viewingTab = $scope.defaultTab;

          // Track the number of results on each tab.
          $scope.numResultsPerTab = {};
          for (var i = 0; i < $scope.tabs.length; i++) {
            $scope.numResultsPerTab[$scope.tabs[i]] = 0;
          }
          $scope.numResultsPerTab[$scope.defaultTab] = $scope.imagePairs.length;

          // Add index and tab fields to all records.
          for (var i = 0; i < $scope.imagePairs.length; i++) {
            $scope.imagePairs[i].index = i;
            $scope.imagePairs[i].tab = $scope.defaultTab;
          }

          // Arrays within which the user can toggle individual elements.
          $scope.selectedImagePairs = [];

          // Sets within which the user can toggle individual elements.
          $scope.hiddenResultTypes = {};
          $scope.hiddenResultTypes[
              constants.KEY__RESULT_TYPE__FAILUREIGNORED] = true;
          $scope.hiddenResultTypes[
              constants.KEY__RESULT_TYPE__NOCOMPARISON] = true;
          $scope.hiddenResultTypes[
              constants.KEY__RESULT_TYPE__SUCCEEDED] = true;
          $scope.allResultTypes = Object.keys(
              $scope.extraColumnHeaders[constants.KEY__EXTRACOLUMN__RESULT_TYPE]
                                       [constants.KEY__VALUES_AND_COUNTS]);
          $scope.hiddenConfigs = {};
          $scope.allConfigs = Object.keys(
              $scope.extraColumnHeaders[constants.KEY__EXTRACOLUMN__CONFIG]
                                       [constants.KEY__VALUES_AND_COUNTS]);

          // Associative array of partial string matches per category.
          $scope.categoryValueMatch = {};
          $scope.categoryValueMatch.builder = "";
          $scope.categoryValueMatch.test = "";

          // If any defaults were overridden in the URL, get them now.
          $scope.queryParameters.load();

          $scope.updateResults();
          $scope.loadingMessage = "";
          $scope.windowTitle = "Current GM Results";
        }
      }
    ).error(
      function(data, status, header, config) {
        $scope.loadingMessage = "Failed to load results of type '"
            + $scope.resultsToLoad + "'";
        $scope.windowTitle = "Failed to Load GM Results";
      }
    );


    //
    // Select/Clear/Toggle all tests.
    //

    /**
     * Select all currently showing tests.
     */
    $scope.selectAllImagePairs = function() {
      var numImagePairsShowing = $scope.limitedImagePairs.length;
      for (var i = 0; i < numImagePairsShowing; i++) {
        var index = $scope.limitedImagePairs[i].index;
        if (!$scope.isValueInArray(index, $scope.selectedImagePairs)) {
          $scope.toggleValueInArray(index, $scope.selectedImagePairs);
        }
      }
    }

    /**
     * Deselect all currently showing tests.
     */
    $scope.clearAllImagePairs = function() {
      var numImagePairsShowing = $scope.limitedImagePairs.length;
      for (var i = 0; i < numImagePairsShowing; i++) {
        var index = $scope.limitedImagePairs[i].index;
        if ($scope.isValueInArray(index, $scope.selectedImagePairs)) {
          $scope.toggleValueInArray(index, $scope.selectedImagePairs);
        }
      }
    }

    /**
     * Toggle selection of all currently showing tests.
     */
    $scope.toggleAllImagePairs = function() {
      var numImagePairsShowing = $scope.limitedImagePairs.length;
      for (var i = 0; i < numImagePairsShowing; i++) {
        var index = $scope.limitedImagePairs[i].index;
        $scope.toggleValueInArray(index, $scope.selectedImagePairs);
      }
    }


    //
    // Tab operations.
    //

    /**
     * Change the selected tab.
     *
     * @param tab (string): name of the tab to select
     */
    $scope.setViewingTab = function(tab) {
      $scope.viewingTab = tab;
      $scope.updateResults();
    }

    /**
     * Move the imagePairs in $scope.selectedImagePairs to a different tab,
     * and then clear $scope.selectedImagePairs.
     *
     * @param newTab (string): name of the tab to move the tests to
     */
    $scope.moveSelectedImagePairsToTab = function(newTab) {
      $scope.moveImagePairsToTab($scope.selectedImagePairs, newTab);
      $scope.selectedImagePairs = [];
      $scope.updateResults();
    }

    /**
     * Move a subset of $scope.imagePairs to a different tab.
     *
     * @param imagePairIndices (array of ints): indices into $scope.imagePairs
     *        indicating which test results to move
     * @param newTab (string): name of the tab to move the tests to
     */
    $scope.moveImagePairsToTab = function(imagePairIndices, newTab) {
      var imagePairIndex;
      var numImagePairs = imagePairIndices.length;
      for (var i = 0; i < numImagePairs; i++) {
        imagePairIndex = imagePairIndices[i];
        $scope.numResultsPerTab[$scope.imagePairs[imagePairIndex].tab]--;
        $scope.imagePairs[imagePairIndex].tab = newTab;
      }
      $scope.numResultsPerTab[newTab] += numImagePairs;
    }


    //
    // $scope.queryParameters:
    // Transfer parameter values between $scope and the URL query string.
    //
    $scope.queryParameters = {};

    // load and save functions for parameters of each type
    // (load a parameter value into $scope from nameValuePairs,
    //  save a parameter value from $scope into nameValuePairs)
    $scope.queryParameters.copiers = {
      'simple': {
        'load': function(nameValuePairs, name) {
          var value = nameValuePairs[name];
          if (value) {
            $scope[name] = value;
          }
        },
        'save': function(nameValuePairs, name) {
          nameValuePairs[name] = $scope[name];
        }
      },

      'categoryValueMatch': {
        'load': function(nameValuePairs, name) {
          var value = nameValuePairs[name];
          if (value) {
            $scope.categoryValueMatch[name] = value;
          }
        },
        'save': function(nameValuePairs, name) {
          nameValuePairs[name] = $scope.categoryValueMatch[name];
        }
      },

      'set': {
        'load': function(nameValuePairs, name) {
          var value = nameValuePairs[name];
          if (value) {
            var valueArray = value.split(',');
            $scope[name] = {};
            $scope.toggleValuesInSet(valueArray, $scope[name]);
          }
        },
        'save': function(nameValuePairs, name) {
          nameValuePairs[name] = Object.keys($scope[name]).join(',');
        }
      },

    };

    // parameter name -> copier objects to load/save parameter value
    $scope.queryParameters.map = {
      'resultsToLoad':         $scope.queryParameters.copiers.simple,
      'displayLimitPending':   $scope.queryParameters.copiers.simple,
      'showThumbnailsPending': $scope.queryParameters.copiers.simple,
      'imageSizePending':      $scope.queryParameters.copiers.simple,
      'sortColumnSubdict':     $scope.queryParameters.copiers.simple,
      'sortColumnKey':         $scope.queryParameters.copiers.simple,

      'hiddenResultTypes': $scope.queryParameters.copiers.set,
      'hiddenConfigs':     $scope.queryParameters.copiers.set,
    };
    $scope.queryParameters.map[constants.KEY__EXTRACOLUMN__BUILDER] =
        $scope.queryParameters.copiers.categoryValueMatch;
    $scope.queryParameters.map[constants.KEY__EXTRACOLUMN__TEST] =
        $scope.queryParameters.copiers.categoryValueMatch;

    // Loads all parameters into $scope from the URL query string;
    // any which are not found within the URL will keep their current value.
    $scope.queryParameters.load = function() {
      var nameValuePairs = $location.search();
      angular.forEach($scope.queryParameters.map,
                      function(copier, paramName) {
                        copier.load(nameValuePairs, paramName);
                      }
                     );
    };

    // Saves all parameters from $scope into the URL query string.
    $scope.queryParameters.save = function() {
      var nameValuePairs = {};
      angular.forEach($scope.queryParameters.map,
                      function(copier, paramName) {
                        copier.save(nameValuePairs, paramName);
                      }
                     );
      $location.search(nameValuePairs);
    };


    //
    // updateResults() and friends.
    //

    /**
     * Set $scope.areUpdatesPending (to enable/disable the Update Results
     * button).
     *
     * TODO(epoger): We could reduce the amount of code by just setting the
     * variable directly (from, e.g., a button's ng-click handler).  But when
     * I tried that, the HTML elements depending on the variable did not get
     * updated.
     * It turns out that this is due to variable scoping within an ng-repeat
     * element; see http://stackoverflow.com/questions/15388344/behavior-of-assignment-expression-invoked-by-ng-click-within-ng-repeat
     *
     * @param val boolean value to set $scope.areUpdatesPending to
     */
    $scope.setUpdatesPending = function(val) {
      $scope.areUpdatesPending = val;
    }

    /**
     * Update the displayed results, based on filters/settings,
     * and call $scope.queryParameters.save() so that the new filter results
     * can be bookmarked.
     */
    $scope.updateResults = function() {
      $scope.renderStartTime = window.performance.now();
      $log.debug("renderStartTime: " + $scope.renderStartTime);
      $scope.displayLimit = $scope.displayLimitPending;
      // TODO(epoger): Every time we apply a filter, AngularJS creates
      // another copy of the array.  Is there a way we can filter out
      // the imagePairs as they are displayed, rather than storing multiple
      // array copies?  (For better performance.)

      if ($scope.viewingTab == $scope.defaultTab) {

        // TODO(epoger): Until we allow the user to reverse sort order,
        // there are certain columns we want to sort in a different order.
        var doReverse = (
            ($scope.sortColumnKey ==
             constants.KEY__DIFFERENCE_DATA__PERCENT_DIFF_PIXELS) ||
            ($scope.sortColumnKey ==
             constants.KEY__DIFFERENCE_DATA__WEIGHTED_DIFF));

        $scope.filteredImagePairs =
            $filter("orderBy")(
                $filter("removeHiddenImagePairs")(
                    $scope.imagePairs,
                    $scope.hiddenResultTypes,
                    $scope.hiddenConfigs,
                    $scope.categoryValueMatch.builder,
                    $scope.categoryValueMatch.test,
                    $scope.viewingTab
                ),
                $scope.getSortColumnValue, doReverse);
        $scope.limitedImagePairs = $filter("limitTo")(
            $scope.filteredImagePairs, $scope.displayLimit);
      } else {
        $scope.filteredImagePairs =
            $filter("orderBy")(
                $filter("filter")(
                    $scope.imagePairs,
                    {tab: $scope.viewingTab},
                    true
                ),
                $scope.getSortColumnValue);
        $scope.limitedImagePairs = $scope.filteredImagePairs;
      }
      $scope.showThumbnails = $scope.showThumbnailsPending;
      $scope.imageSize = $scope.imageSizePending;
      $scope.setUpdatesPending(false);
      $scope.queryParameters.save();
    }

    /**
     * This function is called when the results have been completely rendered
     * after updateResults().
     */
    $scope.resultsUpdatedCallback = function() {
      $scope.renderEndTime = window.performance.now();
      $log.debug("renderEndTime: " + $scope.renderEndTime);
    }

    /**
     * Re-sort the displayed results.
     *
     * @param subdict (string): which subdictionary
     *     (constants.KEY__DIFFERENCE_DATA, constants.KEY__EXPECTATIONS_DATA,
     *      constants.KEY__EXTRA_COLUMN_VALUES) the sort column key is within
     * @param key (string): sort by value associated with this key in subdict
     */
    $scope.sortResultsBy = function(subdict, key) {
      $scope.sortColumnSubdict = subdict;
      $scope.sortColumnKey = key;
      $scope.updateResults();
    }

    /**
     * For a particular ImagePair, return the value of the column we are
     * sorting on (according to $scope.sortColumnSubdict and
     * $scope.sortColumnKey).
     *
     * @param imagePair: imagePair to get a column value out of.
     */
    $scope.getSortColumnValue = function(imagePair) {
      if ($scope.sortColumnSubdict in imagePair) {
        return imagePair[$scope.sortColumnSubdict][$scope.sortColumnKey];
      } else {
        return undefined;
      }
    }

    /**
     * Set $scope.categoryValueMatch[name] = value, and update results.
     *
     * @param name
     * @param value
     */
    $scope.setCategoryValueMatch = function(name, value) {
      $scope.categoryValueMatch[name] = value;
      $scope.updateResults();
    }

    /**
     * Update $scope.hiddenResultTypes so that ONLY this resultType is showing,
     * and update the visible results.
     *
     * @param resultType
     */
    $scope.showOnlyResultType = function(resultType) {
      $scope.hiddenResultTypes = {};
      // TODO(epoger): Maybe change $scope.allResultTypes to be a Set like
      // $scope.hiddenResultTypes (rather than an array), so this operation is
      // simpler (just assign or add allResultTypes to hiddenResultTypes).
      $scope.toggleValuesInSet($scope.allResultTypes, $scope.hiddenResultTypes);
      $scope.toggleValueInSet(resultType, $scope.hiddenResultTypes);
      $scope.updateResults();
    }

    /**
     * Update $scope.hiddenResultTypes so that ALL resultTypes are showing,
     * and update the visible results.
     */
    $scope.showAllResultTypes = function() {
      $scope.hiddenResultTypes = {};
      $scope.updateResults();
    }

    /**
     * Update $scope.hiddenConfigs so that ONLY this config is showing,
     * and update the visible results.
     *
     * @param config
     */
    $scope.showOnlyConfig = function(config) {
      $scope.hiddenConfigs = {};
      $scope.toggleValuesInSet($scope.allConfigs, $scope.hiddenConfigs);
      $scope.toggleValueInSet(config, $scope.hiddenConfigs);
      $scope.updateResults();
    }

    /**
     * Update $scope.hiddenConfigs so that ALL configs are showing,
     * and update the visible results.
     */
    $scope.showAllConfigs = function() {
      $scope.hiddenConfigs = {};
      $scope.updateResults();
    }


    //
    // Operations for sending info back to the server.
    //

    /**
     * Tell the server that the actual results of these particular tests
     * are acceptable.
     *
     * TODO(epoger): This assumes that the original expectations are in
     * imageSetA, and the actuals are in imageSetB.
     *
     * @param imagePairsSubset an array of test results, most likely a subset of
     *        $scope.imagePairs (perhaps with some modifications)
     */
    $scope.submitApprovals = function(imagePairsSubset) {
      $scope.submitPending = true;

      // Convert bug text field to null or 1-item array.
      var bugs = null;
      var bugNumber = parseInt($scope.submitAdvancedSettings['bug']);
      if (!isNaN(bugNumber)) {
        bugs = [bugNumber];
      }

      // TODO(epoger): This is a suboptimal way to prevent users from
      // rebaselining failures in alternative renderModes, but it does work.
      // For a better solution, see
      // https://code.google.com/p/skia/issues/detail?id=1748 ('gm: add new
      // result type, RenderModeMismatch')
      var encounteredComparisonConfig = false;

      var updatedExpectations = [];
      for (var i = 0; i < imagePairsSubset.length; i++) {
        var imagePair = imagePairsSubset[i];
        var updatedExpectation = {};
        updatedExpectation[constants.KEY__EXPECTATIONS_DATA] =
            imagePair[constants.KEY__EXPECTATIONS_DATA];
        updatedExpectation[constants.KEY__EXTRA_COLUMN_VALUES] =
            imagePair[constants.KEY__EXTRA_COLUMN_VALUES];
        updatedExpectation[constants.KEY__NEW_IMAGE_URL] =
            imagePair[constants.KEY__IMAGE_B_URL];
        if (0 == updatedExpectation[constants.KEY__EXTRA_COLUMN_VALUES]
                                   [constants.KEY__EXTRACOLUMN__CONFIG]
                                   .indexOf('comparison-')) {
          encounteredComparisonConfig = true;
        }

        // Advanced settings...
        if (null == updatedExpectation[constants.KEY__EXPECTATIONS_DATA]) {
          updatedExpectation[constants.KEY__EXPECTATIONS_DATA] = {};
        }
        updatedExpectation[constants.KEY__EXPECTATIONS_DATA]
                          [constants.KEY__EXPECTATIONS__REVIEWED] =
            $scope.submitAdvancedSettings[
                constants.KEY__EXPECTATIONS__REVIEWED];
        if (true == $scope.submitAdvancedSettings[
            constants.KEY__EXPECTATIONS__IGNOREFAILURE]) {
          // if it's false, don't send it at all (just keep the default)
          updatedExpectation[constants.KEY__EXPECTATIONS_DATA]
                            [constants.KEY__EXPECTATIONS__IGNOREFAILURE] = true;
        }
        updatedExpectation[constants.KEY__EXPECTATIONS_DATA]
                          [constants.KEY__EXPECTATIONS__BUGS] = bugs;

        updatedExpectations.push(updatedExpectation);
      }
      if (encounteredComparisonConfig) {
        alert("Approval failed -- you cannot approve results with config " +
            "type comparison-*");
        $scope.submitPending = false;
        return;
      }
      var modificationData = {};
      modificationData[constants.KEY__EDITS__MODIFICATIONS] =
          updatedExpectations;
      modificationData[constants.KEY__EDITS__OLD_RESULTS_HASH] =
          $scope.header[constants.KEY__HEADER__DATAHASH];
      modificationData[constants.KEY__EDITS__OLD_RESULTS_TYPE] =
          $scope.header[constants.KEY__HEADER__TYPE];
      $http({
        method: "POST",
        url: "/edits",
        data: modificationData
      }).success(function(data, status, headers, config) {
        var imagePairIndicesToMove = [];
        for (var i = 0; i < imagePairsSubset.length; i++) {
          imagePairIndicesToMove.push(imagePairsSubset[i].index);
        }
        $scope.moveImagePairsToTab(imagePairIndicesToMove,
                                   "HackToMakeSureThisImagePairDisappears");
        $scope.updateResults();
        alert("New baselines submitted successfully!\n\n" +
            "You still need to commit the updated expectations files on " +
            "the server side to the Skia repo.\n\n" +
            "When you click OK, your web UI will reload; after that " +
            "completes, you will see the updated data (once the server has " +
            "finished loading the update results into memory!) and you can " +
            "submit more baselines if you want.");
        // I don't know why, but if I just call reload() here it doesn't work.
        // Making a timer call it fixes the problem.
        $timeout(function(){location.reload();}, 1);
      }).error(function(data, status, headers, config) {
        alert("There was an error submitting your baselines.\n\n" +
            "Please see server-side log for details.");
        $scope.submitPending = false;
      });
    }


    //
    // Operations we use to mimic Set semantics, in such a way that
    // checking for presence within the Set is as fast as possible.
    // But getting a list of all values within the Set is not necessarily
    // possible.
    // TODO(epoger): move into a separate .js file?
    //

    /**
     * Returns the number of values present within set "set".
     *
     * @param set an Object which we use to mimic set semantics
     */
    $scope.setSize = function(set) {
      return Object.keys(set).length;
    }

    /**
     * Returns true if value "value" is present within set "set".
     *
     * @param value a value of any type
     * @param set an Object which we use to mimic set semantics
     *        (this should make isValueInSet faster than if we used an Array)
     */
    $scope.isValueInSet = function(value, set) {
      return (true == set[value]);
    }

    /**
     * If value "value" is already in set "set", remove it; otherwise, add it.
     *
     * @param value a value of any type
     * @param set an Object which we use to mimic set semantics
     */
    $scope.toggleValueInSet = function(value, set) {
      if (true == set[value]) {
        delete set[value];
      } else {
        set[value] = true;
      }
    }

    /**
     * For each value in valueArray, call toggleValueInSet(value, set).
     *
     * @param valueArray
     * @param set
     */
    $scope.toggleValuesInSet = function(valueArray, set) {
      var arrayLength = valueArray.length;
      for (var i = 0; i < arrayLength; i++) {
        $scope.toggleValueInSet(valueArray[i], set);
      }
    }


    //
    // Array operations; similar to our Set operations, but operate on a
    // Javascript Array so we *can* easily get a list of all values in the Set.
    // TODO(epoger): move into a separate .js file?
    //

    /**
     * Returns true if value "value" is present within array "array".
     *
     * @param value a value of any type
     * @param array a Javascript Array
     */
    $scope.isValueInArray = function(value, array) {
      return (-1 != array.indexOf(value));
    }

    /**
     * If value "value" is already in array "array", remove it; otherwise,
     * add it.
     *
     * @param value a value of any type
     * @param array a Javascript Array
     */
    $scope.toggleValueInArray = function(value, array) {
      var i = array.indexOf(value);
      if (-1 == i) {
        array.push(value);
      } else {
        array.splice(i, 1);
      }
    }


    //
    // Miscellaneous utility functions.
    // TODO(epoger): move into a separate .js file?
    //

    /**
     * Returns a human-readable (in local time zone) time string for a
     * particular moment in time.
     *
     * @param secondsPastEpoch (numeric): seconds past epoch in UTC
     */
    $scope.localTimeString = function(secondsPastEpoch) {
      var d = new Date(secondsPastEpoch * 1000);
      return d.toString();
    }

    /**
     * Returns a hex color string (such as "#aabbcc") for the given RGB values.
     *
     * @param r (numeric): red channel value, 0-255
     * @param g (numeric): green channel value, 0-255
     * @param b (numeric): blue channel value, 0-255
     */
    $scope.hexColorString = function(r, g, b) {
      var rString = r.toString(16);
      if (r < 16) {
        rString = "0" + rString;
      }
      var gString = g.toString(16);
      if (g < 16) {
        gString = "0" + gString;
      }
      var bString = b.toString(16);
      if (b < 16) {
        bString = "0" + bString;
      }
      return '#' + rString + gString + bString;
    }

    /**
     * Returns a hex color string (such as "#aabbcc") for the given brightness.
     *
     * @param brightnessString (string): 0-255, 0 is completely black
     *
     * TODO(epoger): It might be nice to tint the color when it's not completely
     * black or completely white.
     */
    $scope.brightnessStringToHexColor = function(brightnessString) {
      var v = parseInt(brightnessString);
      return $scope.hexColorString(v, v, v);
    }

    /**
     * Returns the last path component of image diff URL for a given ImagePair.
     *
     * Depending on which diff this is (whitediffs, pixeldiffs, etc.) this
     * will be relative to different base URLs.
     *
     * We must keep this function in sync with _get_difference_locator() in
     * ../imagediffdb.py
     *
     * @param imagePair: ImagePair to generate image diff URL for
     */
    $scope.getImageDiffRelativeUrl = function(imagePair) {
      var before =
          imagePair[constants.KEY__IMAGE_A_URL] + "-vs-" +
          imagePair[constants.KEY__IMAGE_B_URL];
      return before.replace(/[^\w\-]/g, "_") + ".png";
    }

  }
);
