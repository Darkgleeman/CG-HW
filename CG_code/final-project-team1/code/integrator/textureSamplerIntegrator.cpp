#include <textureSamplerIntegrator.h>

// Constructors
TextureSamplerIntegrator::TextureSamplerIntegrator(shared_ptr<const Camera>& camera, shared_ptr<Sampler>& sampler, 
                                               int imageWidth, int imageHeigh, int samplesPerPixel) : 
    SamplerIntegrator(camera, sampler, imageWidth, imageHeigh, samplesPerPixel, 1) { }

// Methods
/*override*/ Spectrum TextureSamplerIntegrator::Integrate(const Ray& r, const Scene& scene, int depth) const
{
    Float tHit;
    SurfaceInteraction si;
    if (!scene.Intersect(r, &tHit, &si))
        return Spectrum();
    else 
    {
        // Compute differential rays
        ComputeDifferentialRays(r, &si);
        // Compute BSDF
        si.ng.Normalize();
        si.ComputeScatteringFunctions(&si); //! BSDF 的构造函数默认 si.ng 是单位向量
        Spectrum radiance = si.bsdf->F(si.woWorld, si.woWorld);
        // Delete
        delete si.bsdf; // TODO: MemoryArena
        // Return
        return radiance;
    }
}
