#include <triangle.h>

// Constructors
TriangleMesh::TriangleMesh(const shared_ptr<vector<Point3f>>& vertexP, const shared_ptr<vector<Point3i>>& vertexPIndices, 
                           const shared_ptr<vector<Normal3f>>& vertexNg, const shared_ptr<vector<Point3i>>& vertexNgIndices) : 
    vertexP(vertexP), 
    vertexPIndices(vertexPIndices), 
    vertexNg(vertexNg), 
    vertexNgIndices(vertexNgIndices)
{ }

// Methods
/*static*/ vector<shared_ptr<Triangle>> TriangleMesh::GenerateTriangles(
    const shared_ptr<TriangleMesh>& mesh, 
    const Transform* objectToWorld, const Transform* worldToObject)
{
    vector<shared_ptr<Triangle>> triangles;
    Assert(mesh->vertexPIndices->size() == mesh->vertexNgIndices->size(), 
           "TriangleMesh::GenerateTriangles(): Data size unmatched");
    for (size_t i = 0; i < mesh->vertexPIndices->size(); ++i)
        triangles.push_back(make_shared<Triangle>(objectToWorld, worldToObject, mesh, i, i));
    return triangles;
}



// Constructors
Triangle::Triangle(const Transform* objectToWorld, const Transform* worldToObject, 
                   const shared_ptr<TriangleMesh>& mesh, int pIdx, int ngIdx) : 
    TransformedShape(objectToWorld, worldToObject), 
    mesh(mesh), 
    pIdx(pIdx), 
    ngIdx(ngIdx)
{
    // 通过 index 获取顶点坐标
    Point3f p0 = (*mesh->vertexP)[(*mesh->vertexPIndices)[pIdx].x];
    Point3f p1 = (*mesh->vertexP)[(*mesh->vertexPIndices)[pIdx].y];
    Point3f p2 = (*mesh->vertexP)[(*mesh->vertexPIndices)[pIdx].z];
    // 计算每个三角形面积
    Float a2 = Point3f::DistanceSquared(p0, p1);
    Float b2 = Point3f::DistanceSquared(p1, p2);
    Float c2 = Point3f::DistanceSquared(p2, p0);
    Float a2b2 = a2 * b2;
    Float a2_add_b2_sub_c2 = a2 + b2 - c2;
    // 加入总面积
    area = 0.25 * std::sqrt(4 * a2b2 - a2_add_b2_sub_c2 * a2_add_b2_sub_c2);
}

// Methods
/*override*/ bool Triangle::Intersect(const Ray& ray, Float* tHit, SurfaceInteraction* si) const
{
    Ray rayLocal = (*worldToObject)(ray); // 将射线变换到三角形 local position
    // 通过 index 获取顶点坐标
    Point3f p0 = (*mesh->vertexP)[(*mesh->vertexPIndices)[pIdx].x];
    Point3f p1 = (*mesh->vertexP)[(*mesh->vertexPIndices)[pIdx].y];
    Point3f p2 = (*mesh->vertexP)[(*mesh->vertexPIndices)[pIdx].z];
    // Ray cast 检测
    Normal3f nPlane = Normal3f(Normal3f::Cross(p1 - p0, p2 - p0));
    Float nPlane_dot_d = Normal3f::Dot(nPlane, rayLocal.d);
    Float t = (Normal3f::Dot(nPlane, p0) - Normal3f::Dot(nPlane, rayLocal.o)) / nPlane_dot_d;
    if (t < 0 || t > rayLocal.tMax) // 不在 t 范围内
        return false;
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
        return false;
    // 获取法向量
    Normal3f ng0 = (*mesh->vertexNg)[(*mesh->vertexNgIndices)[ngIdx].x];
    Normal3f ng1 = (*mesh->vertexNg)[(*mesh->vertexNgIndices)[ngIdx].y];
    Normal3f ng2 = (*mesh->vertexNg)[(*mesh->vertexNgIndices)[ngIdx].z];
    
    // Ray
    ray.tMax = *tHit = t; //! 减小 tMax，方便检测场景中的其他物体
    // Local si
    si->p = p;
    si->ng = Normal3f(w0 * ng0 + w1 * ng1 + w2 * ng2);
    si->sg = Vector3f(Normal3f::Cross(si->ng, (q1q1 < q2q2 ? q2 : q1)));
    si->woWorld = Vector3f(-rayLocal.d);
    // TODO: Mesh 的 uv 怎么办？
    // TODO: Mesh 的 sg, tg 怎么办？
    // World si
    *si = (*objectToWorld)(*si);
    return true;
}
/*override*/ AABound3f Triangle::ObjectBound() const
{
    Point3f p0 = (*mesh->vertexP)[(*mesh->vertexPIndices)[pIdx].x];
    Point3f p1 = (*mesh->vertexP)[(*mesh->vertexPIndices)[pIdx].y];
    Point3f p2 = (*mesh->vertexP)[(*mesh->vertexPIndices)[pIdx].z];
    Float minX = Min(p0.x, p1.x, p2.x);
    Float minY = Min(p0.y, p1.y, p2.y);
    Float minZ = Min(p0.z, p1.z, p2.z);
    Float maxX = Max(p0.x, p1.x, p2.x);
    Float maxY = Max(p0.y, p1.y, p2.y);
    Float maxZ = Max(p0.z, p1.z, p2.z);
    return AABound3f(Point3f(minX, minY, minZ), Point3f(maxX, maxY, maxZ));
}
/*override*/ Float Triangle::Area() const
{
    return area;
}
/*override*/ Normal3f Triangle::Ng(const Ray& incidentRayWorld) const
{
    // TODO: Triangle::Ng()
    NotImplemented();
    return Normal3f();
}
/*override*/ Interaction Triangle::Sample(const Point2f& samples, Float* pdf) const
{
    // TODO: Triangle::Ng()
    NotImplemented();
    return Interaction();
}

#ifdef INTEL
/*override*/ int Triangle::GenerateEmbreeGeometry(RTCDevice device, RTCScene scene) const
{
    // TODO: Triangle::GenerateEmbreeGeometry()
    NotImplemented();
    return -1;
}
#endif
