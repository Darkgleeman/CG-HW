#ifndef BIT_GRID_ACCEL_MESH_H
#define BIT_GRID_ACCEL_MESH_H

#include <mesh.h>

class BitGridAccelMesh : public Mesh
{
public:
    // Shared fields
    static vector<uint8_t*> shared_grid;
    static vector<vector<int>**> shared_gridTriangles;

    // Constructor
    BitGridAccelMesh(const Transform* meshToWorld, const Transform* worldToMesh, 
                     const shared_ptr<const vector<Point3f>>& vertexP, const shared_ptr<const vector<Point3i>>& vertexPIndices, 
                     const shared_ptr<const vector<Normal3f>>& vertexNg, const shared_ptr<const vector<Point3i>>& vertexNgIndices, 
                     const Point3i& gridDepth, 
                     Float meshToGridScale, const Vector3f& meshToGridTranslation, 
                     const uint8_t* grid, vector<int>** gridTriangles);
    ~BitGridAccelMesh();

    // Methods
    static void LoadFromFile(const string& path, int maxGridDepthPerDim, MeshFileType fileType, MeshLoadFlag loadFlags, 
                             shared_ptr<vector<Point3f>>* vertexP, shared_ptr<vector<Point3i>>* vertexPIndices, 
                             shared_ptr<vector<Normal3f>>* vertexNg, shared_ptr<vector<Point3i>>* vertexNgIndices, 
                             Point3i* gridDepth, 
                             Float* meshToGridScale, Vector3f* meshToGridTranslation, 
                             uint8_t** grid, vector<int>*** gridTriangles);
    static void LoadFromFile(const string& path, bool toFile, 
                             shared_ptr<vector<Point3f>>* vertexP, shared_ptr<vector<Point3i>>* vertexPIndices, 
                             shared_ptr<vector<Normal3f>>* vertexNg, shared_ptr<vector<Point3i>>* vertexNgIndices, 
                             Point3i* gridDepth, 
                             Float* meshToGridScale, Vector3f* meshToGridTranslation, 
                             uint8_t** grid, vector<int>*** gridTriangles);
    virtual bool Intersect(const Ray& ray, Float* tHit, SurfaceInteraction* si) const override;

private:
    // Fields
    const uint8_t* grid;
    vector<int>** gridTriangles;

    // Supporting fields
    const int gridShifter_gridDepth_Y;  // order(gridDepth.y)
    const int gridShifter_gridDepth_Z;  // order(gridDepth.z)
    const int gridShifter_gridDepth_YZ; // order(gridDepth.y) + order(gridDepth.z)
    static constexpr Float gridRelax_getBound = 0.501f;

    // Supporting methods
    //! BitIdx:      每个 bit     在三维坐标中的 index：[0, gridDepth.x-1] × [0, gridDepth.y-1] × [0, gridDepth.z-1]
    //! ByteIdx:     每个 uint8_t 在三维坐标中的 index：[0, gridDepth.x-1] × [0, gridDepth.y-1] × [0, (gridDepth.z>>3)-1]
    //! GridBitIdx:  每个 bit     在一维坐标中的 index: [0, gridDepth.x×gridDepth.y×gridDepth.z-1]
    //! GridByteIdx: 每个 byte    在一维坐标中的 index: = GridBitIdx >> 3
    static inline AABound3f GridGetBitBlockBound_Relaxed(int bitIdxX, int bitIdxY, int bitIdxZ)
    {
        return AABound3f(Point3f(bitIdxX - gridRelax_getBound, 
                                 bitIdxY - gridRelax_getBound, 
                                 bitIdxZ - gridRelax_getBound), 
                         Point3f(bitIdxX + 1 + gridRelax_getBound, 
                                 bitIdxY + 1 + gridRelax_getBound, 
                                 bitIdxZ + 1 + gridRelax_getBound));
    }
    inline bool GridIsInside(const Point3f& gridCoordPosition) const //! 这是一个效率很低的函数，应当尽可能地少用
    {
        int bitIdxX = (int)(gridCoordPosition.x); //! 这里本应该用 std::floor
        int bitIdxY = (int)(gridCoordPosition.y); //! 因为 -0.5 会被 (int) 转换为 0 而不是 -1
        int bitIdxZ = (int)(gridCoordPosition.z); //! 但 std::floor 远没有 (int) 快
        int orderY = bitIdxY >> gridShifter_gridDepth_Y; //! 比直接判断 bitIdx < depth 快一点
        int orderZ = bitIdxZ >> gridShifter_gridDepth_Z;
        return bitIdxX >= 0 && bitIdxX < gridDepth.x && orderY == 0 && orderZ == 0;
    }
    //! WARNING: 即使 return false，也会修改输入，为了更快
    inline bool GridIsInside(const Point3f& gridCoordPosition, 
                             int* bitIdxX, int* bitIdxY, int* bitIdxZ) const //! 这是一个效率很低的函数，应当尽可能地少用
    {
        *bitIdxX = (int)(gridCoordPosition.x); //! 这里本应该用 std::floor
        *bitIdxY = (int)(gridCoordPosition.y); //! 因为 -0.5 会被 (int) 转换为 0 而不是 -1
        *bitIdxZ = (int)(gridCoordPosition.z); //! 但 std::floor 远没有 (int) 快
        int orderY = *bitIdxY >> gridShifter_gridDepth_Y; //! 比直接判断 bitIdx < depth 快一点
        int orderZ = *bitIdxZ >> gridShifter_gridDepth_Z;
        return *bitIdxX >= 0 && *bitIdxX < gridDepth.x && orderY == 0 && orderZ == 0;
    }
    inline int GridGetGridByteIdx(int bitIdxX, int bitIdxY, int bitIdxZ) const //! 这是个效率不高的函数，因为可以不需要整体算完 gridBitIdx 再 shift 的，实际上只需要对 z 做 shift
    {
        return GridGetGridBitIdx(bitIdxX, bitIdxY, bitIdxZ) >> 3;
    }
    inline int GridGetGridBitIdx(int bitIdxX, int bitIdxY, int bitIdxZ) const
    {
        return (bitIdxX << gridShifter_gridDepth_YZ) + 
               (bitIdxY << gridShifter_gridDepth_Z)  + 
               (bitIdxZ);
    }
    inline bool GridGetHasItem(int gridByteIdx, int bitIdxZ) const
    {
        unsigned int block = (unsigned int)(grid[gridByteIdx]);
        return block != 0 && ((block >> (bitIdxZ & 0b111)) & 1); //! 效果非常强大的优化
    }

    // Static supporting methods
    static inline bool GridIsInside(const Point3f& gridCoordPosition, 
                                    int gridDepthX, int shifter_gridDepth_Y, int shifter_gridDepth_Z) //! 这是一个效率很低的函数，应当尽可能地少用
    {
        int bitIdxX = (int)(gridCoordPosition.x); //! 这里本应该用 std::floor
        int bitIdxY = (int)(gridCoordPosition.y); //! 因为 -0.5 会被 (int) 转换为 0 而不是 -1
        int bitIdxZ = (int)(gridCoordPosition.z); //! 但 std::floor 远没有 (int) 快
        int orderY = bitIdxY >> shifter_gridDepth_Y; //! 比直接判断 bitIdx < depth 快一点
        int orderZ = bitIdxZ >> shifter_gridDepth_Z;
        return bitIdxX >= 0 && bitIdxX < gridDepthX && orderY == 0 && orderZ == 0;
    }
    static inline int GridGetGridByteIdx(int bitIdxX, int bitIdxY, int bitIdxZ, 
                                         int shifter_gridDepth_YZ, int shifter_gridDepth_Z) //! 这是个效率不高的函数，因为可以不需要整体算完 gridBitIdx 再 shift 的，实际上只需要对 z 做 shift
    {
        return GridGetGridBitIdx(bitIdxX, bitIdxY, bitIdxZ, shifter_gridDepth_YZ, shifter_gridDepth_Z) >> 3;
    }
    static inline int GridGetGridBitIdx(int bitIdxX, int bitIdxY, int bitIdxZ, 
                                        int shifter_gridDepth_YZ, int shifter_gridDepth_Z)
    {
        return (bitIdxX << shifter_gridDepth_YZ) + 
               (bitIdxY << shifter_gridDepth_Z)  + 
               (bitIdxZ);
    }
    static inline void GridSetHasItem(int bitIdxX, int bitIdxY, int bitIdxZ, 
                                      uint8_t* grid, int shifter_gridDepth_YZ, int shifter_gridDepth_Z)
    {
        uint8_t* block = &(grid[GridGetGridByteIdx(bitIdxX, bitIdxY, bitIdxZ, shifter_gridDepth_YZ, shifter_gridDepth_Z)]);
        int bitIdxInByte = bitIdxZ & 0b111;
        *block = *block | (1 << bitIdxInByte);
    }
};

#endif