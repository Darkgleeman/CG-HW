#include <plane.h>

// Constructors
Plane::Plane(const Transform* objectToWorld, const Transform* worldToObject) : TransformedShape(objectToWorld, worldToObject)
{
    // Area
    Vector3f scale = objectToWorld->Scale();
    area = scale.x * scale.z;
}

// Methods
/*override*/ bool Plane::Intersect(const Ray& ray, Float* tHit, SurfaceInteraction* si) const
{
    // Ray cast 检测
    Ray rayLocal = (*worldToObject)(ray); // 将射线变换到 plane local coordinate，即 x0 = -0.5, x1 = 0.5, z0 = -0.5, z1 = 0.5, y = 0
    Float t = -rayLocal.o.y / rayLocal.d.y;
    if (t < 0 || t > rayLocal.tMax)
        return false;
    Float x = rayLocal.o.x + t * rayLocal.d.x;
    Float z = rayLocal.o.z + t * rayLocal.d.z;
    if (x < -0.5 || x > 0.5 || z < -0.5 || z > 0.5)
        return false;
    // Ray
    ray.tMax = *tHit = t; //! 减小 tMax，方便检测场景中的其他物体
    // Local si
    si->p = Point3f(x, 0, z);
    si->ng = Normal3f(0, 1, 0); //! 由于可以直接通过 Dot(si->ng, si->woWorld) 来判断某个 intersection 是否是 outward hit。因此 ng 总是指向几何外侧
    si->sg = Vector3f(1, 0, 0); //! 左手系的叉乘规则相反
    si->tg = Vector3f(0, 0, 1);
    si->woWorld = Vector3f(-rayLocal.d);
    si->uv = Point2f(x + 0.5, z + 0.5);
    // World si
    *si = (*objectToWorld)(*si);
    return true;
}

/*override*/ AABound3f Plane::ObjectBound() const
{
    return AABound3f(Point3f(-0.5, -0.001, -0.5), Point3f(0.5, 0.001, 0.5));
}

/*override*/ Float Plane::Area() const
{
    return area;
}

/*override*/ Normal3f Plane::Ng(const Ray& incidentRayWorld) const
{
    return (*objectToWorld)(Normal3f(0, 1, 0));
}

/*override*/ Interaction Plane::Sample(const Point2f& samples, Float* pdf) const
{
    SurfaceInteraction si;
    si.p  = (*objectToWorld)(Point3f(samples.x - 0.5, 0, samples.y - 0.5));
    si.ng = (*objectToWorld)(Normal3f(0, 1, 0));
    si.sg = (*objectToWorld)(Vector3f(1, 0, 0));
    si.tg = (*objectToWorld)(Vector3f(0, 0, 1));
    si.uv = Point2f(samples.x, samples.y);
    *pdf = PdfArea();
    return si;
}

#ifdef INTEL
/*override*/ int Plane::GenerateEmbreeGeometry(RTCDevice device, RTCScene scene) const
{
    if (sizeof(Float) != sizeof(float))
    {
        Error("Plane::GenerateEmbreeGeometry(): Should use float instead of double for embree");
        exit(-1);
    }
    RTCGeometry geom = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_TRIANGLE);
    // Vertices
    Point3f* vertices = (Point3f*)rtcSetNewGeometryBuffer(geom, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, sizeof(Point3f), 4);
    vertices[0].x = -0.5; vertices[0].y = 0; vertices[0].z = -0.5;
    vertices[1].x =  0.5; vertices[1].y = 0; vertices[1].z = -0.5;
    vertices[2].x = -0.5; vertices[2].y = 0; vertices[2].z =  0.5;
    vertices[3].x =  0.5; vertices[3].y = 0; vertices[3].z =  0.5;
    for (int i = 0; i < 4; ++i)
        vertices[i] = (*objectToWorld)(vertices[i]);
    // Indices
    Point3u* indices = (Point3u*)rtcSetNewGeometryBuffer(geom, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3, sizeof(Point3u), 2);
    indices[0].x = 0; indices[0].y = 1, indices[0].z = 2;
    indices[1].x = 2; indices[1].y = 1, indices[1].z = 3;
    // Commit
    rtcCommitGeometry(geom);
    int geomID = rtcAttachGeometry(scene, geom);
    rtcReleaseGeometry(geom);
    return geomID;
}
#endif
