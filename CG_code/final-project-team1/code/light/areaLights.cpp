#include <areaLights.h>

// Constructors
DiffuseAreaLight::DiffuseAreaLight(const Spectrum& le, const shared_ptr<Shape>& shape, bool isTwoSide/* = false*/) : AreaLight(le, shape), isTwoSide(isTwoSide) { }

// Methods
/*override*/ Spectrum DiffuseAreaLight::Le(const Ray& incidentRayWorld) const
{
    if (isTwoSide == false && Normal3f::Dot(incidentRayWorld.d, shape->Ng(incidentRayWorld)) >= 0)
        return Spectrum();
    return le;
}

/*override*/ Spectrum DiffuseAreaLight::Sample_Le(
        const Interaction& ref, const Point2f& samples, 
        Interaction* wAtLight, Float* pdf) const
{
    // Sample shape
    *wAtLight = shape->Sample(ref, samples, pdf);
    // Get Le
    if (*pdf > 0)
        return Le(Ray(ref.p, Vector3f(wAtLight->p - ref.p)));
    else
        return Spectrum();
}

/*override*/ Float DiffuseAreaLight::Pdf(const Interaction& ref, const Vector3f& wToLight) const
{
    return shape->PdfW(ref, wToLight);
}
