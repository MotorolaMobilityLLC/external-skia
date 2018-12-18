/*
 * Copyright 2010 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPDFTypes_DEFINED
#define SkPDFTypes_DEFINED

#include "SkRefCnt.h"
#include "SkScalar.h"
#include "SkTHash.h"
#include "SkTo.h"
#include "SkTypes.h"

#include <new>
#include <type_traits>
#include <utility>
#include <vector>

class SkData;
class SkPDFCanon;
class SkPDFDocument;
class SkPDFObjNumMap;
class SkPDFObject;
class SkStreamAsset;
class SkString;
class SkWStream;
struct SkPDFObjectSerializer;

#ifdef SK_PDF_IMAGE_STATS
    #include <atomic>
#endif

struct SkPDFIndirectReference {
    int fValue = -1;
    explicit operator bool() { return fValue != -1; }
};

inline static bool operator==(SkPDFIndirectReference u, SkPDFIndirectReference v) {
    return u.fValue == v.fValue;
}

inline static bool operator!=(SkPDFIndirectReference u, SkPDFIndirectReference v) {
    return u.fValue != v.fValue;
}


/** \class SkPDFObject

    A PDF Object is the base class for primitive elements in a PDF file.  A
    common subtype is used to ease the use of indirect object references,
    which are common in the PDF format.

*/
class SkPDFObject : public SkRefCnt {
public:
    /** Subclasses must implement this method to print the object to the
     *  PDF file.
     *  @param catalog  The object catalog to use.
     *  @param stream   The writable output stream to send the output to.
     */
    virtual void emitObject(SkWStream* stream) const = 0;

    /**
     *  Adds all transitive dependencies of this object to the
     *  catalog.  Implementations should respect the catalog's object
     *  substitution map.
     */
    virtual void addResources(SkPDFObjNumMap* catalog) const {}

    /**
     *  Release all resources associated with this SkPDFObject.  It is
     *  an error to call emitObject() or addResources() after calling
     *  drop().
     */
    virtual void drop() {}

    virtual ~SkPDFObject() {}

    SkPDFIndirectReference fIndirectReference;

private:
    typedef SkRefCnt INHERITED;
};

////////////////////////////////////////////////////////////////////////////////

template <class T>
class SkStorageFor {
public:
    const T& get() const { return *reinterpret_cast<const T*>(&fStore); }
    T& get() { return *reinterpret_cast<T*>(&fStore); }
    // Up to caller to keep track of status.
    template<class... Args> void init(Args&&... args) {
        new (&this->get()) T(std::forward<Args>(args)...);
    }
    void destroy() { this->get().~T(); }
private:
    typename std::aligned_storage<sizeof(T), alignof(T)>::type fStore;
};

/**
   A SkPDFUnion is a non-virtualized implementation of the
   non-compound, non-specialized PDF Object types: Name, String,
   Number, Boolean.
 */
class SkPDFUnion {
public:
    // Move contstructor and assignemnt operator destroy the argument
    // and steal their references (if needed).
    SkPDFUnion(SkPDFUnion&& other);
    SkPDFUnion& operator=(SkPDFUnion&& other);

    ~SkPDFUnion();

    /** The following nine functions are the standard way of creating
        SkPDFUnion objects. */

    static SkPDFUnion Int(int32_t);

    static SkPDFUnion Int(size_t v) { return SkPDFUnion::Int(SkToS32(v)); }

    static SkPDFUnion Bool(bool);

    static SkPDFUnion Scalar(SkScalar);

    static SkPDFUnion ColorComponent(uint8_t);

    static SkPDFUnion ColorComponentF(float);

    /** These two functions do NOT take ownership of char*, and do NOT
        copy the string.  Suitable for passing in static const
        strings. For example:
          SkPDFUnion n = SkPDFUnion::Name("Length");
          SkPDFUnion u = SkPDFUnion::String("Identity"); */

    /** SkPDFUnion::Name(const char*) assumes that the passed string
        is already a valid name (that is: it has no control or
        whitespace characters).  This will not copy the name. */
    static SkPDFUnion Name(const char*);

    /** SkPDFUnion::String will encode the passed string.  This will
        not copy the name. */
    static SkPDFUnion String(const char*);

    /** SkPDFUnion::Name(SkString) does not assume that the
        passed string is already a valid name and it will escape the
        string. */
    static SkPDFUnion Name(SkString);

    /** SkPDFUnion::String will encode the passed string. */
    static SkPDFUnion String(SkString);

    static SkPDFUnion Object(sk_sp<SkPDFObject>);
    static SkPDFUnion ObjRef(sk_sp<SkPDFObject>);

    static SkPDFUnion Ref(SkPDFIndirectReference);

    /** These two non-virtual methods mirror SkPDFObject's
        corresponding virtuals. */
    void emitObject(SkWStream*) const;
    void addResources(SkPDFObjNumMap*) const;

    bool isName() const;

private:
    union {
        int32_t fIntValue;
        bool fBoolValue;
        SkScalar fScalarValue;
        const char* fStaticString;
        SkStorageFor<SkString> fSkString;
        SkPDFObject* fObject;
    };
    enum class Type : char {
        /** It is an error to call emitObject() or addResources() on an
            kDestroyed object. */
        kDestroyed = 0,
        kInt,
        kColorComponent,
        kColorComponentF,
        kBool,
        kScalar,
        kName,
        kString,
        kNameSkS,
        kStringSkS,
        kObjRef,
        kObject,
        kRef,
    };
    Type fType;

    SkPDFUnion(Type);
    SkPDFUnion(Type, int32_t);
    SkPDFUnion(Type, bool);
    SkPDFUnion(Type, SkScalar);
    SkPDFUnion(Type, SkString);
    // We do not now need copy constructor and copy assignment, so we
    // will disable this functionality.
    SkPDFUnion& operator=(const SkPDFUnion&) = delete;
    SkPDFUnion(const SkPDFUnion&) = delete;
};
static_assert(sizeof(SkString) == sizeof(void*), "SkString_size");

// Exposed for unit testing.
void SkPDFWriteString(SkWStream* wStream, const char* cin, size_t len);

////////////////////////////////////////////////////////////////////////////////

#if 0  // Enable if needed.
/** This class is a SkPDFUnion with SkPDFObject virtuals attached.
    The only use case of this is when a non-compound PDF object is
    referenced indirectly. */
class SkPDFAtom final : public SkPDFObject {
public:
    void emitObject(SkWStream* stream) final;
    void addResources(SkPDFObjNumMap* const final;
    SkPDFAtom(SkPDFUnion&& v) : fValue(std::move(v) {}

private:
    const SkPDFUnion fValue;
    typedef SkPDFObject INHERITED;
};
#endif  // 0

////////////////////////////////////////////////////////////////////////////////

/** \class SkPDFArray

    An array object in a PDF.
*/
class SkPDFArray final : public SkPDFObject {
public:
    /** Create a PDF array. Maximum length is 8191.
     */
    SkPDFArray();
    ~SkPDFArray() override;

    // The SkPDFObject interface.
    void emitObject(SkWStream* stream) const override;
    void addResources(SkPDFObjNumMap*) const override;
    void drop() override;

    /** The size of the array.
     */
    size_t size() const;

    /** Preallocate space for the given number of entries.
     *  @param length The number of array slots to preallocate.
     */
    void reserve(int length);

    /** Appends a value to the end of the array.
     *  @param value The value to add to the array.
     */
    void appendInt(int32_t);
    void appendColorComponent(uint8_t);
    void appendBool(bool);
    void appendScalar(SkScalar);
    void appendName(const char[]);
    void appendName(SkString);
    void appendString(const char[]);
    void appendString(SkString);
    void appendObject(sk_sp<SkPDFObject>);
    void appendObjRef(sk_sp<SkPDFObject>);
    void appendRef(SkPDFIndirectReference);

private:
    std::vector<SkPDFUnion> fValues;
    void append(SkPDFUnion&& value);
    SkDEBUGCODE(bool fDumped;)
};

static inline void SkPDFArray_Append(SkPDFArray* a, int v) { a->appendInt(v); }

static inline void SkPDFArray_Append(SkPDFArray* a, SkScalar v) { a->appendScalar(v); }

template <typename T, typename... Args>
inline void SkPDFArray_Append(SkPDFArray* a, T v, Args... args) {
    SkPDFArray_Append(a, v);
    SkPDFArray_Append(a, args...);
}

template <typename... Args>
inline sk_sp<SkPDFArray> SkPDFMakeArray(Args... args) {
    auto ret = sk_make_sp<SkPDFArray>();
    ret->reserve(sizeof...(Args));
    SkPDFArray_Append(ret.get(), args...);
    return ret;
}

/** \class SkPDFDict

    A dictionary object in a PDF.
*/
class SkPDFDict : public SkPDFObject {
public:
    /** Create a PDF dictionary.
     *  @param type   The value of the Type entry, nullptr for no type.
     */
    explicit SkPDFDict(const char type[] = nullptr);

    ~SkPDFDict() override;

    // The SkPDFObject interface.
    void emitObject(SkWStream* stream) const override;
    void addResources(SkPDFObjNumMap*) const override;
    void drop() override;

    /** The size of the dictionary.
     */
    size_t size() const;

    /** Preallocate space for n key-value pairs */
    void reserve(int n);

    /** Add the value to the dictionary with the given key.
     *  @param key   The text of the key for this dictionary entry.
     *  @param value The value for this dictionary entry.
     */
    void insertObject(const char key[], sk_sp<SkPDFObject>);
    void insertObject(SkString, sk_sp<SkPDFObject>);
    void insertObjRef(const char key[], sk_sp<SkPDFObject>);
    void insertObjRef(SkString, sk_sp<SkPDFObject>);
    void insertRef(const char key[], SkPDFIndirectReference);
    void insertRef(SkString, SkPDFIndirectReference);

    /** Add the value to the dictionary with the given key.
     *  @param key   The text of the key for this dictionary entry.
     *  @param value The value for this dictionary entry.
     */
    void insertBool(const char key[], bool value);
    void insertInt(const char key[], int32_t value);
    void insertInt(const char key[], size_t value);
    void insertScalar(const char key[], SkScalar value);
    void insertColorComponentF(const char key[], SkScalar value);
    void insertName(const char key[], const char nameValue[]);
    void insertName(const char key[], SkString nameValue);
    void insertString(const char key[], const char value[]);
    void insertString(const char key[], SkString value);

    /** Emit the dictionary, without the "<<" and ">>".
     */
    void emitAll(SkWStream* stream) const;

private:
    struct Record {
        SkPDFUnion fKey;
        SkPDFUnion fValue;
    };
    std::vector<Record> fRecords;
    SkDEBUGCODE(bool fDumped;)
};

#ifdef SK_PDF_LESS_COMPRESSION
    static constexpr bool kSkPDFDefaultDoDeflate = false;
#else
    static constexpr bool kSkPDFDefaultDoDeflate = true;
#endif

SkPDFIndirectReference SkPDFStreamOut(sk_sp<SkPDFDict> dict,
                                      std::unique_ptr<SkStreamAsset> stream,
                                      SkPDFDocument* doc,
                                      bool deflate = kSkPDFDefaultDoDeflate);

////////////////////////////////////////////////////////////////////////////////

/** \class SkPDFObjNumMap

    The PDF Object Number Map manages object numbers.  It is used to
    create the PDF cross reference table.
*/
class SkPDFObjNumMap : SkNoncopyable {
public:
    SkPDFObjNumMap(SkPDFObjectSerializer* s) : fIndirectReferenceSource(s) {}

    /** Add the passed object to the catalog, as well as all its dependencies.
     *  @param obj   The object to add.  If nullptr, this is a noop.
     */
    void addObjectRecursively(SkPDFObject* obj);

    /** Get the object number for the passed object.
     *  @param obj         The object of interest.
     */
    int getObjectNumber(SkPDFObject* obj) const {
        return SkASSERT(obj), obj->fIndirectReference.fValue;
    }
    const std::vector<sk_sp<SkPDFObject>>& objects() const { return fObjects; }

private:
    friend struct SkPDFObjectSerializer;
    SkPDFObjectSerializer* fIndirectReferenceSource;
    std::vector<sk_sp<SkPDFObject>> fObjects;
};

////////////////////////////////////////////////////////////////////////////////

#ifdef SK_PDF_IMAGE_STATS
extern std::atomic<int> gDrawImageCalls;
extern std::atomic<int> gJpegImageObjects;
extern std::atomic<int> gRegularImageObjects;
extern void SkPDFImageDumpStats();
#endif // SK_PDF_IMAGE_STATS

#endif
