#ifndef STOCHASTIC_SAMPLER_INTEGRATOR_H
#define STOCHASTIC_SAMPLER_INTEGRATOR_H

#include <samplerIntegrator.h>

class StochasticSamplerIntegrator : public SamplerIntegrator
{
public:
    // Constructors
    StochasticSamplerIntegrator(shared_ptr<const Camera>& camera, shared_ptr<Sampler>& sampler, 
                                int imageWidth, int imageHeigh, int samplesPerPixel, int depth);

    // Methods
    virtual Spectrum Integrate(const Ray& r, const Scene& scene, int depth) const override;
private:
};

#endif