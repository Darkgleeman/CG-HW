#include <volume.h>
#include <openvdb/tools/Interpolation.h>

//* API Methods ------------------------------------------------------------------------------------------------*//

void GenerateRainbowGrid(string vdbFilePath, string vdbGridName)
{
    openvdb::initialize();

    // Load float grid
    string path = GetFilePath(vdbFilePath);
    openvdb::io::File iFile(path);
    iFile.open();
    openvdb::FloatGrid::Ptr fGrid = openvdb::gridPtrCast<openvdb::FloatGrid>(iFile.readGrid(vdbGridName));
    iFile.close();
    openvdb::CoordBBox gridBound_vdb = fGrid->evalActiveVoxelBoundingBox();
    openvdb::Vec3f pMin = gridBound_vdb.min().asVec3s();
    openvdb::Vec3f pMax = gridBound_vdb.max().asVec3s();
    openvdb::Vec3f diagonal = pMax - pMin;

    // Create rainbow vec grid from float grid
    openvdb::Vec3fGrid::Ptr vGrid = openvdb::Vec3fGrid::create();
    openvdb::Vec3fGrid::Accessor vAccessor = vGrid->getAccessor();
    for (auto it = fGrid->beginValueOn(); it; ++it)
    {
        // Compute coordinate and offset
        openvdb::Coord coord = it.getCoord();
        openvdb::Vec3f offset = (coord.asVec3s() - pMin) / diagonal;
        offset[0] = std::clamp(offset[0], float(0), float(1)); //! Clamp to avoid floating point error
        offset[1] = std::clamp(offset[1], float(0), float(1));
        offset[2] = std::clamp(offset[2], float(0), float(1));
        // Cubic lerp
        float value = it.getValue();
        openvdb::Vec3f color = Lerp(
            openvdb::Vec3f(0, 0, 0), openvdb::Vec3f(0, 0, 1), openvdb::Vec3f(0, 1, 0), openvdb::Vec3f(0, 1, 1), 
            openvdb::Vec3f(1, 0, 0), openvdb::Vec3f(1, 0, 1), openvdb::Vec3f(1, 1, 0), openvdb::Vec3f(1, 1, 1), 
            offset.x(), offset.y(), offset.z());
        // Set value
        vAccessor.setValue(coord, color * value);
    }

    // Create new vdbfile
    vGrid->setName("density");
    openvdb::io::File(path.substr(0, path.length() - 4) + "_rainbow.vdb").write({vGrid});
}



//* Volume -----------------------------------------------------------------------------------------------------*//

/*virtual*/ Volume::~Volume() { }



//* VolumeFloatGrid --------------------------------------------------------------------------------------------*//

// TODO: 不是，这还得考虑双精度单精度问题啊喂

// Constructors
VolumeFloatGrid::VolumeFloatGrid(string vdbFilePath, string vdbGridName)
{
    openvdb::initialize();
    // Load grid
    openvdb::io::File file(GetFilePath(vdbFilePath));
    file.open();
    grid = openvdb::gridPtrCast<openvdb::FloatGrid>(file.readGrid(vdbGridName));
    file.close();
    // Compute gridBound
    openvdb::CoordBBox gridBound_vdb = grid->evalActiveVoxelBoundingBox();
    openvdb::Coord boundMin = gridBound_vdb.min();
    openvdb::Coord boundMax = gridBound_vdb.max();
    gridBoundi = AABound3i(Point3i(boundMin.x(), boundMin.y(), boundMin.z()), Point3i(boundMax.x(), boundMax.y(), boundMax.z()));
    gridBoundf = AABound3f(Point3f(boundMin.x(), boundMin.y(), boundMin.z()), Point3f(boundMax.x(), boundMax.y(), boundMax.z()));
    // Compute minValue and maxValue
    grid->evalMinMax(gridMinValue, gridMaxValue);
}

// Methods
/*override*/ Transform VolumeFloatGrid::LocalToGrid() const
{
    Point3f boundCenter = GridBoundf().Center();
    Vector3f boundDiagonal = GridBoundf().Diagonal();
    return Transform(boundCenter, Quaternion::Identity(), boundDiagonal);
}

AABound3i VolumeFloatGrid::GridBoundi() const { return gridBoundi; }
AABound3f VolumeFloatGrid::GridBoundf() const { return gridBoundf; }
float VolumeFloatGrid::GridMinValue() const { return gridMinValue; }
float VolumeFloatGrid::GridMaxValue() const { return gridMaxValue; }
openvdb::FloatGrid::ConstAccessor VolumeFloatGrid::ConstAccessor() const { return grid->getConstAccessor(); }

float VolumeFloatGrid::Sample(const Point3f& posGrid) const
{
    thread_local openvdb::FloatGrid::ConstAccessor accessor = grid->getConstAccessor();
    const openvdb::Vec3f sampleAt(posGrid.x, posGrid.y, posGrid.z);
    return openvdb::tools::QuadraticSampler::sample(accessor, sampleAt);
}



//* VolumeVecGrid ----------------------------------------------------------------------------------------------*//

// Constructors
VolumeVecGrid::VolumeVecGrid(string vdbFilePath, string vdbGridName)
{
    openvdb::initialize();
    // Load grid
    openvdb::io::File file(GetFilePath(vdbFilePath));
    file.open();
    grid = openvdb::gridPtrCast<openvdb::Vec3fGrid>(file.readGrid(vdbGridName));
    file.close();
    // Compute gridBound
    openvdb::CoordBBox gridBound_vdb = grid->evalActiveVoxelBoundingBox();
    openvdb::Coord boundMin = gridBound_vdb.min();
    openvdb::Coord boundMax = gridBound_vdb.max();
    gridBoundi = AABound3i(Point3i(boundMin.x(), boundMin.y(), boundMin.z()), Point3i(boundMax.x(), boundMax.y(), boundMax.z()));
    gridBoundf = AABound3f(Point3f(boundMin.x(), boundMin.y(), boundMin.z()), Point3f(boundMax.x(), boundMax.y(), boundMax.z()));
    // Compute minValue and maxValue
    openvdb::Vec3f gridMinValue_vdb, gridMaxValue_vdb;
    grid->evalMinMax(gridMinValue_vdb, gridMaxValue_vdb);
    gridMinValue = Vector3f(gridMinValue_vdb.x(), gridMinValue_vdb.y(), gridMinValue_vdb.z());
    gridMaxValue = Vector3f(gridMaxValue_vdb.x(), gridMaxValue_vdb.y(), gridMaxValue_vdb.z());
}

// Methods
/**
 * @brief Local coordinate 当然是大家最喜欢的 [-0.5~0.5]^3
 */
/*override*/ Transform VolumeVecGrid::LocalToGrid() const
{
    Point3f boundCenter = GridBoundf().Center();
    Vector3f boundDiagonal = GridBoundf().Diagonal();
    return Transform(boundCenter, Quaternion::Identity(), boundDiagonal);
}

AABound3i VolumeVecGrid::GridBoundi() const { return gridBoundi; }
AABound3f VolumeVecGrid::GridBoundf() const { return gridBoundf; }
Vector3f VolumeVecGrid::GridMinValue() const { return gridMinValue; }
Vector3f VolumeVecGrid::GridMaxValue() const { return gridMaxValue; }
openvdb::Vec3fGrid::ConstAccessor VolumeVecGrid::ConstAccessor() const { return grid->getConstAccessor(); }

Vector3f VolumeVecGrid::Sample(const Point3f& posGrid) const
{
    thread_local openvdb::Vec3fGrid::ConstAccessor accessor = grid->getConstAccessor();
    const openvdb::Vec3f sampleAt(posGrid.x, posGrid.y, posGrid.z);
    openvdb::Vec3f sample = openvdb::tools::QuadraticSampler::sample(accessor, sampleAt);
    return Vector3f(sample.x(), sample.y(), sample.z());
}



//* VolumeFloatKDTree ------------------------------------------------------------------------------------------*//

// VolumeKDTreeBuilderNode: Definition
class VolumeKDTreeBuilderNode
{
public:
    // Fields
    int splitAxis;
    int splitPos;
    float maxValue;
    float minValue;
    AABound3i bound;
    VolumeKDTreeBuilderNode* left;
    VolumeKDTreeBuilderNode* righ;

    // Constructors
    VolumeKDTreeBuilderNode(
            int splitAxis, int splitPos, 
            float maxValue, float minValue, 
            const Point3i& iMin, const Point3i& iMax, 
            VolumeKDTreeBuilderNode* left, VolumeKDTreeBuilderNode* righ) : 
        splitAxis(splitAxis), splitPos(splitPos), 
        maxValue(maxValue), minValue(minValue), 
        bound(iMin, iMax), 
        left(left), righ(righ)
    { }

    // Methods
    string ToString() const
    {
        std::stringstream builder;
        const VolumeKDTreeBuilderNode* stack[VolumeFloatKDTree::KDTreeMaxDepth];
        int iStack = 0;
        const VolumeKDTreeBuilderNode* curr = this;
        int currDepth = 0;
        while (true)
        {
            if (curr->splitAxis == 3) // Leaf node
            {
                for (int i = 0; i < currDepth; ++i)
                    builder << "  ";
                builder << "bound=[" << curr->bound.pMin.x << "~" << curr->bound.pMax.x << ", " 
                                     << curr->bound.pMin.y << "~" << curr->bound.pMax.y << ", " 
                                     << curr->bound.pMin.z << "~" << curr->bound.pMax.z << "] " 
                        << "value=[" << curr->minValue << ", " << curr->maxValue << "]" << std::endl;
                if (iStack == 0)
                    break;
                curr = stack[--iStack];
                --currDepth;
            }
            else // Interior node
            {
                for (int i = 0; i < currDepth; ++i)
                    builder << "  ";
                builder << "bound=[" << curr->bound.pMin.x << "~" << curr->bound.pMax.x << ", " 
                                     << curr->bound.pMin.y << "~" << curr->bound.pMax.y << ", " 
                                     << curr->bound.pMin.z << "~" << curr->bound.pMax.z << "] " 
                        << "splitAt[" << curr->splitAxis << "]=" << curr->splitPos << std::endl;
                stack[iStack++] = curr->righ;
                curr = curr->left;
                ++currDepth;
            }
        }
        return builder.str();
    }
};

// VolumeKDTreeBuilderNode: Supporting functions
static float VolumeKDTreeBuilderNode_LargestRectangle(
        const vector<float>& data, 
        int* largestStart, 
        int* largestBack)
{
    const int n = (int)data.size();
    float maxArea = 0;
    for (int i = 0; i < n; ++i)
    {
        int l = i;
        while (l - 1 >= 0 && data[l - 1] >= data[i])
            --l;
        int r = i;
        while (r + 1  < n && data[r + 1] >= data[i])
            ++r;
        float area = (r - l + 1) * data[i];
        if (area > maxArea)
        {
            maxArea = area;
            *largestStart = l;
            *largestBack = r;
        }
    }
    return maxArea;
}

// VolumeKDTreeBuilderNode: Functions
static VolumeKDTreeBuilderNode* VolumeKDTreeBuilderNode_Build(
        const openvdb::FloatGrid::ConstAccessor density, float densityScale, 
        bool considerKdelt, bool considerKsubt, 
        const Point3i& iMin, const Point3i& iMax, 
        int depth, int* maxDepth)
{
    // Splitting information
    float reducedAreaOptimal = 0;
    int splitAxis = 3;
    int lOptimal, rOptimal;

    // For each axis, try splitting
    if (depth < VolumeFloatKDTree::KDTreeMaxDepth)
    {
        for (int axis = 0; axis < 3; ++axis)
        {
            // Stop if length is small
            int axisLength = iMax[axis] - iMin[axis] + 1;
            if (axisLength <= VolumeFloatKDTree::KDTreeExpectedAxisLengthPerNode)
                continue;
            // Compute kplus, kdelt
            vector<float> kplus(axisLength);
            vector<float> kdelt(axisLength);
            vector<float> ksubt(axisLength);
            float kplusMax = 0, kdeltMax = 0;
            for (int i = iMin[axis]; i <= iMax[axis]; ++i)
            {
                float minValue = Infinity;
                float maxValue = 0;
                const int axisNext1 = (axis + 1) % 3;
                const int axisNext2 = (axis + 2) % 3;
                for (int u = iMin[axisNext1]; u <= iMax[axisNext1]; ++u)
                    for (int v = iMin[axisNext2]; v <= iMax[axisNext2]; ++v)
                    {
                        openvdb::Coord coord;
                        coord[axis] = i;
                        coord[axisNext1] = u;
                        coord[axisNext2] = v;
                        if (!density.isValueOn(coord))
                            continue;
                        float dense = density.getValue(coord) * densityScale;
                        minValue = std::min(minValue, dense);
                        maxValue = std::max(maxValue, dense);
                    }
                minValue = std::min(minValue, maxValue);
                int iK = i - iMin[axis];
                kplus[iK] = maxValue;
                kdelt[iK] = maxValue - minValue;
                ksubt[iK] = minValue;
                kplusMax = std::max(kplusMax, kplus[iK]);
                kdeltMax = std::max(kdeltMax, kdelt[iK]);
            }
            for (int i = 0; i < (int)kplus.size(); ++i)
            {
                kplus[i] = kplusMax - kplus[i];
                kdelt[i] = kdeltMax - kdelt[i];
            }

            // Compute reduced area for kplus
            int lCurr = 0, rCurr = (int)kplus.size() - 1;
            float areaCurr = VolumeKDTreeBuilderNode_LargestRectangle(kplus, &lCurr, &rCurr);
            if (areaCurr < Epsilonx10)
                continue;
            int numAdditional = 2;
            if (lCurr == 0)
                --numAdditional;
            if (rCurr == (int)kplus.size() - 1)
                --numAdditional;
            if (numAdditional == 0)
            {
                Error("Unknown bug");
                exit(-1);
            }
            float reducedAreaCurr = areaCurr - numAdditional;
            // Try update
            if (reducedAreaCurr > reducedAreaOptimal)
            {
                reducedAreaOptimal = reducedAreaCurr;
                splitAxis = axis;
                lOptimal = lCurr + iMin[axis];
                rOptimal = rCurr + iMin[axis];
            }

            // Compute reduced area for kdelt
            if (considerKdelt)
            {
                lCurr = 0, rCurr = (int)kdelt.size() - 1;
                areaCurr = VolumeKDTreeBuilderNode_LargestRectangle(kdelt, &lCurr, &rCurr);
                if (areaCurr < Epsilonx10)
                    continue;
                numAdditional = 2;
                if (lCurr == 0)
                    --numAdditional;
                if (rCurr == (int)kdelt.size() - 1)
                    --numAdditional;
                if (numAdditional == 0)
                {
                    Error("Unknown bug");
                    exit(-1);
                }
                reducedAreaCurr = VolumeFloatKDTree::KDTreeWeighOfKDelta * (areaCurr - numAdditional); // TODO: Magic number
                // Try update
                if (reducedAreaCurr > reducedAreaOptimal)
                {
                    reducedAreaOptimal = reducedAreaCurr;
                    splitAxis = axis;
                    lOptimal = lCurr + iMin[axis];
                    rOptimal = rCurr + iMin[axis];
                }
            }

            // Compute reduced area for ksubt
            if (considerKsubt)
            {
                lCurr = 0, rCurr = (int)kdelt.size() - 1;
                areaCurr = VolumeKDTreeBuilderNode_LargestRectangle(ksubt, &lCurr, &rCurr);
                if (areaCurr < Epsilonx10)
                    continue;
                numAdditional = 2;
                if (lCurr == 0)
                    --numAdditional;
                if (rCurr == (int)kdelt.size() - 1)
                    --numAdditional;
                if (numAdditional == 0)
                {
                    Error("Unknown bug");
                    exit(-1);
                }
                reducedAreaCurr = VolumeFloatKDTree::KDTreeWeighOfKSubtract * (areaCurr - numAdditional); // TODO: Magic number
                // Try update
                if (reducedAreaCurr > reducedAreaOptimal)
                {
                    reducedAreaOptimal = reducedAreaCurr;
                    splitAxis = axis;
                    lOptimal = lCurr + iMin[axis];
                    rOptimal = rCurr + iMin[axis];
                }
            }
        }
    }

    // Try construct interior node
    if (splitAxis != 3)
    {
        // Compute split position
        int mid = (iMin[splitAxis] + iMax[splitAxis]) / 2;
        int lToMid = std::abs(mid - lOptimal);
        int rToMid = std::abs(rOptimal - mid);
        int splitPos;
        Point3i iMax_new = iMax;
        Point3i iMin_new = iMin;
        if (lToMid <= rToMid)
        {
            // Split the grid into: [..., lOptimal], [lOptimal + 1, ...]
            splitPos = lOptimal;
            iMax_new[splitAxis] = lOptimal;
            iMin_new[splitAxis] = lOptimal + 1;
        }
        else
        {
            // Split the grid into: [..., rOptimal], [rOptimal + 1, ...]
            splitPos = rOptimal;
            iMax_new[splitAxis] = rOptimal;
            iMin_new[splitAxis] = rOptimal + 1;
        }
        // Construct node
        if ((splitPos != iMin[splitAxis]) && (splitPos != iMax[splitAxis])) // Interior node
        {
            return new VolumeKDTreeBuilderNode(
                splitAxis, splitPos, 
                0, 0, 
                iMin, iMax, 
                VolumeKDTreeBuilderNode_Build(density, densityScale, considerKdelt, considerKsubt, iMin, iMax_new, depth + 1, maxDepth), 
                VolumeKDTreeBuilderNode_Build(density, densityScale, considerKdelt, considerKsubt, iMin_new, iMax, depth + 1, maxDepth));
        }
    }

    // Construct leaf node
    float nodeMaxValue = 0; // Compute maxValue and minValue of the node
    float nodeMinValue = Infinity;
    openvdb::FloatGrid grid;
    for (int i = iMin.x; i <= iMax.x; ++i)
        for (int j = iMin.y; j <= iMax.y; ++j)
            for (int k = iMin.z; k <= iMax.z; ++k)
            {
                openvdb::Coord coord(i, j, k);
                if (!density.isValueOn(coord))
                    continue;
                float dense = density.getValue(coord);
                nodeMaxValue = std::max(nodeMaxValue, dense);
                nodeMinValue = std::min(nodeMinValue, dense);
            }
    nodeMinValue = std::min(nodeMinValue, nodeMaxValue); //! 防一手浮点数
    uint64_t nodeMaxValue_int = 0;
    memcpy(&nodeMaxValue_int, &nodeMaxValue, sizeof(float));
    nodeMaxValue_int |= uint64_t(3); //! 将 magnitude 最后两位置为 0b11，表示 leaf node
    memcpy(&nodeMaxValue, &nodeMaxValue_int, sizeof(float));
    *maxDepth = std::max(*maxDepth, depth);
    return new VolumeKDTreeBuilderNode( // Construct leaf node
        3, -1, 
        nodeMaxValue, nodeMinValue, 
        iMin, iMax, 
        nullptr, 
        nullptr);
}

static void VolumeKDTreeBuilderNode_CompressTo(
        void* tree, const VolumeKDTreeBuilderNode* node)
{
    //! 注：不强求 float 为 32 位
    float* kdTree = (float*)tree;
    struct ToVisitInfo { const VolumeKDTreeBuilderNode* node; int compressIdx; };
    ToVisitInfo stack[VolumeFloatKDTree::KDTreeMaxDepth];
    int iStack = 0;
    int compressIdx = 1;
    while (true)
    {
        float* compressAt = kdTree + (compressIdx << 1);
        if (node->splitAxis == 3) // Leaf node
        {
            memcpy(compressAt, &node->maxValue, sizeof(float));
            memcpy(compressAt + 1, &node->minValue, sizeof(float));
            if (iStack == 0)
                break;
            --iStack;
            node = stack[iStack].node;
            compressIdx = stack[iStack].compressIdx;
        }
        else // Interior node
        {
            const float zero = 0; //! Memcpy to set zero, avoiding memory align problems
            memcpy(compressAt, &zero, sizeof(float));
            memcpy(compressAt + 1, &zero, sizeof(float));
            uint32_t splitAxis_32 = uint32_t(node->splitAxis); //! Convert to uint32_t first, avoiding memory align problems
            uint32_t splitPos_32 = uint32_t(node->splitPos);
            memcpy(compressAt, &splitAxis_32, 2);
            memcpy(compressAt + 1, &splitPos_32, sizeof(uint32_t));
            stack[iStack].node = node->righ;
            stack[iStack].compressIdx = (compressIdx << 1) + 1;
            ++iStack;
            node = node->left;
            compressIdx <<= 1;
        }
    }
}

static void VolumeKDTreeBuilderNode_Delete(VolumeKDTreeBuilderNode* node)
{
    VolumeKDTreeBuilderNode* stack[VolumeFloatKDTree::KDTreeMaxDepth];
    int iStack = 0;
    while (true)
    {
        if (node->splitAxis == 3) // Leaf node
        {
            delete node;
            if (iStack == 0)
                break;
            node = stack[--iStack];
        }
        else
        {
            VolumeKDTreeBuilderNode* left = node->left;
            stack[iStack++] = node->righ;
            delete node;
            node = left;
        }
    }
}

// VolumeFloatKDTree: Constructors
VolumeFloatKDTree::VolumeFloatKDTree(string vdbFilePath, string vdbGridName, float densityScale, bool considerKdelta, bool considerKsubt) : 
    VolumeFloatGrid(vdbFilePath, vdbGridName)
{
    // Build
    AABound3i bound = GridBoundi();
    int depth = 0;
    VolumeKDTreeBuilderNode* root = VolumeKDTreeBuilderNode_Build(ConstAccessor(), densityScale, considerKdelta, considerKsubt, bound.pMin, bound.pMax, 0, &depth);
    // Error(root->ToString());

    // Compress
    kdTree = calloc(1 << (depth + 1), sizeof(float) << 1);
    VolumeKDTreeBuilderNode_CompressTo(kdTree, root);

    // Free memory
    VolumeKDTreeBuilderNode_Delete(root);
}

/*override*/ VolumeFloatKDTree::~VolumeFloatKDTree() { free(kdTree); }

// VolumeFloatKDTree: Methods
/*friend*/ std::ostream& /*VolumeFloatKDTree::*/operator<<(std::ostream& out, const VolumeFloatKDTree& t)
{
    //! 注：不强求 float 为 32 位
    int stack[VolumeFloatKDTree::KDTreeMaxDepth];
    int iStack = 0;
    int i = 1;
    int currDepth = 0;
    while (true)
    {
        if (t.KDTreeSplitAxis(i) == 3) // Leaf node
        {
            for (int i = 0; i < currDepth; ++i)
                out << "  ";
            out << "value=[" << t.KDTreeMinValue(i) << ", " << t.KDTreeMaxValue(i) << "]" << std::endl;
            if (iStack == 0)
                break;
            i = stack[--iStack];
            --currDepth;
        }
        else // Interior node
        {
            for (int i = 0; i < currDepth; ++i)
                out << "  ";
            out << "splitAt[" << t.KDTreeSplitAxis(i) << "]=" << t.KDTreeSplitPos(i) << std::endl;
            stack[iStack++] = (i << 1) + 1;
            i <<= 1;
            ++currDepth;
        }
    }
    return out;
}
