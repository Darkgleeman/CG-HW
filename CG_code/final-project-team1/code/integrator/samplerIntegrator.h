#ifndef SAMPLER_INTEGRATOR_H
#define SAMPLER_INTEGRATOR_H

#include <filesystem>
#include <integrator.h>
#include <samplers.h>
#include <spectrums.h>

class SamplerIntegrator : public Integrator
{
public:
    // Fields
    const shared_ptr<const Camera> camera;
    vector<Point2f> pixelSamples_LU;
    vector<Point2f> pixelSamples_RU;
    vector<Point2f> pixelSamples_LD;
    vector<Point2f> pixelSamples_RD;
    const shared_ptr<Sampler> sampler;
    const int imageWidth;
    const int imageHeigh;
    const int samplesPerPixel;
    const int depth;

    // Constructors
    SamplerIntegrator(shared_ptr<const Camera>& camera, shared_ptr<Sampler>& sampler, 
                      int imageWidth, int imageHeigh, int samplesPerPixel, int depth);

    // Methods
    virtual void Render(const Scene& scene) override;

    //! 继承于 SamplerIntegrator 的类都需要定义 Integrate()，即定义积分规则
    virtual Spectrum Integrate(const Ray& r, const Scene& scene, int depth) const = 0;

protected:
    // Supporting methods
    void ComputeDifferentialRays(const Ray& ray, SurfaceInteraction* si) const;
    pair<Ray, Ray> GenerateRayAtNextPixel(Ray ray) const;
    Ray GenerateRayAtNextPixelX(Ray ray) const;
    Ray GenerateRayAtNextPixelY(Ray ray) const;

private:
    // Supporting fields
    const Vector3f pixelPosOffset_X;
    const Vector3f pixelPosOffset_Y;
};

#endif