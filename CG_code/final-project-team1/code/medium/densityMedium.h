#ifndef DENSITY_MEDIUM_H
#define DENSITY_MEDIUM_H

#include <medium.h>
#include <volume.h>

//* DensityMediums ---------------------------------------------------------------------------------------------*//

/**
 * @brief Heterogeneous density medium 在空间中的分布仅由 μt 决定，且空间中每个位置的 albedo 恒定。
 *        而参数 density 是原始的 .VDB 文件数据，因此 μt = density * toμt。
 *        同时，我们假定 medium 每个位置的 albedo 恒定。
 * @param density 原始的 .vdb 文件数据
 * @param toMut  μt = density * toμt
 * @param albedo  albedo = μs / μt，故 μs = albedo * μt
 */
class DensityGridMedium : public HeteroMedium
{
public:
    // Constructors
    DensityGridMedium(
            const VolumeFloatGrid* density, 
            const Spectrum& toMut, 
            const Spectrum& albedo, 
            const PhaseFunction* phase, 
            const Transform& worldToMedium);

protected:
    // Fields
    const VolumeFloatGrid* density;
    const Spectrum toMut;
    const Spectrum albedo;
    const PhaseFunction* phase;
    const Transform worldToMedium;
};

class DensityKDTreeMedium : public HeteroMedium
{
public:
    // Constructors
    DensityKDTreeMedium(
            const VolumeFloatKDTree* density, 
            const Spectrum& toMut, 
            const Spectrum& albedo, 
            const PhaseFunction* phase, 
            const Transform& worldToMedium);

protected:
    // Fields
    const VolumeFloatKDTree* density;
    const Spectrum toMut;
    const Spectrum albedo;
    const PhaseFunction* phase;
    const Transform worldToMedium;
};



//* DensityGridMediums -----------------------------------------------------------------------------------------*//

/**
 * @brief Parameters:
 *            Float[Point3f] density
 *            Spectrum       toμt
 *            Spectrum       albedo
 *        Fields：
 *            Float          μbar           = density.MaxValue() * toμt.Maximum()
 *            Float          μbar_inv       = 1 / μbar
 *        Sample:
 *            Spectrum       μt             = density[samplePos] * toμt
 *            Spectrum       μs             = albedo * μt
 *            Spectrum       μa             = μt - μs
 *            Spectrum       μn             = Spectrum(μbar) - μt
 *            Float          weighedAbsAvga = (μa * weigh).Absed().Average()
 *            Float          weighedAbsAvgs = (μs * weigh).Absed().Average()
 *            Float          weighedAbsAvgn = (μn * weigh).Absed().Average()
 *            Float          c_inv          = 1 / (weighedAbsAvga + weighedAbsAvgs + weighedAbsAvgn)
 *            Float          pa             = weighedAbsAvga * c_inv
 *            Float          pn             = weighedAbsAvgn * c_inv
 *            Float          1 - pn         = 1 - pn
 */
class DensityGridDeltaMedium : public DensityGridMedium
{
public:
    // Constructors
    DensityGridDeltaMedium(
        const VolumeFloatGrid* density, 
        const Spectrum& toMut, 
        const Spectrum& albedo, 
        const PhaseFunction* phase, 
        const Transform& worldToMedium);
    virtual ~DensityGridDeltaMedium() override;

    // Methods
    virtual Spectrum Sample(const Ray& rayWorld, Sampler& sampler, Spectrum* weigh, MediumInteraction* mi) const override;
    virtual Spectrum Tr(const Ray& rayWorld, Sampler& sampler) const override;

private:
    // Fields
    Float mubar;
    Float mubar_inv;
};

/**
 * @brief Parameters:
 *            Float[Point3f] density
 *            Spectrum       toμt
 *            Spectrum       albedo
 *        Fields：
 *            Float          μbar           = density.MaxValue() * toμt.Maximum()
 *            Float          μbar_inv       = 1 / μbar
 *            Spectrum       μtC            = density.MinValue() * toμt
 *            Spectrum       μsC            = albedo * μtC
 *            Spectrum       μaC            = μtC - μsC
 *            Float          paC            = (μaC * μbar_inv).Average()
 *            Float          psC            = (μsC * μbar_inv).Average()
 *            Float          pResidual      = Spectrum(1) - paC - psC
 *        Sample:
 *            Spectrum       μt             = density[samplePos] * toμt
 *            Spectrum       μs             = albedo * μt
 *            Spectrum       μa             = μt - μs
 *            Spectrum       μn             = Spectrum(μbar) - μt
 *            Spectrum       μaR            = μa - μaC
 *            Spectrum       μsR            = μs - μsC
 *            Float          weighedAbsAvga = (μaR * weigh).Absed().Average()
 *            Float          weighedAbsAvgs = (μsR * weigh).Absed().Average()
 *            Float          weighedAbsAvgn = (μn  * weigh).Absed().Average()
 *            Float          c_inv          = 1 / (weighedAbsAvga + weighedAbsAvgs + weighedAbsAvgn)
 *            Float          pa             = pResidual * weighedAbsAvga * c_inv
 *            Float          ps             = pResidual * weighedAbsAvgs * c_inv
 *            Float          pn             = pResidual * weighedAbsAvgn * c_inv
 */
class DensityGridDecompositionMedium : public DensityGridMedium
{
public:
    // Constructors
    DensityGridDecompositionMedium(
        const VolumeFloatGrid* density, 
        const Spectrum& toMut, 
        const Spectrum& albedo, 
        const PhaseFunction* phase, 
        const Transform& worldToMedium);
    virtual ~DensityGridDecompositionMedium() override;

    // Methods
    virtual Spectrum Sample(const Ray& rayWorld, Sampler& sampler, Spectrum* weigh, MediumInteraction* mi) const override;
    virtual Spectrum Tr(const Ray& rayWorld, Sampler& sampler) const override;

private:
    // Fields
    Float mubar;
    Float mubar_inv;
    Spectrum muaC;
    Spectrum musC;
    Spectrum mutC;
    Float paC;
    Float psC;
    Float pResidual;
    Float mutC_homoC;
    Float mubarR_homoC_inv;
};



//* DensityKDTreeMediums ---------------------------------------------------------------------------------------*//

class DensityKDTreeDeltaMedium : public DensityKDTreeMedium
{
public:
    // Constructors
    DensityKDTreeDeltaMedium(
        const VolumeFloatKDTree* density, 
        const Spectrum& toMut, 
        const Spectrum& albedo, 
        const PhaseFunction* phase, 
        const Transform& worldToMedium);
    virtual ~DensityKDTreeDeltaMedium() override;

    // Methods
    virtual Spectrum Sample(const Ray& rayWorld, Sampler& sampler, Spectrum* weigh, MediumInteraction* mi) const override;
    virtual Spectrum Tr(const Ray& rayWorld, Sampler& sampler) const override;

private:
    // Fields
    Float toMut_maximum;
};

class DensityKDTreeDecompositionMedium : public DensityKDTreeMedium
{
public:
    // Constructors
    DensityKDTreeDecompositionMedium(
        const VolumeFloatKDTree* density, 
        const Spectrum& toMut, 
        const Spectrum& albedo, 
        const PhaseFunction* phase, 
        const Transform& worldToMedium);
    virtual ~DensityKDTreeDecompositionMedium() override;

    // Methods
    virtual Spectrum Sample(const Ray& rayWorld, Sampler& sampler, Spectrum* weigh, MediumInteraction* mi) const override;
    virtual Spectrum Tr(const Ray& rayWorld, Sampler& sampler) const override;

private:
    // Fields
    Float toMut_maximum;
};

#endif