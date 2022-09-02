#ifndef TRIANGLE_H
#define TRIANGLE_H

#include <shape.h>

class Triangle;

class TriangleMesh
{
public:
    // Fields
    const shared_ptr<vector<Point3f>> vertexP;
    const shared_ptr<vector<Point3i>> vertexPIndices;
    const shared_ptr<vector<Normal3f>> vertexNg;
    const shared_ptr<vector<Point3i>> vertexNgIndices;

    // Constructors
    TriangleMesh(const shared_ptr<vector<Point3f>>& vertexP, const shared_ptr<vector<Point3i>>& vertexPIndices, 
                 const shared_ptr<vector<Normal3f>>& vertexNg, const shared_ptr<vector<Point3i>>& vertexNgIndices);

    // Methods
    static vector<shared_ptr<Triangle>> GenerateTriangles(
        const shared_ptr<TriangleMesh>& mesh, 
        const Transform* objectToWorld, const Transform* worldToObject);
};

class Triangle : public TransformedShape
{
public:
    // Constructors
    Triangle(const Transform* objectToWorld, const Transform* worldToObject, 
             const shared_ptr<TriangleMesh>& mesh, int pIdx, int ngIdx);

    // Methods
    virtual bool Intersect(const Ray& ray, Float* tHit, SurfaceInteraction* si) const override;
    virtual AABound3f ObjectBound() const override;
    virtual Float Area() const override;
    virtual Normal3f Ng(const Ray& incidentRayWorld) const override;

    virtual Interaction Sample(const Point2f& samples, Float* pdf) const override;

    #ifdef INTEL
    virtual int GenerateEmbreeGeometry(RTCDevice device, RTCScene scene) const override;
    #endif
private:
    // Fields
    shared_ptr<TriangleMesh> mesh;
    int pIdx;
    int ngIdx;
    Float area;
};

#endif