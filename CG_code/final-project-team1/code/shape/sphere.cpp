#include <sphere.h>

// Constructors
Sphere::Sphere(const Transform* objectToWorld, const Transform* worldToObject) : TransformedShape(objectToWorld, worldToObject)
{
    // Area
    const Float p = std::log2((long double)3);
    const Float pInv = (long double)1 / std::log2((long double)3);
    Vector3f abc = Vector3f(0.5 * objectToWorld->Scale());
    Float ap = std::pow(abc.x, p), bp = std::pow(abc.y, p), cp = std::pow(abc.z, p);
    area = 2 * Pi * std::pow(ap * bp + ap * cp + bp * cp, pInv); // 对于半轴为 a, b, c 的椭球，表面积近似为 Area = 2π(a^p*b^p + a^p*c^p + b^p*c^p)^(1/p)
}

// Methods
/*override*/ bool Sphere::Intersect(const Ray& ray, Float* tHit, SurfaceInteraction* si) const
{
    // Ray cast 检测
    Ray rayLocal = (*worldToObject)(ray); // 将射线变换到 sphere local coordinate，即 center = (0, 0, 0), radius = 0.5
    Float a = rayLocal.d.NormSquared();
    Float b_half = Vector3f::Dot(rayLocal.o, rayLocal.d); // rayLocal.o = rayLocal.o - center
    Float c = rayLocal.o.NormSquared() - 0.25; // 0.25 = 0.5 * 0.5 = radius * radius
    Float discriminant = b_half * b_half - a * c; // 求根公式判别式
    if (discriminant < 0) // 不相交
        return false;
    Float d_sqrted = std::sqrt(discriminant);
    Float root = (-b_half - d_sqrted) / a;
    if (root < 0 || rayLocal.tMax < root) // 相交，但不在给定范围内
    {
        root = (-b_half + d_sqrted) / a;
        if (root < 0 || rayLocal.tMax < root)
            return false;
    }
    // Ray
    ray.tMax = *tHit = root; //! 减小 tMax，方便检测场景中的其他物体
    // Local si
    si->p = rayLocal(root);
    si->ng = Normal3f(si->p.x, si->p.y, si->p.z); //! 由于可以直接通过 Dot(si->ng, si->woWorld) 来判断某个 intersection 是否是 outward hit。因此 ng 总是指向几何外侧
    Float phi_uv = std::atan2(si->p.z, si->p.x) + Pi;
    Float theta_uv = std::acos(std::clamp(-si->p.y * 2, Float(-1), Float(1))); //! 浮点数越界会造成 nan，因此有必要 clamp
    Vector3f sg_dir(-si->p.z, 0, si->p.x);
    si->sg = Vector3f(sg_dir * (Pi * std::sin(theta_uv) / sg_dir.Norm()));
    Vector3f tg_dir(Vector3f::Cross(si->sg, si->ng)); //! 左手系的叉乘规则相反
    si->tg = Vector3f(tg_dir * (0.5 * Pi / tg_dir.Norm()));
    si->woWorld = Vector3f(-rayLocal.d);
    si->uv = Point2f(phi_uv / (2 * Pi), theta_uv / Pi);
    // World si
    *si = (*objectToWorld)(*si);
    return true;
}

/*override*/ AABound3f Sphere::ObjectBound() const
{
    return AABound3f(Point3f(-0.5, -0.5, -0.5), Point3f(0.5, 0.5, 0.5));
}

/*override*/ Float Sphere::Area() const
{
    return area;
}

/*override*/ Normal3f Sphere::Ng(const Ray& incidentRayWorld) const
{
    // TODO: Sphere::Ng()
    NotImplemented();
    return Normal3f();
}

/*override*/ Interaction Sphere::Sample(const Point2f& samples, Float* pdf) const
{
    // TODO: Sphere::Sample()
    NotImplemented();
    return Interaction();
}

/*override*/ Interaction Sphere::Sample(const Interaction& ref, 
                                        const Point2f& samples, Float* pdf) const
{
    // TODO: Sphere::Sample()
    NotImplemented();
    return Interaction();
}

#ifdef INTEL
/*override*/ int Sphere::GenerateEmbreeGeometry(RTCDevice device, RTCScene scene) const
{
    // TODO: Sphere::GenerateEmbreeGeometry()
    NotImplemented();
    return -1;
}
#endif