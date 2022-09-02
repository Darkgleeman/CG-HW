#include <volumePathSamplerIntegrator.h>

// Supporting methods
inline Float PowerHeuristic(int numf, Float pdf_f, int numg, Float pdf_g)
{
    Float f = numf * pdf_f, g = numg * pdf_g;
    return (f * f) / (f * f + g * g);
}

static Spectrum EstimateDirect(const Interaction& inter, const Light& light, 
                               const Point2f& sampleBSDF, const Point2f& sampleLight, 
                               const Scene& scene, bool specularBounce)
{
    // TODO: Implement MIS
    NotImplemented();
    return Spectrum();
    // BxDFType bsdfFlags = specularBounce ? BxDFType::All : BxDFType(int(BxDFType::All) & ~int(BxDFType::Specular));
    // Spectrum ld;
    // // Sample light with MIS
    // Interaction wi_inter;
    // Float pdf_light = 0, pdf_bsfd = 0;
    // Spectrum le = light.Sample_Le(inter, scene, sampleLight, &wi_inter, &pdf_light);
    // Vector3f wi(wi_inter.p - inter.p);
    // if (pdf_light > 0 && !le.IsBlack())
    // {
    //     // Compute by light and compute fcos
    //     Spectrum fcos_byLight;
    //     if (true) // TODO: inter.IsSurfaceIntersection()
    //     {
    //         const SurfaceInteraction& si = (const SurfaceInteraction&)inter;
    //         Spectrum f = si.bsdf->F(si.woWorld, wi, bsdfFlags);
    //         fcos_byLight = f * std::abs(Normal3f::Dot(si.ng, wi.Normalized())); //! 默认 si.ng 是单位向量
    //         pdf_bsfd = si.bsdf->Pdf(si.woWorld, wi, bsdfFlags);
    //     }
    //     else // TODO: MediumIntersection
    //     {
    //         NotImplemented();
    //     }
    //     // Add light's contribution to ld
    //     if (!fcos_byLight.IsBlack())
    //     {
    //         // TODO: Handle media
    //         if (false) // TODO: light.IsDeltaLight()
    //         {
    //             NotImplemented();
    //         }
    //         else
    //         {
    //             Float weight = PowerHeuristic(1, pdf_light, 1, pdf_bsfd);
    //             ld += fcos_byLight * le * weight / pdf_light;
    //         }
    //     }
    // }

    // // Sample BSDF with MIS
    // if (true) // TODO: !light.IsDeltaLight()
    // {
    //     // Sample by BSDF and compute fcos
    //     Spectrum fcos_byBSDF;
    //     bool sampledSpecular = false;
    //     if (true) // TODO: inter.IsSurfaceIntersection()
    //     {
    //         BxDFType sampledType;
    //         const SurfaceInteraction& si = (const SurfaceInteraction&)inter;
    //         Spectrum f = si.bsdf->Sample_F(si.woWorld, &wi, sampleBSDF, &pdf_bsfd, &sampledType, bsdfFlags);
    //         fcos_byBSDF = f * std::abs(Normal3f::Dot(si.ng, wi.Normalized())); //! 默认 si.ng 是单位向量
    //         sampledSpecular = int(sampledType) & int(BxDFType::Specular);
    //     }
    //     else // TODO: MediumIntersection
    //     {
    //         NotImplemented();
    //     }
    //     // Add BSDF's contribution to ld
    //     if (!fcos_byBSDF.IsBlack() && pdf_bsfd > 0)
    //     {
    //         Float weight = 1;
    //         if (!sampledSpecular)
    //         {
    //             pdf_light = light.Pdf(inter, scene, wi);
    //             if (pdf_light == 0)
    //                 return ld;
    //             weight = PowerHeuristic(1, pdf_bsfd, 1, pdf_light);
    //         }
    //         Ray rayToLight(Point3f(inter.p + 0.0075 * wi), wi);
    //         Float tHit;
    //         SurfaceInteraction inter_light;
    //         bool hasIntersection = scene.Intersect(rayToLight, &tHit, &inter_light); // TODO: MediumIntersection
    //         if (hasIntersection)
    //         {
    //             if (inter_light.primitive->IsGeometricPrimitive() && ((const GeometricPrimitive*)(inter_light.primitive))->areaLight.get() == &light)
    //                 le = inter_light.Le(rayToLight);
    //         }
    //         else
    //             le = light.Le(rayToLight);
    //         if (!le.IsBlack())
    //             ld += fcos_byBSDF * le * weight / pdf_bsfd;
    //     }
    // }
    // return ld;
}

static Spectrum UniformSampleOneLight(const Interaction& inter, const Scene& scene, const shared_ptr<Sampler>& sampler)
{
    int numLights = (int)scene.lights.size();
    if (numLights == 0)
        return Spectrum();
    const std::shared_ptr<Light>& light = scene.lights[std::min(int(sampler->Get1D() * numLights), numLights - 1)];
    Point2f sampleBSDF = sampler->Get2D();
    Point2f sampleLight = sampler->Get2D();
    return Float(numLights) * EstimateDirect(inter, *light, sampleBSDF, sampleLight, scene, false);
}

// Constructors
VolumePathSamplerIntegrator::VolumePathSamplerIntegrator(shared_ptr<const Camera>& camera, shared_ptr<Sampler>& sampler, 
                                                           int imageWidth, int imageHeigh, int samplesPerPixel, int depth) : 
    SamplerIntegrator(camera, sampler, imageWidth, imageHeigh, samplesPerPixel, depth) { }

// Methods
/*override*/ Spectrum VolumePathSamplerIntegrator::Integrate(const Ray& r, const Scene& scene, int depth) const
{
    Spectrum radiance;
    Ray ray(r);
    bool specularBounce = false;
    Spectrum beta(Float(1));
    Spectrum mediumScatteringWeigh(Float(1));
    const Medium* medium = camera->medium;

    int numIterations = 0;
    for (int bounces = 0, rebounces = 0; ; ++bounces)
    {
        ++numIterations;
        if (numIterations / 500 > 0 && numIterations % 500 == 0)
            printf("\n\n(Thread %d): Long loop detected in VolumePathSamplerIntegrator::Integrate(): bounce=%d, rebounce=%d, ray=(%f,%f,%f; %f,%f,%f; %f)\n\n", omp_get_thread_num(), bounces, rebounces, ray.o.x, ray.o.y, ray.o.z, ray.d.x, ray.d.y, ray.d.z, ray.tMax);
        // Intersect ray with scene
        Float tHit;
        SurfaceInteraction si;
        bool hasIntersection = scene.Intersect(ray, &tHit, &si);

        // Medium transition
        if (si.surface.IsMediumTransition())
        {
            Assert(medium == si.surface.mediumIn || medium == si.surface.mediumOu, 
                   "VolumePathSamplerIntegrator::Integrate(): Medium mismatch");
            const Medium* newMedium;
            if (si.byEmbree)
                newMedium = Vector3f::Dot(si.ng, ray.d) > 0 ? si.surface.mediumOu : si.surface.mediumIn;
            else
                newMedium = Vector3f::Dot(si.ng, ray.d) < 0 ? si.surface.mediumOu : si.surface.mediumIn;
            if (medium != newMedium)
                mediumScatteringWeigh = Spectrum(Float(1));
            medium = newMedium;
        }

        // Try sample medium
        MediumInteraction mi;
        if (medium)
        {
            Spectrum albedo = medium->Sample(ray, *sampler, &mediumScatteringWeigh, &mi);
            radiance += beta * mi.emission; // TODO: Implement emission
            beta *= albedo;
        }

        // Early exit if beta is black or trashed
        if (beta.IsBlack() || beta.HasNans())
            break;
        
        // Sample medium success
        if (mi.phase != nullptr) // Illumination by medium
        {
            // Reach max depth
            if (bounces == this->depth)
                break;

            // Volume direction illumination
            const shared_ptr<Light>& light = scene.lights[std::clamp(size_t(sampler->Get1D() * scene.lights.size()), size_t(0), scene.lights.size() - 1)]; // Random sample 1 light
            Interaction wAtLight;
            Float pdf_light = 0;
            Spectrum le = light->Sample_Le(mi, sampler->Get2D(), &wAtLight, &pdf_light);
            mi.woWorld.Normalize();
            Vector3f wToLight = light->IsInfiniteAreaLight() ? Vector3f(-wAtLight.ng) : Vector3f(wAtLight.p - mi.p);
            float tHit_direct;
            SurfaceInteraction si_direct;
            Spectrum tr_direct;
            if ((light->IsInfiniteAreaLight()  ^ scene.IntersectTr(Ray(Point3f(mi.p + 0.0075 * wToLight), wToLight), *sampler, medium, &tHit_direct, &si_direct, &tr_direct)) && // 与场景相交
                (light->IsInfiniteAreaLight() || Point3f::DistanceSquared(wAtLight.p, si_direct.p) < 0.001)) // 确实是该点
            {
                Spectrum p(mi.phase->P(mi.woWorld, Vector3f(wToLight.Normalized()))); // TODO: 这里 woWorld 是否需要单位化？但下面 spawn ray 不能单位化啊
                radiance += tr_direct * beta * le * p * scene.lights.size() / pdf_light;
            }
            // radiance += beta * UniformSampleOneLight();

            // Volume scattering
            Vector3f wi; // TODO: 这里 woWorld 是否需要单位化？
            mi.phase->Sample_P(mi.woWorld, &wi, sampler->Get2D());
            ray = Ray(Point3f(mi.p + 0.0075 * wi), wi);
            specularBounce = false;
        }
        else // Illumination by surface
        {
            if (!hasIntersection) // No intersection
            {
                for (const shared_ptr<Light>& light : scene.lights)
                    if (light->IsInfiniteAreaLight())
                        radiance += beta * light->Le(ray);
                break;
            }
            else // Has intersection
            {
                // Direct emission and specular
                if (bounces == 0 || specularBounce)
                    radiance += beta * si.Le(ray);

                // Reach max depth
                if (bounces == this->depth)
                    break;

                // Compute BSDF
                si.ng.Normalize();
                si.ComputeScatteringFunctions(&si); //! BSDF 的构造函数默认 si.ng 是单位向量
                if (!si.bsdf) // Empty BSDF, as indicators
                {
                    ray = Ray(Point3f(si.p + 0.0075 * ray.d), ray.d);
                    --bounces; // TODO: 可能会死循环
                    ++rebounces;
                    if (rebounces == 50) // TODO: Magic number
                    {
                        // Error("\nWARNING: VolumePathSamplerIntegrator::Integrate(): Terminated a path with too many rebounces");
                        break;
                    }
                    continue;
                }

                // Surface direct illumination (Apply β: L = β0Le1 + β1Le2 + β2Le3 + ...)
                const shared_ptr<Light>& light = scene.lights[std::clamp(size_t(sampler->Get1D() * scene.lights.size()), size_t(0), scene.lights.size() - 1)]; // Random sample 1 light
                Interaction wAtLight;
                Float pdf_light = 0;
                Spectrum le = light->Sample_Le(si, sampler->Get2D(), &wAtLight, &pdf_light);
                Vector3f wToLight = light->IsInfiniteAreaLight() ? Vector3f(-wAtLight.ng) : Vector3f(wAtLight.p - si.p);
                float tHit_direct;
                SurfaceInteraction si_direct;
                Spectrum tr_direct;
                if ((light->IsInfiniteAreaLight()  ^ scene.IntersectTr(Ray(Point3f(si.p + 0.0075 * wToLight), wToLight), *sampler, medium, &tHit_direct, &si_direct, &tr_direct)) && // 与场景相交
                    (light->IsInfiniteAreaLight() || Point3f::DistanceSquared(wAtLight.p, si_direct.p) < 0.001)) // 确实是该点
                {
                    wToLight.Normalize();
                    Spectrum albedoF = si.bsdf->F(si.woWorld, wToLight);
                    radiance += tr_direct * beta * le * albedoF * std::abs(Normal3f::Dot(si.ng, wToLight)) * scene.lights.size() / pdf_light;
                }
                // radiance += beta * UniformSampleOneLight(si, scene, sampler);

                // Surface indirect illumination (Update β)
                Vector3f wi;
                Float pdf_bsdf;
                BxDFType sampledType = BxDFType::Null;
                Spectrum albedoF = si.bsdf->Sample_F(si.woWorld, &wi, sampler->Get2D(), &pdf_bsdf, &sampledType, BxDFType::All);
                wi.Normalize();
                beta *= albedoF * std::abs(Normal3f::Dot(si.ng, wi)) / pdf_bsdf;

                // Update state machine
                ray = Ray(Point3f(si.p + 0.0075 * wi), wi);
                if ((int(sampledType) & (int)BxDFType::Specular) == 0)
                    specularBounce = false;
                else
                    specularBounce = true;

                // Russian roulette
                if (bounces > 3)
                {
                    beta /= 1; // TODO: Russian roulette: 涉及 XYZ color，非常难搞，先放一放
                }

                // Clear
                delete si.bsdf; // TODO: MemoryArena

                //* debug ------------------------ //
                // radiance = Spectrum();
                // si.ng.Normalize();
                // radiance[0] = si.ng.x;
                // radiance[1] = si.ng.y;
                // radiance[2] = si.ng.z;
                // return radiance;
                //* debug ------------------------ //
            }
        }
    }

    // Return
    return radiance;
}