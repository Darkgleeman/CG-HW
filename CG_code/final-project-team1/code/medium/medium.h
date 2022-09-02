#ifndef MEDIUM_H
#define MEDIUM_H

#include <vector.h>
#include <ray.h>
#include <spectrums.h>
#include <sampler.h>

//* Phase functions --------------------------------------------------------------------------------------------*//

class PhaseFunction
{
public:
    // Constructors
    virtual ~PhaseFunction();

    // Methods
    virtual Float P(const Vector3f& wo, const Vector3f& wi) const = 0;
    virtual Float Sample_P(const Vector3f& wo, Vector3f* wi, const Point2f& samples) const = 0;
};

class HenyeyGreenstein : public PhaseFunction
{
public:
    // Constructors
    HenyeyGreenstein(Float g);

    // Methods
    virtual Float P(const Vector3f& wo, const Vector3f& wi) const override;
    virtual Float Sample_P(const Vector3f& wo, Vector3f* wi, const Point2f& samples) const override;

private:
    // Fields
    const Float g;
};



//* Medium -----------------------------------------------------------------------------------------------------*//

class MediumInteraction;
class Medium
{
public:
    // Constructors
    virtual ~Medium();

    // Methods
    virtual Spectrum Sample(const Ray& rayWorld, Sampler& sampler, Spectrum* weigh, MediumInteraction* mi) const = 0;
    virtual Spectrum Tr(const Ray& rayWorld, Sampler& sampler) const = 0;
};

class HomoMedium : public Medium { };
class HeteroMedium : public Medium { };



//* MediumInterface --------------------------------------------------------------------------------------------*//

class Surface
{
public:
    // Fields
    const Medium* mediumIn;
    const Medium* mediumOu;

    // Constructors
    Surface();
    Surface(const Medium* mediumIn);
    Surface(const Medium* mediumIn, const Medium* mediumOu);

    // Methods
    bool IsMediumTransition() const;
};


#endif