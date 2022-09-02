#ifndef AREA_LIGHTS_H
#define AREA_LIGHTS_H

#include <light.h>
#include <shape.h>

class DiffuseAreaLight : public AreaLight
{
public:
    // Fields
    const bool isTwoSide;
    // Constructors
    DiffuseAreaLight(const Spectrum& le, const shared_ptr<Shape>& shape, bool isTwoSide = false);

    // Methods
    virtual Spectrum Le(const Ray& incidentRayWorld) const override;
    virtual Spectrum Sample_Le(const Interaction& ref, const Point2f& samples, 
                               Interaction* wAtLight, Float* pdf) const override;
    virtual Float Pdf(const Interaction& ref, const Vector3f& wToLight) const override;
};

#endif