/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkUnicode_DEFINED
#define SkUnicode_DEFINED

#include "include/core/SkTypes.h"
#include "src/core/SkSpan.h"
#include "src/utils/SkUTF.h"
#include <vector>

#if !defined(SKUNICODE_IMPLEMENTATION)
    #define SKUNICODE_IMPLEMENTATION 0
#endif

#if !defined(SKUNICODE_API)
    #if defined(SKSHAPER_DLL)
        #if defined(_MSC_VER)
            #if SKUNICODE_IMPLEMENTATION
                #define SKUNICODE_API __declspec(dllexport)
            #else
                #define SKUNICODE_API __declspec(dllimport)
            #endif
        #else
            #define SKUNICODE_API __attribute__((visibility("default")))
        #endif
    #else
        #define SKUNICODE_API
    #endif
#endif

class SKUNICODE_API SkBidiIterator {
public:
    typedef int32_t Position;
    typedef uint8_t Level;
    struct Region {
        Region(Position start, Position end, Level level)
            : start(start), end(end), level(level) { }
        Position start;
        Position end;
        Level level;
    };
    enum Direction {
        kLTR,
        kRTL,
    };
    virtual ~SkBidiIterator() = default;
    virtual Position getLength() = 0;
    virtual Level getLevelAt(Position) = 0;
    static void ReorderVisual(const Level runLevels[], int levelsCount, int32_t logicalFromVisual[]);
};

class SKUNICODE_API SkBreakIterator {
public:
    typedef int32_t Position;
    typedef int32_t Status;
    virtual ~SkBreakIterator() = default;
    virtual Position first() = 0;
    virtual Position current() = 0;
    virtual Position next() = 0;
    virtual Position preceding(Position offset) = 0;
    virtual Position following(Position offset) = 0;
    virtual Status status() = 0;
    virtual bool isDone() = 0;
    virtual bool setText(const char utftext8[], int utf8Units) = 0;
};

class SKUNICODE_API SkScriptIterator {
 public:
    typedef uint32_t ScriptID;
    virtual ~SkScriptIterator() = default;
    virtual bool getScript(SkUnichar u, ScriptID* script) = 0;
};

class SKUNICODE_API SkUnicode {
    public:
        typedef uint32_t CombiningClass;
        typedef uint32_t GeneralCategory;
        enum class TextDirection {
            kLTR,
            kRTL,
        };
        typedef size_t Position;
        typedef uint8_t BidiLevel;
        struct BidiRegion {
            BidiRegion(Position start, Position end, BidiLevel level)
              : start(start), end(end), level(level) { }
            Position start;
            Position end;
            BidiLevel level;
        };
        enum class LineBreakType {
            kSoftLineBreak,
            kHardLineBreak
        };

        enum class BreakType {
            kWords,
            kGraphemes,
            kLines
        };
        struct LineBreakBefore {
            LineBreakBefore(Position pos, LineBreakType breakType)
              : pos(pos), breakType(breakType) { }
            Position pos;
            LineBreakType breakType;
        };

        virtual ~SkUnicode() = default;

        virtual bool isControl(SkUnichar utf8) = 0;
        virtual bool isWhitespace(SkUnichar utf8) = 0;
        virtual bool isSpace(SkUnichar utf8) = 0;
        virtual SkString convertUtf16ToUtf8(const std::u16string& utf16) = 0;

        // Methods used in SkShaper
        virtual std::unique_ptr<SkBidiIterator> makeBidiIterator
            (const uint16_t text[], int count, SkBidiIterator::Direction) = 0;
        virtual std::unique_ptr<SkBidiIterator> makeBidiIterator
            (const char text[], int count, SkBidiIterator::Direction) = 0;
        virtual std::unique_ptr<SkBreakIterator> makeBreakIterator
            (const char locale[], BreakType breakType) = 0;
        virtual std::unique_ptr<SkScriptIterator> makeScriptIterator() = 0;

        // High level methods (that we actually use somewhere=SkParagraph)
        virtual bool getBidiRegions
               (const char utf8[], int utf8Units, TextDirection dir, std::vector<BidiRegion>* results) = 0;
        virtual bool getLineBreaks
               (const char utf8[], int utf8Units, std::vector<LineBreakBefore>* results) = 0;
        virtual bool getWords
               (const char utf8[], int utf8Units, std::vector<Position>* results) = 0;
        virtual bool getGraphemes
               (const char utf8[], int utf8Units, std::vector<Position>* results) = 0;

        template <typename Callback>
        void forEachCodepoint(const char* utf8, int32_t utf8Units, Callback&& callback) {
            const char* current = utf8;
            const char* end = utf8 + utf8Units;
            while (current < end) {
                auto before = current - utf8;
                SkUnichar unichar = SkUTF::NextUTF8(&current, end);
                if (unichar < 0) unichar = 0xFFFD;
                auto after = current - utf8;
                callback(unichar, before, after);
            }
        }

        virtual void reorderVisual(const BidiLevel runLevels[], int levelsCount, int32_t logicalFromVisual[]) = 0;

        static std::unique_ptr<SkUnicode> Make();
};

#endif // SkUnicode_DEFINED
