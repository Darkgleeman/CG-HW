#ifndef PACKET_SAMPLER_INTEGRATOR_H
#define PACKET_SAMPLER_INTEGRATOR_H

#include <integrator.h>
#include <samplers.h>
#include <spectrums.h>

class PacketSamplerIntegrator : public Integrator
{
public:
    // Fields
    const shared_ptr<const Camera> camera;
    const shared_ptr<Sampler> sampler;
    const int imageWidth;
    const int imageHeigh;
    const int samplesPerPixel;
    const int depth;
    const int packetSize;

    // Constructors
    PacketSamplerIntegrator(shared_ptr<const Camera>& camera, shared_ptr<Sampler>& sampler, 
                            int imageWidth, int imageHeigh, int samplesPerPixel, int depth, 
                            int packetSize);

    // Methods
    virtual void Render(const Scene& scene) override;

    //! 继承于 PacketSamplerIntegrator 的类都需要定义 Integrate()，即定义积分规则
    virtual void Integrate(const Ray* rs, const Scene& scene, int depth, Spectrum* integration) const = 0;
};

#endif