#ifndef PRIMITIVE_LIST_H
#define PRIMITIVE_LIST_H

#include <aggregate.h>
#include <geometricPrimitive.h>

class PrimitiveList : public Aggregate
{
public:
    // Constructors
    PrimitiveList();
    PrimitiveList(const vector<shared_ptr<Primitive>>& primitives);

    // Methods
    void PushBack(const shared_ptr<Primitive>& primitive);

    virtual bool Intersect(const Ray& ray, Float* tHit, SurfaceInteraction* si) const override;
    virtual AABound3f WorldBound() const override;

// private:
    // Fields
    vector<shared_ptr<Primitive>> primitives;
};

#endif