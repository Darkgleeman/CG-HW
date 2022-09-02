#ifndef RAY_H
#define RAY_H

#include <vector.h>

class Ray
{
public:
    // Fields
    Point3f o;
    Vector3f d;
    mutable Float tMax;

    // Constructors
    Ray();
    Ray(const Point3f& o, const Vector3f& d, Float tMax = Infinity);

    // Operators
    Point3f operator()(Float t) const;
};

#endif