#include <quaternion.h>

// Constructors
Quaternion::Quaternion() : Quaternion(1, 0, 0, 0) { }
Quaternion::Quaternion(Float w, Float x, Float y, Float z) : w(w), x(x), y(y), z(z) { }
Quaternion::Quaternion(Float w, const Vector3f& xyz) : Quaternion(w, xyz.x, xyz.y, xyz.z) { }
Quaternion::Quaternion(const Vector3f& rotateAxis, Float rotateRadians)
{
    Vector3f axis_normalized = Vector3f(rotateAxis.Normalized());
    Float radians_half = rotateRadians / 2, sin_radians_half = sin(radians_half);
    w = cos(radians_half), x = axis_normalized.x * sin_radians_half, y = axis_normalized.y * sin_radians_half, z = axis_normalized.z * sin_radians_half;
}

// Methods
Float Quaternion::Dot(const Quaternion& that) const { return w * that.w + x * that.x + y * that.y + z * that.z; }
Float Quaternion::Norm() const { return sqrt(w * w + x * x + y * y + z * z); }
Quaternion Quaternion::Normalized() const { return this->operator/(Norm()); }
void Quaternion::Normalize() { Float magnitude = Norm(); w /= magnitude, x /= magnitude, y /= magnitude, z /= magnitude; }
Quaternion Quaternion::Conjugate() const { return Quaternion(w, -x, -y, -z); }
Quaternion Quaternion::Inverse() const { return Conjugate() / this->Dot(*this); }
/*static*/ Quaternion Quaternion::Lerp(const Quaternion& q1, const Quaternion& q2, Float t) { return q1 * ((Float)1 - t) + q2 * t; }
/*static*/ Quaternion Quaternion::Slerp(const Quaternion& q1, const Quaternion& q2, Float t)
{
    Float q1dotq2 = q1.Dot(q2), angle = acos(q1dotq2), sinAngle = sqrt(1 - q1dotq2 * q1dotq2);
    return q1 * (sin(angle * (1 - t)) / sinAngle) + q2 * (sin(angle * t) / sinAngle);
}
void Quaternion::Rotate(const Vector3f& axis, Float radians) { Quaternion leftq(axis, radians); (*this) = leftq * (*this); }
Quaternion Quaternion::Rotated(const Vector3f& axis, Float radians) const { Quaternion leftq(axis, radians); return leftq * (*this); }

// Operators
/*friend*/ std::ostream& /*Quaternion::*/operator<<(std::ostream& out, const Quaternion q) { out << '(' << q.w << ", " << q.x << ", " << q.y << ", " << q.z << ')'; return out; }
Quaternion Quaternion::operator+(const Quaternion& that) const { return Quaternion(w + that.w, x + that.x, y + that.y, z + that.z); }
Quaternion Quaternion::operator*(Float scalar) const { return Quaternion(scalar * w, scalar * x, scalar * y, scalar * z); }
/*friend*/ Quaternion /*Quaternion::*/operator*(Float scalar, const Quaternion& q) { return q * scalar; }
Quaternion Quaternion::operator*(const Quaternion& that) const { return Quaternion(w * that.w - x * that.x - y * that.y - z * that.z, w * that.x + x * that.w + y * that.z - z * that.y, w * that.y - x * that.z + y * that.w + z * that.x, w * that.z + x * that.y - y * that.x + z * that.w); }
Vector3f Quaternion::operator*(const Vector3f& v) const
{
    Float rot_sin2 = x * x + y * y + z * z;
    Float rot_sin = sqrt(rot_sin2);
    Vector3f axis_normalized(x / rot_sin, y / rot_sin, z / rot_sin);
    return Vector3f((w * w - rot_sin2) * v + 2 * ((w * rot_sin) * Vector3f::Cross(axis_normalized, v) + (rot_sin2 * Vector3f::Dot(axis_normalized, v)) * axis_normalized));
}
Quaternion Quaternion::operator/(Float divide) const { return Quaternion(w / divide, x / divide, y / divide, z / divide); }
