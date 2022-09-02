#ifndef MATERIAL_H
#define MATERIAL_H

#include <interaction.h>
#include <bxdfs.h>
#include <bsdf.h>
#include <spectrums.h>
#include <textures.h>

class Material
{
public:
    virtual void ComputeScatteringFunctions(SurfaceInteraction* si) const = 0;
};

class MatteMaterial : public Material
{
public:
    // Fields
    const shared_ptr<TextureSi<Spectrum>> albedo;
    const Float roughness;
    
    // Constructors
    MatteMaterial(const shared_ptr<TextureSi<Spectrum>>& albedo, 
                  Float roughness);

    // Methods
    virtual void ComputeScatteringFunctions(SurfaceInteraction* si) const override;
};

class MirrorMaterial : public Material
{
public:
    // Fields
    const shared_ptr<TextureSi<Spectrum>> albedo;
    const Float etaOu;
    const Float etaIn;

    // Constructors
    MirrorMaterial(const shared_ptr<TextureSi<Spectrum>>& albedo, 
                   Float etaOu, Float etaIn);

    // Methods
    virtual void ComputeScatteringFunctions(SurfaceInteraction* si) const override;
private:
    // Fields
    FresnelOne fresnel;
};

class GlassMaterial : public Material
{
public:
    // Fields
    const shared_ptr<TextureSi<Spectrum>> albedoR;
    const shared_ptr<TextureSi<Spectrum>> albedoT;
    const Float etaOu;
    const Float etaIn;

    // Constructors
    GlassMaterial(const shared_ptr<TextureSi<Spectrum>>& albedoR, const shared_ptr<TextureSi<Spectrum>>& albedoT, 
                  Float etaOu, Float etaIn);

    // Methods
    virtual void ComputeScatteringFunctions(SurfaceInteraction* si) const override;
private:
    // Fields
    FresnelDielectric fresnelDielectric;
};

class PlasticMaterial : public Material
{
public:
    // Fields
    const shared_ptr<TextureSi<Spectrum>> kd, ks;
    const Float roughness; // TODO: 实现 memory arena 之后改为 texture
    const bool remapRoughness;

    // Constructors
    PlasticMaterial(const shared_ptr<TextureSi<Spectrum>>& kd, 
                    const shared_ptr<TextureSi<Spectrum>>& ks, 
                    Float roughness, 
                    bool remapRoughness, 
                    Float etaOu, Float etaIn);

    // Methods
    virtual void ComputeScatteringFunctions(SurfaceInteraction* si) const override;
private:
    // Fields
    FresnelDielectric fresnelDielectric;
    BeckmannDistribution distribution;
};

class PhongMaterial : public Material
{
public:
    // Fields
    const shared_ptr<BumpMap> bumpMap;
    const shared_ptr<TextureSi<Spectrum>> albedo;
    const int smoothness;

    // Constructors
    PhongMaterial(const shared_ptr<BumpMap>& bumpMap, 
                  const shared_ptr<TextureSi<Spectrum>>& albedo, 
                  int smoothness);

    // Methods
    virtual void ComputeScatteringFunctions(SurfaceInteraction* si) const override;
};

class TextureMaterial : public Material
{
public:
    // Fields
    const shared_ptr<TextureSi<Spectrum>> texture;

    // Constructors
    TextureMaterial(const shared_ptr<TextureSi<Spectrum>>& texture);

    // Methods
    virtual void ComputeScatteringFunctions(SurfaceInteraction* si) const override;
};

#endif