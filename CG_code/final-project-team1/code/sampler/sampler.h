#ifndef SAMPLER_H
#define SAMPLER_H

#include <vector.h>

class Sampler
{
public:
    // Methods
    virtual Float Get1D() = 0;
    virtual Point2f Get2D() = 0;
    virtual Point3f Get3D() = 0;
    Float Get1D(Float min, Float max);

    virtual bool IsRandomSampler() const = 0;

    // Static methods
    static Vector2f GetInUnitDisk(const Point2f& samples);
    static Vector3f GetInUnitSphere(const Point2f& samples);
    static Vector3f GetInUnitHemisphere(const Point2f& samples);
    static Vector3f GetOnUnitHemisphere(const Point2f& samples);
};

#endif