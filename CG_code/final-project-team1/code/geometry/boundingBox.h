#ifndef BOUNDING_BOX_H
#define BOUNDING_BOX_H

#include <vector.h>
#include <ray.h>

class AABound3f
{
public:
    // Fields
    Point3f pMin, pMax;

    // Constructors
    AABound3f();
    AABound3f(const Point3f& p);
    AABound3f(const Point3f& p1, const Point3f& p2);

    // Methods
    bool Intersect(const Ray& ray, Float* tHit0 = nullptr, Float* tHit1 = nullptr) const;
    bool Intersect(const Ray& ray, const Vector3f& dirInv, const int isDirNeg[3]) const;
    bool Intersect(const Point3f& p0, const Point3f& p1, const Point3f& p2) const;
    Point3f Center() const;
    Point3f Lerp(const Vector3f& t) const;
    static AABound3f Union(const AABound3f& bound, const Point3f& point);
    static AABound3f Union(const AABound3f& bound1, const AABound3f& bound2);
    Vector3f Diagonal() const;
    int MaximumExtent() const;
    Vector3f Offset(const Point3f& p) const;
    Float Area() const;

    // Operators
    Point3f& operator[](int i);
    const Point3f& operator[](int i) const;
};

class AABound3i
{
public:
    // Fields
    Point3i pMin, pMax;

    // Constructors
    AABound3i();
    AABound3i(const Point3i& p);
    AABound3i(const Point3i& p1, const Point3i& p2);
};

#endif