#include <volumePacketSamplerIntegrator.h>

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
VolumePacketSamplerIntegrator::VolumePacketSamplerIntegrator(
        shared_ptr<const Camera>& camera, shared_ptr<Sampler>& sampler, 
        int imageWidth, int imageHeigh, int samplesPerPixel, int depth, 
        int packetSize) : 
    PacketSamplerIntegrator(camera, sampler, imageWidth, imageHeigh, samplesPerPixel, depth, packetSize)
{ }

/*override*/ void VolumePacketSamplerIntegrator::Integrate(const Ray* rs, const Scene& scene, int depth, Spectrum* integration) const
{
    if (packetSize == 4)
        Integrate4(rs, scene, 0, integration);
    else if (packetSize == 8)
        Integrate8(rs, scene, 0, integration);
    else if (packetSize == 16)
        Integrate16(rs, scene, 0, integration);
}

// Methods
void VolumePacketSamplerIntegrator::Integrate4(const Ray rs[4], const Scene& scene, int depth, Spectrum integration[4]) const
{
    // TODO: Implement this
    NotImplemented();
    return;

    integration[0] = integration[1] = integration[2] = integration[3] = Spectrum();
    Ray rays[4] = { rs[0], rs[1], rs[2], rs[3] };
    bool specularBounces[4] = { false, false, false, false };
    Spectrum betas[4] = { Spectrum(Float(1)),  Spectrum(Float(1)), Spectrum(Float(1)), Spectrum(Float(1)) };
    Spectrum mediumScatteringWeighs[4] = { Spectrum(Float(1)), Spectrum(Float(1)), Spectrum(Float(1)), Spectrum(Float(1)) };
    const Medium* mediums[4] = { camera->medium, camera->medium, camera->medium, camera->medium };
    bool hasFinisheds[4] = { false, false, false, false };
    int bounces[4] = { 0, 0, 0, 0 };

    while (true)
    {
        // Check return
        bool shouldReturn = true;
        for (int p = 0; p < 4; ++p)
            if (!hasFinisheds[p])
            {
                shouldReturn = false;
                break;
            }
        if (shouldReturn)
            return;

        // Intersect ray with scene
        Float tHits[4];
        SurfaceInteraction sis[4];
        bool hasIntersections[4];
        scene.Intersect4(rays, tHits, sis, hasIntersections); // TODO: HasFinishes 可以用来作为 packet casting 的参数，但这时候应该换成对偶的另一形式

        // Medium transition
        for (int p = 0; p < 4; ++p)
        {
            // To next packet member if has finished
            if (hasFinisheds[p])
                continue;

            // Medium transition
            if (sis[p].surface.IsMediumTransition())
            {
                Assert(mediums[p] == sis[p].surface.mediumIn || mediums[p] == sis[p].surface.mediumOu, 
                    "VolumePacketSamplerIntegrator::Integrate(): Medium mismatch");
                const Medium* newMedium;
                if (sis[p].byEmbree)
                    newMedium = Vector3f::Dot(sis[p].ng, rays[p].d) > 0 ? sis[p].surface.mediumOu : sis[p].surface.mediumIn;
                else
                    newMedium = Vector3f::Dot(sis[p].ng, rays[p].d) < 0 ? sis[p].surface.mediumOu : sis[p].surface.mediumIn;
                if (mediums[p] != newMedium)
                    mediumScatteringWeighs[p] = Spectrum(Float(1));
                mediums[p] = newMedium;
            }

            // Try sample medium
            MediumInteraction mi;
            if (mediums[p])
            {
                Spectrum albedo = mediums[p]->Sample(rays[p], *sampler, &mediumScatteringWeighs[p], &mi);
                integration[p] += betas[p] * mi.emission; // TODO: Implement emission
                betas[p] *= albedo;
            }

            // Early exit if beta is black or trashed
            if (betas[p].IsBlack() || betas[p].HasNans())
            {
                hasFinisheds[p] = true;
                continue;
            }
            
            // Sample medium success
            if (mi.phase != nullptr) // Illumination by medium
            {
                // Reach max depth
                if (bounces[p] == this->depth)
                {
                    hasFinisheds[p] = true;
                    continue;
                }

                // Volume direction illumination
                const shared_ptr<Light>& light = scene.lights[std::clamp(size_t(sampler->Get1D() * scene.lights.size()), size_t(0), scene.lights.size() - 1)]; // Random sample 1 light
                Interaction wAtLight;
                Float pdf_light = 0;
                Spectrum le = light->Sample_Le(mi, sampler->Get2D(), &wAtLight, &pdf_light);
                mi.woWorld.Normalize();
                Vector3f wToLight(wAtLight.p - mi.p);
                float tHit_direct;
                SurfaceInteraction si_direct;
                Spectrum tr_direct;
                if (scene.IntersectTr(Ray(Point3f(mi.p + 0.0075 * wToLight), wToLight), *sampler, mediums[p], &tHit_direct, &si_direct, &tr_direct) && // 与场景相交
                    Point3f::DistanceSquared(wAtLight.p, si_direct.p) < 0.001) // 确实是该点
                {
                    Spectrum phase(mi.phase->P(mi.woWorld, Vector3f(wToLight.Normalized()))); // TODO: 这里 woWorld 是否需要单位化？但下面 spawn ray 不能单位化啊
                    integration[p] += tr_direct * betas[p] * le * phase * scene.lights.size() / pdf_light;
                }
                // radiance += beta * UniformSampleOneLight();

                // Volume scattering
                Vector3f wi; // TODO: 这里 woWorld 是否需要单位化？
                mi.phase->Sample_P(mi.woWorld, &wi, sampler->Get2D());
                rays[p] = Ray(Point3f(mi.p + 0.0075 * wi), wi);
                specularBounces[p] = false;
            }
            else // Illumination by surface
            {
                if (!hasIntersections[p]) // No intersection
                {
                    for (const shared_ptr<Light>& light : scene.lights)
                        if (light->IsInfiniteAreaLight())
                            integration[p] += betas[p] * light->Le(rays[p]);
                    hasFinisheds[p] = true;
                    continue;
                }
                else // Has intersection
                {
                    // Direct emission and specular
                    if (bounces[p] == 0 || specularBounces[p])
                        integration[p] += betas[p] * sis[p].Le(rays[p]);

                    // Reach max depth
                    if (bounces[p] == this->depth)
                    {
                        hasFinisheds[p] = true;
                        continue;
                    }

                    // Compute BSDF
                    sis[p].ng.Normalize();
                    sis[p].ComputeScatteringFunctions(&sis[p]); //! BSDF 的构造函数默认 si.ng 是单位向量
                    if (!sis[p].bsdf) // Empty BSDF, as indicators
                    {
                        rays[p] = Ray(Point3f(sis[p].p + 0.0075 * rays[p].d), rays[p].d);
                        --bounces[p]; // TODO: 可能会死循环
                        continue;
                    }

                    // Surface direct illumination (Apply β: L = β0Le1 + β1Le2 + β2Le3 + ...)
                    const shared_ptr<Light>& light = scene.lights[std::clamp(size_t(sampler->Get1D() * scene.lights.size()), size_t(0), scene.lights.size() - 1)]; // Random sample 1 light
                    Interaction wAtLight;
                    Float pdf_light = 0;
                    Spectrum le = light->Sample_Le(sis[p], sampler->Get2D(), &wAtLight, &pdf_light);
                    Vector3f wToLight(wAtLight.p - sis[p].p);
                    float tHit_direct;
                    SurfaceInteraction si_direct;
                    Spectrum tr_direct;
                    if (scene.IntersectTr(Ray(Point3f(sis[p].p + 0.0075 * wToLight), wToLight), *sampler, mediums[p], &tHit_direct, &si_direct, &tr_direct) && // 与场景相交
                        Point3f::DistanceSquared(wAtLight.p, si_direct.p) < 0.001) // 确实是该点
                    {
                        wToLight.Normalize();
                        Spectrum albedoF = sis[p].bsdf->F(sis[p].woWorld, wToLight);
                        integration[p] += tr_direct * betas[p] * le * albedoF * std::abs(Normal3f::Dot(sis[p].ng, wToLight)) * scene.lights.size() / pdf_light;
                    }
                    // radiance += beta * UniformSampleOneLight(si, scene, sampler);

                    // Surface indirect illumination (Update β)
                    Vector3f wi;
                    Float pdf_bsdf;
                    BxDFType sampledType = BxDFType::Null;
                    Spectrum albedoF = sis[p].bsdf->Sample_F(sis[p].woWorld, &wi, sampler->Get2D(), &pdf_bsdf, &sampledType, BxDFType::All);
                    wi.Normalize();
                    betas[p] *= albedoF * std::abs(Normal3f::Dot(sis[p].ng, wi)) / pdf_bsdf;

                    // Update state machine
                    rays[p] = Ray(Point3f(sis[p].p + 0.0075 * wi), wi);
                    if ((int(sampledType) & (int)BxDFType::Specular) == 0)
                        specularBounces[p] = false;
                    else
                        specularBounces[p] = true;

                    // Russian roulette
                    if (bounces[p] > 3)
                    {
                        betas[p] /= 1; // TODO: Russian roulette: 涉及 XYZ color，非常难搞，先放一放
                    }

                    // Clear
                    delete sis[p].bsdf; // TODO: MemoryArena
                }
            }
            ++bounces[p];
        }
    }
}

void VolumePacketSamplerIntegrator::Integrate8(const Ray rs[8], const Scene& scene, int depth, Spectrum integration[8]) const
{
    // TODO: Implement VolumePacketSamplerIntegrator::Integrate8()
    NotImplemented();
}

void VolumePacketSamplerIntegrator::Integrate16(const Ray rs[16], const Scene& scene, int depth, Spectrum integration[16]) const
{
    // TODO: Implement VolumePacketSamplerIntegrator::Integrate16()
    NotImplemented();
}
