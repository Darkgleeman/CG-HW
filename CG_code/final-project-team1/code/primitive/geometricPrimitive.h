#ifndef GEOMETRIC_PRIMITIVE_H
#define GEOMETRIC_PRIMITIVE_H

#include <primitive.h>
#include <boundingBox.h>
#include <shape.h>
#include <material.h>
#include <light.h>

class GeometricPrimitive : public Primitive
{
public:
    // Fields
    const shared_ptr<Shape> shape;
    const shared_ptr<Material> material;
    const Surface surface;
    shared_ptr<AreaLight> areaLight;

    // Constructors
    GeometricPrimitive(const shared_ptr<Shape>& shape, 
                       const shared_ptr<Material>& material, 
                       const Surface& surface);

    // Methods
    /**
     * @brief 间接地调用 shape->Intersect(ray, tHit, si)
     */
    virtual bool Intersect(const Ray& ray, Float* tHit, SurfaceInteraction* si) const override;
    /**
     * @brief 间接地调用 shape->WorldBound()
     */
    virtual AABound3f WorldBound() const override;
    /**
     * @brief 间接地调用 material->ComputeScatteringFunction(si)
     */
    virtual void ComputeScatteringFunctions(SurfaceInteraction* si) const override;
    /**
     * @brief 间接地调用 areaLight->Le(incidentRayWorld)
     */
    virtual Spectrum Le(const Ray& incidentRayWorld) const override;

    virtual bool IsVolume() const override;
    virtual Surface GetSurface() const override;

    virtual bool IsGeometricPrimitive() const override;

    #ifdef INTEL
    virtual int GenerateEmbreeGeometry(RTCDevice device, RTCScene scene) const override;
    #endif
private:
};

#endif