#include <primitive.h>

// Constructors
/*virtual*/ Primitive::~Primitive() { }

// Methods
/*virtual*/ void Primitive::ComputeScatteringFunctions(SurfaceInteraction* si) const
{
    Assert(false, 
           "Primitive::ComputeScatteringFunctions(): This function should never be called");
}
/*virtual*/ Spectrum Primitive::Le(const Ray& incidentRayWorld) const
{
    return Spectrum();
}

/*virtual*/ bool Primitive::IsVolume() const
{
    return false;
}

/*virtual*/ Surface Primitive::GetSurface() const
{
    return Surface(nullptr, nullptr);
}

/*virtual*/ bool Primitive::IsGeometricPrimitive() const
{
    return false;
}

#ifdef INTEL
/*virtual*/ int Primitive::GenerateEmbreeGeometry(RTCDevice device, RTCScene scene) const
{
    return -1;
}
#endif