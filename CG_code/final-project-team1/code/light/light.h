#ifndef LIGHT_H
#define LIGHT_H

#include <vector.h>
#include <spectrums.h>
#include <scene.h>

class Interaction;
class Shape;
class Ray;

class Light
{
public:
    // Methods
    virtual Spectrum Le(const Ray& incidentRayWorld) const = 0;
    virtual Spectrum Sample_Le(const Interaction& ref, const Point2f& samples, 
                               Interaction* wAtLight, Float* pdf) const = 0;
    virtual Float Pdf(const Interaction& ref, const Vector3f& wToLight) const = 0;
    virtual bool IsInfiniteAreaLight() const;
};


class AreaLight : public Light
{
public:
    // Constructors
    AreaLight(const Spectrum& le, const shared_ptr<Shape>& shape);
    
    // Methods
protected:
    // Fields
    const Spectrum le;
    const shared_ptr<Shape> shape; // TODO: 可以改成 primitive 其实，因为需要用到 medium
};

#endif