#ifndef BXDF_H
#define BXDF_H

#include <geometrics.h>
#include <spectrums.h>
#include <sampler.h>

// Utility functions
inline Float CosTheta(const Vector3f& w) { return w.z; }
inline Float Cos2Theta(const Vector3f& w) { return CosTheta(w) * CosTheta(w); }
inline Float AbsCosTheta(const Vector3f& w) { return std::abs(CosTheta(w)); }
inline Float Sin2Theta(const Vector3f& w) { return std::max(Float(0), 1 - Cos2Theta(w)); }
inline Float SinTheta(const Vector3f& w) { return std::sqrt(Sin2Theta(w)); }
inline Float TanTheta(const Vector3f& w) { return SinTheta(w) / CosTheta(w); }
inline Float Tan2Theta(const Vector3f& w) { return Sin2Theta(w) / Cos2Theta(w); }
inline Float CosPhi(const Vector3f& w) { Float sinTheta = SinTheta(w); return (sinTheta == 0) ? 1 : std::clamp(w.x / sinTheta, Float(-1), Float(1)); }
inline Float SinPhi(const Vector3f& w) { Float sinTheta = SinTheta(w); return (sinTheta == 0) ? 0 : std::clamp(w.y / sinTheta, Float(-1), Float(1)); }
inline Float Cos2Phi(const Vector3f& w) { Float cosPhi = CosPhi(w); return cosPhi * cosPhi; }
inline Float Sin2Phi(const Vector3f& w) { Float sinPhi = SinPhi(w); return sinPhi * sinPhi; }
inline bool SameHemisphere(const Vector3f& wo, const Vector3f& wi) { return wo.z * wi.z > 0; }

enum class BxDFType : int
{
    Null          = 0, 
    Reflection    = 1 << 0, 
    Transmisstion = 1 << 1, 
    Diffuse       = 1 << 2, 
    Glossy        = 1 << 3, 
    Specular      = 1 << 4, 
    All = Reflection | Transmisstion | Diffuse | Glossy | Specular, 
};

class BxDF
{
public:
    // Fields
    const BxDFType type;

    // Constructor
    BxDF(BxDFType type);

    // Methods
    bool MatchesFlags(BxDFType t) const;

    virtual Spectrum F(const Vector3f& wo, const Vector3f& wi) const = 0;
    virtual Spectrum Sample_F(const Vector3f& wo, Vector3f* wi, 
                              const Point2f& sample, Float* pdf, 
                              BxDFType* sampledType = nullptr) const;
    virtual Spectrum Rho(const Vector3f& wo, int nSamples, const Point2f* samples) const;
    virtual Spectrum Rho(int nSamples, const Point2f* samples1, const Point2f* samples2) const;
    virtual Float Pdf(const Vector3f& wo, const Vector3f& wi) const;
};

class Fresnel
{
public:
    // Static methods
    static Float FrDielectric(Float etaOu, Float etaIn, Float cosThetaFrom);
    // Methods
    virtual Spectrum Evaluate(Float cosThetaFrom) const = 0;
};

class FresnelOne : public Fresnel
{
public:
    // Constructors
    FresnelOne();
    // Methods
    virtual Spectrum Evaluate(Float cosThetaFrom) const override;
};

class FresnelDielectric : public Fresnel
{
public:
    // Fields
    const Float etaOu;
    const Float etaIn;
    // Constructors
    FresnelDielectric(Float etaOu, Float etaIn);
    // Methods
    virtual Spectrum Evaluate(Float cosThetaFrom) const override;
};

class FresnelConductor : public Fresnel
{
public:
    // Fields
    const Spectrum etaOu;
    const Spectrum etaIn;
    const Spectrum k;
    // Constructors
    FresnelConductor(const Spectrum& etaOu, const Spectrum& etaIn, const Spectrum& k);
    // Methods
    virtual Spectrum Evaluate(Float cosThetaFrom) const override;
};

#endif