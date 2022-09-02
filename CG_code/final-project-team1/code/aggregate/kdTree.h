#ifndef KD_TREE_H
#define KD_TREE_H

#include <aggregate.h>
#include <geometricPrimitive.h>
#include <primitiveList.h>

class KDTree : public Aggregate
{
public:
    // Constants
    static constexpr int MaxNumPrimsPerNode = 64; //! 2 ** 6 = 64
    static constexpr int ExpectedNumPrimsPerNode = 5;
    static constexpr int MaxDepth = 20;

    // Constructors
    KDTree(const vector<shared_ptr<Primitive>>& primitives);
    KDTree(const KDTree& that) = delete;
    virtual ~KDTree() override;

    // Methods
    virtual bool Intersect(const Ray& ray, Float* tHit, SurfaceInteraction* si) const override;
    virtual AABound3f WorldBound() const override;
    inline float SplitPos(int i) const
    {
        return *((float*)tree + i);
    }
    inline int SplitAxis(int i) const
    {
        return ((int)(*((uint32_t*)tree + i)) & 0b11);
    }
    inline int PrimitivesBegin(int i) const
    {
        return ((int)(*((uint32_t*)tree + i)) >> 8);
    }
    inline int NumPrimitives(int i) const
    {
        return (((int)(*((uint32_t*)tree + i)) & 255) >> 2);
    }

    // Operators
    friend std::ostream& operator<<(std::ostream& out, const KDTree& t);

private:
    // Fields
    vector<shared_ptr<Primitive>> primitives;
    //! Tree format: 32-bit nodes
    //! Interior:
    //!     SplitPos:        [0, 31]
    //!     SplitAxis:       [0,  2] x=0b00, y=0b01, z=0b10, leaf=0b11
    //! Leaf:
    //!     PrimitivesBegin: [8, 31]
    //!     NumPrimitives:   [2,  7]
    void* tree;
    int depth;
    size_t numPrimitives;
    int numNodes;
    AABound3f worldBound;
};

#endif