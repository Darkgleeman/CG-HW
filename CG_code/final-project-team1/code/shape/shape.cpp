#include <shape.h>

// Methods
Float Shape::PdfArea() const
{
    return 1 / Area();
}



// Constructors
TransformedShape::TransformedShape(const Transform* objectToWorld, const Transform* worldToObject) : 
    objectToWorld(objectToWorld), worldToObject(worldToObject) { }

// Methods
/*override*/ AABound3f TransformedShape::WorldBound() const
{
    return (*objectToWorld)(ObjectBound());
}

/*override*/ Interaction TransformedShape::Sample(
        const Interaction& ref, 
        const Point2f& samples, Float* pdf) const
{
    // 对面积采样，获取 pdf(A)
    Float pdfByArea;
    Interaction surfaceSample = Sample(samples, &pdfByArea);
    *pdf = PdfW(ref, Vector3f(surfaceSample.p - ref.p));
    return surfaceSample;
}

/*override*/ Float TransformedShape::PdfW(const Interaction& ref, const Vector3f& rayDirToShape) const
{
    Vector3f rayDirToShape_unit(rayDirToShape.Normalized());
    Ray ray(Point3f(ref.p + 0.0075 * rayDirToShape), rayDirToShape);
    Float tHit;
    SurfaceInteraction si;

    // 至少也要和这个物体相交，否则无法计算 pdf
    if (!Intersect(ray, &tHit, &si))
        return 0;
    else
        return 
            Point3f::DistanceSquared(ref.p, si.p) * PdfArea() / 
            std::abs(Vector3f::Dot(si.ng.Normalized(), -rayDirToShape_unit));
}

// /*override*/ Float TransformedShape::PdfW(const Interaction& ref, const Scene& scene, const Interaction& surfaceSample) const
// {
//     // Ray cast 检测采样点 surfaceSample.p 是否可见
//     Float tHit_scene;
//     SurfaceInteraction si_scene;
//     Vector3f rayFromRef_d = Vector3f((surfaceSample.p - ref.p).Normalized());
//     Ray rayFromRef(Point3f(ref.p + 0.0075 * rayFromRef_d), rayFromRef_d);
//     if (scene.Intersect(rayFromRef, &tHit_scene, &si_scene) &&         // 与场景相交
//         Point3f::DistanceSquared(surfaceSample.p, si_scene.p) < 0.001) // 确实是该点
//     {
//         return 
//             Point3f::DistanceSquared(ref.p, surfaceSample.p) * PdfArea() / 
//             std::abs(Vector3f::Dot(surfaceSample.ng.Normalized(), -rayFromRef_d)); // TODO: 从数学的角度，这个负号不需要
//     }
//     else
//         return 0;
// }

// /*override*/ Float TransformedShape::PdfW(const Interaction& ref, const Scene& scene, const Vector3f& rayDirToShape) const
// {
//     // 从 ref 向 rayDirToShape 做 ray cast，得到 surfaceSample
//     Ray rayFromRef(Point3f(ref.p + 0.0075 * rayDirToShape), rayDirToShape);
//     Float tHit_shape;
//     SurfaceInteraction si_shape;
//     if (Intersect(rayFromRef, &tHit_shape, &si_shape)) // 与物体相交
//     {
//         return PdfW(ref, scene, si_shape);
//     }
//     else
//         return 0;
// }