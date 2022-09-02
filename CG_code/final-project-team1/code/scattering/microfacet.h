#ifndef MICROFACET_H
#define MICROFACET_H

#include <bxdf.h>

class MicrofacetDistribution //! 默认参数为单位向量
{
public:
    // Fields
    const bool sampleVisibleArea;
    // Constructors
    MicrofacetDistribution(bool sampleVisibleArea = true);
    // Methods
    virtual Float D(const Vector3f& wh) const = 0;
    virtual Float Lambda(const Vector3f& w) const = 0;
    Float G1(const Vector3f& w) const; //! 通常 G1 与 wh 无关，因此只需要参数 w
    virtual Float G(const Vector3f& wo, const Vector3f& wi) const; //! 定义为 virtual，因为默认的 G 是个简化计算模型

    virtual Vector3f Sample_Wh(const Vector3f& wo, const Point2f& samples) const = 0;
    Float Pdf(const Vector3f& wo, const Vector3f& wh) const;
};

class BeckmannDistribution : public MicrofacetDistribution
{
public:
    // Fields
    const Float alphaX, alphaY;
    // Constructors
    BeckmannDistribution(Float alphaX, Float alphaY, bool sampleVisibleArea = true);
    // Methods
    virtual Float D(const Vector3f& wh) const override;
    virtual Float Lambda(const Vector3f& w) const override;

    virtual Vector3f Sample_Wh(const Vector3f& wo, const Point2f& samples) const override;
    // Static methods
    static Float RoughnessToAlpha(Float roughness);
};

#endif