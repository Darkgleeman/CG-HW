#ifndef GRID_ACCEL_MESH_H
#define GRID_ACCEL_MESH_H

#include <mesh.h>

// #define STAR_GRID

class GridAccelMesh : public Mesh
{
public:
    // Constructor
    GridAccelMesh(const Transform* meshToWorld, const Transform* worldToMesh, 
                  const shared_ptr<const vector<Point3f>>& vertexP, const shared_ptr<const vector<Point3i>>& vertexPIndices, 
                  const shared_ptr<const vector<Normal3f>>& vertexNg, const shared_ptr<const vector<Point3i>>& vertexNgIndices, 
                  Point3i gridDepth, 
                  Float meshToGridScale, Vector3f meshToGridTranslation);
    ~GridAccelMesh();

    // Methods
    virtual bool Intersect(const Ray& ray, Float* tHit, SurfaceInteraction* si) const override;
private:
    // Fields
    #ifdef STAR_GRID
    vector<int>**** grid;
    #else
    vector<int>** grid;
    #endif

    // Supporting fields
    #ifdef STAR_GRID
    #else
    const int gridShifter_blockDim_Z;  // order(gridDepth.z >> 3)
    const int gridShifter_blockDim_YZ; // order(gridDepth.y) + order(gridDepth.z >> 3)
    #endif
    static constexpr Float gridRelax_getBound = 0.501f;

    // Supporting methods
    //! GridIdx:  [0, gridDepth.x * gridDepth.y * gridDepth.z - 1]
    //! BlockIdx: [0, gridDepth.x-1] × [0, gridDepth.y-1] × [0, gridDepth.z-1]
    inline AABound3f GridGetBlockBound_Relaxed(int blockIdxX, int blockIdxY, int blockIdxZ) const
    {
        return AABound3f(Point3f(blockIdxX - gridRelax_getBound, 
                                 blockIdxY - gridRelax_getBound, 
                                 blockIdxZ - gridRelax_getBound), 
                         Point3f(blockIdxX + 1 + gridRelax_getBound, 
                                 blockIdxY + 1 + gridRelax_getBound, 
                                 blockIdxZ + 1 + gridRelax_getBound));
    }
    inline bool GridIsInside(const Point3f& gridCoordPosition) const //! 这是一个效率很低的函数，应当尽可能地少用
    {
        int blockIdxX = int(gridCoordPosition.x);
        int blockIdxY = int(gridCoordPosition.y);
        int blockIdxZ = int(gridCoordPosition.z);
        return blockIdxX >= 0 && blockIdxY >= 0 && blockIdxZ >= 0 && 
               blockIdxX < gridDepth.x && blockIdxY < gridDepth.y && blockIdxZ < gridDepth.z;
    }
    //! WARNING: 即使 return false，也会修改输入，为了更快
    inline bool GridIsInside(const Point3f& gridCoordPosition, 
                             int* blockIdxX, int* blockIdxY, int* blockIdxZ) const //! 这是一个效率很低的函数，应当尽可能地少用
    {
        *blockIdxX = int(gridCoordPosition.x);
        *blockIdxY = int(gridCoordPosition.y);
        *blockIdxZ = int(gridCoordPosition.z);
        return *blockIdxX >= 0 && *blockIdxY >= 0 && *blockIdxZ >= 0 && 
               *blockIdxX < gridDepth.x && *blockIdxY < gridDepth.y && *blockIdxZ < gridDepth.z;
    }
    #ifdef STAR_GRID
    #else
    inline int GridGetGridIdx(int blockIdxX, int blockIdxY, int blockIdxZ) const
    {
        return (blockIdxX << gridShifter_blockDim_YZ) + 
               (blockIdxY << gridShifter_blockDim_Z)  + 
               (blockIdxZ >> 3);
    }
    #endif
};

#endif