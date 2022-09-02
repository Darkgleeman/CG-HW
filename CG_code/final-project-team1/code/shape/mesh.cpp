#include <mesh.h>
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

// Constructors
Mesh::Mesh(const Transform* objectToWorld, const Transform* worldToObject, 
           const shared_ptr<const vector<Point3f>>& vertexP, const shared_ptr<const vector<Point3i>>& vertexPIndices, 
           const shared_ptr<const vector<Normal3f>>& vertexNg, const shared_ptr<const vector<Point3i>>& vertexNgIndices, 
           const Point3i& gridDepth, 
           Float meshToGridScale, const Vector3f& meshToGridTranslation) : 
    TransformedShape(objectToWorld, worldToObject), 
    vertexP(vertexP), 
    vertexPIndices(vertexPIndices), 
    vertexNg(vertexNg), 
    vertexNgIndices(vertexNgIndices), 
    gridDepth(gridDepth), 
    meshToGridScale(meshToGridScale), 
    meshToGridScaleInv(1 / meshToGridScale), 
    meshToGridTranslation(meshToGridTranslation)
{
    Assert(vertexPIndices->size() == vertexNgIndices->size(), 
           "Mesh::Mesh(): Array size unmatched");
    area = 0;
    for (size_t i = 0; i < vertexPIndices->size(); ++i) // 对于每个三角形
    {
        // 通过 index 获取顶点坐标
        Point3f p0 = (*vertexP)[(*vertexPIndices)[i].x];
        Point3f p1 = (*vertexP)[(*vertexPIndices)[i].y];
        Point3f p2 = (*vertexP)[(*vertexPIndices)[i].z];
        // 计算每个三角形面积
        Float a2 = Point3f::DistanceSquared(p0, p1);
        Float b2 = Point3f::DistanceSquared(p1, p2);
        Float c2 = Point3f::DistanceSquared(p2, p0);
        Float a2b2 = a2 * b2;
        Float a2_add_b2_sub_c2 = a2 + b2 - c2;
        // 加入总面积
        area += 0.25 * std::sqrt(4 * a2b2 - a2_add_b2_sub_c2 * a2_add_b2_sub_c2);
    }
}

/*static*/ void Mesh::LoadFromFile(
        const string& path, int maxGridDepthPerDim, MeshFileType fileType, MeshLoadFlag loadFlags, 
        shared_ptr<vector<Point3f>>* vertexP, shared_ptr<vector<Point3i>>* vertexPIndices, 
        shared_ptr<vector<Normal3f>>* vertexNg, shared_ptr<vector<Point3i>>* vertexNgIndices, 
        Point3i* gridDepth, 
        Float* meshToGridScale, Vector3f* meshToGridTranslation)
{
    Assert(IsPower2(maxGridDepthPerDim), 
           "Mesh::LoadFromFile(): MaxGridDepthPerDim should be power of 2 to make it faster");
    // Load
    if (fileType == MeshFileType::CS171)
    {
        string relativePath = GetFilePath(path);
        std::ifstream fin;
        fin.open(relativePath);
        size_t numP, numNg, numIndices;
        fin >> numP >> numNg >> numIndices;
        (*vertexP)->resize(numP);
        (*vertexNg)->resize(numNg);
        (*vertexPIndices)->resize(numIndices);
        (*vertexNgIndices)->resize(numIndices);
        // 读取 position
        for (size_t i = 0; i < numP; ++i)
        {
            Float x, y, z;
            fin >> x >> y >> z;
            (*vertexP)->operator[](i) = Point3f(Point3f(x, y, z));
        }
        // 读取 geometric normal
        for (size_t i = 0; i < numNg; ++i)
        {
            Float x, y, z;
            fin >> x >> y >> z;
            (*vertexNg)->operator[](i) = Normal3f(x, y, z);
        }
        // 读取 indices
        for (size_t i = 0; i < numIndices; ++i)
        {
            size_t v0, n0, v1, n1, v2, n2;
            fin >> v0 >> n0 >> v1 >> n1 >> v2 >> n2;
            (*vertexPIndices)->operator[](i) = Point3i(v0, v1, v2);
            (*vertexNgIndices)->operator[](i) = Point3i(n0, n1, n2);
        }
        fin.close();
    }
    else if (fileType == MeshFileType::OBJ)
    {
        string relativePath = GetFilePath(path);
        std::ifstream fin;
        fin.open(relativePath);

        (*vertexP)->resize(0);
        (*vertexNg)->resize(0);
        (*vertexPIndices)->resize(0);
        (*vertexNgIndices)->resize(0);

        char buffer[256];
        std::stringstream stream;
        while (!fin.eof())
        {
            fin.getline(buffer, 256);
            if (buffer[0] == 'v' && buffer[1] == ' ')
            {
                stream.str("");
                stream.clear();
                stream << buffer;
                char flag;
                Float x, y, z;
                stream >> flag >> x >> y >> z;
                (*vertexP)->push_back(Point3f(x, y, z));
            }
            if (buffer[0] == 'f' && buffer[1] == ' ')
            {
                stream.str("");
                stream.clear();
                stream << buffer;
                char flag;
                int x, y, z;
                stream >> flag >> x >> y >> z;
                (*vertexPIndices)->push_back(Point3i(x - 1, y - 1, z - 1));
            }
        }
        if ((*vertexNg)->size() == 0) // 如果没有提供法向量
        {
            bool HasWarned = false;
            Assert((int(loadFlags) & int(MeshLoadFlag::RecalculateNormal)) == 0, 
                   "Mesh::LoadFromFile(): Mesh file does not contain normal data");
            for (int p = 0; p < (int)(*vertexP)->size(); ++p)
            {
                Normal3f normalSum(0, 0, 0);
                int normalCount = 0;
                for (int pIdx = 0; pIdx < (int)(*vertexPIndices)->size(); ++pIdx) // 对于每个面
                {
                    const Point3i& vertexPIndice = (*vertexPIndices)->operator[](pIdx);
                    if (vertexPIndice.x == p || vertexPIndice.y == p || vertexPIndice.z == p) // 如果面包含这个顶点
                    {
                        // 计算平面法向量，以叉乘为权重
                        Point3f p0 = (*vertexP)->operator[](vertexPIndice.x);
                        Point3f p1 = (*vertexP)->operator[](vertexPIndice.y);
                        Point3f p2 = (*vertexP)->operator[](vertexPIndice.z);
                        Normal3f normal(Vector3f::Cross(p1 - p0, p2 - p0));
                        // 添加到 normalSum //TODO
                        if (normalCount > 0 && Normal3f::Dot(normalSum, normal) < 0 && 
                            HasWarned == false) //! 存在矛盾情况
                        {
                            HasWarned = true;
                            Error("WARNING: Mesh::LoadFromFile(): Ambiguous normal input");
                        }
                        normalSum += normal;
                        ++normalCount;
                    }
                }
                Assert(normalCount > 0, 
                    "Mesh::LoadFromFile(): Vertex should be included in at least one triangle");
                (*vertexNg)->push_back(Normal3f((normalSum / normalCount).Normalized()));
            }
            *vertexNgIndices = *vertexPIndices; //! P 与 Ng 共享相同的 indices
        }
        fin.close();
    }
    else if (fileType == MeshFileType::TINY_OBJ)
    {
        tinyobj::ObjReaderConfig readerConfig;
        readerConfig.mtl_search_path = "./"; // TODO: 这啥意思

        tinyobj::ObjReader reader;
        string relativePath = path;
        for (int depth = 0; depth < 10; ++depth)
            if (!reader.ParseFromFile(relativePath, readerConfig))
            {
                relativePath = "../" + relativePath;
                if (depth < 4)
                    continue;
                if (!reader.Error().empty())
                    std::clog << "TinyObjReader: " << reader.Error();
                exit(1);
            }

        if (!reader.Warning().empty())
            std::cout << "TinyObjReader: " << reader.Warning();

        auto &attrib = reader.GetAttrib();
        auto &shapes = reader.GetShapes();
        auto &materials = reader.GetMaterials();

        (*vertexP)->resize(0);
        (*vertexPIndices)->resize(0);
        vector<int> vertexPIndices_int;
        for (size_t i = 0; i < attrib.vertices.size(); i += 3)
            (*vertexP)->push_back(Point3f(attrib.vertices[i], attrib.vertices[i + 1], attrib.vertices[i + 2]));
        // texCoords.resize(vertices.size()); // TODO: 
        (*vertexNg)->resize((*vertexP)->size());
        // Loop over shapes
        for (size_t s = 0; s < shapes.size(); s++) {
            // Loop over faces(polygon)
            size_t index_offset = 0;
            for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
                size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

                // Loop over vertices in the face.
                for (size_t v = 0; v < fv; v++) {
                    tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                    vertexPIndices_int.push_back(idx.vertex_index);
                    // if (!attrib.texcoords.empty()) {
                    // texCoords[idx.vertex_index] =
                    //     vec2(attrib.texcoords[idx.texcoord_index * 2 + 0],
                    //         attrib.texcoords[idx.texcoord_index * 2 + 1]);
                    // }
                    (*vertexNg)->operator[](idx.vertex_index) = Normal3f(Normal3f(
                        attrib.normals[idx.normal_index * 3 + 0],
                        attrib.normals[idx.normal_index * 3 + 1],
                        attrib.normals[idx.normal_index * 3 + 2]).Normalized());
                }
                index_offset += fv;
            }
        }
        for (size_t i = 0; i < vertexPIndices_int.size(); i += 3)
            (*vertexPIndices)->push_back(Point3i(vertexPIndices_int[i], vertexPIndices_int[i + 1], vertexPIndices_int[i + 2]));
        *vertexNgIndices = *vertexPIndices; //! 直接赋值 shared_ptr 即可
    }
    else
    {
        Assert(false, 
               "Mesh::LoadFromFile(): Undefined file type");
    }

    //! 对于 Mesh，必须要更早地处理 flags，因为 meshToGridTranslation 依赖于坐标轴等信息
    // Flags
    if ((int(loadFlags) & int(MeshLoadFlag::RHandToLHand)) != 0) // 原 mesh 为右手系，需要变换到左手系
    {
        for (Point3f& p : **vertexP)
            p.z = -p.z;
        for (Normal3f& ng : **vertexNg)
            ng.z = -ng.z;
    }
    if ((int(loadFlags) & int(MeshLoadFlag::YStartFrom0)) != 0) // 如果 y 不是从 0 开始，会导致模型很难摆放
    {
        Float yMin = Infinity;
        for (const Point3f& p : **vertexP)
            if (p.y < yMin)
                yMin = p.y;
        yMin += Epsilon; //! 防止模型悬空
        for (Point3f& p : **vertexP)
            p.y -= yMin;
    }
    if ((int(loadFlags) & int(MeshLoadFlag::XZToCenter)) != 0) // X 和 Z 轴偏离中心太多，会导致模型很难摆放
    {
        AABound3f pBound;
        for (const Point3f& p : **vertexP)
            pBound = AABound3f::Union(pBound, p);
        Point3f center = Point3f(0.5 * (pBound.pMax + pBound.pMin));
        Float offsetX = -center.x;
        Float offsetZ = -center.z;
        for (Point3f& p : **vertexP)
            p.x += offsetX, p.z += offsetZ;
    }
    if ((int(loadFlags) & int(MeshLoadFlag::RecalculateNormal)) != 0)
    {
        // 对每个点 p，先找到所有包含 p 的面
        vector<Normal3f> newNg;
        for (int p = 0; p < (int)(*vertexP)->size(); ++p)
        {
            Normal3f normalSum(0, 0, 0);
            int normalCount = 0;
            for (int pIdx = 0; pIdx < (int)(*vertexPIndices)->size(); ++pIdx) // 对于每个面
            {
                const Point3i& vertexPIndice = (*vertexPIndices)->operator[](pIdx);
                if (vertexPIndice.x == p || vertexPIndice.y == p || vertexPIndice.z == p) // 如果面包含这个顶点
                {
                    // 计算平面法向量，以叉乘为权重
                    Point3f p0 = (*vertexP)->operator[](vertexPIndice.x);
                    Point3f p1 = (*vertexP)->operator[](vertexPIndice.y);
                    Point3f p2 = (*vertexP)->operator[](vertexPIndice.z);
                    Normal3f normal(Vector3f::Cross(p1 - p0, p2 - p0));
                    // 用原给定的 normal 方向来确定新 normal 方向
                    int axis = (vertexPIndice.x == p) ? 0 : (vertexPIndice.y == p) ? 1 : 2;
                    Normal3f originalNg((*vertexNg)->operator[]((*vertexNgIndices)->operator[](pIdx)[axis]));
                    Normal3f candidateNg((Normal3f::Dot(normal, originalNg) >= 0) ? normal : -normal);
                    // 添加到 normalSum
                    Assert(normalCount == 0 || Normal3f::Dot(normalSum, candidateNg) >= 0, //! 存在矛盾情况
                           "Mesh::LoadFromFile(): Ambiguous input");
                    normalSum += candidateNg;
                    ++normalCount;
                }
            }
            Assert(normalCount > 0, 
                   "Mesh::LoadFromFile(): Vertex should be included in at least one triangle");
            newNg.push_back(Normal3f((normalSum / normalCount).Normalized()));
        }
        **vertexNg = newNg; //! 拷贝
        *vertexNgIndices = *vertexPIndices; //! P 与 Ng 共享相同的 indices
    }
    // TODO: 更多的 flags

    // Compute meshToGridTranslation, meshToGridScale
    AABound3f pBound;
    for (const Point3f& p : **vertexP)
        pBound = AABound3f::Union(pBound, p);
    *meshToGridScale = Float(maxGridDepthPerDim) / pBound.Diagonal()[pBound.MaximumExtent()] - Epsilonx1000; //! 将最长轴缩放到 maxGridDepthPerDim-δ
    *meshToGridTranslation = Vector3f(-pBound.pMin * *meshToGridScale + Vector3f(Epsilonx10, Epsilonx10, Epsilonx10)); //! 将模型摆放到 (0+δ, 0+δ, 0+δ) 需要的位移
    //! 任何一条射线可以在 mesh coordinate 通过 apply meshToGridScale and meshToGridTranslation 变换到 grid coordinate

    // 将所有三角形的顶点坐标从 mesh coordinate 变换到 grid coordinate
    for (Point3f& p : **vertexP)
        p = Point3f(p * *meshToGridScale + *meshToGridTranslation);

    // Compute gridDepth
    Point3f gridCoordSup = Point3f(pBound.pMax * *meshToGridScale + *meshToGridTranslation); //! 仍然可能越界
    *gridDepth = Point3i((int)(ceil(gridCoordSup.x)), //! Round to 2 的指数是为了查询数组加速，因此 x 轴不需要满足这个性质
                         RoundUpToPower2(int(gridCoordSup.y)), 
                         RoundUpToPower2(int(gridCoordSup.z)));

    // Check
    Assert((*gridDepth)[pBound.MaximumExtent()] == maxGridDepthPerDim, 
           "Mesh::LoadFromFile(): Maximum extent of gridDepth not match maxGridDepthPerDim");
    Assert(gridDepth->z >= 8, 
           "Mesh::LoadFromFile(): GridDepth.z should larger than 8"); //!  grid 的实现将 z 轴除 8 倍
    Assert(gridDepth->x >= 0 && gridDepth->y >= 0 && gridDepth->z >= 0 && 
           gridDepth->x <= maxGridDepthPerDim && gridDepth->y <= maxGridDepthPerDim && gridDepth->z <= maxGridDepthPerDim, 
           "Mesh::LoadFromFile(): GridDepth out of bound");
    #ifndef NDEBUG
    Point3f gridCoordInf = Point3f(pBound.pMin * *meshToGridScale + *meshToGridTranslation); //! 仍然可能越界
    #endif
    Assert(gridCoordInf.x >= 0 && 
           gridCoordInf.y >= 0 && 
           gridCoordInf.z >= 0, 
           "Mesh::LoadFromFile(): PBound.pMin out of bound in grid coordinate");
    Assert(gridCoordSup.x < gridDepth->x && 
           gridCoordSup.y < gridDepth->y && 
           gridCoordSup.z < gridDepth->z, 
           "Mesh::LoadFromFile(): PBound.pMax out of bound in grid coordinate");
    //! 至此，我们可以断言：pBound.pMax/pMin 变换到 grid coordinate 总是在 [0, gridDepth.xyz) 范围内，永远不会越界
    //! 因此可以放心地用 (int) 来获取对应的 bounding index

    if ((int(loadFlags) & int(MeshLoadFlag::ToFile)) != 0)
    {
        string relativePath = GetFilePath(path);
        int iSplit = relativePath.find('.', 2);
        relativePath = relativePath.substr(0, iSplit + 1);
        relativePath += "mesh";
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
        // Close
        fout.close();
    }
}

/*static*/ void Mesh::LoadFromFile(
        const string& path, 
        shared_ptr<vector<Point3f>>* vertexP, shared_ptr<vector<Point3i>>* vertexPIndices, 
        shared_ptr<vector<Normal3f>>* vertexNg, shared_ptr<vector<Point3i>>* vertexNgIndices, 
        Point3i* gridDepth, 
        Float* meshToGridScale, Vector3f* meshToGridTranslation)
{
    string relativePath = GetFilePath(path);
    int iSplit = relativePath.find('.', 2);
    string postfix = relativePath.substr(iSplit + 1, relativePath.length());
    Assert(postfix == "mesh" || postfix == "accelmesh", 
           "Mesh::LoadFromFile(): Error file postfix");
    std::ifstream fin;
    uint64_t fBufferX = 0, fBufferY = 0, fBufferZ = 0;
    // Open
    fin.open(relativePath);
    // P
    int numP; fin >> numP;
    (*vertexP)->resize(numP);
    for (int i = 0; i < numP; ++i)
    {
        Point3f& p = (*vertexP)->operator[](i);
        fin >> fBufferX >> fBufferY >> fBufferZ;
        memcpy(&p.x, &fBufferX, sizeof(Float));
        memcpy(&p.y, &fBufferY, sizeof(Float));
        memcpy(&p.z, &fBufferZ, sizeof(Float));
    }
    // PIdx
    int numPIdx; fin >> numPIdx;
    (*vertexPIndices)->resize(numPIdx);
    for (int i = 0; i < numPIdx; ++i)
    {
        Point3i& pIdx = (*vertexPIndices)->operator[](i);
        fin >> pIdx.x >> pIdx.y >> pIdx.z;
    }
    // Ng
    int numNg; fin >> numNg;
    (*vertexNg)->resize(numNg);
    for (int i = 0; i < numNg; ++i)
    {
        Normal3f& ng = (*vertexNg)->operator[](i);
        fin >> fBufferX >> fBufferY >> fBufferZ;
        memcpy(&ng.x, &fBufferX, sizeof(Float));
        memcpy(&ng.y, &fBufferY, sizeof(Float));
        memcpy(&ng.z, &fBufferZ, sizeof(Float));
    }
    // NgIdx
    int numNgIdx; fin >> numNgIdx;
    (*vertexNgIndices)->resize(numNgIdx);
    for (int i = 0; i < numNgIdx; ++i)
    {
        Point3i& ngIdx = (*vertexNgIndices)->operator[](i);
        fin >> ngIdx.x >> ngIdx.y >> ngIdx.z;
    }
    // GridDepth
    fin >> gridDepth->x >> gridDepth->y >> gridDepth->z;
    // MeshToGridScale
    fin >> fBufferX;
    memcpy(meshToGridScale, &fBufferX, sizeof(Float));
    // MeshToGridTranslation
    fin >> fBufferX >> fBufferY >> fBufferZ;
    memcpy(&meshToGridTranslation->x, &fBufferX, sizeof(Float));
    memcpy(&meshToGridTranslation->y, &fBufferY, sizeof(Float));
    memcpy(&meshToGridTranslation->z, &fBufferZ, sizeof(Float));
    // Close
    fin.close();
}

/*override*/ bool Mesh::Intersect(const Ray& ray, Float* tHit, SurfaceInteraction* si) const
{
    bool isHitted = false;
    Ray rayLocal = (*worldToObject)(ray); // 将射线变换到 mesh local coordinate
    for (size_t i = 0; i < vertexPIndices->size(); ++i) // 对于每个三角形
    {
        // 通过 index 获取顶点坐标
        Point3f p0 = (*vertexP)[(*vertexPIndices)[i].x];
        Point3f p1 = (*vertexP)[(*vertexPIndices)[i].y];
        Point3f p2 = (*vertexP)[(*vertexPIndices)[i].z];
        // Ray cast 检测
        Normal3f nPlane = Normal3f(Normal3f::Cross(p1 - p0, p2 - p0));
        Float nPlane_dot_d = Normal3f::Dot(nPlane, rayLocal.d);
        Float t = (Normal3f::Dot(nPlane, p0) - Normal3f::Dot(nPlane, rayLocal.o)) / nPlane_dot_d;
        if (t < 0 || t > rayLocal.tMax) // 不在 t 范围内
            continue;
        Point3f p = rayLocal(t);
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
        Normal3f ng0 = (*vertexNg)[(*vertexNgIndices)[i].x];
        Normal3f ng1 = (*vertexNg)[(*vertexNgIndices)[i].y];
        Normal3f ng2 = (*vertexNg)[(*vertexNgIndices)[i].z];
        // Status
        isHitted = true;
        // RayLocal
        rayLocal.tMax = t;
        // Local si
        si->p = p;
        si->ng = Normal3f(w0 * ng0 + w1 * ng1 + w2 * ng2);
        si->sg = Vector3f(Normal3f::Cross(si->ng, (q1q1 < q2q2 ? q2 : q1)));
        // TODO: Mesh 的 uv 怎么办？
        // TODO: Mesh 的 sg, tg 怎么办？
    }
    if (isHitted)
    {
        // Ray
        ray.tMax = *tHit = rayLocal.tMax;
        // Local si
        si->woWorld = Vector3f(-rayLocal.d);
        // TODO: Mesh 的 uv 怎么办？
        // TODO: Mesh 的 sg, tg 怎么办？
        // World si
        *si = (*objectToWorld)(*si);
        return true;
    }
    else
        return false;
}

/*override*/ AABound3f Mesh::ObjectBound() const
{
    Float minX =  Infinity, minY =  Infinity, minZ =  Infinity;
    Float maxX = -Infinity, maxY = -Infinity, maxZ = -Infinity;
    for (const Point3f& p : *vertexP)
    {
        minX = std::min(minX, p.x);
        maxX = std::max(maxX, p.x);
        minY = std::min(minY, p.y);
        maxY = std::max(maxY, p.y);
        minZ = std::min(minZ, p.z);
        maxZ = std::max(maxZ, p.z);
    }
    return AABound3f(Point3f(minX, minY, minZ), Point3f(maxX, maxY, maxZ));
}

/*override*/ Float Mesh::Area() const
{
    return area;
}

/*override*/ Normal3f Mesh::Ng(const Ray& incidentRayWorld) const
{
    // TODO: Mesh::Ng()
    NotImplemented();
    return Normal3f();
}

/*override*/ Interaction Mesh::Sample(const Point2f& samples, Float* pdf) const
{
    // TODO: Mesh::Sample()
    NotImplemented();
    return Interaction();
}

/*override*/ Interaction Mesh::Sample(const Interaction& ref, 
                                        const Point2f& samples, Float* pdf) const
{
    // TODO: Mesh::Sample()
    NotImplemented();
    return Interaction();
}
