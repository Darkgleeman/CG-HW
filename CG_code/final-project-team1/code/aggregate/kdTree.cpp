#include <kdTree.h>

//* KDTreeBuilderNode ------------------------------------------------------------------------------------------*//

class KDTreeBuilderNode
{
public:
    // Fields
    const int splitAxis;
    const float splitPos;
    int primitivesBegin;
    int numPrimitives;
    KDTreeBuilderNode* left;
    KDTreeBuilderNode* righ;
    // Constructors
    KDTreeBuilderNode(int splitAxis, Float splitPos, 
                      int primitivesBegin, int numPrimitives, 
                      KDTreeBuilderNode* left, KDTreeBuilderNode* righ);
    // Methods
    string ToString() const;
    void ToArray(vector<KDTreeBuilderNode*>* array);
    void ToPrimitivesData(vector<int>* primitivesBegin, vector<int>* numPrimitives) const;
    void FromPrimitivesData(const vector<int>& primitivesBegin, const vector<int>& numPrimitives);
    int NumTreePrimitives() const;
    int NumTreeNodes() const;
    int Depth() const;
    static void Delete(const KDTreeBuilderNode* node);
    static KDTreeBuilderNode* Build(const vector<shared_ptr<Primitive>>& primitives, 
                                    const vector<AABound3f>& primitiveBounds, 
                                    const vector<Point3f>& primitiveCentroids, 
                                    vector<int>* primitiveIndices, 
                                    vector<int>* mergedPrimitiveIndices, 
                                    int maxDepth);
    void CompressTo(vector<int>* uncompressedPrimitiveIndices, void* tree);
};

// Constructors
KDTreeBuilderNode::KDTreeBuilderNode(
        int splitAxis, Float splitPos, 
        int primitivesBegin, int numPrimitives, 
        KDTreeBuilderNode* left, KDTreeBuilderNode* righ) : 
    splitAxis(splitAxis), splitPos(splitPos), 
    primitivesBegin(primitivesBegin), numPrimitives(numPrimitives), 
    left(left), righ(righ)
{
    if (splitAxis == 3 && numPrimitives >= KDTree::MaxNumPrimsPerNode) //! 超出上限 64，塞进另一个 aggregate
    {
        std::stringstream builder;
        builder << "WARNING: KDTreeBuilderNode::KDTreeBuilderNode(): Number of primitives per node out of bound, affecting performance, number=" << numPrimitives;
        Error(builder.str());
    }
}

// KDTreeBuilderNode supporting methods
static void KDTreeBuilderNode_ToString(const KDTreeBuilderNode* node, int depth, std::stringstream* builder)
{
    for (int d = 0; d < depth; ++d)
        *builder << "| ";
    if (node->splitAxis == 3) // Leaf
    {
        *builder << '[' << node->primitivesBegin << ' ' << node->primitivesBegin + node->numPrimitives << ')' << std::endl;
    }
    else // Interior
    {
        *builder << (node->splitAxis == 0 ? 'x' : (node->splitAxis == 1 ? 'y' : (node->splitAxis == 2 ? 'z' : '?'))) << ' ' << node->splitPos << std::endl;
        KDTreeBuilderNode_ToString(node->left, depth + 1, builder); //! 一支为 nullptr，则另一支必为 nullptr，即为叶子节点
        KDTreeBuilderNode_ToString(node->righ, depth + 1, builder);
    }
}
static void KDTreeBuilderNode_ToArray(KDTreeBuilderNode* node, vector<KDTreeBuilderNode*>* array)
{
    if (node->splitAxis == 3) // Leaf
        array->push_back(node);
    else // Interior //! 一支为 nullptr，则另一支必为 nullptr，即为叶子节点
    {
        KDTreeBuilderNode_ToArray(node->left, array);
        KDTreeBuilderNode_ToArray(node->righ, array);
    }
}
static void KDTreeBuilderNode_ToPrimitivesData(const KDTreeBuilderNode* node, vector<int>* primitivesBegin, vector<int>* numPrimitives)
{
    if (node->splitAxis == 3) // Leaf
    {
        primitivesBegin->push_back(node->primitivesBegin);
        numPrimitives->push_back(node->numPrimitives);
    }
    else // Interior //! 一支为 nullptr，则另一支必为 nullptr，即为叶子节点
    {
        KDTreeBuilderNode_ToPrimitivesData(node->left, primitivesBegin, numPrimitives);
        KDTreeBuilderNode_ToPrimitivesData(node->righ, primitivesBegin, numPrimitives);
    }
}
static void KDTreeBuilderNode_FromPrimitivesData(KDTreeBuilderNode* node, const vector<int>& primitivesBegin, const vector<int>& numPrimitives, int* currIdx)
{
    if (node->splitAxis == 3) // Leaf
    {
        node->primitivesBegin = primitivesBegin[*currIdx];
        node->numPrimitives = numPrimitives[*currIdx];
        ++(*currIdx);
    }
    else // Interior //! 一支为 nullptr，则另一支必为 nullptr，即为叶子节点
    {
        KDTreeBuilderNode_FromPrimitivesData(node->left, primitivesBegin, numPrimitives, currIdx);
        KDTreeBuilderNode_FromPrimitivesData(node->righ, primitivesBegin, numPrimitives, currIdx);
    }
}
static int KDTreeBuilderNode_NumTreePrimitives(const KDTreeBuilderNode* node)
{
    if (node->splitAxis == 3) // Leaf
        return node->numPrimitives;
    else // Interior //! 一支为 nullptr，则另一支必为 nullptr，即为叶子节点
        return KDTreeBuilderNode_NumTreePrimitives(node->left) + KDTreeBuilderNode_NumTreePrimitives(node->righ);
}
static int KDTreeBuilderNode_NumTreeNodes(const KDTreeBuilderNode* node)
{
    if (node->splitAxis == 3) // Leaf
        return 1;
    else // Interior //! 一支为 nullptr，则另一支必为 nullptr，即为叶子节点
        return KDTreeBuilderNode_NumTreeNodes(node->left) + KDTreeBuilderNode_NumTreeNodes(node->righ);
}
static int KDTreeBuilderNode_Depth(const KDTreeBuilderNode* node, int currDepth)
{
    if (node->splitAxis == 3) // Leaf
        return currDepth;
    else // Interior //! 一支为 nullptr，则另一支必为 nullptr，即为叶子节点
        return std::max(KDTreeBuilderNode_Depth(node->left, currDepth + 1), KDTreeBuilderNode_Depth(node->righ, currDepth + 1));
}
static void KDTreeBuilderNode_CompressTo(void* tree, const KDTreeBuilderNode* node, int compressIdx)
{
    // 将 node 压缩拷贝到 compressIdx 对应的位置
    if (node->splitAxis == 3) // Leaf
    {
        Assert(node->primitivesBegin < std::pow(2, 24) && 
               node->numPrimitives < KDTree::MaxNumPrimsPerNode, //! 2 ** 6 = 64
               "KDTree::KDTreeBuilderNode::Compress(): Primitive indices out of bound");
        uint32_t primitivesData = (uint32_t(node->primitivesBegin) << 8) + (uint32_t(node->numPrimitives) << 2) + 3;
        memcpy((uint32_t*)tree + compressIdx, &primitivesData, sizeof(uint32_t));
    }
    else // Interior
    {
        Assert(node->splitAxis >= 0 && node->splitAxis < 3, 
               "KDTree::KDTreeBuilderNode::Compress(): Undefined split axis");
        memcpy((float*)tree + compressIdx, &(node->splitPos), sizeof(float));
        // 递归 compress children
        KDTreeBuilderNode_CompressTo(tree, node->left, compressIdx << 1);
        KDTreeBuilderNode_CompressTo(tree, node->righ, (compressIdx << 1) + 1);
    }
}
// KDTree supporting methods
#ifndef NDEBUG
static string KDTree_BuildingDataToString(const KDTree& tree)
{
    std::stringstream builder;
    int i = 1;
    int currDepth = 0;
    while (true)
    {
        for (int d = 0; d < currDepth; ++d)
            builder << "| ";
        // 检查是 interior 还是 leaf
        if (tree.SplitAxis(i) == 3) // Leaf
        {
            builder << '[' << tree.PrimitivesBegin(i) << ' ' << tree.PrimitivesBegin(i) + tree.NumPrimitives(i) << ')' << std::endl;
            if ((i & 1) == 0) // 是左叶子
                ++i; // i 指向右叶子
            else // 是右叶子
            {
                do
                {
                    i >>= 1;
                    --currDepth;
                    if (currDepth == -1) //! 不存在非右节点，则退出
                        return builder.str();
                } while ((i & 1) == 1);
                ++i;
            }
        }
        else // Interior
        {
            int axis = tree.SplitAxis(i);
            builder << (axis == 0 ? 'x' : (axis == 1 ? 'y' : (axis == 2 ? 'z' : '?'))) << ' ' << tree.SplitPos(i) << std::endl;
            i <<= 1; // i 指向 left child //! 由于 splitAxis != 3, 一定存在 children
            ++currDepth;
        }
    }
    return builder.str();
}
#endif

// KDTreeBuilderNode methods
string KDTreeBuilderNode::ToString() const
{
    std::stringstream builder;
    KDTreeBuilderNode_ToString(this, 0, &builder);
    return builder.str();
}
void KDTreeBuilderNode::ToArray(vector<KDTreeBuilderNode*>* array)
{
    Assert(array->size() == 0, 
           "KDTreeBuilderNode::ToArray(): Input array should be empty");
    KDTreeBuilderNode_ToArray(this, array);
}
void KDTreeBuilderNode::ToPrimitivesData(vector<int>* primitivesBegin, vector<int>* numPrimitives) const
{
    Assert(primitivesBegin->size() == 0 && numPrimitives->size() == 0, 
           "KDTreeBuilderNode::ToPrimitivesData(): Input arrays should be empty");
    KDTreeBuilderNode_ToPrimitivesData(this, primitivesBegin, numPrimitives);
}
void KDTreeBuilderNode::FromPrimitivesData(const vector<int>& primitivesBegin, const vector<int>& numPrimitives)
{
    int i = 0;
    KDTreeBuilderNode_FromPrimitivesData(this, primitivesBegin, numPrimitives, &i);
}
int KDTreeBuilderNode::NumTreePrimitives() const
{
    return KDTreeBuilderNode_NumTreePrimitives(this);
}
int KDTreeBuilderNode::NumTreeNodes() const
{
    return KDTreeBuilderNode_NumTreeNodes(this);
}
int KDTreeBuilderNode::Depth() const
{
    return KDTreeBuilderNode_Depth(this, 0);
}
/*static*/ void KDTreeBuilderNode::Delete(const KDTreeBuilderNode* node)
{
    if (node->splitAxis == 3) // Leaf
        delete node;
    else // Interior //! 一支为 nullptr，则另一支必为 nullptr，即为叶子节点
    {
        Delete(node->left);
        Delete(node->righ);
    }
}

/*static*/ KDTreeBuilderNode* KDTreeBuilderNode::Build(
    const vector<shared_ptr<Primitive>>& primitives, 
    const vector<AABound3f>& primitiveBounds, 
    const vector<Point3f>& primitiveCentroids, 
    vector<int>* primitiveIndices, 
    vector<int>* mergedPrimitiveIndices, 
    int maxDepth)
{
    // 计算 boundsBound 和 centroidBound，取 centroidBound 最长轴为 splitAxis
    AABound3f boundsBound, centroidsBound;
    for (int idx : *primitiveIndices)
    {
        boundsBound = AABound3f::Union(boundsBound, primitiveBounds[idx]);
        centroidsBound = AABound3f::Union(centroidsBound, primitiveCentroids[idx]);
    }
    int splitAxis = centroidsBound.MaximumExtent(); // TODO: 这并不是最好的做法

    // 定义 buckets
    constexpr int nBuckets = 12;
    struct BucketInfo
    {
        int count = 0;
        AABound3f bound;
    };
    BucketInfo buckets[nBuckets];

    // 初始化每个 bucket
    for (int idx : *primitiveIndices)
    {
        int b = nBuckets * centroidsBound.Offset(primitiveCentroids[idx])[splitAxis]; // 考虑重心落在 buckets 中的 primitives
        if (b == nBuckets)
            b = nBuckets - 1;
        ++buckets[b].count;
        buckets[b].bound = AABound3f::Union(buckets[b].bound, primitiveBounds[idx]);
    }

    // 计算每个 bucket 的 cost
    Float cost[nBuckets - 1];
    for (int i = 0; i < nBuckets - 1; ++i)
    {
        AABound3f b0, b1;
        int count0 = 0, count1 = 0;
        for (int j = 0; j <= i; ++j)
        {
            b0 = AABound3f::Union(b0, buckets[j].bound);
            count0 += buckets[j].count;
        }
        for (int j = i + 1; j < nBuckets; ++j)
        {
            b1 = AABound3f::Union(b1, buckets[j].bound);
            count1 += buckets[j].count;
        }
        cost[i] = 0.125f + (count0 * b0.Area() + count1 * b1.Area()) / boundsBound.Area(); // TODO: KDTree 不能直接沿用 BVH 的 SAH
    }

    // 获取 cost 最小的 bucket
    Float minCost = cost[0];
    int minCostSplitBucket = 0;
    for (int i = 1; i < nBuckets - 1; ++i)
        if (cost[i] < minCost)
        {
            minCost = cost[i];
            minCostSplitBucket = i;
        }

    Float leafCost = primitiveIndices->size(); // TODO: PBRT 中所有图元的权重都是 1，但这里显然不是如此……
    if ((primitiveIndices->size() <= KDTree::ExpectedNumPrimsPerNode && minCost < leafCost) || maxDepth == 0) // Leaf
    {
        int primitivesBegin = (int)mergedPrimitiveIndices->size();
        std::copy(primitiveIndices->begin(), primitiveIndices->end(), std::back_inserter(*mergedPrimitiveIndices));
        return new KDTreeBuilderNode(
            3, 0, 
            primitivesBegin, primitiveIndices->size(), 
            nullptr, nullptr);
    }
    else // Interior
    {
        // 获取 minCostSplitBucket 对应的 primitive 序号
        auto pMid = std::partition(primitiveIndices->begin(), primitiveIndices->end(), 
            [&](int idx)
            {
                int b = nBuckets * centroidsBound.Offset(primitiveCentroids[idx])[splitAxis];
                if (b == nBuckets)
                    b = nBuckets - 1;
                return b < minCostSplitBucket;
            });

        // 通过序号查找对应的 primitive
        int mid = pMid - primitiveIndices->begin();
        std::nth_element(primitiveIndices->begin(), primitiveIndices->begin() + mid, primitiveIndices->end(), 
            [&](int a, int b)
            {
                return primitiveCentroids[a][splitAxis] < primitiveCentroids[b][splitAxis];
            });
        //! 定义 primitive 的 pMas[axis] + δ 为 splitPos
        float splitPos = (float)(primitiveBounds[(*primitiveIndices)[mid]].pMax[splitAxis]); //! 由于 splitPos 会被扰动，有必要 + δ
        splitPos = splitPos >= 0 ? splitPos * 1.001 : splitPos * 0.999; //! 加法不如乘法好用
        //! 将 splitPos 的最后两位置为 splitAxis
        uint32_t splitPos_i;
        memcpy(&splitPos_i, &splitPos, sizeof(uint32_t)); //! 要求 uint32_t 与 float 尺寸相等
        constexpr uint32_t cutter_lower2 = (std::numeric_limits<uint32_t>::max() >> 2) << 2;
        splitPos_i = (splitPos_i & cutter_lower2) | uint32_t(splitAxis); //! 转换末尾两位
        memcpy(&splitPos, &splitPos_i, sizeof(float)); //! 要求 uint32_t 与 float 尺寸相等

        // 根据 splitPos 计算新的 primitiveIndices
        vector<int> lPrimitiveIndices;
        for (int idx : *primitiveIndices)
            if (primitiveBounds[idx].pMin[splitAxis] <= splitPos)
                lPrimitiveIndices.push_back(idx);
        if (lPrimitiveIndices.size() == primitiveIndices->size()) //! 如果所有节点都和左半边有接触，说明 cut lower 2 发生了浮点数问题，或者数据本来就有问题，一定要直接退出，否则永远无法停止
        {
            int primitivesBegin = (int)mergedPrimitiveIndices->size();
            std::copy(primitiveIndices->begin(), primitiveIndices->end(), std::back_inserter(*mergedPrimitiveIndices));
            return new KDTreeBuilderNode(
                3, 0, 
                primitivesBegin, primitiveIndices->size(), 
                nullptr, nullptr);
        }
        vector<int> rPrimitiveIndices;
        for (int idx : *primitiveIndices)
            if (primitiveBounds[idx].pMax[splitAxis] >= splitPos)
                rPrimitiveIndices.push_back(idx);
        if (rPrimitiveIndices.size() == primitiveIndices->size()) //! 如果所有节点都和右半边有接触，说明 cut lower 2 发生了浮点数问题，或者数据本来就有问题，一定要直接退出，否则永远无法停止
        {
            int primitivesBegin = (int)mergedPrimitiveIndices->size();
            std::copy(primitiveIndices->begin(), primitiveIndices->end(), std::back_inserter(*mergedPrimitiveIndices));
            return new KDTreeBuilderNode(
                3, 0, 
                primitivesBegin, primitiveIndices->size(), 
                nullptr, nullptr);
        }

        // 构造中间节点
        return new KDTreeBuilderNode(
            splitAxis, splitPos, 
            -1, -1, 
            Build(primitives, primitiveBounds, primitiveCentroids, &lPrimitiveIndices, mergedPrimitiveIndices, maxDepth - 1), 
            Build(primitives, primitiveBounds, primitiveCentroids, &rPrimitiveIndices, mergedPrimitiveIndices, maxDepth - 1));
    }
}

void KDTreeBuilderNode::CompressTo(vector<int>* uncompressedPrimitiveIndices, void* tree)
{
    // 初始化检查数组
    vector<int> originaltreePrimitivesBegin;
    vector<int> originalTreeNumPrimitives;
    ToPrimitivesData(&originaltreePrimitivesBegin, &originalTreeNumPrimitives);
    vector<int> primitiveIndices = *uncompressedPrimitiveIndices;
    vector<int> treePrimitivesBegin = originaltreePrimitivesBegin;
    vector<int> treePrimitiveBlocksBegin = originaltreePrimitivesBegin;
    vector<int> treeNumPrimitiveBlocks = originalTreeNumPrimitives;
    vector<vector<int>> intersections;

    // Sort every blocks
    for (size_t i = 0; i < treePrimitiveBlocksBegin.size(); ++i)
        std::sort(primitiveIndices.begin() + treePrimitiveBlocksBegin[i], primitiveIndices.begin() + treePrimitiveBlocksBegin[i] + treeNumPrimitiveBlocks[i]);
    
    // 计算每两个相邻 blocks 的交集
    for (size_t i = 0; i < treePrimitiveBlocksBegin.size() - 1; ++i)
    {
        vector<int> intersection;
        std::set_intersection(primitiveIndices.begin() + treePrimitiveBlocksBegin[i    ], primitiveIndices.begin() + treePrimitiveBlocksBegin[i    ] + treeNumPrimitiveBlocks[i    ], 
                              primitiveIndices.begin() + treePrimitiveBlocksBegin[i + 1], primitiveIndices.begin() + treePrimitiveBlocksBegin[i + 1] + treeNumPrimitiveBlocks[i + 1], 
                              std::back_inserter(intersection));
        intersections.push_back(intersection);
    }

    // Compress indices
    while (true)
    {
        // 找到 intersections 中 size 最大的元素
        size_t maxInterI = -1;
        size_t maxInter = 0;
        for (size_t i = 0; i < intersections.size(); ++i)
            if (intersections[i].size() > maxInter)
            {
                maxInterI = i;
                maxInter = intersections[i].size();
            }
        // 不存在 intersection，则退出
        if (maxInterI == size_t(-1))
            break;
        // 根据 intersection 修改 block
        vector<int> newBlock;
        std::copy(primitiveIndices.begin(), primitiveIndices.begin() + treePrimitiveBlocksBegin[maxInterI], std::back_inserter(newBlock));
        std::set_difference(primitiveIndices.begin() + treePrimitiveBlocksBegin[maxInterI], primitiveIndices.begin() + treePrimitiveBlocksBegin[maxInterI] + treeNumPrimitiveBlocks[maxInterI], 
                            intersections[maxInterI].begin(), intersections[maxInterI].end(), 
                            std::back_inserter(newBlock));
        std::copy(intersections[maxInterI].begin(), intersections[maxInterI].end(), std::back_inserter(newBlock));
        std::set_difference(primitiveIndices.begin() + treePrimitiveBlocksBegin[maxInterI + 1], primitiveIndices.begin() + treePrimitiveBlocksBegin[maxInterI + 1] + treeNumPrimitiveBlocks[maxInterI + 1], 
                            intersections[maxInterI].begin(), intersections[maxInterI].end(), 
                            std::back_inserter(newBlock));
        std::copy(primitiveIndices.begin() + treePrimitiveBlocksBegin[maxInterI + 1] + treeNumPrimitiveBlocks[maxInterI + 1], primitiveIndices.end(), std::back_inserter(newBlock));
        primitiveIndices = newBlock;
        // 根据 intersection 修改 treePrimitiveBlocksBegin 和 treeNumPrimitiveBlocks
        for (size_t i = maxInterI + 1; i < treePrimitiveBlocksBegin.size(); ++i)
        {
            treePrimitivesBegin[i] -= intersections[maxInterI].size();
            if (i > maxInterI + 1)
                treePrimitiveBlocksBegin[i] -= intersections[maxInterI].size();
        }
        treeNumPrimitiveBlocks[maxInterI    ] -= intersections[maxInterI].size();
        treeNumPrimitiveBlocks[maxInterI + 1] -= intersections[maxInterI].size();
        // 对应的 intersection 清空
        intersections[maxInterI].clear();
        // 重新计算两侧 intersections
        if (maxInterI > 0)
        {
            vector<int> newLInter;
            std::set_intersection(primitiveIndices.begin() + treePrimitiveBlocksBegin[maxInterI - 1], primitiveIndices.begin() + treePrimitiveBlocksBegin[maxInterI - 1] + treeNumPrimitiveBlocks[maxInterI - 1], 
                                  primitiveIndices.begin() + treePrimitiveBlocksBegin[maxInterI    ], primitiveIndices.begin() + treePrimitiveBlocksBegin[maxInterI    ] + treeNumPrimitiveBlocks[maxInterI    ], 
                                  std::back_inserter(newLInter));
            intersections[maxInterI - 1] = newLInter;
        }
        if (maxInterI < treePrimitiveBlocksBegin.size() - 1)
        {
            vector<int> newRInter;
            std::set_intersection(primitiveIndices.begin() + treePrimitiveBlocksBegin[maxInterI    ], primitiveIndices.begin() + treePrimitiveBlocksBegin[maxInterI    ] + treeNumPrimitiveBlocks[maxInterI    ], 
                                  primitiveIndices.begin() + treePrimitiveBlocksBegin[maxInterI + 1], primitiveIndices.begin() + treePrimitiveBlocksBegin[maxInterI + 1] + treeNumPrimitiveBlocks[maxInterI + 1], 
                                  std::back_inserter(newRInter));
            intersections[maxInterI + 1] = newRInter;
        }
    }

    // 将压缩应用到 nodes
    FromPrimitivesData(treePrimitivesBegin, originalTreeNumPrimitives);
    //! 再次从 node 中加载数据，预防压缩应用错误
    treePrimitivesBegin.clear();
    treeNumPrimitiveBlocks.clear();
    ToPrimitivesData(&treePrimitivesBegin, &treeNumPrimitiveBlocks);

    //! 检查压缩正确性
    Assert(((originaltreePrimitivesBegin.size() == originalTreeNumPrimitives.size()) == 
           (treePrimitivesBegin.size() == treeNumPrimitiveBlocks.size())), 
           "KDTree::KDTreeBuilderNode::CompressTo(): Size error after compression");
    for (size_t i = 0; i < treePrimitiveBlocksBegin.size(); ++i)
    {
        vector<int> blockUncompressed;
        vector<int> blockCompressed;
        int blockSize_uncompressed = originalTreeNumPrimitives[i];
        int blockSize_compressed = treeNumPrimitiveBlocks[i];
        std::copy(uncompressedPrimitiveIndices->begin() + originaltreePrimitivesBegin[i], uncompressedPrimitiveIndices->begin() + originaltreePrimitivesBegin[i] + blockSize_uncompressed, std::back_inserter(blockUncompressed));
        std::copy(primitiveIndices.begin() + treePrimitivesBegin[i], primitiveIndices.begin() + treePrimitivesBegin[i] + blockSize_compressed, std::back_inserter(blockCompressed));
        std::sort(blockUncompressed.begin(), blockUncompressed.end());
        std::sort(blockCompressed.begin(), blockCompressed.end());
        Assert(std::equal(blockUncompressed.begin(), blockUncompressed.end(), blockCompressed.begin()), 
               "KDTree::KDTreeBuilderNode::CompressTo(): Compression error");
    }

    // 将压缩应用到 primitiveIndices
    *uncompressedPrimitiveIndices = primitiveIndices;

    // Compress
    KDTreeBuilderNode_CompressTo(tree, this, 1);
}



//* KDTree -----------------------------------------------------------------------------------------------------*//

// Constructors
KDTree::KDTree(const vector<shared_ptr<Primitive>>& primitives)
{
    // Check
    Assert(sizeof(float) == 4, 
           "KDTree::KDTree(): Float should be 32 bits");
    // Build
    vector<AABound3f> primitiveWorldBounds(primitives.size());
    for (size_t i = 0; i < primitives.size(); ++i)
        primitiveWorldBounds[i] = primitives[i]->WorldBound();
    vector<Point3f> primitiveCentroids(primitives.size());
    for (size_t i = 0; i < primitives.size(); ++i)
        primitiveCentroids[i] = primitiveWorldBounds[i].Center();
    vector<int> primitiveIndices(primitives.size());
    for (size_t i = 0; i < primitives.size(); ++i)
        primitiveIndices[i] = i;
    vector<int> mergedPrimitiveIndices(0);
    KDTreeBuilderNode* root = KDTreeBuilderNode::Build(primitives, primitiveWorldBounds, primitiveCentroids, &primitiveIndices, &mergedPrimitiveIndices, MaxDepth);
    // 处理 numPrimitives out of bound
    vector<shared_ptr<Primitive>> primitivesExtended = primitives;
    vector<KDTreeBuilderNode*> builderNodes;
    root->ToArray(&builderNodes);
    for (size_t i = 0; i < builderNodes.size(); ++i)
    {
        if (builderNodes[i]->numPrimitives >= KDTree::MaxNumPrimsPerNode)
        {
            // Extend primitives
            vector<shared_ptr<Primitive>> listItems;
            for (auto it = mergedPrimitiveIndices.begin() + builderNodes[i]->primitivesBegin; it != mergedPrimitiveIndices.begin() + builderNodes[i]->primitivesBegin + builderNodes[i]->numPrimitives; ++it)
                listItems.push_back(primitivesExtended[*it]);
            primitivesExtended.push_back(make_shared<PrimitiveList>(listItems));
            // Update merged indices
            vector<int> newMergedPrimitiveIndices;
            std::copy(mergedPrimitiveIndices.begin(), mergedPrimitiveIndices.begin() + builderNodes[i]->primitivesBegin, std::back_inserter(newMergedPrimitiveIndices));
            newMergedPrimitiveIndices.push_back(primitivesExtended.size() - 1);
            if (i < builderNodes.size() - 1)
                std::copy(mergedPrimitiveIndices.begin() + builderNodes[i + 1]->primitivesBegin, mergedPrimitiveIndices.end(), std::back_inserter(newMergedPrimitiveIndices));
            mergedPrimitiveIndices = newMergedPrimitiveIndices;
            // Update nodes
            builderNodes[i]->numPrimitives = 1;
            for (size_t k = i + 1; k < builderNodes.size(); ++k)
                builderNodes[k]->primitivesBegin -= listItems.size() - 1;
        }
    }
    // Allocate memory for compressed tree
    depth = root->Depth();
    numPrimitives = primitives.size();
    numNodes = root->NumTreeNodes();
    int treeSize = std::pow(2, depth + 1);
    tree = malloc(treeSize * 4);
    memset(tree, 0, treeSize * 4);
    // Compress
    root->CompressTo(&mergedPrimitiveIndices, tree);
    // 根据 indices 排序 primitives
    this->primitives.resize(mergedPrimitiveIndices.size());
    for (size_t i = 0; i < mergedPrimitiveIndices.size(); ++i)
        this->primitives[i] = primitivesExtended[mergedPrimitiveIndices[i]];
    // Free builder
    Assert(KDTree_BuildingDataToString(*this).compare(root->ToString()) == 0, //! 有必要保证一定是无损压缩，通过比较字符串来确定
           "KDTree::KDTree(): Compression error");
    KDTreeBuilderNode::Delete(root);
    // 计算 whole world bound
    for (const AABound3f& bound : primitiveWorldBounds)
        worldBound = AABound3f::Union(worldBound, bound);
}
/*override*/ KDTree::~KDTree()
{
    free(tree);
}

// Methods
/*override*/ bool KDTree::Intersect(const Ray& ray, Float* tHit, SurfaceInteraction* si) const
{
    Float tMin, tMax;
    if (!worldBound.Intersect(ray, &tMin, &tMax)) //! 规范起见，不允许调用 WorldBound()，而应该直接使用 worldBound
        return false;
    Vector3f rayDInv(1 / ray.d.x, 1 / ray.d.y, 1 / ray.d.z);
    bool isNonPositiveAxis[3] = { ray.d.x <= 0, ray.d.y <= 0, ray.d.z <= 0 };
    struct KDTreeTask
    {
        int idx;
        Float tMin;
        Float tMax;
    };
    KDTreeTask* tasks = new KDTreeTask[depth];
    int iTasks = 0;
    int i = 1;

    bool isHit = false;
    while (true)
    {
        if (ray.tMax < tMin)
            break;
        int axis = SplitAxis(i);
        if (axis == 3) // Leaf
        {
            int primitivesBegin = PrimitivesBegin(i);
            int numPrimitives = NumPrimitives(i);
            for (auto ptr = primitives.begin() + primitivesBegin; ptr != primitives.begin() + primitivesBegin + numPrimitives; ++ptr)
            {
                if ((*ptr)->Intersect(ray, tHit, si))
                    isHit = true;
            }
            if (iTasks > 0)
            {
                --iTasks;
                i = tasks[iTasks].idx;
                tMin = tasks[iTasks].tMin;
                tMax = tasks[iTasks].tMax;
            }
            else
                break;
        }
        else // Interior
        {
            Float splitPos = (Float)SplitPos(i);
            Float tSplit = (splitPos - ray.o[axis]) * rayDInv[axis];

            int firstChild, secondChild;
            bool belowfirst = (ray.o[axis]  < splitPos) || 
                              (ray.o[axis] == splitPos && isNonPositiveAxis[axis]);
            int leftChild = i << 1;
            int rightChild = leftChild + 1;
            if (belowfirst)
            {
                firstChild = leftChild;
                secondChild = rightChild;
            }
            else
            {
                secondChild = leftChild;
                firstChild = rightChild;
            }

            if (tSplit > tMax || tSplit <= 0)
                i = firstChild;
            else if (tSplit < tMin)
                i = secondChild;
            else
            {
                tasks[iTasks].idx = secondChild;
                tasks[iTasks].tMin = tSplit;
                tasks[iTasks].tMax = tMax;
                ++iTasks;
                i = firstChild;
                tMax = tSplit;
            }
        }
    }
    delete[] tasks;
    return isHit;
}
/*override*/ AABound3f KDTree::WorldBound() const
{
    return worldBound;
}

// Operators
/*friend*/ std::ostream& /*KDTree::*/operator<<(std::ostream& out, const KDTree& t)
{
    Point3f pMin = t.worldBound.pMin;
    Point3f pMax = t.worldBound.pMax;
    struct KDTreeTask
    {
        int splitAxis;
        Float pMin;
        Float pMax;
    };
    KDTreeTask* tasks = new KDTreeTask[t.depth];
    int iTasks = 0;
    int i = 1;
    int currDepth = 0;
    out << "KDTree Accelerator:" << std::endl;
    out << "    numPrims=" << t.numPrimitives << std::endl; 
    out << "    numNodes=" << t.numNodes << std::endl;
    out << "    depth   =" << t.depth << std::endl;
    out << "    world   =" << '[' << t.worldBound.pMin.x << '~' << t.worldBound.pMax.x << ", " << t.worldBound.pMin.y << '~' << t.worldBound.pMax.y << ", " << t.worldBound.pMin.z << '~' << t.worldBound.pMax.z << ']' << std::endl;
    while (true)
    {
        out << "    ";
        for (int d = 0; d < currDepth; ++d)
            out << "| ";
        // 检查是 interior 还是 leaf
        if (t.SplitAxis(i) == 3) // Leaf
        {
            AABound3f boundsBound;
            for (auto it = t.primitives.begin() + t.PrimitivesBegin(i); it != t.primitives.begin() + t.PrimitivesBegin(i) + t.NumPrimitives(i); ++it)
                boundsBound = AABound3f::Union(boundsBound, (*it)->WorldBound());
            out << '[' << t.PrimitivesBegin(i) << ' ' << t.PrimitivesBegin(i) + t.NumPrimitives(i) << ')' << "    " 
                << "split=" << '[' << pMin.x << '~' << pMax.x << ", " << pMin.y << '~' << pMax.y << ", " << pMin.z << '~' << pMax.z << ']' << "    "
                << "prims=" << '[' << boundsBound.pMin.x << '~' << boundsBound.pMax.x << ", " << boundsBound.pMin.y << '~' << boundsBound.pMax.y << ", " << boundsBound.pMin.z << '~' << boundsBound.pMax.z << ']' << std::endl;
            if ((i & 1) == 0) // 是左叶子
            {
                ++i; // i 指向右叶子
                int axis = tasks[iTasks - 1].splitAxis;
                pMin[axis] = pMax[axis];
                pMax[axis] = tasks[iTasks - 1].pMax;
            }
            else // 是右叶子
            {
                do
                {
                    i >>= 1;
                    --currDepth;
                    --iTasks;
                    pMin[tasks[iTasks].splitAxis] = tasks[iTasks].pMin;
                    pMax[tasks[iTasks].splitAxis] = tasks[iTasks].pMax;
                    if (currDepth == -1) //! 不存在非右节点，则退出
                    {
                        delete[] tasks;
                    }
                } while ((i & 1) == 1);
                ++i;
                int axis = tasks[iTasks - 1].splitAxis;
                pMin[axis] = pMax[axis];
                pMax[axis] = tasks[iTasks - 1].pMax;
            }
        }
        else // Interior
        {
            int axis = t.SplitAxis(i);
            out << (axis == 0 ? 'x' : (axis == 1 ? 'y' : (axis == 2 ? 'z' : '?'))) << ' ' << t.SplitPos(i) << "    " 
                << "split=" << '[' << pMin.x << '~' << pMax.x << ", " << pMin.y << '~' << pMax.y << ", " << pMin.z << '~' << pMax.z << ']' << std::endl;
            tasks[iTasks].splitAxis = axis;
            tasks[iTasks].pMin = pMin[axis];
            tasks[iTasks].pMax = pMax[axis];
            ++iTasks;
            pMax[axis] = t.SplitPos(i);
            i <<= 1; // i 指向 left child //! 由于 splitAxis != 3, 一定存在 children
            ++currDepth;
        }
    }
    delete[] tasks;
    return out;
}