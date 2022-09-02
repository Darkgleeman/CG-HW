#include <vector.h>

//* Iso3f ------------------------------------------------------------------------------------------------------*//

// Constructors
Iso3f::Iso3f() : Iso3f(0, 0, 0) {  }
Iso3f::Iso3f(Float x, Float y, Float z) : x(x), y(y), z(z) { }
Iso3f::Iso3f(Float* xyz) : x(xyz[0]), y(xyz[1]), z(xyz[2]) { }
Iso3f::Iso3f(const Iso3f& that) : x(that.x), y(that.y), z(that.z) { }

// Methods
/*static*/ Float Iso3f::Dot(const Iso3f& l, const Iso3f& r) { return l.x * r.x + l.y * r.y + l.z * r.z; }
/*static*/ Iso3f Iso3f::Cross(const Iso3f& l, const Iso3f& r) { return Iso3f(l.y * r.z - l.z * r.y, l.z * r.x - l.x * r.z, l.x * r.y - l.y * r.x); }
Float Iso3f::Norm() const { return sqrt(x * x + y * y + z * z); }
Float Iso3f::NormSquared() const { return x * x + y * y + z * z; }
Iso3f Iso3f::Normalized() const { return this->operator/(Norm()); }
void Iso3f::Normalize() { Float magnitude = Norm(); x /= magnitude, y /= magnitude, z /= magnitude; }
Iso3f Iso3f::Absed() const { return Iso3f(std::abs(x), std::abs(y), std::abs(z)); }
void Iso3f::Abs() { x = std::abs(x), y = std::abs(y), z = std::abs(z); }
/*static*/ Float Iso3f::Distance(const Iso3f& a, const Iso3f& b) { return (a - b).Norm(); }
/*static*/ Float Iso3f::DistanceSquared(const Iso3f& a, const Iso3f& b) { return (a - b).NormSquared(); }
Iso3f Iso3f::Proj(const Iso3f& projected) const { Float magnitude2 = projected.x * projected.x + projected.y * projected.y + projected.z * projected.z; return Dot(*this, projected) / magnitude2 * projected; }
/*static*/ Iso3f Iso3f::Reflect(const Iso3f& w, const Iso3f& normal) { return 2 * Dot(w, normal) * normal - w; }
/*static*/ bool Iso3f::Refract(const Iso3f& w, const Iso3f& normal, Float refractionRatio, Iso3f* wRefracted)
{
    Float cosTheta = std::min(Dot(w, normal), Float(1)); // cosθ = -R·n = w·n
    return Refract(w, normal, refractionRatio, wRefracted, cosTheta);
}
/*static*/ bool Iso3f::Refract(const Iso3f& w, const Iso3f& normal, Float refractionRatio, Iso3f* wRefracted, Float cosTheta)
{
    Iso3f rOut_perp = refractionRatio * (cosTheta * normal - w); // R'_perp = (η/η')·(R+ncosθ)
    Float rOut_perp_normSquared = rOut_perp.NormSquared();
    if (rOut_perp_normSquared >= 1) // Total internal refraction
        return false;
    Iso3f rOut_Proj = -sqrt(fabs(1.0f - rOut_perp_normSquared)) * normal; // R'_proj = -sqrt(1 - R'_perp^2)*n
    *wRefracted = rOut_perp + rOut_Proj; // R' = R'_perp + R'_proj
    return true;
}
/*static*/ Iso3f Iso3f::Lerp(const Iso3f& v1, const Iso3f& v2, Float t) { return v1 + (v2 - v1) * t; }
/*static*/ Iso3f Iso3f::Lerp(const Iso3f& v00, const Iso3f& v01, const Iso3f& v10, const Iso3f& v11, Float u, Float v)
{
    Float uv = u * v;
    Float u_sub_uv = u - uv;
    return (1 - u_sub_uv - v) * v00 + u_sub_uv * v01 + (v - uv) * v10 + uv * v11;
}
void Iso3f::Rotate(const Iso3f& axis, Float radians) { (*this) = Rotated(axis, radians); }
Iso3f Iso3f::Rotated(const Iso3f& axis, Float radians) const
{
    Float rot_cos = cos(radians / (Float)2), rot_sin = sin(radians / (Float)2), rot_sin2 = rot_sin * rot_sin;
    Iso3f axis_normalized = axis.Normalized();
    return (rot_cos * rot_cos - rot_sin2) * (*this) + 2 * ((rot_cos * rot_sin) * (Cross(axis_normalized, *this)) + rot_sin2 * (Dot(axis_normalized, *this) * axis_normalized));
}
Iso3f Iso3f::ToSpherical() const
{
    Float r = std::sqrt(x * x + y * y + z * z);
    return Iso3f(
        r, 
        std::atan2(y, x), 
        std::acos(z / r));
}
/*static*/ Iso3f Iso3f::FromSpherical(Float phi, Float sinTheta, Float cosTheta)
{
    return Vector3f(sinTheta * std::cos(phi), sinTheta * std::sin(phi), cosTheta);
}
int Iso3f::MaximumExtent() const
{
    if (x > y && x > z)
        return 0;
    else if (y > z)
        return 1;
    else
        return 2;
}
int Iso3f::MaximumAbsExtent() const
{
    Float absX = std::abs(x);
    Float absY = std::abs(y);
    Float absZ = std::abs(z);
    if (absX > absY && absX > absZ)
        return 0;
    else if (absY > absZ)
        return 1;
    else
        return 2;
}
int Iso3f::MinimumExtent() const
{
    if (x < y && x < z)
        return 0;
    else if (y < z)
        return 1;
    else
        return 2;
}
int Iso3f::MinimumAbsExtent() const
{
    Float absX = std::abs(x);
    Float absY = std::abs(y);
    Float absZ = std::abs(z);
    if (absX < absY && absX < absZ)
        return 0;
    else if (absY < absZ)
        return 1;
    else
        return 2;
}
bool Iso3f::HasNan() const { return std::isnan(x) || std::isnan(y) || std::isnan(z); }
bool Iso3f::IsNearZero() const
{
    const Float s = 1e-8f;
    return (std::fabs(x) < s) && (std::fabs(y) < s) && (std::fabs(z) < s);
}
/*static*/ void Iso3f::CoordinateSystem(const Iso3f& v1, Iso3f* v2, Iso3f* v3)
{
    if (std::abs(v1.x) > std::abs(v1.y))
        *v2 = Vector3f(-v1.z, 0, v1.x) / std::sqrt(v1.x * v1.x + v1.z * v1.z);
    else
        *v2 = Vector3f(0, v1.z, -v1.y) / std::sqrt(v1.y * v1.y + v1.z * v1.z);
    *v3 = Cross(v1, *v2);
}

// Operators
Iso3f& Iso3f::operator=(const Iso3f& that) { x = that.x, y = that.y, z = that.z; return *this; }
/*friend*/ std::ostream& /*Iso3f::*/operator<<(std::ostream& out, const Iso3f v) { out << '(' << v.x << ", " << v.y << ", " << v.z << ')'; return out; }
Float Iso3f::operator[] (int i) const { return i == 0 ? x : (i == 1 ? y : z); }
Float& Iso3f::operator[] (int i) { return i == 0 ? x : (i == 1 ? y : z); }
Iso3f Iso3f::operator+(const Iso3f& that) const { return Iso3f(x + that.x, y + that.y, z + that.z); }
Iso3f& Iso3f::operator+=(const Iso3f& that) { x += that.x, y += that.y, z += that.z; return *this; }
Iso3f Iso3f::operator-() const { return Iso3f(-x, -y, -z); }
Iso3f Iso3f::operator-(const Iso3f& that) const { return Iso3f(x - that.x, y - that.y, z - that.z); }
Iso3f& Iso3f::operator-=(const Iso3f& that) { x -= that.x, y -= that.y, z -= that.z; return *this; }
Iso3f Iso3f::operator*(Float scalar) const { return Iso3f(scalar * x, scalar * y, scalar * z); }
Iso3f& Iso3f::operator*=(Float scalar) { x *= scalar, y *= scalar, z *= scalar; return *this; }
/*friend*/ Iso3f /*Iso3f::*/operator*(Float scalar, const Iso3f& v) { return v * scalar; }
Iso3f Iso3f::operator*(const Iso3f& that) const { return Iso3f(x * that.x, y * that.y, z * that.z); }
Iso3f& Iso3f::operator*=(const Iso3f& that) { x *= that.x, y *= that.y, z *= that.z; return *this; }
Iso3f Iso3f::operator/(Float divide) const { return Iso3f(x / divide, y / divide, z / divide); }
Iso3f& Iso3f::operator/=(Float divide) { x /= divide, y /= divide, z /= divide; return *this; }
Iso3f Iso3f::operator/(const Iso3f& that) const { return Iso3f(x / that.x, y / that.y, z / that.z); }
Iso3f& Iso3f::operator/=(const Iso3f& that) { x /= that.x, y /= that.y, z /= that.z; return *this; }
bool Iso3f::operator==(const Iso3f& that) const { return x == that.x && y == that.y && z == that.z; }

//* Vector3f / Point3f / Normal3f ------------------------------------------------------------------------------*//

// Constructors
Vector3f::Vector3f() : Iso3f(0, 0, 0) {  }
Vector3f::Vector3f(Float x, Float y, Float z) : Iso3f(x, y, z) { }
Vector3f::Vector3f(Float* xyz) : Iso3f(xyz) { }
Vector3f::Vector3f(const Vector3f& that) : Iso3f(that.x, that.y, that.z) { }
/*explicit*/ Vector3f::Vector3f(const Iso3f& that) : Vector3f(that.x, that.y, that.z) { }
/*explicit*/ Vector3f::Vector3f(const Point3f& that) : Vector3f(that.x, that.y, that.z) { }
/*explicit*/ Vector3f::Vector3f(const Normal3f& that) : Vector3f(that.x, that.y, that.z) { }

// Constructors
Point3f::Point3f() : Iso3f(0, 0, 0) {  }
Point3f::Point3f(Float x, Float y, Float z) : Iso3f(x, y, z) { }
Point3f::Point3f(Float* xyz) : Iso3f(xyz) { }
Point3f::Point3f(const Point3f& that) : Iso3f(that.x, that.y, that.z) { }
/*explicit*/ Point3f::Point3f(const Iso3f& that) : Point3f(that.x, that.y, that.z) { }
/*explicit*/ Point3f::Point3f(const Vector3f& that) : Point3f(that.x, that.y, that.z) { }
/*explicit*/ Point3f::Point3f(const Normal3f& that) : Point3f(that.x, that.y, that.z) { }

// Constructors
Normal3f::Normal3f() : Iso3f(0, 0, 0) {  }
Normal3f::Normal3f(Float x, Float y, Float z) : Iso3f(x, y, z) { }
Normal3f::Normal3f(Float* xyz) : Iso3f(xyz) { }
Normal3f::Normal3f(const Normal3f& that) : Iso3f(that.x, that.y, that.z) { }
/*explicit*/ Normal3f::Normal3f(const Iso3f& that) : Normal3f(that.x, that.y, that.z) { }
/*explicit*/ Normal3f::Normal3f(const Vector3f& that) : Normal3f(that.x, that.y, that.z) { }
/*explicit*/ Normal3f::Normal3f(const Point3f& that) : Normal3f(that.x, that.y, that.z) { }



//* Iso2f ------------------------------------------------------------------------------------------------------*//

// Constructors
Iso2f::Iso2f() : Iso2f(0, 0) {  }
Iso2f::Iso2f(Float x, Float y) : x(x), y(y) { }
Iso2f::Iso2f(const Iso2f& that) : x(that.x), y(that.y) { }

// Methods
/*static*/ Float Iso2f::Dot(const Iso2f& l, const Iso2f& r) { return l.x * r.x + l.y * r.y; }
Float Iso2f::Norm() const { return sqrt(x * x + y * y); }
Float Iso2f::NormSquared() const { return x * x + y * y; }
Iso2f Iso2f::Normalized() const { return this->operator/(Norm()); }
void Iso2f::Normalize() { Float magnitude = Norm(); x /= magnitude, y /= magnitude; }
/*static*/ Float Iso2f::Distance(const Iso2f& a, const Iso2f& b) { return (a - b).Norm(); }
/*static*/ Float Iso2f::DistanceSquared(const Iso2f& a, const Iso2f& b) { return (a - b).NormSquared(); }
/*static*/ Iso2f Iso2f::Lerp(const Iso2f& v1, const Iso2f& v2, Float t) { return v1 + (v2 - v1) * t; }
bool Iso2f::HasNan() const { return std::isnan(x) || std::isnan(y); }
bool Iso2f::IsNearZero() const
{
    const Float s = 1e-8f;
    return (std::fabs(x) < s) && (std::fabs(y) < s);
}

// Operators
Iso2f& Iso2f::operator=(const Iso2f& that) { x = that.x, y = that.y; return *this; }
/*friend*/ std::ostream& /*Iso2f::*/operator<<(std::ostream& out, const Iso2f v) { out << '(' << v.x << ", " << v.y << ')'; return out; }
Float Iso2f::operator[] (int i) const { return i == 0 ? x : y; }
Float& Iso2f::operator[] (int i) { return i == 0 ? x : y; }
Iso2f Iso2f::operator+(const Iso2f& that) const { return Iso2f(x + that.x, y + that.y); }
Iso2f& Iso2f::operator+=(const Iso2f& that) { x += that.x, y += that.y; return *this; }
Iso2f Iso2f::operator-() const { return Iso2f(-x, -y); }
Iso2f Iso2f::operator-(const Iso2f& that) const { return Iso2f(x - that.x, y - that.y); }
Iso2f& Iso2f::operator-=(const Iso2f& that) { x -= that.x, y -= that.y; return *this; }
Iso2f Iso2f::operator*(Float scalar) const { return Iso2f(scalar * x, scalar * y); }
Iso2f& Iso2f::operator*=(Float scalar) { x *= scalar, y *= scalar; return *this; }
/*friend*/ Iso2f /*Iso2f::*/operator*(Float scalar, const Iso2f& v) { return v * scalar; }
Iso2f Iso2f::operator*(const Iso2f& that) const { return Iso2f(x * that.x, y * that.y); }
Iso2f& Iso2f::operator*=(const Iso2f& that) { x *= that.x, y *= that.y; return *this; }
Iso2f Iso2f::operator/(Float divide) const { return Iso2f(x / divide, y / divide); }
Iso2f& Iso2f::operator/=(Float divide) { x /= divide, y /= divide; return *this; }
bool Iso2f::operator==(const Iso2f& that) const { return x == that.x && y == that.y; }

//* Vector2f / Point2f / ---------------------------------------------------------------------------------------*//

Vector2f::Vector2f() : Iso2f(0, 0) {  }
Vector2f::Vector2f(Float x, Float y) : Iso2f(x, y) { }
Vector2f::Vector2f(const Vector2f& that) : Iso2f(that.x, that.y) { }
/*explicit*/ Vector2f::Vector2f(const Iso2f& that) : Vector2f(that.x, that.y) { }

Point2f::Point2f() : Iso2f(0, 0) {  }
Point2f::Point2f(Float x, Float y) : Iso2f(x, y) { }
Point2f::Point2f(const Point2f& that) : Iso2f(that.x, that.y) { }
/*explicit*/ Point2f::Point2f(const Iso2f& that) : Point2f(that.x, that.y) { }



//* Vector4f ---------------------------------------------------------------------------------------------------*//

// Constructors
Vector4f::Vector4f() : Vector4f(0, 0, 0, 0) {  }
Vector4f::Vector4f(Float x, Float y, Float z, Float w) : x(x), y(y), z(z), w(w) { }
Vector4f::Vector4f(const Vector4f& that) : x(that.x), y(that.y), z(that.z), w(that.w) { }

//* Point3i  ---------------------------------------------------------------------------------------------------*//

// Constructors
Point3i::Point3i() : Point3i(0, 0, 0) { }
Point3i::Point3i(int x, int y, int z) : x(x), y(y), z(z) { }
Point3i::Point3i(const Point3i& that) : x(that.x), y(that.y), z(that.z) { }

// Operators
/*friend*/ std::ostream& /*Iso3f::*/operator<<(std::ostream& out, const Point3i v) { out << '(' << v.x << ", " << v.y << ", " << v.z << ')'; return out; }
int Point3i::operator[] (int i) const { return i == 0 ? x : (i == 1 ? y : z); }
int& Point3i::operator[] (int i) { return i == 0 ? x : (i == 1 ? y : z); }
bool Point3i::operator==(const Point3i& that) const { return x == that.x && y == that.y && z == that.z; }

//* Point3u  ---------------------------------------------------------------------------------------------------*//

// Constructors
Point3u::Point3u() : Point3u(0, 0, 0) { }
Point3u::Point3u(unsigned x, unsigned y, unsigned z) : x(x), y(y), z(z) { }
Point3u::Point3u(const Point3u& that) : x(that.x), y(that.y), z(that.z) { }

// Operators
/*friend*/ std::ostream& /*Iso3f::*/operator<<(std::ostream& out, const Point3u v) { out << '(' << v.x << ", " << v.y << ", " << v.z << ')'; return out; }
unsigned Point3u::operator[] (int i) const { return i == 0 ? x : (i == 1 ? y : z); }
unsigned& Point3u::operator[] (int i) { return i == 0 ? x : (i == 1 ? y : z); }
bool Point3u::operator==(const Point3u& that) const { return x == that.x && y == that.y && z == that.z; }