#include <infiniteAreaLight.h>

InfiniteAreaLight::InfiniteAreaLight(const shared_ptr<Texture2f<Spectrum>>& le, const shared_ptr<Distribution2f<Vector3f>>& distribution) : 
    le(le), 
    distribution(distribution) //! 最基本的情况：UniformSphereDistribution2f
{ }

// Methods
/*override*/ Spectrum InfiniteAreaLight::Le(const Ray& incidentRayWorld) const
{
    Point3f spherical = Point3f(incidentRayWorld.d.ToSpherical());
    return le->Evaluate(Iso2f(spherical.y, spherical.z));
}

/*override*/ Spectrum InfiniteAreaLight::Sample_Le(
        const Interaction& ref, const Point2f& samples, 
        Interaction* wAtLight, Float* pdf) const
{
    Vector3f dirToLight = distribution->Sample(samples); //! 最基本的情况：*dirToLight = Sampler::GetInUnitSphere(samples);
    wAtLight->p = Point3f(ref.p + dirToLight); //! 对于 InfiniteAreaLight，p 永远不会被用到
    wAtLight->ng = Normal3f(-dirToLight);      //! 因此 ng 需要保存射向 light 的方向
    *pdf = Pdf(ref, dirToLight);
    return Le(Ray(Point3f(ref.p + 0.0075 * dirToLight), dirToLight));
}

/*override*/ Float InfiniteAreaLight::Pdf(const Interaction& ref, const Vector3f& wToLight) const
{
    //! 对于 UniformSphereDistribution2f，为什么 pdf = Pix4Inv 的原因：
    //!     定义 infinite area light: 从原点 (0, 0, 0) 向单位球做 ray cast，设接触点为 (φ, θ)
    //!     则 le = texture(φ, θ)
    //!     因此 pdf(ω) = (r^2 / cosθ) * pdf(A), 由于是单位球，r = 1, cosθ = 1
    //!     故 pdf(ω) = pdf(A) = 1 / 单位球表面积 = Pix4Inv
    return distribution->Pdf(wToLight); //! 最基本的情况：*pdf = Pix4Inv;
}

/*override*/ bool InfiniteAreaLight::IsInfiniteAreaLight() const
{
    return true;
}