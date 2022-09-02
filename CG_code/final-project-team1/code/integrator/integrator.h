#ifndef INTEGRATOR_H
#define INTEGRATOR_H

#include <geometrics.h>
#include <scenes.h>
#include <cameras.h>
#include <materials.h>
#include <lights.h>
#include <primitives.h>

class Integrator
{
public:
    // Methods
    virtual void Render(const Scene& scene) = 0;

protected:
    // Supporting methods
    Point3i RGBFloatsToRGB256(Float* rgbFloats);
};



#endif