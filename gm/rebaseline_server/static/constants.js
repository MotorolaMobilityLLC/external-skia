/*
 * Constants used by our imagediff-viewing Javascript code.
 */
var module = angular.module(
    'ConstantsModule',
    []
);

module.constant('constants', (function() {
  return {
    // NOTE: Keep these in sync with ../column.py
    KEY__EXTRACOLUMNHEADERS: 'extraColumnHeaders',
    KEY__EXTRACOLUMNHEADERS__HEADER_TEXT: 'headerText',
    KEY__EXTRACOLUMNHEADERS__HEADER_URL: 'headerUrl',
    KEY__EXTRACOLUMNHEADERS__IS_FILTERABLE: 'isFilterable',
    KEY__EXTRACOLUMNHEADERS__IS_SORTABLE: 'isSortable',
    KEY__EXTRACOLUMNHEADERS__VALUES_AND_COUNTS: 'valuesAndCounts',

    // NOTE: Keep these in sync with ../imagediffdb.py
    KEY__DIFFERENCE_DATA__MAX_DIFF_PER_CHANNEL: 'maxDiffPerChannel',
    KEY__DIFFERENCE_DATA__NUM_DIFF_PIXELS: 'numDifferingPixels',
    KEY__DIFFERENCE_DATA__PERCENT_DIFF_PIXELS: 'percentDifferingPixels',
    KEY__DIFFERENCE_DATA__PERCEPTUAL_DIFF: 'perceptualDifference',
    KEY__DIFFERENCE_DATA__WEIGHTED_DIFF: 'weightedDiffMeasure',

    // NOTE: Keep these in sync with ../imagepair.py
    KEY__DIFFERENCE_DATA: 'differenceData',
    KEY__EXPECTATIONS_DATA: 'expectations',
    KEY__EXTRA_COLUMN_VALUES: 'extraColumns',
    KEY__IMAGE_A_URL: 'imageAUrl',
    KEY__IMAGE_B_URL: 'imageBUrl',
    KEY__IS_DIFFERENT: 'isDifferent',

    // NOTE: Keep these in sync with ../imagepairset.py
    KEY__IMAGEPAIRS: 'imagePairs',
    KEY__IMAGESETS: 'imageSets',
    KEY__IMAGESETS__FIELD__BASE_URL: 'baseUrl',
    KEY__IMAGESETS__FIELD__DESCRIPTION: 'description',
    KEY__IMAGESETS__SET__DIFFS: 'diffs',
    KEY__IMAGESETS__SET__IMAGE_A: 'imageA',
    KEY__IMAGESETS__SET__IMAGE_B: 'imageB',
    KEY__IMAGESETS__SET__WHITEDIFFS: 'whiteDiffs',

    // NOTE: Keep these in sync with ../results.py
    REBASELINE_SERVER_SCHEMA_VERSION_NUMBER: 3,
    KEY__EXPECTATIONS__BUGS: 'bugs',
    KEY__EXPECTATIONS__IGNOREFAILURE: 'ignore-failure',
    KEY__EXPECTATIONS__REVIEWED: 'reviewed-by-human',
    KEY__EXTRACOLUMN__BUILDER: 'builder',
    KEY__EXTRACOLUMN__CONFIG: 'config',
    KEY__EXTRACOLUMN__RESULT_TYPE: 'resultType',
    KEY__EXTRACOLUMN__TEST: 'test',
    KEY__HEADER: 'header',
    KEY__HEADER__DATAHASH: 'dataHash',
    KEY__HEADER__IS_EDITABLE: 'isEditable',
    KEY__HEADER__IS_EXPORTED: 'isExported',
    KEY__HEADER__IS_STILL_LOADING: 'resultsStillLoading',
    KEY__HEADER__RESULTS_ALL: 'all',
    KEY__HEADER__RESULTS_FAILURES: 'failures',
    KEY__HEADER__SCHEMA_VERSION: 'schemaVersion',
    KEY__HEADER__TIME_NEXT_UPDATE_AVAILABLE: 'timeNextUpdateAvailable',
    KEY__HEADER__TIME_UPDATED: 'timeUpdated',
    KEY__HEADER__TYPE: 'type',
    KEY__NEW_IMAGE_URL: 'newImageUrl',
    KEY__RESULT_TYPE__FAILED: 'failed',
    KEY__RESULT_TYPE__FAILUREIGNORED: 'failure-ignored',
    KEY__RESULT_TYPE__NOCOMPARISON: 'no-comparison',
    KEY__RESULT_TYPE__SUCCEEDED: 'succeeded',

    // NOTE: Keep these in sync with ../server.py
    KEY__EDITS__MODIFICATIONS: 'modifications',
    KEY__EDITS__OLD_RESULTS_HASH: 'oldResultsHash',
    KEY__EDITS__OLD_RESULTS_TYPE: 'oldResultsType',
  }
})())
