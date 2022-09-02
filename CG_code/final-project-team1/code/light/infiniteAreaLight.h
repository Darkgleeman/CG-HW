#ifndef INFINITE_AREA_LIGHT_H
#define INFINITE_AREA_LIGHT_H

#include <light.h>
#include <samplers.h>
#include <textures.h>
#include <distributions.h>

class InfiniteAreaLight : public Light
{
public:
    // Fields
    const shared_ptr<Texture2f<Spectrum>> le;
    const shared_ptr<Distribution2f<Vector3f>> distribution;

    // Constructors
    InfiniteAreaLight(const shared_ptr<Texture2f<Spectrum>>& le, const shared_ptr<Distribution2f<Vector3f>>& distribution);
    
    // Methods
    virtual Spectrum Le(const Ray& incidentRayWorld) const override;
    virtual Spectrum Sample_Le(const Interaction& ref, const Point2f& samples, 
                               Interaction* wAtLight, Float* pdf) const override;
    virtual Float Pdf(const Interaction& ref, const Vector3f& wToLight) const override;
    virtual bool IsInfiniteAreaLight() const override;
};

#endif