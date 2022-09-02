#ifndef VOLUME_PATH_SAMPLER_INTEGRATOR
#define VOLUME_PATH_SAMPLER_INTEGRATOR

#include <samplerIntegrator.h>

class VolumePathSamplerIntegrator : public SamplerIntegrator
{
public:
    // Constructors
    VolumePathSamplerIntegrator(shared_ptr<const Camera>& camera, shared_ptr<Sampler>& sampler, 
                                int imageWidth, int imageHeigh, int samplesPerPixel, int depth);

    // Methods
    virtual Spectrum Integrate(const Ray& r, const Scene& scene, int depth) const override;
};

#endif