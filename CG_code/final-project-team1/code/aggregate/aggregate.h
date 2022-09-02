#ifndef AGGREGATE_H
#define AGGREGATE_H

#include <primitive.h>

class Aggregate : public Primitive
{
public:
    virtual bool IsEmbreeAccel() const;
    virtual void Intersect4(const Ray rays[4], Float tHit[4], SurfaceInteraction si[4], bool hits[4]) const;
};

#endif