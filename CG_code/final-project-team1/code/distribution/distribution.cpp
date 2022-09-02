#include <distribution.h>

// Constructor
UniformSphereDistribution2f::UniformSphereDistribution2f() : 
    FunctionDistribution2f(
        std::function([](const Iso2f& samples) { return Sampler::GetInUnitSphere(Point2f(samples)); }), 
        std::function([](const Vector3f& value) { return Pix4Inv; }))
{ }





// Constructor
CosineDistribution2f::CosineDistribution2f(const Vector3f& ref) : 
    FunctionDistribution2f(
        std::function([](const Iso2f& samples)
        {
            Float phi = 2 * Pi * samples.x;
            Float sinTheta = std::sqrt(samples.y);
            Float x = std::cos(phi) * sinTheta;
            Float y = std::sin(phi) * sinTheta;
            Float z = std::sqrt(1 - samples.y);
            return Vector3f(x, y, z);
        }), 
        std::function([](const Vector3f& value)
        {
            return (value.z <= 0) ? 0 : value.z / value.Norm() * PiInv;
        })), 
    ref(ref.Normalized()), 
    s(Vector3f::Cross(this->ref, (std::abs(this->ref.x) > 0.9 ? Vector3f(0, 1, 0) : Vector3f(1, 0, 0))).Normalized()), 
    t(Vector3f::Cross(this->ref, s))
{ }

// Methods
/*override*/ Vector3f CosineDistribution2f::Sample(const Iso2f& samples) const
{
    Vector3f sample_sampleCoord(FunctionDistribution2f::Sample(samples));
    return Vector3f(sample_sampleCoord.x * s + sample_sampleCoord.y * t + sample_sampleCoord.z * ref);
}

/*override*/ Float CosineDistribution2f::Pdf(const Vector3f& value) const
{
    Vector3f value_sampleCoord(
        value.x * s.x + value.y * t.x + value.z * ref.x, 
        value.x * s.y + value.y * t.y + value.z * ref.y, 
        value.x * s.z + value.y * t.z + value.z * ref.z);
    return FunctionDistribution2f::Pdf(value_sampleCoord);
}








// Constructor
CosineNDistribution2f::CosineNDistribution2f(int n, const Vector3f& ref) : 
    FunctionDistribution2f(
        std::function([n](const Iso2f& samples)
        {
            Float phi = 2 * Pi * samples.x;
            Float cosTheta = std::pow(samples.y, Float(1) / (n + 1));
            Float sinTheta = std::sqrt(1 - cosTheta * cosTheta);
            Float x = std::cos(phi) * sinTheta;
            Float y = std::sin(phi) * sinTheta;
            Float z = std::sqrt(1 - samples.y);
            return Vector3f(x, y, z);
        }), 
        std::function([n](const Vector3f& value)
        {
            return (value.z <= 0) ? 0 : std::pow(value.z / value.Norm(), n) * (n + 1) * 0.5 * PiInv ;
        })), 
    n(n), 
    ref(ref.Normalized()), 
    s(Vector3f::Cross(this->ref, (std::abs(this->ref.x) > 0.9 ? Vector3f(0, 1, 0) : Vector3f(1, 0, 0))).Normalized()), 
    t(Vector3f::Cross(this->ref, s))
{ }

// Methods
/*override*/ Vector3f CosineNDistribution2f::Sample(const Iso2f& samples) const
{
    Vector3f sample_sampleCoord(FunctionDistribution2f::Sample(samples));
    // Error(sample_sampleCoord);
    return Vector3f(sample_sampleCoord.x * s + sample_sampleCoord.y * t + sample_sampleCoord.z * ref);
}

/*override*/ Float CosineNDistribution2f::Pdf(const Vector3f& value) const
{
    Vector3f value_sampleCoord(
        value.x * s.x + value.y * t.x + value.z * ref.x, 
        value.x * s.y + value.y * t.y + value.z * ref.y, 
        value.x * s.z + value.y * t.z + value.z * ref.z);
    return FunctionDistribution2f::Pdf(value_sampleCoord);
}