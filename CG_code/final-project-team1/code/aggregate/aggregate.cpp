#include <aggregate.h>

/*virtual*/ bool Aggregate::IsEmbreeAccel() const
{
    return false;
}

/*virtual*/ void Aggregate::Intersect4(const Ray rays[4], Float tHit[4], SurfaceInteraction si[4], bool hits[4]) const
{
    Error("Aggregate::Intersect4(): Current accelerator does not support packet tracking");
    exit(-1);
}