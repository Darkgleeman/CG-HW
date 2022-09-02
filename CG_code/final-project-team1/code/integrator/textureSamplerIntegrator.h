#ifndef TEXTURE_SAMPLER_INTEGRATOR_H
#define TEXTURE_SAMPLER_INTEGRATOR_H

#include <samplerIntegrator.h>

class TextureSamplerIntegrator : public SamplerIntegrator
{
public:
    // Constructors
    TextureSamplerIntegrator(shared_ptr<const Camera>& camera, shared_ptr<Sampler>& sampler, 
                             int imageWidth, int imageHeigh, int samplesPerPixel);

    // Methods
    virtual Spectrum Integrate(const Ray& r, const Scene& scene, int depth) const override;
};

#endif