#include <boundingBox.h>

//* AABound3f --------------------------------------------------------------------------------------------------*//

// Constructor
AABound3f::AABound3f() : pMin(Infinity, Infinity, Infinity), pMax(-Infinity, -Infinity, -Infinity) { }
AABound3f::AABound3f(const Point3f& p) : pMin(p), pMax(p) { }
AABound3f::AABound3f(const Point3f& p1, const Point3f& p2) : pMin(std::min(p1.x, p2.x), std::min(p1.y, p2.y), std::min(p1.z, p2.z)), pMax(std::max(p1.x, p2.x), std::max(p1.y, p2.y), std::max(p1.z, p2.z)) { }

// Methods
bool AABound3f::Intersect(const Ray& ray, Float* tHit0/* = nullptr*/, Float* tHit1/* = nullptr*/) const
{
    Float t0 = 0, t1 = ray.tMax;
    for (int i = 0; i < 3; ++i)
    {
        Float invRayDir = 1 / ray.d[i];
        Float tNear = (pMin[i] - ray.o[i]) * invRayDir;
        Float tFar  = (pMax[i] - ray.o[i]) * invRayDir;
        if (tNear > tFar)
            std::swap(tNear, tFar);
        t0 = tNear > t0 ? tNear : t0;
        t1 = tFar  < t1 ? tFar  : t1;
        if (t0 > t1)
            return false;
    }
    if (tHit0) *tHit0 = t0;
    if (tHit1) *tHit1 = t1;
    return true;
}
bool AABound3f::Intersect(const Ray& ray, const Vector3f& dirInv, const int isDirNeg[3]) const
{
    const AABound3f &bounds = *this;
    Float tMin =  (bounds[  isDirNeg[0]].x - ray.o.x) * dirInv.x;
    Float tMax =  (bounds[1-isDirNeg[0]].x - ray.o.x) * dirInv.x;
    Float tyMin = (bounds[  isDirNeg[1]].y - ray.o.y) * dirInv.y;
    Float tyMax = (bounds[1-isDirNeg[1]].y - ray.o.y) * dirInv.y;
    tMax *= 1 + 2 * Gamma(3);
    tyMax *= 1 + 2 * Gamma(3);
    if (tMin > tyMax || tyMin > tMax) 
        return false;
    if (tyMin > tMin) tMin = tyMin; 
    if (tyMax < tMax) tMax = tyMax;

    Float tzMin = (bounds[  isDirNeg[2]].z - ray.o.z) * dirInv.z; 
    Float tzMax = (bounds[1-isDirNeg[2]].z - ray.o.z) * dirInv.z; 
    tzMax *= 1 + 2 * Gamma(3);
    if (tMin > tzMax || tzMin > tMax) 
        return false; 
    if (tzMin > tMin) 
        tMin = tzMin; 
    if (tzMax < tMax) 
        tMax = tzMax;

    return (tMin < ray.tMax) && (tMax > 0);
}
bool AABound3f::Intersect(const Point3f& p0, const Point3f& p1, const Point3f& p2) const
{
    Vector3f extents = Vector3f(0.5 * (pMax - pMin)); // 半轴
    Point3f center = Point3f(pMin + extents);
    Point3f p0_boundSpace = Point3f(p0 - center);
    Point3f p1_boundSpace = Point3f(p1 - center);
    Point3f p2_boundSpace = Point3f(p2 - center);
    Vector3f p0p1 = Vector3f(p1 - p0);
    Vector3f p1p2 = Vector3f(p2 - p1);
    Vector3f p2p0 = Vector3f(p0 - p2);

    Float p0_proj, p1_proj, p2_proj;
    Float p_projMin, p_projMax;
    Float r_projMax;

    // Test 9 axis: AABB 和三角形的边叉乘

    Vector3f axis_n0_p0p1(Vector3f::Cross(Vector3f(1, 0, 0), p0p1));
    Vector3f axis_n0_p1p2(Vector3f::Cross(Vector3f(1, 0, 0), p1p2));
    Vector3f axis_n0_p2p0(Vector3f::Cross(Vector3f(1, 0, 0), p2p0));
    Vector3f axis_n1_p0p1(Vector3f::Cross(Vector3f(0, 1, 0), p0p1));
    Vector3f axis_n1_p1p2(Vector3f::Cross(Vector3f(0, 1, 0), p1p2));
    Vector3f axis_n1_p2p0(Vector3f::Cross(Vector3f(0, 1, 0), p2p0));
    Vector3f axis_n2_p0p1(Vector3f::Cross(Vector3f(0, 0, 1), p0p1));
    Vector3f axis_n2_p1p2(Vector3f::Cross(Vector3f(0, 0, 1), p1p2));
    Vector3f axis_n2_p2p0(Vector3f::Cross(Vector3f(0, 0, 1), p2p0));
    Vector3f axis_triangleNormal(Vector3f::Cross(p0p1, p1p2));

    p0_proj = Vector3f::Dot(p0_boundSpace, axis_n0_p0p1);
    p1_proj = Vector3f::Dot(p1_boundSpace, axis_n0_p0p1);
    p2_proj = Vector3f::Dot(p2_boundSpace, axis_n0_p0p1);
    p_projMin = Min(p0_proj, p1_proj, p2_proj);
    p_projMax = Max(p0_proj, p1_proj, p2_proj);
    r_projMax = std::abs(Vector3f::Dot(Vector3f(extents.x, 0, 0), axis_n0_p0p1)) + 
                std::abs(Vector3f::Dot(Vector3f(0, extents.y, 0), axis_n0_p0p1)) + 
                std::abs(Vector3f::Dot(Vector3f(0, 0, extents.z), axis_n0_p0p1));
    if (p_projMax < -r_projMax || p_projMin > r_projMax) return false;

    p0_proj = Vector3f::Dot(p0_boundSpace, axis_n0_p1p2);
    p1_proj = Vector3f::Dot(p1_boundSpace, axis_n0_p1p2);
    p2_proj = Vector3f::Dot(p2_boundSpace, axis_n0_p1p2);
    p_projMin = Min(p0_proj, p1_proj, p2_proj);
    p_projMax = Max(p0_proj, p1_proj, p2_proj);
    r_projMax = std::abs(Vector3f::Dot(Vector3f(extents.x, 0, 0), axis_n0_p1p2)) + 
                std::abs(Vector3f::Dot(Vector3f(0, extents.y, 0), axis_n0_p1p2)) + 
                std::abs(Vector3f::Dot(Vector3f(0, 0, extents.z), axis_n0_p1p2));
    if (p_projMax < -r_projMax || p_projMin > r_projMax) return false;

    p0_proj = Vector3f::Dot(p0_boundSpace, axis_n0_p2p0);
    p1_proj = Vector3f::Dot(p1_boundSpace, axis_n0_p2p0);
    p2_proj = Vector3f::Dot(p2_boundSpace, axis_n0_p2p0);
    p_projMin = Min(p0_proj, p1_proj, p2_proj);
    p_projMax = Max(p0_proj, p1_proj, p2_proj);
    r_projMax = std::abs(Vector3f::Dot(Vector3f(extents.x, 0, 0), axis_n0_p2p0)) + 
                std::abs(Vector3f::Dot(Vector3f(0, extents.y, 0), axis_n0_p2p0)) + 
                std::abs(Vector3f::Dot(Vector3f(0, 0, extents.z), axis_n0_p2p0));
    if (p_projMax < -r_projMax || p_projMin > r_projMax) return false;

    p0_proj = Vector3f::Dot(p0_boundSpace, axis_n1_p0p1);
    p1_proj = Vector3f::Dot(p1_boundSpace, axis_n1_p0p1);
    p2_proj = Vector3f::Dot(p2_boundSpace, axis_n1_p0p1);
    p_projMin = Min(p0_proj, p1_proj, p2_proj);
    p_projMax = Max(p0_proj, p1_proj, p2_proj);
    r_projMax = std::abs(Vector3f::Dot(Vector3f(extents.x, 0, 0), axis_n1_p0p1)) + 
                std::abs(Vector3f::Dot(Vector3f(0, extents.y, 0), axis_n1_p0p1)) + 
                std::abs(Vector3f::Dot(Vector3f(0, 0, extents.z), axis_n1_p0p1));
    if (p_projMax < -r_projMax || p_projMin > r_projMax) return false;

    p0_proj = Vector3f::Dot(p0_boundSpace, axis_n1_p1p2);
    p1_proj = Vector3f::Dot(p1_boundSpace, axis_n1_p1p2);
    p2_proj = Vector3f::Dot(p2_boundSpace, axis_n1_p1p2);
    p_projMin = Min(p0_proj, p1_proj, p2_proj);
    p_projMax = Max(p0_proj, p1_proj, p2_proj);
    r_projMax = std::abs(Vector3f::Dot(Vector3f(extents.x, 0, 0), axis_n1_p1p2)) + 
                std::abs(Vector3f::Dot(Vector3f(0, extents.y, 0), axis_n1_p1p2)) + 
                std::abs(Vector3f::Dot(Vector3f(0, 0, extents.z), axis_n1_p1p2));
    if (p_projMax < -r_projMax || p_projMin > r_projMax) return false;

    p0_proj = Vector3f::Dot(p0_boundSpace, axis_n1_p2p0);
    p1_proj = Vector3f::Dot(p1_boundSpace, axis_n1_p2p0);
    p2_proj = Vector3f::Dot(p2_boundSpace, axis_n1_p2p0);
    p_projMin = Min(p0_proj, p1_proj, p2_proj);
    p_projMax = Max(p0_proj, p1_proj, p2_proj);
    r_projMax = std::abs(Vector3f::Dot(Vector3f(extents.x, 0, 0), axis_n1_p2p0)) + 
                std::abs(Vector3f::Dot(Vector3f(0, extents.y, 0), axis_n1_p2p0)) + 
                std::abs(Vector3f::Dot(Vector3f(0, 0, extents.z), axis_n1_p2p0));
    if (p_projMax < -r_projMax || p_projMin > r_projMax) return false;

    p0_proj = Vector3f::Dot(p0_boundSpace, axis_n2_p0p1);
    p1_proj = Vector3f::Dot(p1_boundSpace, axis_n2_p0p1);
    p2_proj = Vector3f::Dot(p2_boundSpace, axis_n2_p0p1);
    p_projMin = Min(p0_proj, p1_proj, p2_proj);
    p_projMax = Max(p0_proj, p1_proj, p2_proj);
    r_projMax = std::abs(Vector3f::Dot(Vector3f(extents.x, 0, 0), axis_n2_p0p1)) + 
                std::abs(Vector3f::Dot(Vector3f(0, extents.y, 0), axis_n2_p0p1)) + 
                std::abs(Vector3f::Dot(Vector3f(0, 0, extents.z), axis_n2_p0p1));
    if (p_projMax < -r_projMax || p_projMin > r_projMax) return false;

    p0_proj = Vector3f::Dot(p0_boundSpace, axis_n2_p1p2);
    p1_proj = Vector3f::Dot(p1_boundSpace, axis_n2_p1p2);
    p2_proj = Vector3f::Dot(p2_boundSpace, axis_n2_p1p2);
    p_projMin = Min(p0_proj, p1_proj, p2_proj);
    p_projMax = Max(p0_proj, p1_proj, p2_proj);
    r_projMax = std::abs(Vector3f::Dot(Vector3f(extents.x, 0, 0), axis_n2_p1p2)) + 
                std::abs(Vector3f::Dot(Vector3f(0, extents.y, 0), axis_n2_p1p2)) + 
                std::abs(Vector3f::Dot(Vector3f(0, 0, extents.z), axis_n2_p1p2));
    if (p_projMax < -r_projMax || p_projMin > r_projMax) return false;

    p0_proj = Vector3f::Dot(p0_boundSpace, axis_n2_p2p0);
    p1_proj = Vector3f::Dot(p1_boundSpace, axis_n2_p2p0);
    p2_proj = Vector3f::Dot(p2_boundSpace, axis_n2_p2p0);
    p_projMin = Min(p0_proj, p1_proj, p2_proj);
    p_projMax = Max(p0_proj, p1_proj, p2_proj);
    r_projMax = std::abs(Vector3f::Dot(Vector3f(extents.x, 0, 0), axis_n2_p2p0)) + 
                std::abs(Vector3f::Dot(Vector3f(0, extents.y, 0), axis_n2_p2p0)) + 
                std::abs(Vector3f::Dot(Vector3f(0, 0, extents.z), axis_n2_p2p0));
    if (p_projMax < -r_projMax || p_projMin > r_projMax) return false;

    // Test 3

    p0_proj = p0_boundSpace.x;
    p1_proj = p1_boundSpace.x;
    p2_proj = p2_boundSpace.x;
    p_projMin = Min(p0_proj, p1_proj, p2_proj);
    p_projMax = Max(p0_proj, p1_proj, p2_proj);
    r_projMax = extents.x;
    if (p_projMax < -r_projMax || p_projMin > r_projMax) return false;

    p0_proj = p0_boundSpace.y;
    p1_proj = p1_boundSpace.y;
    p2_proj = p2_boundSpace.y;
    p_projMin = Min(p0_proj, p1_proj, p2_proj);
    p_projMax = Max(p0_proj, p1_proj, p2_proj);
    r_projMax = extents.y;
    if (p_projMax < -r_projMax || p_projMin > r_projMax) return false;

    p0_proj = p0_boundSpace.z;
    p1_proj = p1_boundSpace.z;
    p2_proj = p2_boundSpace.z;
    p_projMin = Min(p0_proj, p1_proj, p2_proj);
    p_projMax = Max(p0_proj, p1_proj, p2_proj);
    r_projMax = extents.z;
    if (p_projMax < -r_projMax || p_projMin > r_projMax) return false;
    
    // Test 1

    p_projMin = p_projMax = Vector3f::Dot(p0_boundSpace, axis_triangleNormal);
    r_projMax = std::abs(Vector3f::Dot(Vector3f(extents.x, 0, 0), axis_triangleNormal)) + 
                std::abs(Vector3f::Dot(Vector3f(0, extents.y, 0), axis_triangleNormal)) + 
                std::abs(Vector3f::Dot(Vector3f(0, 0, extents.z), axis_triangleNormal));
    if (p_projMax < -r_projMax || p_projMin > r_projMax) return false;

    return true;
}
Point3f AABound3f::Center() const
{
    return Point3f((pMin + pMax) * 0.5);
}
Point3f AABound3f::Lerp(const Vector3f& t) const
{
    return Point3f(
        ::Lerp(pMin.x, pMax.x, t.x), 
        ::Lerp(pMin.y, pMax.y, t.y), 
        ::Lerp(pMin.z, pMax.z, t.z)
    );
}
/*static*/ AABound3f AABound3f::Union(const AABound3f& bound, const Point3f& point)
{
    return AABound3f(
        Point3f(std::min(bound.pMin.x, point.x), std::min(bound.pMin.y, point.y), std::min(bound.pMin.z, point.z)), 
        Point3f(std::max(bound.pMax.x, point.x), std::max(bound.pMax.y, point.y), std::max(bound.pMax.z, point.z)));
}
/*static*/ AABound3f AABound3f::Union(const AABound3f& bound1, const AABound3f& bound2)
{
    return AABound3f(
        Point3f(std::min(bound1.pMin.x, bound2.pMin.x), std::min(bound1.pMin.y, bound2.pMin.y), std::min(bound1.pMin.z, bound2.pMin.z)),
        Point3f(std::max(bound1.pMax.x, bound2.pMax.x), std::max(bound1.pMax.y, bound2.pMax.y), std::max(bound1.pMax.z, bound2.pMax.z)));
}

Vector3f AABound3f::Diagonal() const
{
    return Vector3f(pMax - pMin);
}

int AABound3f::MaximumExtent() const
{
    Vector3f diagonal = Diagonal();
    if (diagonal.x > diagonal.y && diagonal.x > diagonal.z)
        return 0;
    else if (diagonal.y > diagonal.z)
        return 1;
    else
        return 2;
}

Vector3f AABound3f::Offset(const Point3f& p) const
{
    Vector3f o(p - pMin);
    if (pMax.x > pMin.x) o.x /= pMax.x - pMin.x;
    if (pMax.y > pMin.y) o.y /= pMax.y - pMin.y;
    if (pMax.z > pMin.z) o.z /= pMax.z - pMin.z;
    return o;
}

Float AABound3f::Area() const
{
    Vector3f diagonal = Diagonal();
    return 2 * (diagonal.x * diagonal.y + diagonal.x * diagonal.z + diagonal.y * diagonal.z);
}

// Operators
Point3f& AABound3f::operator[](int i) { return i == 0 ? pMin : pMax; }
const Point3f& AABound3f::operator[](int i) const { return i == 0 ? pMin : pMax; }



//* AABound3i --------------------------------------------------------------------------------------------------*//

// Constructors
AABound3i::AABound3i() : pMin(IntMax, IntMax, IntMax), pMax(IntMin, IntMin, IntMin) { }
AABound3i::AABound3i(const Point3i& p) : pMin(p), pMax(p) { }
AABound3i::AABound3i(const Point3i& p1, const Point3i& p2) : pMin(std::min(p1.x, p2.x), std::min(p1.y, p2.y), std::min(p1.z, p2.z)), pMax(std::max(p1.x, p2.x), std::max(p1.y, p2.y), std::max(p1.z, p2.z)) { }