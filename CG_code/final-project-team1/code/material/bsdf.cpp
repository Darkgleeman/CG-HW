#include <bsdf.h>

// Constructor
BSDF::BSDF(const SurfaceInteraction& si) : 
        ng(si.ng), //! 默认 si.ng 为单位向量
        sg(si.sg.Normalized()), 
        tg(Normal3f::Cross(ng, sg)) { }

BSDF::~BSDF()
{
    for (int i = 0; i < nBxDFs; ++i)
        delete bxdfs[i]; // TODO: MemoryArena
}

// Methods
void BSDF::Add(BxDF* bxdf)
{
    Assert(nBxDFs < MaxNBxDFs, 
           "BSDF::Add(): Too many BxDFs");
    bxdfs[nBxDFs++] = bxdf;
}

int BSDF::NumComponents(BxDFType flags/* = BxDFType::All*/) const
{
    int num = 0;
    for (int i = 0; i < nBxDFs; ++i)
        if (bxdfs[i]->MatchesFlags(flags)) ++num;
    return num;
}

Vector3f BSDF::WorldToLocal(const Vector3f& v) const
{
    return Vector3f(Vector3f::Dot(v, sg), Vector3f::Dot(v, tg), Normal3f::Dot(v, ng));
}

Vector3f BSDF::LocalToWorld(const Vector3f& v) const
{
    return Vector3f(
        sg.x * v.x + tg.x * v.y + ng.x * v.z, 
        sg.y * v.x + tg.y * v.y + ng.y * v.z, 
        sg.z * v.x + tg.z * v.y + ng.z * v.z);
}

Spectrum BSDF::F(const Vector3f& woWorld, const Vector3f& wiWorld, BxDFType flags/* = BxDFType::All*/) const
{
    Vector3f wo = WorldToLocal(woWorld), wi = WorldToLocal(wiWorld);
    bool reflect = Normal3f::Dot(wiWorld, ng) * Normal3f::Dot(woWorld, ng) > 0;
    Spectrum f;
    for (int i = 0; i < nBxDFs; ++i)
        if (bxdfs[i]->MatchesFlags(flags) && (
            ( reflect && (int(bxdfs[i]->type) & int(BxDFType::Reflection))) || 
            (!reflect && (int(bxdfs[i]->type) & int(BxDFType::Transmisstion))) ))
            f += bxdfs[i]->F(wo, wi);
    return f;
}

Spectrum BSDF::Sample_F(const Vector3f& woWorld, Vector3f* wiWorld, 
                        const Point2f& sample, Float* pdf, 
                        BxDFType* sampledType/* = nullptr*/, BxDFType type/* = BxDFType::All*/) const
{
    // Choose which BxDF to sample
    int numMatchingComps = NumComponents(type);
    if (numMatchingComps == 0)
    {
        *pdf = 0;
        if (sampledType) *sampledType = BxDFType::Null;
        return Spectrum();
    }
    int comp = std::min((int)std::floor(sample.x * numMatchingComps), numMatchingComps - 1);
    BxDF* bxdf = nullptr;
    int count = comp;
    for (int i = 0; i < nBxDFs; ++i)
    {
        if (bxdfs[i]->MatchesFlags(type) && count-- == 0)
        {
            bxdf = bxdfs[i];
            break;
        }
    }
    // Remap BxDF sample to [0, 1]^2 by recovering sample.x
    Point2f sampleRemapped(sample.x * numMatchingComps - comp, sample.y);
    // Sample chosen BxDF
    Vector3f wo = WorldToLocal(woWorld), wi;
    *pdf = 0;
    if (sampledType) *sampledType = bxdf->type;
    Spectrum f = bxdf->Sample_F(wo, &wi, sampleRemapped, pdf, sampledType);
    if (*pdf == 0) return Spectrum();
    *wiWorld = LocalToWorld(wi);
    // Compute overall PDF with all matching BxDFs
    if (!(int(bxdf->type) & int(BxDFType::Specular)) && numMatchingComps > 1)
        for (int i = 0; i < nBxDFs; ++i)
            if (bxdfs[i] != bxdf && bxdfs[i]->MatchesFlags(type))
                *pdf += bxdfs[i]->Pdf(wo, wi);
    if (numMatchingComps > 1) *pdf /= numMatchingComps;
    // Compute value of BSDF for sampled direction
    if (!(int(bxdf->type) & int(BxDFType::Specular)) && numMatchingComps > 1)
    {
        bool reflect = Normal3f::Dot(woWorld, ng) * Normal3f::Dot(*wiWorld, ng) > 0;
        f = 0.; //! 这里一定需要将 f 置零，因为 bool reflect 只能在 world coordinate 中进行判断，而 bxdf->F/Sample_F 总是在 local coordinate 中计算，这两者并不等价
        for (int i = 0; i < nBxDFs; ++i)
            if (bxdfs[i]->MatchesFlags(type) && (
                ( reflect && (int(bxdfs[i]->type) & int(BxDFType::Reflection))) ||
                (!reflect && (int(bxdfs[i]->type) & int(BxDFType::Transmisstion))) ))
                f += bxdfs[i]->F(wo, wi);
    }
    return f;
}

Spectrum BSDF::Rho(const Vector3f& woWorld, int nSamples, const Point2f* samples, 
                   BxDFType flags/* = BxDFType::All*/) const
{
    Vector3f wo = WorldToLocal(woWorld);
    Spectrum res;
    for (int i = 0; i < nBxDFs; ++i)
        if (bxdfs[i]->MatchesFlags(flags))
            res += bxdfs[i]->Rho(wo, nSamples, samples);
    return res;
}

Spectrum BSDF::Rho(int nSamples, const Point2f* samples1, const Point2f* samples2, 
                   BxDFType flags/* = BxDFType::All*/) const
{
    Spectrum res;
    for (int i = 0; i < nBxDFs; ++i)
        if (bxdfs[i]->MatchesFlags(flags))
            res += bxdfs[i]->Rho(nSamples, samples1, samples2);
    return res;
}

Float BSDF::Pdf(const Vector3f& woWorld, const Vector3f& wiWorld, 
                BxDFType flags/* = BxDFType::All*/) const
{
    if (nBxDFs == 0) return 0;
    Vector3f wo = WorldToLocal(woWorld), wi = WorldToLocal(wiWorld);
    Float pdf = 0;
    int numMatchingComps;
    for (int i = 0; i < nBxDFs; ++i)
        if (bxdfs[i]->MatchesFlags(flags))
        {
            ++numMatchingComps;
            pdf += bxdfs[i]->Pdf(wo, wi);
        }
    return numMatchingComps > 0 ? pdf / numMatchingComps : 0;
}