#include <gridAccelMesh.h>

// Constructors
GridAccelMesh::GridAccelMesh(
        const Transform* meshToWorld, const Transform* worldToMesh, 
        const shared_ptr<const vector<Point3f>>& vertexP, const shared_ptr<const vector<Point3i>>& vertexPIndices, 
        const shared_ptr<const vector<Normal3f>>& vertexNg, const shared_ptr<const vector<Point3i>>& vertexNgIndices, 
        Point3i gridDepth, 
        Float meshToGridScale, Vector3f meshToGridTranslation) : 
    Mesh(meshToWorld, worldToMesh, 
         vertexP, vertexPIndices, 
         vertexNg, vertexNgIndices, 
         gridDepth, 
         meshToGridScale, meshToGridTranslation), 
    // Fields
    #ifdef STAR_GRID
    grid(new vector<int>***[gridDepth.x])
    #else
    grid(new vector<int>*[gridDepth.x * gridDepth.y * (gridDepth.z >> 3)]), 
    gridShifter_blockDim_Z(GetOrderPower2(gridDepth.z >> 3)), 
    gridShifter_blockDim_YZ(GetOrderPower2(gridDepth.y) + gridShifter_blockDim_Z)
    #endif
{
    // 初始化 grid，填满 nullptr
    #ifdef STAR_GRID
    for (int x = 0; x < gridDepth.x; ++x)
    {
        grid[x] = new vector<int>**[gridDepth.x];
        for (int y = 0; y < gridDepth.y; ++y)
        {
            grid[x][y] = new vector<int>*[gridDepth.z >> 3];
            for (int z = 0; z < (gridDepth.z >> 3); ++z)
                grid[x][y][z] = nullptr;
        }
    }
    #else
    int numBlocks = gridDepth.x * gridDepth.y * (gridDepth.z >> 3);
    Assert(numBlocks > 0, 
           "GridAccelMesh::GridAccelMesh(): Int overflow");
    for (int i = 0; i < numBlocks; ++i)
        grid[i] = nullptr;
    #endif
    // 构造 grid
    for (size_t i = 0; i < vertexPIndices->size(); ++i) // 对于每个三角形
    {
        // 通过 index 获取顶点坐标
        Point3f p0((*vertexP)[(*vertexPIndices)[i].x]);
        Point3f p1((*vertexP)[(*vertexPIndices)[i].y]);
        Point3f p2((*vertexP)[(*vertexPIndices)[i].z]);
        // 检查合法性
        Assert(GridIsInside(p0) && GridIsInside(p1) && GridIsInside(p2), 
               "GridAccelMesh::GridAccelMesh(): Triangle out of bound");
        // 获取碰撞检测信息
        Point3f pc((p0 + p1 + p2) / 3); // 质心
        Point3f p0_relaxed(p0 + (p0 - pc) * 0.001); //! Relax
        Point3f p1_relaxed(p1 + (p1 - pc) * 0.001);
        Point3f p2_relaxed(p2 + (p2 - pc) * 0.001);
        Normal3f normal(Vector3f::Cross(p1 - p0, p2 - p0).Normalized() * 
            0.001 * Max(Point3f::Distance(p0, p1), Point3f::Distance(p1, p2), Point3f::Distance(p2, p0))); // Relax
        Point3f p0n_relaxed(p0_relaxed + normal);
        Point3f p1n_relaxed(p1_relaxed + normal);
        Point3f p2n_relaxed(p2_relaxed + normal);
        Point3f p0nNeg_relaxed(p0_relaxed - normal);
        Point3f p1nNeg_relaxed(p1_relaxed - normal);
        Point3f p2nNeg_relaxed(p2_relaxed - normal);
        // 筛选有必要参与碰撞检测的长方体
        #ifndef NDEBUG
        bool isAdded = false;
        #endif
        AABound3f triangleBound;
        triangleBound = AABound3f::Union(triangleBound, p0);
        triangleBound = AABound3f::Union(triangleBound, p1);
        triangleBound = AABound3f::Union(triangleBound, p2);
        int infX = std::clamp(int(triangleBound.pMin.x - 1), 0, gridDepth.x - 1);
        int infY = std::clamp(int(triangleBound.pMin.y - 1), 0, gridDepth.y - 1);
        int infZ = std::clamp(int(triangleBound.pMin.z - 1), 0, gridDepth.z - 1);
        int supX = std::clamp(int(triangleBound.pMax.x + 2), 0, gridDepth.x - 1);
        int supY = std::clamp(int(triangleBound.pMax.y + 2), 0, gridDepth.y - 1);
        int supZ = std::clamp(int(triangleBound.pMax.z + 2), 0, gridDepth.z - 1);
        // 通过三角形与长方体碰撞检测，将三角形塞进对应的 bounds 中
        for (int x = infX; x <= supX; ++x)
            for (int y = infY; y <= supY; ++y)
                for (int z = infZ; z <= supZ; ++z)
                {
                    // 获取 x, y, z 对应的 AABB
                    AABound3f bound = GridGetBlockBound_Relaxed(x, y, z);
                    // 碰撞检测
                    if (bound.Intersect(p0_relaxed, p1_relaxed, p2_relaxed) || 
                        bound.Intersect(p0n_relaxed, p1n_relaxed, p2n_relaxed) || 
                        bound.Intersect(p0nNeg_relaxed, p1nNeg_relaxed, p2nNeg_relaxed))
                    {
                        #ifndef NDEBUG
                        isAdded = true;
                        #endif
                        // Add to grid
                        #ifdef STAR_GRID
                        if (grid[x][y][z >> 3] == nullptr)
                            grid[x][y][z >> 3] = new vector<int>[8];
                        grid[x][y][z >> 3][z & 0b111].push_back(i);
                        #else
                        int gridIdx = GridGetGridIdx(x, y, z);
                        if (grid[gridIdx] == nullptr)
                            grid[gridIdx] = new vector<int>[8];
                        grid[gridIdx][z & 0b111].push_back(i);
                        #endif
                    }
                }
        Assert(isAdded, 
                "GridAccelMesh::GridAccelMesh(): Failed to add the triangle");
    }
}

GridAccelMesh::~GridAccelMesh()
{
    #ifdef STAR_GRID
    for (int x = 0; x < gridDepth.x; ++x)
    {
        for (int y = 0; y < gridDepth.y; ++y)
        {
            for (int z = 0; z < gridDepth.z; ++z)
                if (grid[x][y][z] != nullptr)
                    delete[] grid[x][y][z];
            delete[] grid[x][y];
        }
        delete[] grid[x];
    }
    delete[] grid;
    #else
    int numBlocks = gridDepth.x * gridDepth.y * (gridDepth.z >> 3);
    for (int i = 0; i < numBlocks; ++i)
        delete[] grid[i];
    delete[] grid;
    #endif
}

// Methods
/*override*/ bool GridAccelMesh::Intersect(const Ray& ray, Float* tHit, SurfaceInteraction* si) const
{
    // 将射线从 world coordinate 变换到 mesh/object coordinate
    Ray rayGrid = (*worldToObject)(ray);
    // 将射线从 mesh coordinate 变换到 grid coordinate
    rayGrid.o *= meshToGridScale;
    rayGrid.o += meshToGridTranslation;
    rayGrid.d *= meshToGridScale;
    // 检测最外侧 bound
    AABound3f gridBound(Point3f(0, 0, 0), Point3f(gridDepth.x, gridDepth.y, gridDepth.z));
    Float tGridHit;
    if (std::isnan(rayGrid.tMax) || !gridBound.Intersect(rayGrid, &tGridHit)) //! 当 rayGrid.tMax is nan 时，intersect 永远会成功
        return false; //! 与 AABB 的 intersection 不会改变 rayGrid 的 tMax
    // 计算 rayGrid 与最外侧 bound 的交点
    Point3f pGridHit(rayGrid(tGridHit));
    pGridHit = Point3f( // Clamp to gridBound
        std::clamp(pGridHit.x, Float(0), Float(gridDepth.x)), 
        std::clamp(pGridHit.y, Float(0), Float(gridDepth.y)), 
        std::clamp(pGridHit.z, Float(0), Float(gridDepth.z)));
    // 计算 stride / rayGrid.d，取最小一轴定义 check point 的更新步长
    //! 在 gridGridAccel 中，stride/strideInv 总是 (1, 1, 1)
    Float speedScale = 1 / std::abs(rayGrid.d[rayGrid.d.MaximumAbsExtent()]) - Epsilonx1000; //! Relax
    Vector3f rayStep(rayGrid.d * speedScale);
    Point3f checkPoint(pGridHit); // 初始化 checkPoint 为与 gridBound 的接触点
    // 循环检查 check points
    for (;; checkPoint = Point3f(checkPoint + rayStep))
    {
        // 循环终止条件：检查到底都没找到
        int blockIdxX, blockIdxY, blockIdxZ;
        if (!GridIsInside(checkPoint, &blockIdxX, &blockIdxY, &blockIdxZ))
            return false;
        // Inside，则检查 checkPoint 对应的 bound 中是否有 triangles，没有则直接跳过
        #ifdef STAR_GRID
        if (grid[blockIdxX][blockIdxY][blockIdxZ >> 3] == nullptr)
            continue;
        #else
        int gridIdx = GridGetGridIdx(blockIdxX, blockIdxY, blockIdxZ);
        if (grid[gridIdx] == nullptr)
            continue;
        #endif
        // 如果 bound 中有 triangles，检查所有 triangles
        #ifdef STAR_GRID
        const vector<int>& triangles = grid[blockIdxX][blockIdxY][blockIdxZ >> 3][blockIdxZ & 0b111];
        #else
        const vector<int>& triangles = grid[gridIdx][blockIdxZ & 0b111];
        #endif
        //! 如果射线检测成功才 return true，否则还得 continue
        bool isHitted = false;
        for (int triangleIdx : triangles)
        {
            // 通过 index 获取顶点坐标
            Point3f p0 = (*vertexP)[(*vertexPIndices)[triangleIdx].x];
            Point3f p1 = (*vertexP)[(*vertexPIndices)[triangleIdx].y];
            Point3f p2 = (*vertexP)[(*vertexPIndices)[triangleIdx].z];
            // Ray cast 检测
            Normal3f nPlane = Normal3f(Normal3f::Cross(p1 - p0, p2 - p0));
            Float nPlane_dot_d = Normal3f::Dot(nPlane, rayGrid.d);
            Float t = (Normal3f::Dot(nPlane, p0) - Normal3f::Dot(nPlane, rayGrid.o)) / nPlane_dot_d;
            if (t < 0 || t > rayGrid.tMax) // 不在 t 范围内
                continue;
            Point3f p = rayGrid(t);
            Vector3f r  = Vector3f(p  - p0);
            Vector3f q1 = Vector3f(p1 - p0);
            Vector3f q2 = Vector3f(p2 - p0);
            Float q1q1 = q1.NormSquared();
            Float q2q2 = q2.NormSquared();
            Float q1q2 = Vector3f::Dot(q1, q2);
            Float rq1  = Vector3f::Dot(r , q1);
            Float rq2  = Vector3f::Dot(r , q2);
            Float detInv = Float(1) / (q1q1 * q2q2 - q1q2 * q1q2);
            Float w1 = detInv * (rq1 * q2q2 - rq2 * q1q2);
            Float w2 = detInv * (rq2 * q1q1 - rq1 * q1q2);
            Float w0 = 1 - w1 - w2;
            if (w0 < 0 || w1 < 0 || w2 < 0) // 在 t 范围内，但不在约束平面内
                continue;
            // 获取法向量
            Normal3f ng0 = (*vertexNg)[(*vertexNgIndices)[triangleIdx].x];
            Normal3f ng1 = (*vertexNg)[(*vertexNgIndices)[triangleIdx].y];
            Normal3f ng2 = (*vertexNg)[(*vertexNgIndices)[triangleIdx].z];
            // Status
            isHitted = true;
            // Grid coordinate ray
            rayGrid.tMax = t;
            // Grid coordinate si
            si->p = p;
            si->ng = Normal3f(w0 * ng0 + w1 * ng1 + w2 * ng2);
            si->sg = Vector3f(Normal3f::Cross(si->ng, (q1q1 < q2q2 ? q2 : q1)));
            // TODO: Mesh 的 uv 怎么办？
            // TODO: Mesh 的 sg, tg 怎么办？
        }
        if (isHitted)
        {
            // Ray mutable
            ray.tMax = *tHit = rayGrid.tMax;
            // Grid coordinate si
            si->woWorld = Vector3f(-rayGrid.d);
            // TODO: Mesh 的 uv 怎么办？
            // TODO: Mesh 的 sg, tg 怎么办？
            // Grid to Mesh coordinate si
            si->p -= meshToGridTranslation;
            si->p *= meshToGridScaleInv; //! 乘法加速
            // Mesh to world coordinate si
            *si = (*objectToWorld)(*si);
            return true;
        }
        //! else: continue to 下一个 check point
    }
    return false;
}
