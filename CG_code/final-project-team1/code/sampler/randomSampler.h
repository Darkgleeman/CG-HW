#ifndef RANDOM_SAMPLER_H
#define RANDOM_SAMPLER_H

#include <sampler.h>
#include <random>

class RandomSampler : public Sampler
{
public:
    // Methods
    virtual void UseRealRandomSeed();
    virtual Float Get1D() override;
    virtual Point2f Get2D() override;
    virtual Point3f Get3D() override;

    virtual bool IsRandomSampler() const override;
private:
    // Static fields
    static thread_local std::uniform_real_distribution<Float> distribution;
    static thread_local std::mt19937 generator;
};

#endif