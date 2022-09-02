#ifndef BVH_H
#define BVH_H

#include <aggregate.h>
#include <geometricPrimitive.h>

class BVH : public Aggregate
{
public:
    // Constants
    static constexpr int ExpectedNumPrimsPerNode = 5;

    // Constructors
    BVH(const vector<shared_ptr<Primitive>>& primitives);
    BVH(const BVH& that) = delete;
    virtual ~BVH() override;

    // Methods
    virtual bool Intersect(const Ray& ray, Float* tHit, SurfaceInteraction* si) const override;
    virtual AABound3f WorldBound() const override;

private:
    // Fields
    vector<shared_ptr<Primitive>> primitives;
    //! Tree format: 32 * 7 = 224-bit nodes
    //! Bound:
    //!     pMin/pMax:       [32, 223]
    //! Interior:
    //!     secondChildIdx:  [ 8,  31]
    //! Leaf:
    //!     primitivesBegin: [ 8,  31]
    //!     numPrimitivs:    [ 0,   8]  0 -> interior node
    void* nodes;
    int numNodes;
};

#endif