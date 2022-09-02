#include <cube.h>

// Constructors
Cube::Cube(const Transform* objectToWorld, const Transform* worldToObject) : TransformedShape(objectToWorld, worldToObject)
{
    // Area
    Vector3f scale = objectToWorld->Scale();
    area = 2 * (scale.x * scale.y + scale.y * scale.z + scale.z * scale.x);
}

// Methods
/*override*/ bool Cube::Intersect(const Ray& ray, Float* tHit, SurfaceInteraction* si) const
{
    // 将射线变换到 cube local coordinate，即 pMin = -0.5, pMax = 0.5
    Ray rayLocal = (*worldToObject)(ray);
    // 剔除：六个面中至多只有三个面需要检查
    bool isShouldCheck_X = true, isShouldCheck_NegX = true;
    bool isShouldCheck_Y = true, isShouldCheck_NegY = true;
    bool isShouldCheck_Z = true, isShouldCheck_NegZ = true;
    if (rayLocal.o.x >= 0.5f)
    {
        isShouldCheck_NegX = false;
        if (rayLocal.d.x >= 0) { return false; }
        if (rayLocal.d.y >= 0) isShouldCheck_Y = false; else isShouldCheck_NegY = false;
        if (rayLocal.d.z >= 0) isShouldCheck_Z = false; else isShouldCheck_NegZ = false;
    }
    else if (rayLocal.o.x <= -0.5f)
    {
        isShouldCheck_X = false;
        if (rayLocal.d.x <= 0) { return false; }
        if (rayLocal.d.y >= 0) isShouldCheck_Y = false; else isShouldCheck_NegY = false;
        if (rayLocal.d.z >= 0) isShouldCheck_Z = false; else isShouldCheck_NegZ = false;
    }
    else { if (rayLocal.d.x >= 0) isShouldCheck_NegX = false; else isShouldCheck_X = false; }
    if (rayLocal.o.y >= 0.5f)
    {
        isShouldCheck_NegY = false;
        if (rayLocal.d.y >= 0) { return false; }
        if (rayLocal.d.x >= 0) isShouldCheck_X = false; else isShouldCheck_NegX = false;
        if (rayLocal.d.z >= 0) isShouldCheck_Z = false; else isShouldCheck_NegZ = false;
    }
    else if (rayLocal.o.y <= -0.5f)
    {
        isShouldCheck_Y = false;
        if (rayLocal.d.y <= 0) { return false; }
        if (rayLocal.d.x >= 0) isShouldCheck_X = false; else isShouldCheck_NegX = false;
        if (rayLocal.d.z >= 0) isShouldCheck_Z = false; else isShouldCheck_NegZ = false;
    }
    else { if (rayLocal.d.y >= 0) isShouldCheck_NegY = false; else isShouldCheck_Y = false; }
    if (rayLocal.o.z >= 0.5f)
    {
        isShouldCheck_NegZ = false;
        if (rayLocal.d.z >= 0) { return false; }
        if (rayLocal.d.x >= 0) isShouldCheck_X = false; else isShouldCheck_NegX = false;
        if (rayLocal.d.y >= 0) isShouldCheck_Y = false; else isShouldCheck_NegY = false;
    }
    else if (rayLocal.o.z <= -0.5f)
    {
        isShouldCheck_Z = false;
        if (rayLocal.d.z <= 0) { return false; }
        if (rayLocal.d.x >= 0) isShouldCheck_X = false; else isShouldCheck_NegX = false;
        if (rayLocal.d.y >= 0) isShouldCheck_Y = false; else isShouldCheck_NegY = false;
    }
    else { if (rayLocal.d.z >= 0) isShouldCheck_NegZ = false; else isShouldCheck_Z = false; }
    // 至此，对于所有需要检查的面，计算得到的 t 必为正数 (负数解已被剔除)，且由唯一性，如果 hitPoint 在约束平面内，其必为正确解
    if (isShouldCheck_X)
    {
        Float t = (0.5f - rayLocal.o.x) / rayLocal.d.x;
        Float hitPointY = rayLocal.o.y + t * rayLocal.d.y;
        Float hitPointZ = rayLocal.o.z + t * rayLocal.d.z;
        if (-0.5f <= hitPointY && -0.5f <= hitPointZ && hitPointY <= 0.5f && hitPointZ <= 0.5f)
        {
            // Ray
            ray.tMax = *tHit = t;
            // Local si
            si->p = rayLocal(t);
            si->ng = Normal3f(1,  0, 0);
            si->sg = Vector3f(0, -1, 0); //! 左手系的叉乘规则相反
            si->tg = Vector3f(0,  0, 1);
            si->woWorld = Vector3f(-rayLocal.d);
            si->uv = Point2f(si->p.y + 0.5, si->p.z + 0.5);
            // World si
            *si = (*objectToWorld)(*si);
            return true;
        }
    }
    if (isShouldCheck_Y)
    {
        Float t = (0.5f - rayLocal.o.y) / rayLocal.d.y;
        Float hitPointX = rayLocal.o.x + t * rayLocal.d.x;
        Float hitPointZ = rayLocal.o.z + t * rayLocal.d.z;
        if (-0.5f <= hitPointX && -0.5f <= hitPointZ && hitPointX <= 0.5f && hitPointZ <= 0.5f)
        {
            // Ray
            ray.tMax = *tHit = t;
            // Local si
            si->p = rayLocal(t);
            si->ng = Normal3f(0, 1,  0);
            si->sg = Vector3f(0, 0, -1); //! 左手系的叉乘规则相反
            si->tg = Vector3f(1, 0,  0);
            si->woWorld = Vector3f(-rayLocal.d);
            si->uv = Point2f(si->p.z + 0.5, si->p.x + 0.5);
            // World si
            *si = (*objectToWorld)(*si);
            return true;
        }
    }
    if (isShouldCheck_Z)
    {
        Float t = (0.5f - rayLocal.o.z) / rayLocal.d.z;
        Float hitPointX = rayLocal.o.x + t * rayLocal.d.x;
        Float hitPointY = rayLocal.o.y + t * rayLocal.d.y;
        if (-0.5f <= hitPointX && -0.5f <= hitPointY && hitPointX <= 0.5f && hitPointY <= 0.5f)
        {
            // Ray
            ray.tMax = *tHit = t;
            // Local si
            si->p = rayLocal(t);
            si->ng = Normal3f( 0, 0, 1);
            si->sg = Vector3f(-1, 0, 0); //! 左手系的叉乘规则相反
            si->tg = Vector3f( 0, 1, 0);
            si->woWorld = Vector3f(-rayLocal.d);
            si->uv = Point2f(si->p.x + 0.5, si->p.y + 0.5);
            // World si
            *si = (*objectToWorld)(*si);
            return true;
        }
    }
    if (isShouldCheck_NegX)
    {
        Float t = (-0.5f - rayLocal.o.x) / rayLocal.d.x;
        Float hitPointY = rayLocal.o.y + t * rayLocal.d.y;
        Float hitPointZ = rayLocal.o.z + t * rayLocal.d.z;
        if (-0.5f <= hitPointY && -0.5f <= hitPointZ && hitPointY <= 0.5f && hitPointZ <= 0.5f)
        {
            // Ray
            ray.tMax = *tHit = t;
            // Local si
            si->p = rayLocal(t);
            si->ng = Normal3f(-1,  0, 0);
            si->sg = Vector3f( 0,  0, 1); //! 左手系的叉乘规则相反
            si->tg = Vector3f( 0, -1, 0);
            si->woWorld = Vector3f(-rayLocal.d);
            si->uv = Point2f(si->p.y + 0.5, si->p.z + 0.5);
            // World si
            *si = (*objectToWorld)(*si);
            return true;
        }
    }
    if (isShouldCheck_NegY)
    {
        Float t = (-0.5f - rayLocal.o.y) / rayLocal.d.y;
        Float hitPointX = rayLocal.o.x + t * rayLocal.d.x;
        Float hitPointZ = rayLocal.o.z + t * rayLocal.d.z;
        if (-0.5f <= hitPointX && -0.5f <= hitPointZ && hitPointX <= 0.5f && hitPointZ <= 0.5f)
        {
            // Ray
            ray.tMax = *tHit = t;
            // Local si
            si->p = rayLocal(t);
            si->ng = Normal3f(0, -1,  0);
            si->sg = Vector3f(1,  0,  0); //! 左手系的叉乘规则相反
            si->tg = Vector3f(0,  0, -1);
            si->woWorld = Vector3f(-rayLocal.d);
            si->uv = Point2f(si->p.z + 0.5, si->p.x + 0.5);
            // World si
            *si = (*objectToWorld)(*si);
            return true;
        }
    }
    if (isShouldCheck_NegZ)
    {
        Float t = (-0.5f - rayLocal.o.z) / rayLocal.d.z;
        Float hitPointX = rayLocal.o.x + t * rayLocal.d.x;
        Float hitPointY = rayLocal.o.y + t * rayLocal.d.y;
        if (-0.5f <= hitPointX && -0.5f <= hitPointY && hitPointX <= 0.5f && hitPointY <= 0.5f)
        {
            // Ray
            ray.tMax = *tHit = t;
            // Local si
            si->p = rayLocal(t);
            si->ng = Normal3f( 0, 0, -1);
            si->sg = Vector3f( 0, 1,  0); //! 左手系的叉乘规则相反
            si->tg = Vector3f(-1, 0,  0);
            si->woWorld = Vector3f(-rayLocal.d);
            si->uv = Point2f(si->p.x + 0.5, si->p.y + 0.5);
            // World si
            *si = (*objectToWorld)(*si);
            return true;
        }
    }
    // 六个面均找不到，not hit
    return false;
}

/*override*/ AABound3f Cube::ObjectBound() const
{
    return AABound3f(Point3f(-0.5, -0.5, -0.5), Point3f(0.5, 0.5, 0.5));
}

/*override*/ Float Cube::Area() const
{
    return area;
}

/*override*/ Normal3f Cube::Ng(const Ray& incidentRayWorld) const
{
    // TODO: Cube::Ng()
    NotImplemented();
    return Normal3f();
}

/*override*/ Interaction Cube::Sample(const Point2f& samples, Float* pdf) const
{
    // TODO: Cube::Sample()
    NotImplemented();
    return Interaction();
}

/*override*/ Interaction Cube::Sample(const Interaction& ref, 
                                      const Point2f& samples, Float* pdf) const
{
    // TODO: Cube::Sample()
    NotImplemented();
    return Interaction();
}

#ifdef INTEL
/*override*/ int Cube::GenerateEmbreeGeometry(RTCDevice device, RTCScene scene) const
{
    if (sizeof(Float) != sizeof(float))
    {
        Error("Cube::GenerateEmbreeGeometry(): Should use float instead of double for embree");
        exit(-1);
    }
    RTCGeometry geom = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_TRIANGLE);
    // Vertices
    Point3f* vertices = (Point3f*)rtcSetNewGeometryBuffer(geom, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, sizeof(Point3f), 8);
    vertices[0].x = -0.5; vertices[0].y = -0.5; vertices[0].z = -0.5;
    vertices[1].x =  0.5; vertices[1].y = -0.5; vertices[1].z = -0.5;
    vertices[2].x = -0.5; vertices[2].y =  0.5; vertices[2].z = -0.5;
    vertices[3].x =  0.5; vertices[3].y =  0.5; vertices[3].z = -0.5;
    vertices[4].x = -0.5; vertices[4].y = -0.5; vertices[4].z =  0.5;
    vertices[5].x =  0.5; vertices[5].y = -0.5; vertices[5].z =  0.5;
    vertices[6].x = -0.5; vertices[6].y =  0.5; vertices[6].z =  0.5;
    vertices[7].x =  0.5; vertices[7].y =  0.5; vertices[7].z =  0.5;
    for (int i = 0; i < 8; ++i)
        vertices[i] = (*objectToWorld)(vertices[i]);
    // Indices
    Point3u* indices = (Point3u*)rtcSetNewGeometryBuffer(geom, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3, sizeof(Point3u), 12);
    indices[ 0].x = 0; indices[ 0].y = 1, indices[ 0].z = 2;
    indices[ 1].x = 2; indices[ 1].y = 1, indices[ 1].z = 3;
    indices[ 2].x = 1; indices[ 2].y = 0, indices[ 2].z = 4;
    indices[ 3].x = 1; indices[ 3].y = 4, indices[ 3].z = 5;
    indices[ 4].x = 0; indices[ 4].y = 2, indices[ 4].z = 4;
    indices[ 5].x = 4; indices[ 5].y = 2, indices[ 5].z = 6;
    indices[ 6].x = 3; indices[ 6].y = 1, indices[ 6].z = 5;
    indices[ 7].x = 3; indices[ 7].y = 5, indices[ 7].z = 7;
    indices[ 8].x = 2; indices[ 8].y = 3, indices[ 8].z = 6;
    indices[ 9].x = 6; indices[ 9].y = 3, indices[ 9].z = 7;
    indices[10].x = 5; indices[10].y = 4, indices[10].z = 6;
    indices[11].x = 5; indices[11].y = 6, indices[11].z = 7;
    // Commit
    rtcCommitGeometry(geom);
    unsigned geomID = rtcAttachGeometry(scene, geom);
    rtcReleaseGeometry(geom);
    return geomID;
}
#endif
