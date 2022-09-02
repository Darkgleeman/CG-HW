#ifndef VOLUME_H
#define VOLUME_H

#include <boundingBox.h>
#include <transform.h>
#include <openvdb/openvdb.h>

//* API Methods ------------------------------------------------------------------------------------------------*//

void GenerateRainbowGrid(string vdbFilePath, string vdbGridName);



//* Volume -----------------------------------------------------------------------------------------------------*//

class Volume
{
public:
    // Constructors
    virtual ~Volume();

    // Methods
    virtual Transform LocalToGrid() const = 0;
};



//* VolumeFloatGrid --------------------------------------------------------------------------------------------*//

// TODO: 甚至这也能虚函数，只要接口相同，内部数据不需要是 grid
// TODO: 这可能不大行，因为不同数据结构的 traversal 方式也不太一样

class VolumeFloatGrid : public Volume
{
public:
    // Constructors
    VolumeFloatGrid(string vdbFilePath, string vdbGridName);
    VolumeFloatGrid(const VolumeFloatGrid& that) = delete;

    // Methods
    /**
     * @brief Local coordinate 当然是大家最喜欢的 [-0.5~0.5]^3
     */
    virtual Transform LocalToGrid() const override;
    AABound3i GridBoundi() const;
    AABound3f GridBoundf() const;
    float GridMinValue() const;
    float GridMaxValue() const;
    openvdb::FloatGrid::ConstAccessor ConstAccessor() const;
    float Sample(const Point3f& posGrid) const;

private:
    // Fields
    openvdb::FloatGrid::Ptr grid;
    AABound3i gridBoundi;
    AABound3f gridBoundf;
    float gridMinValue;
    float gridMaxValue;
};



//* VolumeVecGrid ----------------------------------------------------------------------------------------------*//

class VolumeVecGrid : public Volume
{
public:
    // Constructors
    VolumeVecGrid(string vdbFilePath, string vdbGridName);
    VolumeVecGrid(const VolumeVecGrid& that) = delete;

    // Methods
    /**
     * @brief Local coordinate 当然是大家最喜欢的 [-0.5~0.5]^3
     */
    virtual Transform LocalToGrid() const override;
    AABound3i GridBoundi() const;
    AABound3f GridBoundf() const;
    Vector3f GridMinValue() const;
    Vector3f GridMaxValue() const;
    openvdb::Vec3fGrid::ConstAccessor ConstAccessor() const;
    Vector3f Sample(const Point3f& posGrid) const;

private:
    // Fields
    openvdb::Vec3fGrid::Ptr grid;
    AABound3i gridBoundi;
    AABound3f gridBoundf;
    Vector3f gridMinValue;
    Vector3f gridMaxValue;
};



//* VolumeFloatKDTree ------------------------------------------------------------------------------------------*//

class VolumeFloatKDTree : public VolumeFloatGrid
{
public:
    // Constants
    static constexpr int KDTreeMaxDepth = 10;
    static constexpr int KDTreeExpectedAxisLengthPerNode = 10;
    static constexpr float KDTreeWeighOfKDelta = 0.7;
    static constexpr float KDTreeWeighOfKSubtract = 100;

    // Constructors
    VolumeFloatKDTree(string vdbFilePath, string vdbGridName, float densityScale, bool considerKdelta, bool considerKsubt);
    VolumeFloatKDTree(const VolumeFloatKDTree& that) = delete;
    virtual ~VolumeFloatKDTree() override;

    // Methods
    inline int KDTreeSplitAxis(int internalNodeIdx) const
    {
        struct float2 { float x; float y; };
        float2* kdTree_2f = (float2*)kdTree;
        return int(*((uint32_t*)(kdTree_2f + internalNodeIdx))) & 0b11;
    }
    inline int KDTreeSplitPos(int internalNodeIdx) const
    {
        struct float2 { float x; float y; };
        float2* kdTree_2f = (float2*)kdTree;
        return int(*((uint32_t*)(((float*)(kdTree_2f + internalNodeIdx)) + 1)));
    }
    inline float KDTreeMaxValue(int leafNodeIdx) const
    {
        struct float2 { float x; float y; };
        float2* kdTree_2f = (float2*)kdTree;
        return ((float*)(kdTree_2f + leafNodeIdx))[0];
    }
    inline float KDTreeMinValue(int leafNodeIdx) const
    {
        struct float2 { float x; float y; };
        float2* kdTree_2f = (float2*)kdTree;
        return ((float*)(kdTree_2f + leafNodeIdx))[1];
    }

    // Operators
    friend std::ostream& operator<<(std::ostream& out, const VolumeFloatKDTree& t);

private:
    // Fields
    //! Tree format: 64-bit nodes
    //! Interior:
    //!     SplitAxis:       [0,   1] x=0b00, y=0b01, z=0b10, leaf=0b11
    //!     SplitPos:        [32, 63]
    //! Leaf:
    //!     MaxValue:        [0,  31] Note: [0, 1] set to 0b11 to match the split axis
    //!     MinValue:        [32, 63]
    void* kdTree;
};

#endif