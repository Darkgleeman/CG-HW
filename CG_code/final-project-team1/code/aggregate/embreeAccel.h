#ifndef EMBREE_ACCEL_H
#define EMBREE_ACCEL_H

#include <aggregate.h>
#include <geometricPrimitive.h>

#ifdef INTEL
class EmbreeAccel : public Aggregate
{
public:
    // Constructors
    EmbreeAccel(const vector<shared_ptr<Primitive>>& primitives);
    ~EmbreeAccel();

    // Methods
    virtual bool Intersect(const Ray& ray, Float* tHit, SurfaceInteraction* si) const override;
    virtual void Intersect4(const Ray rays[4], Float tHit[4], SurfaceInteraction si[4], bool hits[4]) const override;
    virtual AABound3f WorldBound() const override;

    virtual bool IsEmbreeAccel() const override;

private:
    // Fields
    RTCDevice device;
    RTCScene scene;
    vector<shared_ptr<Primitive>> primitives;
};
#endif

#endif