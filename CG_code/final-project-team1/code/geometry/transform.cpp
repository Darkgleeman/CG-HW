#include <transform.h>

// Constructors
Transform::Transform() : Transform(Point3f(0, 0, 0), Quaternion::Identity(), Vector3f(1, 1, 1)) { }
Transform::Transform(const Matrix4x4& m) : m(m), mInv(m.Inversed()) { }
Transform::Transform(const Matrix4x4& m, const Matrix4x4& mInv) : m(m), mInv(mInv) { }
Transform::Transform(const Point3f& position, const Quaternion& rotation, const Vector3f& scale)
{
    Float
        xx = rotation.x * rotation.x, yy = rotation.y * rotation.y, zz = rotation.z * rotation.z,
        xy = rotation.x * rotation.y, yz = rotation.y * rotation.z, zw = rotation.z * rotation.w, wx = rotation.w * rotation.x,
        xz = rotation.x * rotation.z, yw = rotation.y * rotation.w;
    m = Matrix4x4(
        scale.x * (1 - 2 * (yy + zz)), scale.y * (2 * (xy - zw)), scale.z * (2 * (xz + yw)), position.x, 
        scale.x * (2 * (xy + zw)), scale.y * (1 - 2 * (xx + zz)), scale.z * (2 * (yz - wx)), position.y, 
        scale.x * (2 * (xz - yw)), scale.y * (2 * (yz + wx)), scale.z * (1 - 2 * (xx + yy)), position.z, 
        0, 0, 0, 1);
    mInv = m.Inversed();
}

// Methods
Vector3f Transform::Scale() const
{
    return Vector3f(
        Vector3f(m[0][0], m[1][0], m[2][0]).Norm(), 
        Vector3f(m[0][1], m[1][1], m[2][1]).Norm(), 
        Vector3f(m[0][2], m[1][2], m[2][2]).Norm());
}
void Transform::Scale(const Vector3f& scale)
{
    m[0][0] *= scale.x, m[0][1] *= scale.y, m[0][2] *= scale.z;
    m[1][0] *= scale.x, m[1][1] *= scale.y, m[1][2] *= scale.z;
    m[2][0] *= scale.x, m[2][1] *= scale.y, m[2][2] *= scale.z;
    mInv = m.Inversed(); // TODO: 这也太懒了啊
}
Matrix3x3 Transform::Rotation() const
{
    Vector3f scale(Scale());
    Vector3f scaleInv(1 / scale.x, 1 / scale.y, 1 / scale.z);
    return Matrix3x3(
        m[0][0] * scaleInv.x, m[0][1] * scaleInv.y, m[0][2] * scaleInv.z, 
        m[1][0] * scaleInv.x, m[1][1] * scaleInv.y, m[1][2] * scaleInv.z, 
        m[2][0] * scaleInv.x, m[2][1] * scaleInv.y, m[2][2] * scaleInv.z);
}
Vector3f Transform::Translation() const
{
    return Vector3f(m[0][3], m[1][3], m[2][3]);
}
void Transform::Translate(const Vector3f& translation)
{
    m[0][3] += translation.x;
    m[1][3] += translation.y;
    m[2][3] += translation.z;
    mInv = m.Inversed(); // TODO: 这也太懒了啊
}
Transform Transform::Transposed() const { return Transform(m.Transposed(), mInv.Transposed()); }
Transform Transform::Inversed() const { return Transform(mInv, m); }

// Operators
Point3f Transform::operator()(const Point3f& point) const
{
    Float x = point.x, y = point.y, z = point.z;
    Float xp = m[0][0] * x + m[0][1] * y + m[0][2] * z + m[0][3];
    Float yp = m[1][0] * x + m[1][1] * y + m[1][2] * z + m[1][3];
    Float zp = m[2][0] * x + m[2][1] * y + m[2][2] * z + m[2][3];
    Float wp = m[3][0] * x + m[3][1] * y + m[3][2] * z + m[3][3];
    return Point3f(xp / wp, yp / wp, zp / wp);
}

Vector3f Transform::operator()(const Vector3f& vector) const
{
    Float x = vector.x, y = vector.y, z = vector.z;
    return Vector3f(
        m[0][0] * x + m[0][1] * y + m[0][2] * z,
        m[1][0] * x + m[1][1] * y + m[1][2] * z,
        m[2][0] * x + m[2][1] * y + m[2][2] * z);
}

Normal3f Transform::operator()(const Normal3f& normal) const
{
    Float x = normal.x, y = normal.y, z = normal.z;
    return Normal3f(
        mInv[0][0] * x + mInv[1][0] * y + mInv[2][0] * z,
        mInv[0][1] * x + mInv[1][1] * y + mInv[2][1] * z,
        mInv[0][2] * x + mInv[1][2] * y + mInv[2][2] * z);
}

Ray Transform::operator()(const Ray& ray) const
{
    Point3f  o = (*this)(ray.o);
    Vector3f d = (*this)(ray.d);
    return Ray(o, d, ray.tMax);
}

AABound3f Transform::operator()(const AABound3f& bound) const // TODO: 有待优化
{
    AABound3f ret((*this)(Point3f(bound.pMin.x, bound.pMin.y, bound.pMin.z)));    
    ret = AABound3f::Union(ret, (*this)(Point3f(bound.pMax.x, bound.pMin.y, bound.pMin.z)));
    ret = AABound3f::Union(ret, (*this)(Point3f(bound.pMin.x, bound.pMax.y, bound.pMin.z)));
    ret = AABound3f::Union(ret, (*this)(Point3f(bound.pMin.x, bound.pMin.y, bound.pMax.z)));
    ret = AABound3f::Union(ret, (*this)(Point3f(bound.pMin.x, bound.pMax.y, bound.pMax.z)));
    ret = AABound3f::Union(ret, (*this)(Point3f(bound.pMax.x, bound.pMax.y, bound.pMin.z)));
    ret = AABound3f::Union(ret, (*this)(Point3f(bound.pMax.x, bound.pMin.y, bound.pMax.z)));
    ret = AABound3f::Union(ret, (*this)(Point3f(bound.pMax.x, bound.pMax.y, bound.pMax.z)));
    return ret;
}

SurfaceInteraction Transform::operator()(const SurfaceInteraction& si) const
{
    //! 别少初始化
    // TODO: 别少初始化
    SurfaceInteraction ret;
    ret.p        = (*this)(si.p);
    ret.ng       = (*this)(si.ng);
    ret.sg       = (*this)(si.sg);
    ret.tg       = (*this)(si.tg);
    ret.woWorld  = (*this)(si.woWorld);
    ret.byEmbree = si.byEmbree;

    ret.uv        = si.uv;
    ret.rayONextPixelX = (*this)(si.rayONextPixelX);
    ret.rayONextPixelY = (*this)(si.rayONextPixelY);
    ret.rayDNextPixelX = (*this)(si.rayDNextPixelX);
    ret.rayDNextPixelY = (*this)(si.rayDNextPixelY);
    ret.bsdf      = si.bsdf;
    ret.primitive = si.primitive;
    ret.surface   = si.surface;
    return ret;
}
