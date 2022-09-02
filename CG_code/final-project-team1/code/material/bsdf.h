#ifndef BSDF_H
#define BSDF_H

#include <vector.h>
#include <bxdf.h>

class BSDF
{
public:
    // Fields
    /**
     * @brief Geometric normal, 仅由交点形状决定，与扰动 (如微表面模型) 无关
     */
    const Normal3f ng;
    /**
     * @brief Geometric tangent, 仅由交点形状决定，与扰动 (如微表面模型) 无关
     */
    const Vector3f sg, tg;
    int nBxDFs = 0;
    static constexpr int MaxNBxDFs = 8;
    BxDF* bxdfs[MaxNBxDFs];

    // Constructors
    BSDF(const SurfaceInteraction& si); //! 默认 si.ng 为单位向量
    ~BSDF();

    // Methods
    void Add(BxDF* bxdf);
    int NumComponents(BxDFType flags = BxDFType::All) const;
    Vector3f WorldToLocal(const Vector3f& v) const;
    Vector3f LocalToWorld(const Vector3f& v) const;
    
    Spectrum F(const Vector3f& woWorld, const Vector3f& wiWorld, BxDFType flags = BxDFType::All) const;
    Spectrum Sample_F(const Vector3f& woWorld, Vector3f* wiWorld, 
                      const Point2f& sample, Float* pdf, 
                      BxDFType* sampledType = nullptr, BxDFType flags = BxDFType::All) const;
    Spectrum Rho(const Vector3f& woWorld, int nSamples, const Point2f* samples, 
                 BxDFType flags = BxDFType::All) const;
    Spectrum Rho(int nSamples, const Point2f* samples1, const Point2f* samples2, 
                 BxDFType flags = BxDFType::All) const;
    Float Pdf(const Vector3f& woWorld, const Vector3f& wiWorld, 
              BxDFType flags = BxDFType::All) const;
};

#endif