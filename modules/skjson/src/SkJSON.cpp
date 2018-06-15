/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkJSON.h"

#include "SkStream.h"
#include "SkString.h"

#include <cmath>
#include <tuple>
#include <vector>

namespace skjson {

// #define SK_JSON_REPORT_ERRORS

static_assert( sizeof(Value) == 8, "");
static_assert(alignof(Value) == 8, "");

static constexpr size_t kRecAlign = alignof(Value);

void Value::init_tagged(Tag t) {
    memset(fData8, 0, sizeof(fData8));
    fData8[Value::kTagOffset] = SkTo<uint8_t>(t);
    SkASSERT(this->getTag() == t);
}

// Pointer values store a type (in the upper kTagBits bits) and a pointer.
void Value::init_tagged_pointer(Tag t, void* p) {
    *this->cast<uintptr_t>() = reinterpret_cast<uintptr_t>(p);

    if (sizeof(Value) == sizeof(uintptr_t)) {
        // For 64-bit, we rely on the pointer upper bits being unused/zero.
        SkASSERT(!(fData8[kTagOffset] & kTagMask));
        fData8[kTagOffset] |= SkTo<uint8_t>(t);
    } else {
        // For 32-bit, we need to zero-initialize the upper 32 bits
        SkASSERT(sizeof(Value) == sizeof(uintptr_t) * 2);
        this->cast<uintptr_t>()[kTagOffset >> 2] = 0;
        fData8[kTagOffset] = SkTo<uint8_t>(t);
    }

    SkASSERT(this->getTag()    == t);
    SkASSERT(this->ptr<void>() == p);
}

NullValue::NullValue() {
    this->init_tagged(Tag::kNull);
    SkASSERT(this->getTag() == Tag::kNull);
}

BoolValue::BoolValue(bool b) {
    this->init_tagged(Tag::kBool);
    *this->cast<bool>() = b;
    SkASSERT(this->getTag() == Tag::kBool);
}

NumberValue::NumberValue(int32_t i) {
    this->init_tagged(Tag::kInt);
    *this->cast<int32_t>() = i;
    SkASSERT(this->getTag() == Tag::kInt);
}

NumberValue::NumberValue(float f) {
    this->init_tagged(Tag::kFloat);
    *this->cast<float>() = f;
    SkASSERT(this->getTag() == Tag::kFloat);
}

// Vector recs point to externally allocated slabs with the following layout:
//
//   [size_t n] [REC_0] ... [REC_n-1] [optional extra trailing storage]
//
// Long strings use extra_alloc_size == 1 to store the \0 terminator.
//
template <typename T, size_t extra_alloc_size = 0>
static void* MakeVector(const void* src, size_t size, SkArenaAlloc& alloc) {
    // The Ts are already in memory, so their size should be safe.
    const auto total_size = sizeof(size_t) + size * sizeof(T) + extra_alloc_size;
    auto* size_ptr = reinterpret_cast<size_t*>(alloc.makeBytesAlignedTo(total_size, kRecAlign));
    *size_ptr = size;

    if (size) {
        auto* data_ptr = reinterpret_cast<void*>(size_ptr + 1);
        memcpy(data_ptr, src, size * sizeof(T));
    }

    return size_ptr;
}

ArrayValue::ArrayValue(const Value* src, size_t size, SkArenaAlloc& alloc) {
    this->init_tagged_pointer(Tag::kArray, MakeVector<Value>(src, size, alloc));
    SkASSERT(this->getTag() == Tag::kArray);
}

// Strings have two flavors:
//
// -- short strings (len <= 7) -> these are stored inline, in the record
//    (one byte reserved for null terminator/type):
//
//        [str] [\0]|[max_len - actual_len]
//
//    Storing [max_len - actual_len] allows the 'len' field to double-up as a
//    null terminator when size == max_len (this works 'cause kShortString == 0).
//
// -- long strings (len > 7) -> these are externally allocated vectors (VectorRec<char>).
//
// The string data plus a null-char terminator are copied over.
//
StringValue::StringValue(const char* src, size_t size, SkArenaAlloc& alloc) {
    if (size > kMaxInlineStringSize) {
        this->init_tagged_pointer(Tag::kString, MakeVector<char, 1>(src, size, alloc));

        auto* data = this->cast<VectorValue<char, Value::Type::kString>>()->begin();
        const_cast<char*>(data)[size] = '\0';
        SkASSERT(this->getTag() == Tag::kString);
        return;
    }

    this->init_tagged(Tag::kShortString);

    auto* payload = this->cast<char>();
    if (size) {
        memcpy(payload, src, size);
        payload[size] = '\0';
    }

    const auto len_tag = SkTo<char>(kMaxInlineStringSize - size);
    // This technically overwrites the tag, but is safe because
    //   1) kShortString == 0
    //   2) 0 <= len_tag <= 7
    static_assert(static_cast<uint8_t>(Tag::kShortString) == 0, "please don't break this");
    payload[kMaxInlineStringSize] = len_tag;

    SkASSERT(this->getTag() == Tag::kShortString);
}

ObjectValue::ObjectValue(const Member* src, size_t size, SkArenaAlloc& alloc) {
    this->init_tagged_pointer(Tag::kObject, MakeVector<Member>(src, size, alloc));
    SkASSERT(this->getTag() == Tag::kObject);
}


// Boring public Value glue.

const Value& ObjectValue::operator[](const char* key) const {
    // Reverse search for duplicates resolution (policy: return last).
    const auto* begin  = this->begin();
    const auto* member = this->end();

    while (member > begin) {
        --member;
        if (0 == strcmp(key, member->fKey.as<StringValue>().begin())) {
            return member->fValue;
        }
    }

    static const Value g_null = NullValue();
    return g_null;
}

namespace {

// Lexer/parser inspired by rapidjson [1], sajson [2] and pjson [3].
//
// [1] https://github.com/Tencent/rapidjson/
// [2] https://github.com/chadaustin/sajson
// [3] https://pastebin.com/hnhSTL3h


// bit 0 (0x01) - plain ASCII string character
// bit 1 (0x02) - whitespace
// bit 2 (0x04) - string terminator (" \0 [control chars])
// bit 3 (0x08) - 0-9
// bit 4 (0x10) - 0-9 e E .
static constexpr uint8_t g_token_flags[256] = {
 // 0    1    2    3    4    5    6    7      8    9    A    B    C    D    E    F
    4,   4,   4,   4,   4,   4,   4,   4,     4,   6,   6,   4,   4,   6,   4,   4, // 0
    4,   4,   4,   4,   4,   4,   4,   4,     4,   4,   4,   4,   4,   4,   4,   4, // 1
    3,   1,   4,   1,   1,   1,   1,   1,     1,   1,   1,   1,   1,   1,   0x11,1, // 2
 0x19,0x19,0x19,0x19,0x19,0x19,0x19,0x19,  0x19,0x19,   1,   1,   1,   1,   1,   1, // 3
    1,   1,   1,   1,   1,   0x11,1,   1,     1,   1,   1,   1,   1,   1,   1,   1, // 4
    1,   1,   1,   1,   1,   1,   1,   1,     1,   1,   1,   1,   0,   1,   1,   1, // 5
    1,   1,   1,   1,   1,   0x11,1,   1,     1,   1,   1,   1,   1,   1,   1,   1, // 6
    1,   1,   1,   1,   1,   1,   1,   1,     1,   1,   1,   1,   1,   1,   1,   1, // 7

 // 128-255
    0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0
};

static inline bool is_ws(char c)          { return g_token_flags[static_cast<uint8_t>(c)] & 0x02; }
static inline bool is_sterminator(char c) { return g_token_flags[static_cast<uint8_t>(c)] & 0x04; }
static inline bool is_digit(char c)       { return g_token_flags[static_cast<uint8_t>(c)] & 0x08; }
static inline bool is_numeric(char c)     { return g_token_flags[static_cast<uint8_t>(c)] & 0x10; }

static inline const char* skip_ws(const char* p) {
    while (is_ws(*p)) ++p;
    return p;
}

static inline float pow10(int32_t exp) {
    static constexpr float g_pow10_table[63] =
    {
       1.e-031f, 1.e-030f, 1.e-029f, 1.e-028f, 1.e-027f, 1.e-026f, 1.e-025f, 1.e-024f,
       1.e-023f, 1.e-022f, 1.e-021f, 1.e-020f, 1.e-019f, 1.e-018f, 1.e-017f, 1.e-016f,
       1.e-015f, 1.e-014f, 1.e-013f, 1.e-012f, 1.e-011f, 1.e-010f, 1.e-009f, 1.e-008f,
       1.e-007f, 1.e-006f, 1.e-005f, 1.e-004f, 1.e-003f, 1.e-002f, 1.e-001f, 1.e+000f,
       1.e+001f, 1.e+002f, 1.e+003f, 1.e+004f, 1.e+005f, 1.e+006f, 1.e+007f, 1.e+008f,
       1.e+009f, 1.e+010f, 1.e+011f, 1.e+012f, 1.e+013f, 1.e+014f, 1.e+015f, 1.e+016f,
       1.e+017f, 1.e+018f, 1.e+019f, 1.e+020f, 1.e+021f, 1.e+022f, 1.e+023f, 1.e+024f,
       1.e+025f, 1.e+026f, 1.e+027f, 1.e+028f, 1.e+029f, 1.e+030f, 1.e+031f
    };

    static constexpr int32_t k_exp_offset = SK_ARRAY_COUNT(g_pow10_table) / 2;

    // We only support negative exponents for now.
    SkASSERT(exp <= 0);

    return (exp >= -k_exp_offset) ? g_pow10_table[exp + k_exp_offset]
                                  : std::pow(10.0f, static_cast<float>(exp));
}

class DOMParser {
public:
    explicit DOMParser(SkArenaAlloc& alloc)
        : fAlloc(alloc) {

        fValueStack.reserve(kValueStackReserve);
        fScopeStack.reserve(kScopeStackReserve);
    }

    const Value parse(const char* p, size_t size) {
        if (!size) {
            return this->error(NullValue(), p, "invalid empty input");
        }

        const char* p_stop = p + size - 1;

        // We're only checking for end-of-stream on object/array close('}',']'),
        // so we must trim any whitespace from the buffer tail.
        while (p_stop > p && is_ws(*p_stop)) --p_stop;

        SkASSERT(p_stop >= p && p_stop < p + size);
        if (*p_stop != '}' && *p_stop != ']') {
            return this->error(NullValue(), p_stop, "invalid top-level value");
        }

        p = skip_ws(p);

        switch (*p) {
        case '{':
            goto match_object;
        case '[':
            goto match_array;
        default:
            return this->error(NullValue(), p, "invalid top-level value");
        }

    match_object:
        SkASSERT(*p == '{');
        p = skip_ws(p + 1);

        this->pushObjectScope();

        if (*p == '}') goto pop_object;

        // goto match_object_key;
    match_object_key:
        p = skip_ws(p);
        if (*p != '"') return this->error(NullValue(), p, "expected object key");

        p = this->matchString(p, [this](const char* key, size_t size) {
            this->pushObjectKey(key, size);
        });
        if (!p) return NullValue();

        p = skip_ws(p);
        if (*p != ':') return this->error(NullValue(), p, "expected ':' separator");

        ++p;

        // goto match_value;
    match_value:
        p = skip_ws(p);

        switch (*p) {
        case '\0':
            return this->error(NullValue(), p, "unexpected input end");
        case '"':
            p = this->matchString(p, [this](const char* str, size_t size) {
                this->pushString(str, size);
            });
            break;
        case '[':
            goto match_array;
        case 'f':
            p = this->matchFalse(p);
            break;
        case 'n':
            p = this->matchNull(p);
            break;
        case 't':
            p = this->matchTrue(p);
            break;
        case '{':
            goto match_object;
        default:
            p = this->matchNumber(p);
            break;
        }

        if (!p) return NullValue();

        // goto match_post_value;
    match_post_value:
        SkASSERT(!fScopeStack.empty());

        p = skip_ws(p);
        switch (*p) {
        case ',':
            ++p;
            if (fScopeStack.back() >= 0) {
                goto match_object_key;
            } else {
                goto match_value;
            }
        case ']':
            goto pop_array;
        case '}':
            goto pop_object;
        default:
            return this->error(NullValue(), p - 1, "unexpected value-trailing token");
        }

        // unreachable
        SkASSERT(false);

    pop_object:
        SkASSERT(*p == '}');

        if (fScopeStack.back() < 0) {
            return this->error(NullValue(), p, "unexpected object terminator");
        }

        this->popObjectScope();

        // goto pop_common
    pop_common:
        SkASSERT(*p == '}' || *p == ']');

        if (fScopeStack.empty()) {
            SkASSERT(fValueStack.size() == 1);

            // Success condition: parsed the top level element and reached the stop token.
            return p == p_stop
                ? fValueStack.front()
                : this->error(NullValue(), p + 1, "trailing root garbage");
        }

        ++p;

        goto match_post_value;

    match_array:
        SkASSERT(*p == '[');
        p = skip_ws(p + 1);

        this->pushArrayScope();

        if (*p != ']') goto match_value;

        // goto pop_array;
    pop_array:
        SkASSERT(*p == ']');

        if (fScopeStack.back() >= 0) {
            return this->error(NullValue(), p, "unexpected array terminator");
        }

        this->popArrayScope();

        goto pop_common;

        SkASSERT(false);
        return NullValue();
    }

    std::tuple<const char*, const SkString> getError() const {
        return std::make_tuple(fErrorToken, fErrorMessage);
    }

private:
    SkArenaAlloc&         fAlloc;

    static constexpr size_t kValueStackReserve = 256;
    static constexpr size_t kScopeStackReserve = 128;
    std::vector<Value   > fValueStack;
    std::vector<intptr_t> fScopeStack;

    const char*           fErrorToken = nullptr;
    SkString              fErrorMessage;

    template <typename VectorT>
    void popScopeAsVec(size_t scope_start) {
        SkASSERT(scope_start > 0);
        SkASSERT(scope_start <= fValueStack.size());

        using T = typename VectorT::ValueT;
        static_assert( sizeof(T) >=  sizeof(Value), "");
        static_assert( sizeof(T)  %  sizeof(Value) == 0, "");
        static_assert(alignof(T) == alignof(Value), "");

        const auto scope_count = fValueStack.size() - scope_start,
                         count = scope_count / (sizeof(T) / sizeof(Value));
        SkASSERT(scope_count % (sizeof(T) / sizeof(Value)) == 0);

        const auto* begin = reinterpret_cast<const T*>(fValueStack.data() + scope_start);

        // Instantiate the placeholder value added in onPush{Object/Array}.
        fValueStack[scope_start - 1] = VectorT(begin, count, fAlloc);

        // Drop the current scope.
        fScopeStack.pop_back();
        fValueStack.resize(scope_start);
    }

    void pushObjectScope() {
        // Object placeholder.
        fValueStack.emplace_back();

        // Object scope marker (size).
        fScopeStack.push_back(SkTo<intptr_t>(fValueStack.size()));
    }

    void popObjectScope() {
        const auto scope_start = fScopeStack.back();
        SkASSERT(scope_start > 0);
        this->popScopeAsVec<ObjectValue>(SkTo<size_t>(scope_start));

        SkDEBUGCODE(
            const auto& obj = fValueStack.back().as<ObjectValue>();
            SkASSERT(obj.is<ObjectValue>());
            for (const auto& member : obj) {
                SkASSERT(member.fKey.is<StringValue>());
            }
        )
    }

    void pushArrayScope() {
        // Array placeholder.
        fValueStack.emplace_back();

        // Array scope marker (-size).
        fScopeStack.push_back(-SkTo<intptr_t>(fValueStack.size()));
    }

    void popArrayScope() {
        const auto scope_start = -fScopeStack.back();
        SkASSERT(scope_start > 0);
        this->popScopeAsVec<ArrayValue>(SkTo<size_t>(scope_start));

        SkDEBUGCODE(
            const auto& arr = fValueStack.back().as<ArrayValue>();
            SkASSERT(arr.is<ArrayValue>());
        )
    }

    void pushObjectKey(const char* key, size_t size) {
        SkASSERT(fScopeStack.back() >= 0);
        SkASSERT(fValueStack.size() >= SkTo<size_t>(fScopeStack.back()));
        SkASSERT(!((fValueStack.size() - SkTo<size_t>(fScopeStack.back())) & 1));
        this->pushString(key, size);
    }

    void pushTrue() {
        fValueStack.push_back(BoolValue(true));
    }

    void pushFalse() {
        fValueStack.push_back(BoolValue(false));
    }

    void pushNull() {
        fValueStack.push_back(NullValue());
    }

    void pushString(const char* s, size_t size) {
        fValueStack.push_back(StringValue(s, size, fAlloc));
    }

    void pushInt32(int32_t i) {
        fValueStack.push_back(NumberValue(i));
    }

    void pushFloat(float f) {
        fValueStack.push_back(NumberValue(f));
    }

    template <typename T>
    T error(T&& ret_val, const char* p, const char* msg) {
#if defined(SK_JSON_REPORT_ERRORS)
        fErrorToken = p;
        fErrorMessage.set(msg);
#endif
        return ret_val;
    }

    const char* matchTrue(const char* p) {
        SkASSERT(p[0] == 't');

        if (p[1] == 'r' && p[2] == 'u' && p[3] == 'e') {
            this->pushTrue();
            return p + 4;
        }

        return this->error(nullptr, p, "invalid token");
    }

    const char* matchFalse(const char* p) {
        SkASSERT(p[0] == 'f');

        if (p[1] == 'a' && p[2] == 'l' && p[3] == 's' && p[4] == 'e') {
            this->pushFalse();
            return p + 5;
        }

        return this->error(nullptr, p, "invalid token");
    }

    const char* matchNull(const char* p) {
        SkASSERT(p[0] == 'n');

        if (p[1] == 'u' && p[2] == 'l' && p[3] == 'l') {
            this->pushNull();
            return p + 4;
        }

        return this->error(nullptr, p, "invalid token");
    }

    template <typename MatchFunc>
    const char* matchString(const char* p, MatchFunc&& func) {
        SkASSERT(*p == '"');
        const auto* s_begin = p + 1;

        // TODO: unescape
        for (p = s_begin; !is_sterminator(*p); ++p) {}

        if (*p == '"') {
            func(s_begin, p - s_begin);
            return p + 1;
        }

        return this->error(nullptr, s_begin - 1, "invalid string");
    }

    const char* matchFastFloatDecimalPart(const char* p, int sign, float f, int exp) {
        SkASSERT(exp <= 0);

        for (;;) {
            if (!is_digit(*p)) break;
            f = f * 10.f + (*p++ - '0'); --exp;
            if (!is_digit(*p)) break;
            f = f * 10.f + (*p++ - '0'); --exp;
        }

        if (is_numeric(*p)) {
            SkASSERT(*p == '.' || *p == 'e' || *p == 'E');
            // We either have malformed input, or an (unsupported) exponent.
            return nullptr;
        }

        this->pushFloat(sign * f * pow10(exp));

        return p;
    }

    const char* matchFastFloatPart(const char* p, int sign, float f) {
        for (;;) {
            if (!is_digit(*p)) break;
            f = f * 10.f + (*p++ - '0');
            if (!is_digit(*p)) break;
            f = f * 10.f + (*p++ - '0');
        }

        if (!is_numeric(*p)) {
            // Matched (integral) float.
            this->pushFloat(sign * f);
            return p;
        }

        return (*p == '.') ? this->matchFastFloatDecimalPart(p + 1, sign, f, 0)
                           : nullptr;
    }

    const char* matchFast32OrFloat(const char* p) {
        int sign = 1;
        if (*p == '-') {
            sign = -1;
            ++p;
        }

        const auto* digits_start = p;

        int32_t n32 = 0;

        // This is the largest absolute int32 value we can handle before
        // risking overflow *on the next digit* (214748363).
        static constexpr int32_t kMaxInt32 = (std::numeric_limits<int32_t>::max() - 9) / 10;

        if (is_digit(*p)) {
            n32 = (*p++ - '0');
            for (;;) {
                if (!is_digit(*p) || n32 > kMaxInt32) break;
                n32 = n32 * 10 + (*p++ - '0');
            }
        }

        if (!is_numeric(*p)) {
            // Did we actually match any digits?
            if (p > digits_start) {
                this->pushInt32(sign * n32);
                return p;
            }
            return nullptr;
        }

        if (*p == '.') {
            const auto* decimals_start = ++p;

            int exp = 0;

            for (;;) {
                if (!is_digit(*p) || n32 > kMaxInt32) break;
                n32 = n32 * 10 + (*p++ - '0'); --exp;
                if (!is_digit(*p) || n32 > kMaxInt32) break;
                n32 = n32 * 10 + (*p++ - '0'); --exp;
            }

            if (!is_numeric(*p)) {
                // Did we actually match any digits?
                if (p > decimals_start) {
                    this->pushFloat(sign * n32 * pow10(exp));
                    return p;
                }
                return nullptr;
            }

            if (n32 > kMaxInt32) {
                // we ran out on n32 bits
                return this->matchFastFloatDecimalPart(p, sign, n32, exp);
            }
        }

        return this->matchFastFloatPart(p, sign, n32);
    }

    const char* matchNumber(const char* p) {
        if (const auto* fast = this->matchFast32OrFloat(p)) return fast;

        // slow fallback
        char* matched;
        float f = strtof(p, &matched);
        if (matched > p) {
            this->pushFloat(f);
            return matched;
        }
        return this->error(nullptr, p, "invalid numeric token");
    }
};

void Write(const Value& v, SkWStream* stream) {
    switch (v.getType()) {
    case Value::Type::kNull:
        stream->writeText("null");
        break;
    case Value::Type::kBool:
        stream->writeText(*v.as<BoolValue>() ? "true" : "false");
        break;
    case Value::Type::kNumber:
        stream->writeScalarAsText(*v.as<NumberValue>());
        break;
    case Value::Type::kString:
        stream->writeText("\"");
        stream->writeText(v.as<StringValue>().begin());
        stream->writeText("\"");
        break;
    case Value::Type::kArray: {
        const auto& array = v.as<ArrayValue>();
        stream->writeText("[");
        bool first_value = true;
        for (const auto& v : array) {
            if (!first_value) stream->writeText(",");
            Write(v, stream);
            first_value = false;
        }
        stream->writeText("]");
        break;
    }
    case Value::Type::kObject:
        const auto& object = v.as<ObjectValue>();
        stream->writeText("{");
        bool first_member = true;
        for (const auto& member : object) {
            SkASSERT(member.fKey.getType() == Value::Type::kString);
            if (!first_member) stream->writeText(",");
            Write(member.fKey, stream);
            stream->writeText(":");
            Write(member.fValue, stream);
            first_member = false;
        }
        stream->writeText("}");
        break;
    }
}

} // namespace

SkString Value::toString() const {
    SkDynamicMemoryWStream wstream;
    Write(*this, &wstream);
    const auto data = wstream.detachAsData();
    // TODO: is there a better way to pass data around without copying?
    return SkString(static_cast<const char*>(data->data()), data->size());
}

static constexpr size_t kMinChunkSize = 4096;

DOM::DOM(const char* data, size_t size)
    : fAlloc(kMinChunkSize) {
    DOMParser parser(fAlloc);

    fRoot = parser.parse(data, size);
}

void DOM::write(SkWStream* stream) const {
    Write(fRoot, stream);
}

} // namespace skjson
