/*
* Copyright 2019 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef SkReflected_DEFINED
#define SkReflected_DEFINED

#include "SkColor.h"
#include "SkCurve.h"
#include "SkRefCnt.h"
#include "SkString.h"
#include "SkTArray.h"

class SkFieldVisitor;
class SkRandom;

class SkReflected : public SkRefCnt {
public:
    typedef sk_sp<SkReflected>(*Factory)();
    struct Type {
        const char* fName;
        const Type* fBase;
        Factory     fFactory;

        bool isDerivedFrom(const Type* t) const {
            const Type* base = fBase;
            while (base) {
                if (base == t) {
                    return true;
                }
                base = base->fBase;
            }
            return false;
        }
    };

    virtual const Type* getType() const = 0;
    static const Type* GetType() {
        static Type gType{ "SkReflected", nullptr, nullptr };
        return &gType;
    }

    bool isOfType(const Type* t) const {
        const Type* thisType = this->getType();
        return thisType == t || thisType->isDerivedFrom(t);
    }

    static void Register(const Type* type) {
        gTypes.push_back(type);
    }

    static sk_sp<SkReflected> CreateInstance(const char* name) {
        for (const Type* type : gTypes) {
            if (0 == strcmp(name, type->fName)) {
                return type->fFactory();
            }
        }
        return nullptr;
    }

    virtual void visitFields(SkFieldVisitor*) = 0;

    static void VisitTypes(std::function<void(const Type*)> visitor,
                           const Type* baseType = nullptr);
private:
    static SkSTArray<16, const Type*, true> gTypes;
};

#define REFLECTED(TYPE, BASE)                                    \
    static sk_sp<SkReflected> CreateProc() {                     \
        return sk_sp<SkReflected>(new TYPE());                   \
    }                                                            \
    static const Type* GetType() {                               \
        static Type gType{ #TYPE, BASE::GetType(), CreateProc }; \
        return &gType;                                           \
    }                                                            \
    const Type* getType() const override { return GetType(); }

#define REFLECTED_ABSTRACT(TYPE, BASE)                          \
    static const Type* GetType() {                              \
        static Type gType{ #TYPE, BASE::GetType(), nullptr };   \
        return &gType;                                          \
    }                                                           \
    const Type* getType() const override { return GetType(); }

#define REGISTER_REFLECTED(TYPE) SkReflected::Register(TYPE::GetType())

///////////////////////////////////////////////////////////////////////////////

struct SkField {
    // Other useful flags/properties:
    //   No UI (to avoid editing dangerous things)
    //   Point vs. vector
    //   Range limits
    enum SkFieldFlags {
        kAngle_Field = 0x1,
    };

    SkField(uint32_t flags = 0)
        : fFlags(flags) {}

    uint32_t fFlags;
};

struct SkPoint;

class SkFieldVisitor {
public:
    virtual ~SkFieldVisitor() {}

    virtual void visit(const char*, float&, SkField = SkField()) = 0;
    virtual void visit(const char*, int&, SkField = SkField()) = 0;
    virtual void visit(const char*, bool&, SkField = SkField()) = 0;
    virtual void visit(const char*, SkString&, SkField = SkField()) = 0;

    virtual void visit(const char*, SkPoint&, SkField = SkField()) = 0;
    virtual void visit(const char*, SkColor4f&, SkField = SkField()) = 0;

    virtual void visit(const char* name, SkCurve& c, SkField = SkField()) {
        this->enterObject(name);
        c.visitFields(this);
        this->exitObject();
    }

    template <typename T>
    void visit(const char* name, T& value) {
        this->enterObject(name);
        value.visitFields(this);
        this->exitObject();
    }

    template <typename T, bool MEM_MOVE>
    void visit(const char* name, SkTArray<T, MEM_MOVE>& arr) {
        arr.resize_back(this->enterArray(name, arr.count()));
        for (int i = 0; i < arr.count(); ++i) {
            this->visit(nullptr, arr[i]);
        }
        this->exitArray().apply(arr);
    }

    template <typename T>
    void visit(const char* name, sk_sp<T>& obj) {
        this->enterObject(name);

        sk_sp<SkReflected> newObj = obj;
        this->visit(newObj, T::GetType());
        if (newObj != obj) {
            if (!newObj || newObj->isOfType(T::GetType())) {
                obj.reset(static_cast<T*>(newObj.release()));
            } else {
                obj.reset();
            }
        }

        if (obj) {
            obj->visitFields(this);
        }
        this->exitObject();
    }

protected:
    struct ArrayEdit {
        enum class Verb {
            kNone,
            kRemove,
            kMoveForward,
        };

        Verb fVerb = Verb::kNone;
        int fIndex = 0;

        template <typename T, bool MEM_MOVE>
        void apply(SkTArray<T, MEM_MOVE>& arr) const {
            switch (fVerb) {
            case Verb::kNone:
                break;
            case Verb::kRemove:
                for (int i = fIndex; i < arr.count() - 1; ++i) {
                    arr[i] = arr[i + 1];
                }
                arr.pop_back();
                break;
            case Verb::kMoveForward:
                if (fIndex > 0 && fIndex < arr.count()) {
                    std::swap(arr[fIndex - 1], arr[fIndex]);
                }
                break;
            }
        }
    };

    virtual void enterObject(const char* name) = 0;
    virtual void exitObject() = 0;

    virtual int enterArray(const char* name, int oldCount) = 0;
    virtual ArrayEdit exitArray() = 0;

    virtual void visit(sk_sp<SkReflected>&, const SkReflected::Type* baseType) = 0;
};

#endif // SkReflected_DEFINED
