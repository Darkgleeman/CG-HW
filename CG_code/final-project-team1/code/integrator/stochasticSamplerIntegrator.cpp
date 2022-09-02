#include <stochasticSamplerIntegrator.h>

// Constructors
StochasticSamplerIntegrator::StochasticSamplerIntegrator(shared_ptr<const Camera>& camera, shared_ptr<Sampler>& sampler, 
                                                         int imageWidth, int imageHeigh, int samplesPerPixel, int depth) : 
    SamplerIntegrator(camera, sampler, imageWidth, imageHeigh, samplesPerPixel, depth) { }

// Methods
/*override*/ Spectrum StochasticSamplerIntegrator::Integrate(const Ray& r, const Scene& scene, int depth) const
{
    if (depth > this->depth)
        return Spectrum();
    Float tHit;
    SurfaceInteraction si;
    if (!scene.Intersect(r, &tHit, &si))
        return Spectrum();
        // TODO: return background(r.d);
    else 
    {
        Spectrum radiance;
        if (true) // TODO: Russian roulette
        {
            si.ng.Normalize();
            si.ComputeScatteringFunctions(&si); //! BSDF 的构造函数默认 si.ng 是单位向量
            Vector3f wi;
            Float pdf;
            Spectrum AlbedoF = si.bsdf->Sample_F(si.woWorld, &wi, sampler->Get2D(), &pdf, nullptr, BxDFType::All);
            wi.Normalize();
            radiance = Integrate(Ray(Point3f(si.p + 0.0075 * wi), wi), scene, depth + 1) * AlbedoF * std::abs(Normal3f::Dot(si.ng, wi)) / pdf;
            radiance /= 1; // TODO: Russian roulette
        }
        radiance += si.Le(r); // Light emission
        // Delete
        delete si.bsdf; // TODO: MemoryArena

        //* debug ------------------------ //
        // radiance = Spectrum();
        // si.ng.Normalize();
        // radiance[0] = si.ng.x;
        // radiance[1] = si.ng.y;
        // radiance[2] = si.ng.z;
        //* debug ------------------------ //

        // Return
        return radiance;
    }
}