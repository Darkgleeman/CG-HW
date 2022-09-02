#include <bitGridAccelMesh.h>

// Shared fields
/*static*/ vector<uint8_t*> BitGridAccelMesh::shared_grid;
/*static*/ vector<vector<int>**> BitGridAccelMesh::shared_gridTriangles;

// Constructors
BitGridAccelMesh::BitGridAccelMesh(
        const Transform* meshToWorld, const Transform* worldToMesh, 
        const shared_ptr<const vector<Point3f>>& vertexP, const shared_ptr<const vector<Point3i>>& vertexPIndices, 
        const shared_ptr<const vector<Normal3f>>& vertexNg, const shared_ptr<const vector<Point3i>>& vertexNgIndices, 
        const Point3i& gridDepth, 
        Float meshToGridScale, const Vector3f& meshToGridTranslation, 
        const uint8_t* grid, vector<int>** gridTriangles) : 
    Mesh(meshToWorld, worldToMesh, 
         vertexP, vertexPIndices, 
         vertexNg, vertexNgIndices, 
         gridDepth, 
         meshToGridScale, meshToGridTranslation), 
    // Fields
    grid(grid), 
    gridTriangles(gridTriangles), 
    // Supporting fields
    gridShifter_gridDepth_Y(GetOrderPower2(gridDepth.y)), 
    gridShifter_gridDepth_Z(GetOrderPower2(gridDepth.z)), 
    gridShifter_gridDepth_YZ(gridShifter_gridDepth_Y + gridShifter_gridDepth_Z)
{ }

BitGridAccelMesh::~BitGridAccelMesh()
{
    //! 删除 shared fields，只需要一个 mesh instance 调用
    for (size_t s = 0; s < shared_grid.size(); ++s)
    {
        delete[] shared_grid[s];
        shared_grid[s] = nullptr;
    }
    shared_grid.resize(0);
    int numBits = gridDepth.x * gridDepth.y * gridDepth.z;
    for (size_t s = 0; s < shared_gridTriangles.size(); ++s)
    {
        for (int i = 0; i < numBits; ++i)
            if (shared_gridTriangles[s][i] != nullptr)
            {
                delete shared_gridTriangles[s][i];
                shared_gridTriangles[s][i] = nullptr;
            }
        delete[] shared_gridTriangles[s];
        shared_gridTriangles[s] = nullptr;
    }
    shared_gridTriangles.resize(0);
}

// Methods
/*static*/ void BitGridAccelMesh::LoadFromFile(
    const string& path, int maxGridDepthPerDim, MeshFileType fileType, MeshLoadFlag loadFlags, 
    shared_ptr<vector<Point3f>>* vertexP, shared_ptr<vector<Point3i>>* vertexPIndices, 
    shared_ptr<vector<Normal3f>>* vertexNg, shared_ptr<vector<Point3i>>* vertexNgIndices, 
    Point3i* gridDepth, 
    Float* meshToGridScale, Vector3f* meshToGridTranslation, 
    uint8_t** grid, vector<int>*** gridTriangles)
{
    MeshLoadFlag loadFlagsToFile = MeshLoadFlag((int)loadFlags | (int)MeshLoadFlag::ToFile);
    // Mesh::LoadFromFile
    Mesh::LoadFromFile(
        path, maxGridDepthPerDim, fileType, loadFlagsToFile, 
        vertexP, vertexPIndices, 
        vertexNg, vertexNgIndices, 
        gridDepth, 
        meshToGridScale, meshToGridTranslation);
    // BitGridAccelMesh::LoadFromFile
    string relativePath = GetFilePath(path);
    int iSplit = relativePath.find('.', 2);
    relativePath = relativePath.substr(0, iSplit + 1);
    relativePath += "mesh";
    bool toFile = ((int)loadFlags & (int)MeshLoadFlag::ToFile) != 0;
    BitGridAccelMesh::LoadFromFile(
        relativePath, toFile, 
        vertexP, vertexPIndices, 
        vertexNg, vertexNgIndices, 
        gridDepth, 
        meshToGridScale, meshToGridTranslation, 
        grid, gridTriangles);
    // 删除临时文件
    if (toFile == false)
    {
        #ifndef NDEBUG
        int removeStatus = std::remove(relativePath.c_str());
        #else
        std::remove(relativePath.c_str());
        #endif
        Assert(removeStatus == 0, 
               "BitGridAccelMesh::LoadFromFile(): Remove temporary file error.");
    }
}

/*static*/ void BitGridAccelMesh::LoadFromFile(
    const string& path, bool toFile, 
    shared_ptr<vector<Point3f>>* vertexP, shared_ptr<vector<Point3i>>* vertexPIndices, 
    shared_ptr<vector<Normal3f>>* vertexNg, shared_ptr<vector<Point3i>>* vertexNgIndices, 
    Point3i* gridDepth, 
    Float* meshToGridScale, Vector3f* meshToGridTranslation, 
    uint8_t** grid, vector<int>*** gridTriangles)
{
    // Check
    Assert(*grid == nullptr, 
        "BitGridAccelMesh::LoadFromFile(): Grid should not point to valid address");
    Assert(*gridTriangles == nullptr, 
        "BitGridAccelMesh::LoadFromFile(): Grid should not point to valid address");
    string relativePath = GetFilePath(path);
    int iSplit = relativePath.find('.', 2);
    string postfix = relativePath.substr(iSplit + 1, relativePath.length());
    Assert(postfix == "mesh" || postfix == "accelmesh", 
           "BitGridAccelMesh::LoadFromFile(): Error file postfix");
    // Mesh::LoadFromFile
    Mesh::LoadFromFile(
        path, 
        vertexP, vertexPIndices, 
        vertexNg, vertexNgIndices, 
        gridDepth, 
        meshToGridScale, meshToGridTranslation);
    // BitGridAccelMesh::LoadFromFile
    if (postfix == "mesh")
    {
        // 计算 shifter
        int shifter_gridDepth_Y = GetOrderPower2(gridDepth->y);
        int shifter_gridDepth_Z = GetOrderPower2(gridDepth->z);
        int shifter_gridDepth_YZ = shifter_gridDepth_Y + shifter_gridDepth_Z;
        // 初始化 grid，填满 0
        int numBits = (gridDepth->x * gridDepth->y * gridDepth->z);
        int numBytes = (gridDepth->x * gridDepth->y * gridDepth->z) >> 3;
        Assert(numBits > 0, 
            "BitGridAccelMesh::BitGridAccelMesh(): Int overflow");
        *grid = new uint8_t[numBytes];
        for (int i = 0; i < numBytes; ++i)
            (*grid)[i] = uint8_t(0);
        // 初始化 gridTriangles，填满 nullptr
        *gridTriangles = new vector<int>*[numBits];
        for (int i = 0; i < numBits; ++i)
            (*gridTriangles)[i] = nullptr;
        // 构造 grid：将每个三角形添加到 unordered_map 中
        for (size_t i = 0; i < (*vertexPIndices)->size(); ++i) // 对于每个三角形
        {
            // 通过 index 获取顶点坐标
            Point3f p0((**vertexP)[(**vertexPIndices)[i].x]);
            Point3f p1((**vertexP)[(**vertexPIndices)[i].y]);
            Point3f p2((**vertexP)[(**vertexPIndices)[i].z]);
            // 检查合法性
            if(!(GridIsInside(p0, gridDepth->x, shifter_gridDepth_Y, shifter_gridDepth_Z) && 
                   GridIsInside(p1, gridDepth->x, shifter_gridDepth_Y, shifter_gridDepth_Z) && 
                   GridIsInside(p2, gridDepth->x, shifter_gridDepth_Y, shifter_gridDepth_Z)))
                Error(p0, p1, p2);
            Assert(GridIsInside(p0, gridDepth->x, shifter_gridDepth_Y, shifter_gridDepth_Z) && 
                   GridIsInside(p1, gridDepth->x, shifter_gridDepth_Y, shifter_gridDepth_Z) && 
                   GridIsInside(p2, gridDepth->x, shifter_gridDepth_Y, shifter_gridDepth_Z), 
                   "BitGridAccelMesh::LoadFromFile(): Triangle out of bound");
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
            int infX = std::clamp(int(triangleBound.pMin.x - 1), 0, gridDepth->x - 1);
            int infY = std::clamp(int(triangleBound.pMin.y - 1), 0, gridDepth->y - 1);
            int infZ = std::clamp(int(triangleBound.pMin.z - 1), 0, gridDepth->z - 1);
            int supX = std::clamp(int(triangleBound.pMax.x + 2), 0, gridDepth->x - 1);
            int supY = std::clamp(int(triangleBound.pMax.y + 2), 0, gridDepth->y - 1);
            int supZ = std::clamp(int(triangleBound.pMax.z + 2), 0, gridDepth->z - 1);
            // 通过三角形与长方体碰撞检测，将三角形塞进对应的 bounds 中
            for (int x = infX; x <= supX; ++x)
                for (int y = infY; y <= supY; ++y)
                    for (int z = infZ; z <= supZ; ++z)
                    {
                        // 获取 x, y, z 对应的 AABB
                        AABound3f bound = GridGetBitBlockBound_Relaxed(x, y, z);
                        // 碰撞检测
                        if (bound.Intersect(p0_relaxed, p1_relaxed, p2_relaxed) || 
                            bound.Intersect(p0n_relaxed, p1n_relaxed, p2n_relaxed) || 
                            bound.Intersect(p0nNeg_relaxed, p1nNeg_relaxed, p2nNeg_relaxed))
                        {
                            #ifndef NDEBUG
                            isAdded = true;
                            #endif
                            // Add to grid[]
                            GridSetHasItem(x, y, z, *grid, shifter_gridDepth_YZ, shifter_gridDepth_Z);
                            // Add to hash table
                            int gridBitIdx = GridGetGridBitIdx(x, y, z, shifter_gridDepth_YZ, shifter_gridDepth_Z);
                            if ((*gridTriangles)[gridBitIdx] == nullptr)
                                (*gridTriangles)[gridBitIdx] = new vector<int>();
                            (*gridTriangles)[gridBitIdx]->push_back(i);

                        }
                    }
            Assert(isAdded, 
                "GridAccelMesh::LoadFromFile(): Failed to add the triangle");
        }
        // Save to file
        if (toFile) // TODO: fBuffer
        {
            int iSplit = relativePath.find('.', 2);
            relativePath = relativePath.substr(0, iSplit + 1);
            relativePath += "accelmesh";
            std::ofstream fout;
            uint64_t fBufferX = 0, fBufferY = 0, fBufferZ = 0;
            // Open
            fout.open(relativePath);
            // P
            fout << (*vertexP)->size() << std::endl;
            for (const Point3f& p : **vertexP)
            {
                memcpy(&fBufferX, &p.x, sizeof(Float));
                memcpy(&fBufferY, &p.y, sizeof(Float));
                memcpy(&fBufferZ, &p.z, sizeof(Float));
                fout << fBufferX << ' ' << fBufferY << ' ' << fBufferZ << std::endl;
            }
            // PIdx
            fout << (*vertexPIndices)->size() << std::endl;
            for (const Point3i& pIdx : **vertexPIndices)
                fout << pIdx.x << ' ' << pIdx.y << ' ' << pIdx.z << std::endl;
            // Ng
            fout << (*vertexNg)->size() << std::endl;
            for (const Normal3f& ng : **vertexNg)
            {
                memcpy(&fBufferX, &ng.x, sizeof(Float));
                memcpy(&fBufferY, &ng.y, sizeof(Float));
                memcpy(&fBufferZ, &ng.z, sizeof(Float));
                fout << fBufferX << ' ' << fBufferY << ' ' << fBufferZ << std::endl;
            }
            // NgIdx
            fout << (*vertexNgIndices)->size() << std::endl;
            for (const Point3i& ngIdx : **vertexNgIndices)
                fout << ngIdx.x << ' ' << ngIdx.y << ' ' << ngIdx.z << std::endl;
            // GridDepth
            fout << gridDepth->x << ' ' << gridDepth->y << ' ' << gridDepth->z << std::endl;
            // MeshToGridScale
            memcpy(&fBufferX, meshToGridScale, sizeof(Float));
            fout << fBufferX << std::endl;
            // MeshToGridTranslation
            memcpy(&fBufferX, &meshToGridTranslation->x, sizeof(Float));
            memcpy(&fBufferY, &meshToGridTranslation->y, sizeof(Float));
            memcpy(&fBufferZ, &meshToGridTranslation->z, sizeof(Float));
            fout << fBufferX << ' ' << fBufferY << ' ' << fBufferZ << std::endl;
            // BitGridAccel: Label
            fout << "BitGridAccelMesh" << std::endl;
            // BitGridAccel: grid
            for (int i = 0; i < numBytes; ++i)
            {
                fout << (int)(*grid)[i];
                if (i != numBytes - 1)
                    fout << ' ';
                else
                    fout << std::endl;
            }
            // BitGridAccel: gridTriangles
            for (int i = 0; i < numBits; ++i)
            {
                vector<int>* triangles = (*gridTriangles)[i];
                if (triangles == nullptr)
                    fout << 0;
                else
                {
                    fout << triangles->size();
                    for (size_t i = 0; i < triangles->size(); ++i)
                        fout << ' ' << triangles->operator[](i);
                }
                fout << std::endl;
            }
            // Close
            fout.close();
        }
    }
    else if (postfix == "accelmesh")
    {
        std::ifstream fin;
        // Open
        fin.open(relativePath);
        // Jump to BitGridAccelMesh
        char buffer[256];
        for (size_t i = 0; i < (*vertexP)->size() + (*vertexPIndices)->size() + (*vertexNg)->size() + (*vertexNgIndices)->size() + 7 + 1; ++i)
            fin.getline(buffer, 256);
        // Check
        Assert(strcmp(buffer, "BitGridAccelMesh") == 0, 
            "BitGridAccelMesh::LoadFromFile(): Error file format");
        int numBits = (gridDepth->x * gridDepth->y * gridDepth->z);
        int numBytes = (gridDepth->x * gridDepth->y * gridDepth->z) >> 3;
        *grid = new uint8_t[numBytes];
        *gridTriangles = new vector<int>*[numBits];
        for (int i = 0; i < numBytes; ++i)
        {
            int num;
            fin >> num;
            (*grid)[i] = (uint8_t)num;
        }
        // GridTriangles
        for (int i = 0; i < numBits; ++i)
        {
            int numTriangles; fin >> numTriangles;
            if (numTriangles == 0)
                (*gridTriangles)[i] = nullptr;
            else
            {
                (*gridTriangles)[i] = new vector<int>(numTriangles);
                for (int iTriangle = 0; iTriangle < numTriangles; ++iTriangle)
                    fin >> (*gridTriangles)[i]->operator[](iTriangle);
            }
        }
        // Close
        fin.close();
    }
}

/*override*/ bool BitGridAccelMesh::Intersect(const Ray& ray, Float* tHit, SurfaceInteraction* si) const
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
        std::clamp(pGridHit.x, Float(0), Float(gridDepth.x) - Epsilonx100), 
        std::clamp(pGridHit.y, Float(0), Float(gridDepth.y) - Epsilonx100), 
        std::clamp(pGridHit.z, Float(0), Float(gridDepth.z) - Epsilonx100));
    // 计算 stride / rayGrid.d，取最小一轴定义 check point 的更新步长
    //! 在 bitGridAccel 中，stride/strideInv 总是 (1, 1, 1)
    Float speedScale = 1 / std::abs(rayGrid.d[rayGrid.d.MaximumAbsExtent()]) - Epsilonx1000; //! Relax
    Vector3f rayStep(rayGrid.d * speedScale);
    Point3f checkPoint(pGridHit); // 初始化 checkPoint 为与 gridBound 的接触点
    // 循环检查 check points
    for (;; checkPoint = Point3f(checkPoint + rayStep))
    {
        // 循环终止条件：检查到底都没找到
        int bitIdxX, bitIdxY, bitIdxZ;
        if (!GridIsInside(checkPoint, &bitIdxX, &bitIdxY, &bitIdxZ))
            return false;
        // Inside，则检查 checkPoint 对应的 bound 中是否有 triangles，没有则直接跳过
        int gridBitIdx = GridGetGridBitIdx(bitIdxX, bitIdxY, bitIdxZ);
        if (!GridGetHasItem(gridBitIdx >> 3, bitIdxZ))
            continue;
        // 如果 bound 中有 triangles，检查所有 triangles
        const vector<int>& triangles = *(gridTriangles[gridBitIdx]);
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
            if (t < 0 || t > rayGrid.tMax || std::isnan(t)) // 不在 t 范围内
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
