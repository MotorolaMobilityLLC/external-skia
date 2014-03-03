#include "DMWriteTask.h"

#include "DMUtil.h"
#include "SkColorPriv.h"
#include "SkCommandLineFlags.h"
#include "SkImageEncoder.h"
#include "SkString.h"
#include "SkStream.h"

DEFINE_string2(writePath, w, "", "If set, write GMs here as .pngs.");

namespace DM {

// Splits off the last N suffixes of name (splitting on _) and appends them to out.
// Returns the total number of characters consumed.
static int split_suffixes(int N, const char* name, SkTArray<SkString>* out) {
    SkTArray<SkString> split;
    SkStrSplit(name, "_", &split);
    int consumed = 0;
    for (int i = 0; i < N; i++) {
        // We're splitting off suffixes from the back to front.
        out->push_back(split[split.count()-i-1]);
        consumed += out->back().size() + 1;  // Add one for the _.
    }
    return consumed;
}

WriteTask::WriteTask(const Task& parent, SkBitmap bitmap) : CpuTask(parent), fBitmap(bitmap) {
    const int suffixes = parent.depth() + 1;
    const SkString& name = parent.name();
    const int totalSuffixLength = split_suffixes(suffixes, name.c_str(), &fSuffixes);
    fGmName.set(name.c_str(), name.size()-totalSuffixLength);
}

void WriteTask::makeDirOrFail(SkString dir) {
    if (!sk_mkdir(dir.c_str())) {
        this->fail();
    }
}

// One file that first contains a .png of an SkBitmap, then its raw pixels.
// We use this custom format to avoid premultiplied/unpremultiplied pixel conversions.
struct PngAndRaw {
    static bool Encode(SkBitmap bitmap, const char* path) {
        SkFILEWStream stream(path);
        if (!stream.isValid()) {
            SkDebugf("Can't write %s.\n", path);
            return false;
        }

        // Write a PNG first for humans and other tools to look at.
        if (!SkImageEncoder::EncodeStream(&stream, bitmap, SkImageEncoder::kPNG_Type, 100)) {
            SkDebugf("Can't encode a PNG.\n");
            return false;
        }

        // Then write our secret raw pixels that only DM reads.
        SkAutoLockPixels lock(bitmap);
        return stream.write(bitmap.getPixels(), bitmap.getSize());
    }

    // This assumes bitmap already has allocated pixels of the correct size.
    static bool Decode(const char* path, SkBitmap* bitmap) {
        SkAutoTUnref<SkStreamRewindable> stream(SkStream::NewFromFile(path));
        if (NULL == stream.get()) {
            SkDebugf("Can't read %s.\n", path);
            return false;
        }

        // The raw pixels are at the end of the stream.  Seek ahead to skip beyond the encoded PNG.
        const size_t bitmapBytes = bitmap->getSize();
        if (stream->getLength() < bitmapBytes) {
            SkDebugf("%s is too small to contain the bitmap we're looking for.\n", path);
            return false;
        }
        SkAssertResult(stream->seek(stream->getLength() - bitmapBytes));

        // Read the raw pixels.
        // TODO(mtklein): can we install a pixelref that just hangs onto the stream instead of
        // copying into a malloc'd buffer?
        SkAutoLockPixels lock(*bitmap);
        if (bitmapBytes != stream->read(bitmap->getPixels(), bitmapBytes)) {
            SkDebugf("Couldn't read raw bitmap from %s at %lu of %lu.\n",
                     path, stream->getPosition(), stream->getLength());
            return false;
        }

        SkASSERT(stream->isAtEnd());
        return true;
    }
};

void WriteTask::draw() {
    SkString dir(FLAGS_writePath[0]);
    this->makeDirOrFail(dir);
    for (int i = 0; i < fSuffixes.count(); i++) {
        dir = SkOSPath::SkPathJoin(dir.c_str(), fSuffixes[i].c_str());
        this->makeDirOrFail(dir);
    }
    SkString path = SkOSPath::SkPathJoin(dir.c_str(), fGmName.c_str());
    path.append(".png");
    if (!PngAndRaw::Encode(fBitmap, path.c_str())) {
        this->fail();
    }
}

SkString WriteTask::name() const {
    SkString name("writing ");
    for (int i = 0; i < fSuffixes.count(); i++) {
        name.appendf("%s/", fSuffixes[i].c_str());
    }
    name.append(fGmName.c_str());
    return name;
}

bool WriteTask::shouldSkip() const {
    return FLAGS_writePath.isEmpty();
}

static SkString path_to_expected_image(const char* root, const Task& task) {
    SkString filename = task.name();

    // We know that all names passed in here belong to top-level Tasks, which have a single suffix
    // (8888, 565, gpu, etc.) indicating what subdirectory to look in.
    SkTArray<SkString> suffixes;
    const int suffixLength = split_suffixes(1, filename.c_str(), &suffixes);
    SkASSERT(1 == suffixes.count());

    // We'll look in root/suffix for images.
    const SkString dir = SkOSPath::SkPathJoin(root, suffixes[0].c_str());

    // Remove the suffix and tack on a .png.
    filename.remove(filename.size() - suffixLength, suffixLength);
    filename.append(".png");

    return SkOSPath::SkPathJoin(dir.c_str(), filename.c_str());
}

bool WriteTask::Expectations::check(const Task& task, SkBitmap bitmap) const {
    if (!FLAGS_writePath.isEmpty() && 0 == strcmp(FLAGS_writePath[0], fRoot)) {
        SkDebugf("We seem to be reading and writing %s concurrently.  This won't work.\n", fRoot);
        return false;
    }

    const SkString path = path_to_expected_image(fRoot, task);
    SkBitmap expected;
    expected.allocPixels(bitmap.info());
    if (!PngAndRaw::Decode(path.c_str(), &expected)) {
        return false;
    }

    return BitmapsEqual(expected, bitmap);
}

}  // namespace DM
