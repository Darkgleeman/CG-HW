#include <geometricPrimitive.h>

// Constructors
GeometricPrimitive::GeometricPrimitive(const shared_ptr<Shape>& shape, 
                                       const shared_ptr<Material>& material, 
                                       const Surface& surface) : 
    shape(shape), 
    material(material), 
    surface(surface)
{ }

// Methods
/*virtual*/ bool GeometricPrimitive::Intersect(const Ray& ray, Float* tHit, SurfaceInteraction* si) const
{
    bool isHit = shape->Intersect(ray, tHit, si);
    if (isHit)
    {
        si->primitive = this;
        si->surface = GetSurface(); // 需要在积分器中处理内外向问题
    }
    return isHit;
}
/*override*/ AABound3f GeometricPrimitive::WorldBound() const
{
    return shape->WorldBound();
}
/*override*/ void GeometricPrimitive::ComputeScatteringFunctions(SurfaceInteraction* si) const
{
    if (material)
        material->ComputeScatteringFunctions(si);
}
/*override*/ Spectrum GeometricPrimitive::Le(const Ray& incidentRayWorld) const
{
    return areaLight ? areaLight->Le(incidentRayWorld) : Spectrum();
}
/*override*/ bool GeometricPrimitive::IsVolume() const
{
    return material ? false : true;
}
/*override*/ Surface GeometricPrimitive::GetSurface() const
{
    return surface;
}

/*override*/ bool GeometricPrimitive::IsGeometricPrimitive() const
{
    return true;
}

#ifdef INTEL
/*override*/ int GeometricPrimitive::GenerateEmbreeGeometry(RTCDevice device, RTCScene scene) const
{
    return shape->GenerateEmbreeGeometry(device, scene);
}
#endif