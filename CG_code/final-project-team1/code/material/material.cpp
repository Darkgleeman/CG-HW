#include <material.h>

// Constructors
MatteMaterial::MatteMaterial(const shared_ptr<TextureSi<Spectrum>>& albedo, 
                             Float roughness) : 
    albedo(albedo), 
    roughness(roughness)
{ }

// Methods
/*override*/ void MatteMaterial::ComputeScatteringFunctions(SurfaceInteraction* si) const
{
    si->bsdf = new BSDF(*si); // TODO: MemoryArena
    Float roughness_clamped = std::clamp(roughness, (Float)0, (Float)90);
    Spectrum albedoEval = albedo->Evaluate(*si);
    if (!albedoEval.IsBlack())
    {
        if (roughness_clamped == 0)
            si->bsdf->Add(new LambertianReflection(albedoEval)); // TODO: MemoryArena
        else
            NotImplemented(); // TODO: OrenNayer : BxDF
    }
}



// Constructors
MirrorMaterial::MirrorMaterial(const shared_ptr<TextureSi<Spectrum>>& albedo, 
                               Float etaOu, Float etaIn) : 
    albedo(albedo), 
    etaOu(etaOu), 
    etaIn(etaIn), 
    fresnel()
{ }

// Methods
/*override*/ void MirrorMaterial::ComputeScatteringFunctions(SurfaceInteraction* si) const
{
    si->bsdf = new BSDF(*si); // TODO: MemoryArena
    Spectrum albedoEval = albedo->Evaluate(*si);
    if (!albedoEval.IsBlack())
        si->bsdf->Add(new SpecularReflection(albedoEval, &fresnel)); // TODO: MemoryArena
}




// Constructors
GlassMaterial::GlassMaterial(const shared_ptr<TextureSi<Spectrum>>& albedoR, const shared_ptr<TextureSi<Spectrum>>& albedoT, 
                             Float etaOu, Float etaIn) : 
    albedoR(albedoR), 
    albedoT(albedoT), 
    etaOu(etaOu), 
    etaIn(etaIn), 
    fresnelDielectric(etaOu, etaIn)
{ }

// Methods
/*override*/ void GlassMaterial::ComputeScatteringFunctions(SurfaceInteraction* si) const
{
    si->bsdf = new BSDF(*si); // TODO: MemoryArena
    Spectrum albedoREval = albedoR->Evaluate(*si);
    Spectrum albedoTEval = albedoT->Evaluate(*si);
    if (!albedoREval.IsBlack() && !albedoTEval.IsBlack()) // Reflection & transmission
        si->bsdf->Add(new FresnelSpecular(albedoREval, albedoTEval, etaOu, etaIn)); // TODO: MemoryArena
    else // Reflection | transmission
    {
        if (!albedoREval.IsBlack())
            si->bsdf->Add(new SpecularReflection(albedoREval, &fresnelDielectric)); // TODO: MemoryArena
        if (!albedoTEval.IsBlack())
            si->bsdf->Add(new SpecularTransmission(albedoTEval, etaOu, etaIn)); // TODO: MemoryArena
    }
}



// Constructors
PlasticMaterial::PlasticMaterial(
        const shared_ptr<TextureSi<Spectrum>>& kd, 
        const shared_ptr<TextureSi<Spectrum>>& ks, 
        Float roughness, 
        bool remapRoughness, 
        Float etaOu, Float etaIn) : 
    kd(kd), 
    ks(ks), 
    roughness(remapRoughness ? BeckmannDistribution::RoughnessToAlpha(roughness) : roughness), 
    remapRoughness(remapRoughness), 
    fresnelDielectric(etaOu, etaIn), 
    distribution(this->roughness, this->roughness, false) // TODO: true
{ }

// Methods
/*override*/ void PlasticMaterial::ComputeScatteringFunctions(SurfaceInteraction* si) const
{
    si->bsdf = new BSDF(*si); // TODO: MemoryArena
    Spectrum kdEval = kd->Evaluate(*si);
    Spectrum ksEval = ks->Evaluate(*si);
    if (!kdEval.IsBlack())
        si->bsdf->Add(new LambertianReflection(kdEval)); // TODO: MemoryArena
    if (!ksEval.IsBlack())
    {
        // TODO: Texture roughness
        si->bsdf->Add(new MicrofacetReflection(ksEval, &distribution, &fresnelDielectric)); // TODO: MemoryArena
    }
}



// Constructors
PhongMaterial::PhongMaterial(
    const shared_ptr<BumpMap>& bumpMap, 
    const shared_ptr<TextureSi<Spectrum>>& albedo, int smoothness) : 
    bumpMap(bumpMap), 
    albedo(albedo), 
    smoothness(smoothness)
{ }

// Methods
/*override*/ void PhongMaterial::ComputeScatteringFunctions(SurfaceInteraction* si) const
{
    //! 至此 si->ng 必已单位化
    Assert(!si->uv.HasNan(), 
           "PhongMaterial::ComputeScatteringFunction(): Si->uv has nan");
    if (bumpMap)
        bumpMap->Bump(si);
    si->bsdf = new BSDF(*si); // TODO: MemoryArena
    Spectrum albedoEval = albedo->Evaluate(*si);
    if (!albedoEval.IsBlack())
        si->bsdf->Add(new PhongReflection(albedoEval, smoothness)); // TODO: MemoryArena
}











// Constructors
TextureMaterial::TextureMaterial(const shared_ptr<TextureSi<Spectrum>>& texture) : 
    texture(texture)
{ }

// Methods
/*override*/ void TextureMaterial::ComputeScatteringFunctions(SurfaceInteraction* si) const
{
    si->bsdf = new BSDF(*si); // TODO: MemoryArena
    Spectrum textureEval = texture->Evaluate(*si);
    if (!textureEval.IsBlack())
        si->bsdf->Add(new LambertianReflection(textureEval)); // TODO: MemoryArena
}