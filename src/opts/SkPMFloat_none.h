#include "SkColorPriv.h"

inline void SkPMFloat::set(SkPMColor c) {
    this->setA(SkGetPackedA32(c));
    this->setR(SkGetPackedR32(c));
    this->setG(SkGetPackedG32(c));
    this->setB(SkGetPackedB32(c));
    SkASSERT(this->isValid());
}

inline SkPMColor SkPMFloat::get() const {
    SkASSERT(this->isValid());
    return SkPackARGB32(this->a(), this->r(), this->g(), this->b());
}

inline SkPMColor SkPMFloat::clamped() const {
    float a = this->a(),
          r = this->r(),
          g = this->g(),
          b = this->b();
    a = a < 0 ? 0 : (a > 255 ? 255 : a);
    r = r < 0 ? 0 : (r > 255 ? 255 : r);
    g = g < 0 ? 0 : (g > 255 ? 255 : g);
    b = b < 0 ? 0 : (b > 255 ? 255 : b);
    return SkPackARGB32(a, r, g, b);
}
