#include <bvh.h>

//* Definitions ------------------------------------------------------------------------------------------------*//

struct PrimitiveInfo
{
    int idx;
    AABound3f bound;
    uint32_t mortonCode;
};

//* BVHBuilderNode ---------------------------------------------------------------------------------------------*//

struct BVHBuilderNode
{
    // Fields
    int primitivesBegin;
    int numPrimitives;
    AABound3f bound;
    BVHBuilderNode* left;
    BVHBuilderNode* right;

    // Methods
    string ToString();
};

static void BVHBuilderNode_ToString(const BVHBuilderNode* node, int depth, std::stringstream* builder)
{
    for (int d = 0; d < depth; ++d)
        *builder << "| ";
    if (node->left == nullptr || node->right == nullptr) // Leaf
    {
        *builder << '[' << node->primitivesBegin << ' ' << node->primitivesBegin + node->numPrimitives << "): " 
                 << '[' << node->bound.pMin.x << '~' << node->bound.pMax.x << ", " << node->bound.pMin.y << '~' << node->bound.pMax.y << ", " << node->bound.pMin.z << '~' << node->bound.pMax.z << ']' << std::endl;
    }
    else // Interior
    {
        *builder << '[' << node->bound.pMin.x << '~' << node->bound.pMax.x << ", " << node->bound.pMin.y << '~' << node->bound.pMax.y << ", " << node->bound.pMin.z << '~' << node->bound.pMax.z << ']' << std::endl;
        BVHBuilderNode_ToString(node->left, depth + 1, builder); //! 一支为 nullptr，则另一支必为 nullptr，即为叶子节点
        BVHBuilderNode_ToString(node->right, depth + 1, builder);
    }
}
string BVHBuilderNode::ToString()
{
    std::stringstream builder;
    BVHBuilderNode_ToString(this, 0, &builder);
    return builder.str();
}

static int SplitAt(const vector<PrimitiveInfo>& sortedPrimitiveInfos, int first, int last)
{
    uint32_t firstCode = sortedPrimitiveInfos[first].mortonCode;
    uint32_t lastCode = sortedPrimitiveInfos[last].mortonCode;
    if (firstCode == lastCode)
        return (first + last) >> 1;
    
    // 计算 firstCode 和 lastCode 从最高位开始有几位相同
    int commonPrefix = __builtin_clz(firstCode ^ lastCode);

    // 通过 binary search 寻找第一个发生变化的位置
    // Eg: 00001  00010  00100  00101
    //     commonPrefix = 2
    //     split at 00010
    //         because __clz(00010, 00001) == 3  > commonPrefix
    //                 __clz(00100, 00001) == 2 == commonPrefix
    int splitAt = first; // Initial guess
    int step = last - first;
    do
    {
        step = (step + 1) >> 1; // Exponential decrease
        int newSplit = splitAt + step; // Proposed new position
        if (newSplit < last)
        {
            uint32_t splitCode = sortedPrimitiveInfos[newSplit].mortonCode;
            int splitPrefix = __builtin_clz(firstCode ^ splitCode);
            if (splitPrefix > commonPrefix)
                splitAt = newSplit;
        }
    } while (step > 1);
    return splitAt;
}

static BVHBuilderNode* Build(vector<shared_ptr<Primitive>>* primitives, int* numNodes)
{
    vector<PrimitiveInfo> primitiveInfos(primitives->size());
    vector<Point3f> primitiveCentroids(primitives->size());
    AABound3f wholeBound;

    // Compute bound, centroid, wholeBound
    for (int i = 0; i < (int)primitives->size(); ++i)
    {
        primitiveInfos[i].idx = i;
        primitiveInfos[i].bound = primitives->operator[](i)->WorldBound();
        primitiveCentroids[i] = primitiveInfos[i].bound.Center();
        wholeBound = AABound3f::Union(wholeBound, primitiveCentroids[i]);
    }

    // Compute mortonCode
    for (int i = 0; i < (int)primitives->size(); ++i)
    {
        constexpr int mortonBits = 10;
        constexpr int mortonScale = 1 << mortonBits;
        Vector3f centroidOffset = wholeBound.Offset(primitiveCentroids[i]);
        Vector3f mortonEvaluator(centroidOffset * mortonScale);
        primitiveInfos[i].mortonCode = EncodeMorton3(mortonEvaluator.x, mortonEvaluator.y, mortonEvaluator.z);
    }

    // Sort primitive informations by morton code
    std::sort(primitiveInfos.begin(), primitiveInfos.end(), 
        [](const PrimitiveInfo& a, const PrimitiveInfo& b) { return a.mortonCode < b.mortonCode; });
    
    // Reorder primitives by sorted primitive informations
    vector<shared_ptr<Primitive>> sortedPrimitives(primitiveInfos.size());
    for (int i = 0; i < (int)primitiveInfos.size(); ++i)
        sortedPrimitives[i] = primitives->operator[](primitiveInfos[i].idx);
    *primitives = sortedPrimitives;
    
    // Build
    *numNodes = 0;
    BVHBuilderNode* stack[64];
    int iStack = 0;
    BVHBuilderNode* root = new BVHBuilderNode();
    root->primitivesBegin = 0;
    root->numPrimitives = (int)primitiveInfos.size();
    root->bound = AABound3f();
    root->left = nullptr;
    root->right = nullptr;
    stack[iStack] = root;
    while (true)
    {
        BVHBuilderNode* curr = stack[iStack];
        if (curr->numPrimitives <= BVH::ExpectedNumPrimsPerNode) // Leaf
        {
            ++(*numNodes);
            // Compute bound
            AABound3f bound;
            for (int i = curr->primitivesBegin; i < curr->primitivesBegin + curr->numPrimitives; ++i)
                bound = AABound3f::Union(bound, primitiveInfos[i].bound);
            curr->bound = bound;
            // Stack
            --iStack;
            if (iStack < 0)
                break;
        }
        else // Interior
        {
            if (curr->left == nullptr)
            {
                // Build interior
                curr->left = new BVHBuilderNode();
                curr->left->primitivesBegin = curr->primitivesBegin;
                curr->left->numPrimitives = SplitAt(primitiveInfos, curr->primitivesBegin, curr->primitivesBegin + curr->numPrimitives - 1) - curr->primitivesBegin + 1;
                curr->left->bound = AABound3f();
                curr->left->left = nullptr;
                curr->left->right = nullptr;
                // Stack
                stack[++iStack] = curr->left;
            }
            else if (curr->right == nullptr)
            {
                // Bound
                curr->bound = AABound3f::Union(curr->bound, curr->left->bound);
                // Build interior
                curr->right = new BVHBuilderNode();
                // ++(*numNodes);
                curr->right->primitivesBegin = curr->left->primitivesBegin + curr->left->numPrimitives;
                curr->right->numPrimitives = curr->primitivesBegin + curr->numPrimitives - curr->right->primitivesBegin;
                curr->right->bound = AABound3f();
                curr->right->left = nullptr;
                curr->right->right = nullptr;
                // Stack
                stack[++iStack] = curr->right;
            }
            else
            {
                ++(*numNodes);
                // Bound
                curr->bound = AABound3f::Union(curr->bound, curr->right->bound);
                // Stack
                --iStack;
                if (iStack < 0)
                    break;
            }
        }
    }
    return root;
}

//* BVHNode ----------------------------------------------------------------------------------------------------*//

struct BVHNode
{
    // Fields
    AABound3f bound;
    uint32_t nodeData;

    // Methods
    inline int PrimitivesBegin() const
    {
        return int(nodeData >> 8);
    }
    inline int SecondChildIdx() const
    {
        return int(nodeData >> 8);
    }
    inline int NumPrimitives() const
    {
        return int(nodeData & 255);
    }
};

static void Compress(BVHNode* nodes, const BVHBuilderNode* root)
{
    struct ToVisitInfo
    {
        const BVHBuilderNode* father;
        const BVHBuilderNode* son;
        int fatherOffset;
    };
    ToVisitInfo stack[64];
    int iStack = 0;
    const BVHBuilderNode* currFather = nullptr;
    const BVHBuilderNode* curr = root;
    int currFatherOffset = -1;
    bool isRight = false;
    int offset = 0;
    while (true)
    {
        if (curr->left == nullptr || curr->right == nullptr) // Leaf
        {
            //! 必要的检查
            if (curr->left || curr->right || 
                curr->primitivesBegin >= (1 << 24) || curr->primitivesBegin < 0 || 
                curr->numPrimitives >= (1 << 8) || curr->numPrimitives < 0)
            {
                Error("BVH::Compress(): Failed to compress");
                abort();
            }
            // Compress curr
            BVHNode* linearNode = nodes + offset;
            linearNode->bound = curr->bound;
            linearNode->nodeData = uint32_t((curr->primitivesBegin << 8) + (curr->numPrimitives));
            // Compress father
            if (isRight)
            {
                //! 必要的检查
                if (offset >= (1 << 24) || currFather->primitivesBegin < 0)
                {
                    Error("BVH::Compress(): Failed to compress");
                    abort();
                }
                BVHNode* linearFather = nodes + currFatherOffset;
                linearFather->bound = currFather->bound;
                linearFather->nodeData = uint32_t(offset << 8);
            }
            // Stack
            if (iStack == 0)
                break;
            --iStack;
            currFather = stack[iStack].father;
            curr = stack[iStack].son;
            currFatherOffset = stack[iStack].fatherOffset;
            isRight = true;
            ++offset;
        }
        else // Interior
        {
            //! 必要的检查
            if (!curr->left || !curr->right || 
                curr->primitivesBegin >= (1 << 24) || curr->primitivesBegin < 0 || 
                curr->numPrimitives < 0)
            {
                Error("BVH::Compress(): Failed to compress");
                abort();
            }
            if (isRight)
            {
                //! 必要的检查
                if (offset >= (1 << 24) || currFather->primitivesBegin < 0)
                {
                    Error("BVH::Compress(): Failed to compress");
                    abort();
                }
                BVHNode* linearFather = nodes + currFatherOffset;
                linearFather->bound = currFather->bound;
                linearFather->nodeData = uint32_t(offset << 8);
            }
            // Stack
            stack[iStack] = { curr, curr->right, offset };
            ++iStack;
            // Point to left
            currFather = curr;
            curr = curr->left;
            currFatherOffset = offset;
            isRight = false;
            ++offset;
        }
    }
}

//* BVH --------------------------------------------------------------------------------------------------------*//

// Constructors
BVH::BVH(const vector<shared_ptr<Primitive>>& primitives) : 
    primitives(primitives)
{
    BVHBuilderNode* root = Build(&(this->primitives), &numNodes);
    nodes = new BVHNode[numNodes];
    Compress((BVHNode*)nodes, root);
}
/*override*/ BVH::~BVH()
{
    delete[] (BVHNode*)nodes;
}

// Methods
/*override*/ bool BVH::Intersect(const Ray& ray, Float* tHit, SurfaceInteraction* si) const
{
    bool isHit = false;
    Vector3f dirInv( 1 / ray.d.x, 1 / ray.d.y, 1 / ray.d.z);
    int isDirNeg[3] = { ray.d.x < 0, ray.d.y < 0, ray.d.z < 0 };
    int toVisit[64];
    int iToVisit = 0;
    int iCurr = 0;
    while (true)
    {
        const BVHNode* node = (BVHNode*)nodes + iCurr;
        // Error(node->bound.pMin, node->bound.pMax);
        if (node->bound.Intersect(ray, dirInv, isDirNeg)) // Has interaction with bound
        {
            int numPrimitives = node->NumPrimitives();
            if (numPrimitives > 0) // Leaf
            {
                int primitiveBegin = node->PrimitivesBegin();
                int iEnd = node->PrimitivesBegin() + numPrimitives;
                for (int i = primitiveBegin; i < iEnd; ++i)
                {
                    if (primitives[i]->Intersect(ray, tHit, si))
                        isHit = true;
                }
                if (iToVisit == 0)
                    break;
                iCurr = toVisit[--iToVisit];
            }
            else // Interior
            {
                toVisit[iToVisit++] = node->SecondChildIdx();
                ++iCurr;
            }
        }
        else // No interaction with bound
        {
            if (iToVisit == 0)
                break;
            iCurr = toVisit[--iToVisit];
        }
    }
    return isHit;
}

/*override*/ AABound3f BVH::WorldBound() const
{
    // TODO: 
    NotImplemented();
    return AABound3f();
}