#ifndef SHAPE_H
#define SHAPE_H

#include <ray.h>
#include <transform.h>
#include <interaction.h>
#include <scene.h>
#ifdef INTEL
#include <embree3/rtcore.h>
#endif

class Shape
{
public:
    // Methods
    virtual bool Intersect(const Ray& ray, Float* tHit, SurfaceInteraction* si) const = 0;
    virtual AABound3f ObjectBound() const = 0;
    virtual AABound3f WorldBound() const = 0;
    virtual Float Area() const = 0;
    virtual Normal3f Ng(const Ray& incidentRayWorld) const = 0;
    
    virtual Interaction Sample(const Point2f& samples, Float* pdf) const = 0;
    Float PdfArea() const;
    virtual Interaction Sample(const Interaction& ref, 
                               const Point2f& samples, Float* pdf) const = 0;
    virtual Float PdfW(const Interaction& ref, const Vector3f& rayDirToShape) const = 0;

    #ifdef INTEL
    virtual int GenerateEmbreeGeometry(RTCDevice device, RTCScene scene) const = 0;
    #endif
};

class TransformedShape : public Shape
{
public:
    // Fields
    const Transform* objectToWorld, * worldToObject;

    // Constructors
    TransformedShape(const Transform* objectToWorld, const Transform* worldToObject);

    // Methods
    virtual AABound3f WorldBound() const override;

    virtual Interaction Sample(const Point2f& samples, Float* pdf) const = 0;
    virtual Interaction Sample(const Interaction& ref, 
                               const Point2f& samples, Float* pdf) const override;
    virtual Float PdfW(const Interaction& ref, const Vector3f& rayDirToShape) const override;

    #ifdef INTEL
    virtual int GenerateEmbreeGeometry(RTCDevice device, RTCScene scene) const = 0;
    #endif
};

#endif