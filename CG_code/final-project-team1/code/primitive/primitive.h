#ifndef PRIMITIVE_H
#define PRIMITIVE_H

#include <utils.h>
#include <spectrums.h>
#include <medium.h>
#ifdef INTEL
#include <embree3/rtcore.h>
#endif

class Vector3f;
class Ray;
class Interaction;
class SurfaceInteraction;
class AABound3f;

class Primitive
{
public:
    // Constructors
    virtual ~Primitive();

    // Methods
    virtual bool Intersect(const Ray& ray, Float* tHit, SurfaceInteraction* si) const = 0;
    virtual AABound3f WorldBound() const = 0;
    /**
     * @brief 间接地调用 material->ComputeScatteringFunctions(si)
     */
    virtual void ComputeScatteringFunctions(SurfaceInteraction* si) const;
    virtual Spectrum Le(const Ray& incidentRayWorld) const;
    virtual bool IsVolume() const;
    virtual Surface GetSurface() const;

    virtual bool IsGeometricPrimitive() const;

    #ifdef INTEL
    virtual int GenerateEmbreeGeometry(RTCDevice device, RTCScene scene) const;
    #endif
};

#endif