#ifndef MESH_H
#define MESH_H

#include <utils.h>
#include <shape.h>

enum class MeshFileType
{
    CS171, 
    OBJ, 
    TINY_OBJ, 
};

enum class MeshLoadFlag : int
{
    Null               = 0, 
    RHandToLHand       = 1 << 0, 
    YStartFrom0        = 1 << 1, 
    XZToCenter         = 1 << 2, 
    RecalculateNormal  = 1 << 3, 
    ToFile             = 1 << 4, 
    All = RHandToLHand | YStartFrom0 | XZToCenter | RecalculateNormal | ToFile
};

class Mesh : public TransformedShape
{
public:
    // Fields
    const shared_ptr<const vector<Point3f>>  vertexP;
    const shared_ptr<const vector<Point3i>>  vertexPIndices;
    const shared_ptr<const vector<Normal3f>> vertexNg;
    const shared_ptr<const vector<Point3i>>  vertexNgIndices;
    const Point3i gridDepth; // depth = (2, 2, 2) 即为一个 2 阶魔方
    const Float meshToGridScale;
    const Float meshToGridScaleInv;
    const Vector3f meshToGridTranslation;

    // Constructors
    Mesh(const Transform* objectToWorld, const Transform* worldToObject, 
         const shared_ptr<const vector<Point3f>>& vertexP, const shared_ptr<const vector<Point3i>>& vertexPIndices, 
         const shared_ptr<const vector<Normal3f>>& vertexNg, const shared_ptr<const vector<Point3i>>& vertexNgIndices, 
         const Point3i& gridDepth, 
         Float meshToGridScale, const Vector3f& meshToGridTranslation);

    // Methods
    /**
     * @param maxGridDepthPerDim 每个维度最大允许的 depth，必须为 2 的指数
     * @param gridDepth 加载的 mesh 每个维度的 depth，依赖于 meshToGridScale 和 meshToGridTranslation，必为 2 的指数
     * @param meshToGridScale 加载的 mesh 在 grid coordinate 被放置在 [0, depth.x) × [0, depth.y) × [0, depth.z)，scale 即为从 mesh coordinate 变换到 grid coordinate 需要应用的比例
     * @param meshToGridTranslation 应用 meshToGridScale 之后需要应用的位移
     */
    static void LoadFromFile(const string& path, int maxGridDepthPerDim, MeshFileType fileType, MeshLoadFlag loadFlags, 
                             shared_ptr<vector<Point3f>>* vertexP, shared_ptr<vector<Point3i>>* vertexPIndices, 
                             shared_ptr<vector<Normal3f>>* vertexNg, shared_ptr<vector<Point3i>>* vertexNgIndices, 
                             Point3i* gridDepth, 
                             Float* meshToGridScale, Vector3f* meshToGridTranslation);
    static void LoadFromFile(const string& path, 
                             shared_ptr<vector<Point3f>>* vertexP, shared_ptr<vector<Point3i>>* vertexPIndices, 
                             shared_ptr<vector<Normal3f>>* vertexNg, shared_ptr<vector<Point3i>>* vertexNgIndices, 
                             Point3i* gridDepth, 
                             Float* meshToGridScale, Vector3f* meshToGridTranslation);
    virtual bool Intersect(const Ray& ray, Float* tHit, SurfaceInteraction* si) const override;
    virtual AABound3f ObjectBound() const override;
    virtual Float Area() const override;
    virtual Normal3f Ng(const Ray& incidentRayWorld) const override;

    virtual Interaction Sample(const Point2f& samples, Float* pdf) const override;
    virtual Interaction Sample(const Interaction& ref, 
                               const Point2f& samples, Float* pdf) const override;
private:
    Float area;
};

#endif