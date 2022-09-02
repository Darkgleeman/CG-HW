#include <light.h>

/*virtual*/ bool Light::IsInfiniteAreaLight() const
{
    return false;
}

AreaLight::AreaLight(const Spectrum& le, const shared_ptr<Shape>& shape) : le(le), shape(shape) { }