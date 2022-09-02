#ifndef VOLUME_PACKET_SAMPLER_INTEGRATOR_H
#define VOLUME_PACKET_SAMPLER_INTEGRATOR_H

#include <packetSamplerIntegrator.h>

class VolumePacketSamplerIntegrator : public PacketSamplerIntegrator
{
public:
    // Constructors
    VolumePacketSamplerIntegrator(shared_ptr<const Camera>& camera, shared_ptr<Sampler>& sampler, 
                                  int imageWidth, int imageHeigh, int samplesPerPixel, int depth, 
                                  int packetSize);

    // Methods
    virtual void Integrate(const Ray* rs, const Scene& scene, int depth, Spectrum* integration) const override;

private:
    // Methods
    void Integrate4(const Ray rs[4], const Scene& scene, int depth, Spectrum integration[4]) const;
    void Integrate8(const Ray rs[8], const Scene& scene, int depth, Spectrum integration[8]) const;
    void Integrate16(const Ray rs[16], const Scene& scene, int depth, Spectrum integration[16]) const;
};

#endif