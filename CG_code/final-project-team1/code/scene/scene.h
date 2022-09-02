#ifndef SCENE_H
#define SCENE_H

#include <geometrics.h>
#include <aggregate.h>
#include <medium.h>

class Light;
class SurfaceInteraction;

class Scene
{
public:
    // Fields
    const shared_ptr<Aggregate> aggregate;
    const vector<shared_ptr<Light>> lights;
    // Constructors
    Scene(const shared_ptr<Aggregate>& aggregate, 
          const vector<shared_ptr<Light>>& lights);

    // Methods
    bool Intersect(const Ray& ray, Float* tHit, SurfaceInteraction* si) const;
    void Intersect4(const Ray ray[4], Float tHit[4], SurfaceInteraction si[4], bool hits[4]) const;
    bool IntersectTr(const Ray& ray, Sampler& sampler, const Medium* medium, Float* tHit, SurfaceInteraction* si, Spectrum* tr) const;
    void IntersectTr4(const Ray ray[4], Sampler& sampler, const Medium* medium[4], Float tHit[4], SurfaceInteraction si[4], Spectrum tr[4], bool hits[4]) const;
    Spectrum Tr(const Point3f& p0, const Point3f& p1, Sampler& sampler, const Medium* medium) const;
private:
};

#endif