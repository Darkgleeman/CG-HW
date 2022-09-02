#ifndef CUBE_H
#define CUBE_H

#include <shape.h>

class Cube : public TransformedShape
{
public:
    // Constructors
    Cube(const Transform* objectToWorld, const Transform* worldToObject);

    // Methods
    virtual bool Intersect(const Ray& ray, Float* tHit, SurfaceInteraction* si) const override;
    virtual AABound3f ObjectBound() const override;
    virtual Float Area() const override;
    virtual Normal3f Ng(const Ray& incidentRayWorld) const override;

    virtual Interaction Sample(const Point2f& samples, Float* pdf) const override;
    virtual Interaction Sample(const Interaction& ref, 
                               const Point2f& samples, Float* pdf) const override;

    #ifdef INTEL
    virtual int GenerateEmbreeGeometry(RTCDevice device, RTCScene scene) const override;
    #endif

private:
    // Fields
    Float area;
};

#endif