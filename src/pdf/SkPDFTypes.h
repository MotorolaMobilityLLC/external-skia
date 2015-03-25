
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
#include "SkString.h"
#include "SkTDArray.h"
#include "SkTypes.h"

class SkPDFCatalog;
class SkWStream;

class SkPDFObject;

/** \class SkPDFObject

    A PDF Object is the base class for primitive elements in a PDF file.  A
    common subtype is used to ease the use of indirect object references,
    which are common in the PDF format.
*/
class SkPDFObject : public SkRefCnt {
public:
    SK_DECLARE_INST_COUNT(SkPDFObject)

    /** Subclasses must implement this method to print the object to the
     *  PDF file.
     *  @param catalog  The object catalog to use.
     *  @param stream   The writable output stream to send the output to.
     */
    // TODO(halcanary): make this method const
    virtual void emitObject(SkWStream* stream, SkPDFCatalog* catalog) = 0;

    /**
     *  Adds all transitive dependencies of this object to the
     *  catalog.  Implementations should respect the catalog's object
     *  substitution map.
     */
    virtual void addResources(SkPDFCatalog* catalog) const {}

private:
    typedef SkRefCnt INHERITED;
};

/** \class SkPDFObjRef

    An indirect reference to a PDF object.
*/
class SkPDFObjRef : public SkPDFObject {
public:
    SK_DECLARE_INST_COUNT(SkPDFObjRef)

    /** Create a reference to an existing SkPDFObject.
     *  @param obj The object to reference.
     */
    explicit SkPDFObjRef(SkPDFObject* obj);
    virtual ~SkPDFObjRef();

    // The SkPDFObject interface.
    virtual void emitObject(SkWStream* stream, SkPDFCatalog* catalog) SK_OVERRIDE;
    virtual void addResources(SkPDFCatalog*) const SK_OVERRIDE;

private:
    SkAutoTUnref<SkPDFObject> fObj;

    typedef SkPDFObject INHERITED;
};

/** \class SkPDFInt

    An integer object in a PDF.
*/
class SkPDFInt : public SkPDFObject {
public:
    SK_DECLARE_INST_COUNT(SkPDFInt)

    /** Create a PDF integer (usually for indirect reference purposes).
     *  @param value An integer value between 2^31 - 1 and -2^31.
     */
    explicit SkPDFInt(int32_t value);
    virtual ~SkPDFInt();

    // The SkPDFObject interface.
    virtual void emitObject(SkWStream* stream, SkPDFCatalog* catalog) SK_OVERRIDE;

private:
    int32_t fValue;

    typedef SkPDFObject INHERITED;
};

/** \class SkPDFBool

    An boolean value in a PDF.
*/
class SkPDFBool : public SkPDFObject {
public:
    SK_DECLARE_INST_COUNT(SkPDFBool)

    /** Create a PDF boolean.
     *  @param value true or false.
     */
    explicit SkPDFBool(bool value);
    virtual ~SkPDFBool();

    // The SkPDFObject interface.
    virtual void emitObject(SkWStream* stream, SkPDFCatalog* catalog) SK_OVERRIDE;

private:
    bool fValue;

    typedef SkPDFObject INHERITED;
};

/** \class SkPDFScalar

    A real number object in a PDF.
*/
class SkPDFScalar : public SkPDFObject {
public:
    SK_DECLARE_INST_COUNT(SkPDFScalar)

    /** Create a PDF real number.
     *  @param value A real value.
     */
    explicit SkPDFScalar(SkScalar value);
    virtual ~SkPDFScalar();

    static void Append(SkScalar value, SkWStream* stream);

    // The SkPDFObject interface.
    virtual void emitObject(SkWStream* stream, SkPDFCatalog* catalog) SK_OVERRIDE;

private:
    SkScalar fValue;

    typedef SkPDFObject INHERITED;
};

/** \class SkPDFString

    A string object in a PDF.
*/
class SkPDFString : public SkPDFObject {
public:
    SK_DECLARE_INST_COUNT(SkPDFString)

    /** Create a PDF string. Maximum length (in bytes) is 65,535.
     *  @param value A string value.
     */
    explicit SkPDFString(const char value[]);
    explicit SkPDFString(const SkString& value);

    /** Create a PDF string. Maximum length (in bytes) is 65,535.
     *  @param value     A string value.
     *  @param len       The length of value.
     *  @param wideChars Indicates if the top byte in value is significant and
     *                   should be encoded (true) or not (false).
     */
    SkPDFString(const uint16_t* value, size_t len, bool wideChars);
    virtual ~SkPDFString();

    // The SkPDFObject interface.
    virtual void emitObject(SkWStream* stream, SkPDFCatalog* catalog) SK_OVERRIDE;

    static SkString FormatString(const char* input, size_t len);
    static SkString FormatString(const uint16_t* input, size_t len,
                                 bool wideChars);
private:
    static const size_t kMaxLen = 65535;

    const SkString fValue;

    static SkString DoFormatString(const void* input, size_t len,
                                 bool wideInput, bool wideOutput);

    typedef SkPDFObject INHERITED;
};

/** \class SkPDFName

    A name object in a PDF.
*/
class SkPDFName : public SkPDFObject {
public:
    SK_DECLARE_INST_COUNT(SkPDFName)

    /** Create a PDF name object. Maximum length is 127 bytes.
     *  @param value The name.
     */
    explicit SkPDFName(const char name[]);
    explicit SkPDFName(const SkString& name);
    virtual ~SkPDFName();

    bool operator==(const SkPDFName& b) const;

    // The SkPDFObject interface.
    virtual void emitObject(SkWStream* stream, SkPDFCatalog* catalog) SK_OVERRIDE;

private:
    static const size_t kMaxLen = 127;

    const SkString fValue;

    static SkString FormatName(const SkString& input);

    typedef SkPDFObject INHERITED;
};

/** \class SkPDFArray

    An array object in a PDF.
*/
class SkPDFArray : public SkPDFObject {
public:
    SK_DECLARE_INST_COUNT(SkPDFArray)

    /** Create a PDF array. Maximum length is 8191.
     */
    SkPDFArray();
    virtual ~SkPDFArray();

    // The SkPDFObject interface.
    virtual void emitObject(SkWStream* stream, SkPDFCatalog* catalog) SK_OVERRIDE;
    virtual void addResources(SkPDFCatalog*) const SK_OVERRIDE;

    /** The size of the array.
     */
    int size() { return fValue.count(); }

    /** Preallocate space for the given number of entries.
     *  @param length The number of array slots to preallocate.
     */
    void reserve(int length);

    /** Returns the object at the given offset in the array.
     *  @param index The index into the array to retrieve.
     */
    SkPDFObject* getAt(int index) { return fValue[index]; }

    /** Set the object at the given offset in the array. Ref's value.
     *  @param index The index into the array to set.
     *  @param value The value to add to the array.
     *  @return The value argument is returned.
     */
    SkPDFObject* setAt(int index, SkPDFObject* value);

    /** Append the object to the end of the array and increments its ref count.
     *  @param value The value to add to the array.
     *  @return The value argument is returned.
     */
    SkPDFObject* append(SkPDFObject* value);

    /** Creates a SkPDFInt object and appends it to the array.
     *  @param value The value to add to the array.
     */
    void appendInt(int32_t value);

    /** Creates a SkPDFScalar object and appends it to the array.
     *  @param value The value to add to the array.
     */
    void appendScalar(SkScalar value);

    /** Creates a SkPDFName object and appends it to the array.
     *  @param value The value to add to the array.
     */
    void appendName(const char name[]);

private:
    static const int kMaxLen = 8191;
    SkTDArray<SkPDFObject*> fValue;

    typedef SkPDFObject INHERITED;
};

/** \class SkPDFDict

    A dictionary object in a PDF.
*/
class SkPDFDict : public SkPDFObject {
public:
    SK_DECLARE_INST_COUNT(SkPDFDict)

    /** Create a PDF dictionary. Maximum number of entries is 4095.
     */
    SkPDFDict();

    /** Create a PDF dictionary with a Type entry.
     *  @param type   The value of the Type entry.
     */
    explicit SkPDFDict(const char type[]);

    virtual ~SkPDFDict();

    // The SkPDFObject interface.
    virtual void emitObject(SkWStream* stream, SkPDFCatalog* catalog) SK_OVERRIDE;
    virtual void addResources(SkPDFCatalog*) const SK_OVERRIDE;

    /** The size of the dictionary.
     */
    int size() const;

    /** Add the value to the dictionary with the given key.  Refs value.
     *  @param key   The key for this dictionary entry.
     *  @param value The value for this dictionary entry.
     *  @return The value argument is returned.
     */
    SkPDFObject* insert(SkPDFName* key, SkPDFObject* value);

    /** Add the value to the dictionary with the given key.  Refs value.  The
     *  method will create the SkPDFName object.
     *  @param key   The text of the key for this dictionary entry.
     *  @param value The value for this dictionary entry.
     *  @return The value argument is returned.
     */
    SkPDFObject* insert(const char key[], SkPDFObject* value);

    /** Add the int to the dictionary with the given key.
     *  @param key   The text of the key for this dictionary entry.
     *  @param value The int value for this dictionary entry.
     */
    void insertInt(const char key[], int32_t value);

    /**
     *  Calls insertInt() but asserts in debug builds that the value can be represented
     *  by an int32_t.
     */
    void insertInt(const char key[], size_t value) {
        this->insertInt(key, SkToS32(value));
    }

    /** Add the scalar to the dictionary with the given key.
     *  @param key   The text of the key for this dictionary entry.
     *  @param value The scalar value for this dictionary entry.
     */
    void insertScalar(const char key[], SkScalar value);

    /** Add the name to the dictionary with the given key.
     *  @param key   The text of the key for this dictionary entry.
     *  @param name  The name for this dictionary entry.
     */
    void insertName(const char key[], const char name[]);

    /** Add the name to the dictionary with the given key.
     *  @param key   The text of the key for this dictionary entry.
     *  @param name  The name for this dictionary entry.
     */
    void insertName(const char key[], const SkString& name) {
        this->insertName(key, name.c_str());
    }

    /** Remove all entries from the dictionary.
     */
    void clear();

protected:
    /** Use to remove a single key from the dictionary.
     */
    void remove(const char key[]);

    /** Insert references to all of the key-value pairs from the other
     *  dictionary into this one.
     */
    void mergeFrom(const SkPDFDict& other);

private:
    struct Rec {
        SkPDFName* key;
        SkPDFObject* value;
        Rec(SkPDFName* k, SkPDFObject* v) : key(k), value(v) {}
    };

    static const int kMaxLen = 4095;

    SkTDArray<struct Rec> fValue;

    SkPDFObject* append(SkPDFName* key, SkPDFObject* value);

    typedef SkPDFObject INHERITED;
};

#endif
