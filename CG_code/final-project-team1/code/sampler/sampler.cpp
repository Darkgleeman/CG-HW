#include <sampler.h>

// Methods
Float Sampler::Get1D(Float min, Float max)
{
    return min + (max - min) * Get1D();
}

#include <random> // TODO: class Sampler methods: 必须实现一次必中
static std::uniform_real_distribution<Float> distribution_Neg1To1(-1.0, 1.0);
static std::mt19937 generator;
// Static methods
/*static*/ Vector2f Sampler::GetInUnitDisk(const Point2f& samples) // TODO: class Sampler methods: 必须实现一次必中
{
    while (true)
    {
        Point2f p(distribution_Neg1To1(generator), distribution_Neg1To1(generator));
        if (p.NormSquared() <= 1)
            return Vector2f(p.x, p.y);
    }
}
/*static*/ Vector3f Sampler::GetInUnitSphere(const Point2f& samples) // TODO: class Sampler methods: 必须实现一次必中
{
    while (true)
    {
        Point3f p(distribution_Neg1To1(generator), distribution_Neg1To1(generator), distribution_Neg1To1(generator));
        if (p.NormSquared() <= 1)
            return Vector3f(p.x, p.y, p.z);
    }
}
/*static*/ Vector3f Sampler::GetInUnitHemisphere(const Point2f& samples) // TODO: class Sampler methods: 必须实现一次必中
{
    Vector3f inUnitSphere = GetInUnitSphere(samples);
    inUnitSphere.z = std::abs(inUnitSphere.z);
    return inUnitSphere;
}

/*static*/ Vector3f Sampler::GetOnUnitHemisphere(const Point2f& samples) // TODO: class Sampler methods: 必须实现一次必中
{
    return Vector3f(GetInUnitHemisphere(samples).Normalized());
}