#ifndef DISTRIBUTION_H
#define DISTRIBUTION_H

#include <geometrics.h>
#include <samplers.h>

template <typename T, typename Evaluator>
class Distribution
{
public:
    virtual T Sample(const Evaluator& samples) const = 0;
    virtual Float Pdf(const T& value) const = 0;
};

template <typename T>
class Distribution2f : public Distribution<T, Iso2f>
{
public:
};

template <typename T>
class FunctionDistribution2f : public Distribution2f<T>
{
public:
    // Constructors
    FunctionDistribution2f(const std::function<T(const Iso2f&)> sample, const std::function<Float(const T&)> pdf) : 
        sample(sample), 
        pdf(pdf)
    { }

    // Methods
    virtual T Sample(const Iso2f& samples) const override
    {
        return sample(samples);
    }

    virtual Float Pdf(const T& value) const override
    {
        return pdf(value);
    }

private:
    // Fields
    const std::function<T(const Iso2f&)> sample;
    const std::function<Float(const T&)> pdf;
};

class UniformSphereDistribution2f : public FunctionDistribution2f<Vector3f>
{
public:
    // Constructors
    UniformSphereDistribution2f();
};

class CosineDistribution2f : public FunctionDistribution2f<Vector3f>
{
public:
    // Constructors
    CosineDistribution2f(const Vector3f& ref);

    // Methods
    virtual Vector3f Sample(const Iso2f& samples) const override;
    virtual Float Pdf(const Vector3f& value) const override;

private:
    // Fields
    const Vector3f ref;
    const Vector3f s;
    const Vector3f t;
};

class CosineNDistribution2f : public FunctionDistribution2f<Vector3f>
{
public:
    // Constructors
    CosineNDistribution2f(int n, const Vector3f& ref);

    // Methods
    virtual Vector3f Sample(const Iso2f& samples) const override;
    virtual Float Pdf(const Vector3f& value) const override;

private:
    // Fields
    const int n;
    const Vector3f ref;
    const Vector3f s;
    const Vector3f t;
};

#endif