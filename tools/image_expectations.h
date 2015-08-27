/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef image_expectations_DEFINED
#define image_expectations_DEFINED

#include "SkBitmap.h"
#include "SkJSONCPP.h"
#include "SkOSFile.h"

namespace sk_tools {

    /**
     * The digest of an image (either an image we have generated locally, or an image expectation).
     *
     * Currently, this is always a uint64_t hash digest of an SkBitmap.
     */
    class ImageDigest {
    public:
        /**
         * Create an ImageDigest of a bitmap.
         *
         * Computes the hash of the bitmap lazily, since that is an expensive operation.
         *
         * @param bitmap image to get the digest of
         */
        explicit ImageDigest(const SkBitmap &bitmap);

        /**
         * Create an ImageDigest using a hashType/hashValue pair.
         *
         * @param hashType the algorithm used to generate the hash; for now, only
         *     kJsonValue_Image_ChecksumAlgorithm_Bitmap64bitMD5 is allowed.
         * @param hashValue the value generated by the hash algorithm for a particular image.
         */
        explicit ImageDigest(const SkString &hashType, uint64_t hashValue);

        /**
         * Returns true iff this and other ImageDigest represent identical images.
         */
        bool equals(ImageDigest &other);

        /**
         * Returns the hash digest type as an SkString.
         *
         * For now, this always returns kJsonValue_Image_ChecksumAlgorithm_Bitmap64bitMD5 .
         */
        SkString getHashType();

        /**
         * Returns the hash digest value as a uint64_t.
         *
         * Since the hash is computed lazily, this may take some time, and it may modify
         * some fields on this object.
         */
        uint64_t getHashValue();

    private:
        const SkBitmap fBitmap;
        uint64_t fHashValue;
        bool fComputedHashValue;
    };

    /**
     * Container that holds a reference to an SkBitmap and its ImageDigest.
     */
    class BitmapAndDigest {
    public:
        explicit BitmapAndDigest(const SkBitmap &bitmap);

        const SkBitmap *getBitmapPtr() const;

        /**
         * Returns a pointer to the ImageDigest.
         *
         * Since the hash is computed lazily within the ImageDigest object, we cannot mandate
         * that it be held const.
         */
        ImageDigest *getImageDigestPtr();
    private:
        const SkBitmap fBitmap;
        ImageDigest fImageDigest;
    };

    /**
     * Expected test result: expected image (if any), and whether we want to ignore failures on
     * this test or not.
     *
     * This is just an ImageDigest (or lack thereof, if there is no expectation) and a boolean
     * telling us whether to ignore failures.
     */
    class Expectation {
    public:
        /**
         * No expectation at all.
         */
        explicit Expectation(bool ignoreFailure=kDefaultIgnoreFailure);

        /**
         * Expect an image, passed as hashType/hashValue.
         */
        explicit Expectation(const SkString &hashType, uint64_t hashValue,
                             bool ignoreFailure=kDefaultIgnoreFailure);

        /**
         * Expect an image, passed as a bitmap.
         */
        explicit Expectation(const SkBitmap& bitmap,
                             bool ignoreFailure=kDefaultIgnoreFailure);

        /**
         * Returns true iff we want to ignore failed expectations.
         */
        bool ignoreFailure() const;

        /**
         * Returns true iff there are no allowed results.
         */
        bool empty() const;

        /**
         * Returns true iff we are expecting a particular image, and imageDigest matches it.
         *
         * If empty() returns true, this will return false.
         *
         * If this expectation DOES contain an image, and imageDigest doesn't match it,
         * this method will return false regardless of what ignoreFailure() would return.
         * (The caller can check that separately.)
         */
        bool matches(ImageDigest &imageDigest);

    private:
        static const bool kDefaultIgnoreFailure = false;

        const bool fIsEmpty;
        const bool fIgnoreFailure;
        ImageDigest fImageDigest;  // cannot be const, because it computes its hash lazily
    };

    /**
     * Collects ImageDigests of actually rendered images, perhaps comparing to expectations.
     */
    class ImageResultsAndExpectations {
    public:
        /**
         * Adds expectations from a JSON file, returning true if successful.
         *
         * If the file exists but is empty, it succeeds, and there will be no expectations.
         * If the file does not exist, this will fail.
         *
         * Reasoning:
         * Generating expectations the first time can be a tricky chicken-and-egg
         * proposition.  "I need actual results to turn into expectations... but the only
         * way to get actual results is to run the tool, and the tool won't run without
         * expectations!"
         * We could make the tool run even if there is no expectations file at all, but it's
         * better for the tool to fail if the expectations file is not found--that will tell us
         * quickly if files are not being copied around as they should be.
         * Creating an empty file is an easy way to break the chicken-and-egg cycle and generate
         * the first real expectations.
         */
        bool readExpectationsFile(const char *jsonPath);

        /**
         * Adds this image to the summary of results.
         *
         * @param sourceName name of the source file that generated this result
         * @param fileName relative path to the image output file on local disk
         * @param digest description of the image's contents
         * @param tileNumber if not nullptr, pointer to tile number
         */
        void add(const char *sourceName, const char *fileName, ImageDigest &digest,
                 const int *tileNumber=nullptr);

        /**
         * Adds a key/value pair to the descriptions dict within the summary of results.
         *
         * @param key key within the descriptions dict
         * @param value value to associate with that key
         */
        void addDescription(const char *key, const char *value);

        /**
         * Adds the image base Google Storage URL to the summary of results.
         *
         * @param imageBaseGSUrl the image base Google Storage URL
         */
        void setImageBaseGSUrl(const char *imageBaseGSUrl);

        /**
         * Returns the Expectation for this test.
         *
         * @param sourceName name of the source file that generated this result
         * @param tileNumber if not nullptr, pointer to tile number
         *
         * TODO(stephana): To make this work for GMs, we will need to add parameters for
         * config, and maybe renderMode/builder?
         */
        Expectation getExpectation(const char *sourceName, const int *tileNumber=nullptr);

        /**
         * Writes the summary (as constructed so far) to a file.
         *
         * @param filename path to write the summary to
         */
        void writeToFile(const char *filename) const;

    private:

        /**
         * Read the file contents from filePtr and parse them into jsonRoot.
         *
         * It is up to the caller to close filePtr after this is done.
         *
         * Returns true if successful.
         */
        static bool Parse(SkFILE* filePtr, Json::Value *jsonRoot);

        Json::Value fActualResults;
        Json::Value fDescriptions;
        Json::Value fExpectedJsonRoot;
        Json::Value fExpectedResults;
        Json::Value fImageBaseGSUrl;
    };

} // namespace sk_tools

#endif  // image_expectations_DEFINED
