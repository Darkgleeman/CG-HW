#include <scene.h>

// Constructors
Scene::Scene(const shared_ptr<Aggregate>& aggregate, 
             const vector<shared_ptr<Light>>& lights) : 
    aggregate(aggregate), 
    lights(lights)
{ }

// Methods
bool Scene::Intersect(const Ray& ray, Float* tHit, SurfaceInteraction* si) const
{
    return aggregate->Intersect(ray, tHit, si);
}

void Scene::Intersect4(const Ray ray[4], Float tHit[4], SurfaceInteraction si[4], bool hits[4]) const
{
    aggregate->Intersect4(ray, tHit, si, hits);
}

bool Scene::IntersectTr(const Ray& ray, Sampler& sampler, const Medium* medium, Float* tHit, SurfaceInteraction* si, Spectrum* tr) const
{
    Ray r(ray);
    *tr = Spectrum(Float(1));
    Spectrum mediumScatteringWeigh(Float(1));
    int numIterations = 0;
    while (true)
    {
        ++numIterations;
        if (numIterations / 500 > 0 && numIterations % 500 == 0)
            printf("\n\n(Thread %d): Long loop detected in Scene::IntersectTr(): r=(%f,%f,%f; %f,%f,%f; %f)\n\n", omp_get_thread_num(), r.o.x, r.o.y, r.o.z, r.d.x, r.d.y, r.d.z, r.tMax);

        bool hitSurface = Intersect(r, tHit, si); // TODO: 草，才发现这个 tHit 真是屌用没有啊，不是，我他妈当时为什么要搞这个参数……

        if (si->surface.IsMediumTransition())
        {
            Assert(medium == si->surface.mediumIn || medium == si->surface.mediumOu, 
                   "Scene::IntersectTr(): Medium mismatch");
            const Medium* newMedium;
            if (si->byEmbree)
                newMedium = Vector3f::Dot(si->ng, r.d) > 0 ? si->surface.mediumOu : si->surface.mediumIn;
            else
                newMedium = Vector3f::Dot(si->ng, r.d) < 0 ? si->surface.mediumOu : si->surface.mediumIn;
            if (medium != newMedium)
                mediumScatteringWeigh = Spectrum(Float(1));
            medium = newMedium;
        }

        if (medium)
            *tr *= medium->Tr(r, sampler);

        if (!hitSurface)
            return false;
        
        if (!si->primitive->IsVolume())
            return true; // TODO: Ray.tMax 怎么办

        r = Ray(Point3f(si->p + 0.0075 * r.d), r.d);
    }
}

void Scene::IntersectTr4(const Ray ray[4], Sampler& sampler, const Medium* medium[4], Float tHit[4], SurfaceInteraction si[4], Spectrum tr[4], bool hits[4]) const
{
    Ray r[4] = { ray[0], ray[1], ray[2], ray[3] };
    tr[0] = tr[1] = tr[2] = tr[3] = Spectrum(Float(1));
    Spectrum mediumScatteringWeigh[4] = { Spectrum(Float(1)), Spectrum(Float(1)), Spectrum(Float(1)), Spectrum(Float(1)) };
    bool hasFinished[4] = { false, false, false, false };
    while (true)
    {
        bool hitSurface[4];
        Intersect4(r, tHit, si, hitSurface); // TODO: 草，才发现这个 tHit 真是屌用没有啊，不是，我他妈当时为什么要搞这个参数……

        for (int i = 0; i < 4; ++i)
        {
            if (hasFinished[i])
                continue;

            if (si[i].surface.IsMediumTransition())
            {
                Assert(medium[i] == si[i].surface.mediumIn || medium[i] == si[i].surface.mediumOu, 
                    "Scene::IntersectTr(): Medium mismatch");
                const Medium* newMedium;
                if (si[i].byEmbree)
                    newMedium = Vector3f::Dot(si[i].ng, r[i].d) > 0 ? si[i].surface.mediumOu : si[i].surface.mediumIn;
                else
                    newMedium = Vector3f::Dot(si[i].ng, r[i].d) < 0 ? si[i].surface.mediumOu : si[i].surface.mediumIn;
                if (medium[i] != newMedium)
                    mediumScatteringWeigh[i] = Spectrum(Float(1));
                medium[i] = newMedium;
            }

            if (medium[i])
                tr[i] *= medium[i]->Tr(r[i], sampler);

            if (!hitSurface[i])
            {
                hits[i] = false;
                hasFinished[i] = true;
                continue;
            }
            
            if (!si[i].primitive->IsVolume())
            {
                hits[i] = true;
                hasFinished[i] = true;
                continue;
            }

            r[i] = Ray(Point3f(si[i].p + 0.0075 * r[i].d), r[i].d);
        }
    }
}

Spectrum Scene::Tr(const Point3f& p0, const Point3f& p1, Sampler& sampler, const Medium* medium) const
{
    Vector3f relax(0.005 * (p1 - p0));
    Point3f pStart(p0);
    Point3f pEndRelaxed(p1 - relax);
    Spectrum tr(Float(1));
    Spectrum mediumScatteringWeigh(Float(1));
    while (true)
    {
        Float tHit;
        SurfaceInteraction si;
        Point3f pStartRelaxed(pStart + relax);
        Ray rayRelaxed(pStartRelaxed, Vector3f(pEndRelaxed - pStartRelaxed), 1);
        bool hitSurface = Intersect(rayRelaxed, &tHit, &si);

        if (hitSurface && !si.primitive->IsVolume())
            return Spectrum();
        
        if (si.surface.IsMediumTransition())
        {
            Assert(medium == si.surface.mediumIn || medium == si.surface.mediumOu, 
                   "Scene::Tr(): Medium mismatch");
            const Medium* newMedium;
            if (si.byEmbree)
                newMedium = Vector3f::Dot(si.ng, rayRelaxed.d) > 0 ? si.surface.mediumOu : si.surface.mediumIn;
            else
                newMedium = Vector3f::Dot(si.ng, rayRelaxed.d) < 0 ? si.surface.mediumOu : si.surface.mediumIn;
            if (medium != newMedium)
                mediumScatteringWeigh = Spectrum(Float(1));
            medium = newMedium;
        }

        if (medium)
            tr *= medium->Tr(rayRelaxed, sampler);
        
        if (!hitSurface)
            break;
        
        pStart = si.p;
    }
    return tr;
}