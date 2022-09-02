#include <ray.h>

// Constructors
Ray::Ray() { }
Ray::Ray(const Point3f& o, const Vector3f& d, Float tMax/* = Infinity*/) : o(o), d(d), tMax(tMax) { }

// Operators
Point3f Ray::operator()(Float t) const { return Point3f(o + d * t); }
