#include <primitiveList.h>

// Constructors
PrimitiveList::PrimitiveList() { }
PrimitiveList::PrimitiveList(const vector<shared_ptr<Primitive>>& primitives) : primitives(primitives) { }

// Methods
void PrimitiveList::PushBack(const shared_ptr<Primitive>& primitive)
{
    primitives.push_back(primitive);
}

/*override*/ bool PrimitiveList::Intersect(const Ray& ray, Float* tHit, SurfaceInteraction* si) const
{
    bool isHit = false;
    for (const shared_ptr<Primitive>& primitive : primitives)
    {
        if (primitive->Intersect(ray, tHit, si))
            isHit = true;
    }
    return isHit;
}
/*override*/ AABound3f PrimitiveList::WorldBound() const
{
    AABound3f bound;
    for (const shared_ptr<Primitive>& primitive : primitives)
        bound = AABound3f::Union(bound, primitive->WorldBound());
    return bound;
}