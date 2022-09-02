#include <embreeAccel.h>

#ifdef INTEL

// Constructors
EmbreeAccel::EmbreeAccel(const vector<shared_ptr<Primitive>>& primitives) : 
    primitives(primitives)
{
    device = rtcNewDevice(NULL);
    scene = rtcNewScene(device);
    for (const shared_ptr<Primitive>& primitive : primitives)
        primitive->GenerateEmbreeGeometry(device, scene);
    rtcCommitScene(scene);
}
EmbreeAccel::~EmbreeAccel()
{
    rtcReleaseScene(scene);
    rtcReleaseDevice(device);
}

// Methods
/*override*/ bool EmbreeAccel::Intersect(const Ray& ray, Float* tHit, SurfaceInteraction* si) const
{
    // Copy to embree ray
    RTCRayHit rayHit;
    rayHit.ray.org_x = ray.o.x;
    rayHit.ray.org_y = ray.o.y;
    rayHit.ray.org_z = ray.o.z;
    rayHit.ray.dir_x = ray.d.x;
    rayHit.ray.dir_y = ray.d.y;
    rayHit.ray.dir_z = ray.d.z;
    rayHit.ray.tnear = 0;
    rayHit.ray.tfar = ray.tMax;
    rayHit.ray.mask = 0;
    rayHit.ray.flags = 0;
    rayHit.hit.geomID = RTC_INVALID_GEOMETRY_ID;
    rayHit.hit.instID[0] = RTC_INVALID_GEOMETRY_ID;

    // Ray casting 
    RTCIntersectContext context;
    rtcInitIntersectContext(&context);
    rtcIntersect1(scene, &context, &rayHit);

    // Check intersection
    if (rayHit.hit.geomID != RTC_INVALID_GEOMETRY_ID)
    {
        //! 千万别漏初始化！
        // TODO: 千万别漏初始化！
        // Set interaction information
        ray.tMax = *tHit = rayHit.ray.tfar;
        si->p = ray(ray.tMax);
        si->ng.x = rayHit.hit.Ng_x;
        si->ng.y = rayHit.hit.Ng_y;
        si->ng.z = rayHit.hit.Ng_z;
        Vector3f::CoordinateSystem(si->ng, &si->sg, &si->tg);
        si->woWorld = Vector3f(-ray.d);
        si->byEmbree = true;

        // Set surface interaction information
        // TODO: si->uv = ...;
        // TODO: si->rayONextPixelX = ...;
        // TODO: si->rayONextPixelY = ...;
        // TODO: si->rayDNextPixelX = ...;
        // TODO: si->rayDNextPixelY = ...;
        si->primitive = primitives[rayHit.hit.geomID].get();
        si->surface = primitives[rayHit.hit.geomID]->GetSurface();
        return true;
    }
    else
        return false;
}

/*override*/ void EmbreeAccel::Intersect4(const Ray rays[4], Float tHit[4], SurfaceInteraction si[4], bool hits[4]) const
{
    // Generate embree ray
    RTCRayHit4 rayHits;
    for (int i = 0; i < 4; ++i)
    {
        rayHits.ray.org_x[i] = rays[i].o.x;
        rayHits.ray.org_y[i] = rays[i].o.y;
        rayHits.ray.org_z[i] = rays[i].o.z;
        rayHits.ray.dir_x[i] = rays[i].d.x;
        rayHits.ray.dir_y[i] = rays[i].d.y;
        rayHits.ray.dir_z[i] = rays[i].d.z;
        rayHits.ray.tnear[i] = 0;
        rayHits.ray.tfar[i] = rays[i].tMax;
        rayHits.ray.mask[i] = 0;
        rayHits.ray.flags[i] = 0;
        rayHits.hit.geomID[i] = RTC_INVALID_GEOMETRY_ID;
        rayHits.hit.instID[0][i] = RTC_INVALID_GEOMETRY_ID;
    }

    // Ray casting
    RTCIntersectContext context;
    rtcInitIntersectContext(&context);
    int valid[4] = { -1, -1, -1, -1 };
    rtcIntersect4(valid, scene, &context, &rayHits);

    // Check intersection
    for (int i = 0; i < 4; ++i)
    {
        if (rayHits.hit.geomID[i] != RTC_INVALID_GEOMETRY_ID)
        {
            //! 千万别漏初始化！
            // TODO: 千万别漏初始化！
            // Set interaction information
            rays[i].tMax = tHit[i] = rayHits.ray.tfar[i];
            si[i].p = rays[i](rays[i].tMax);
            si[i].ng.x = rayHits.hit.Ng_x[i];
            si[i].ng.y = rayHits.hit.Ng_y[i];
            si[i].ng.z = rayHits.hit.Ng_z[i];
            Vector3f::CoordinateSystem(si[i].ng, &si[i].sg, &si[i].tg);
            si[i].woWorld = Vector3f(-rays[i].d);
            si[i].byEmbree = true;

            // Set surface interaction information
            // TODO: si[i].uv = ...;
            // TODO: si[i].rayONextPixelX = ...;
            // TODO: si[i].rayONextPixelY = ...;
            // TODO: si[i].rayDNextPixelX = ...;
            // TODO: si[i].rayDNextPixelY = ...;
            si[i].primitive = primitives[rayHits.hit.geomID[i]].get();
            si[i].surface = primitives[rayHits.hit.geomID[i]]->GetSurface();
            hits[i] = true;
        }
        else
            hits[i] = false;
    }
}

/*override*/ AABound3f EmbreeAccel::WorldBound() const
{
    // TODO: Implement this
}

/*virtual*/ bool EmbreeAccel::IsEmbreeAccel() const
{
    return true;
}

#endif