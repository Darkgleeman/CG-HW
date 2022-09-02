#ifndef VECTOR_H
#define VECTOR_H

#include <utils.h>

#ifdef INTEL
#include <immintrin.h>
#endif

//* ////////////////////////////////////////////////////////////////////////////////////////////////////////////*//
//* SISD -------------------------------------------------------------------------------------------------------*//
//* ////////////////////////////////////////////////////////////////////////////////////////////////////////////*//

//* Iso3f ------------------------------------------------------------------------------------------------------*//

class Iso3f 
{
public:
    // Fields
    Float x, y, z;

    // Constructors
    Iso3f();
    Iso3f(Float x, Float y, Float z);
    Iso3f(Float* xyz);
    Iso3f(const Iso3f& that);

    // Methods
    static Float Dot(const Iso3f& l, const Iso3f& r);
    static Iso3f Cross(const Iso3f& l, const Iso3f& r);
    Float Norm() const;
    Float NormSquared() const;
    Iso3f Normalized() const;
    void Normalize();
    Iso3f Absed() const;
    void Abs();
    static Float Distance(const Iso3f& a, const Iso3f& b);
    static Float DistanceSquared(const Iso3f& a, const Iso3f& b);
    Iso3f Proj(const Iso3f& projected) const;
    static Iso3f Reflect(const Iso3f& w, const Iso3f& normal);
    static bool Refract(const Iso3f& w, const Iso3f& normal, Float refractionRatio, Iso3f* wRefracted);
    static bool Refract(const Iso3f& w, const Iso3f& normal, Float refractionRatio, Iso3f* wRefracted, Float cosTheta);
    static Iso3f Lerp(const Iso3f& v1, const Iso3f& v2, Float t);
    static Iso3f Lerp(const Iso3f& v00, const Iso3f& v01, const Iso3f& v10, const Iso3f& v11, Float u, Float v);
    void Rotate(const Iso3f& axis, Float radians);
    Iso3f Rotated(const Iso3f& axis, Float radians) const;
    Iso3f ToSpherical() const;
    static Iso3f FromSpherical(Float phi, Float sinTheta, Float cosTheta);
    int MaximumExtent() const;
    int MaximumAbsExtent() const;
    int MinimumExtent() const;
    int MinimumAbsExtent() const;
    bool HasNan() const;
    bool IsNearZero() const;
    static void CoordinateSystem(const Iso3f& v1, Iso3f* v2, Iso3f* v3);

    // Operators
    Iso3f& operator=(const Iso3f& that);
    friend std::ostream& operator<<(std::ostream& out, const Iso3f v);
    Float operator[] (int i) const;
    Float& operator[] (int i);
    Iso3f operator+(const Iso3f& that) const;
    Iso3f& operator+=(const Iso3f& that);
    Iso3f operator-() const;
    Iso3f operator-(const Iso3f& that) const;
    Iso3f& operator-=(const Iso3f& that);
    Iso3f operator*(Float scalar) const;
    Iso3f& operator*=(Float scalar);
    friend Iso3f operator*(Float scalar, const Iso3f& v);
    Iso3f operator*(const Iso3f& that) const;
    Iso3f& operator*=(const Iso3f& that);
    Iso3f operator/(Float divide) const;
    Iso3f& operator/=(Float divide);
    Iso3f operator/(const Iso3f& that) const;
    Iso3f& operator/=(const Iso3f& that);
    bool operator==(const Iso3f& that) const;

    // Constants
    inline static Iso3f Zero() { return Iso3f(0, 0, 0); }
    inline static Iso3f One()  { return Iso3f(1, 1, 1); }
};

//* Vector3f / Point3f / Normal3f ------------------------------------------------------------------------------*//

class Vector3f;
class Point3f;
class Normal3f;

class Vector3f : public Iso3f
{
public:
    // Constructors
    Vector3f();
    Vector3f(Float x, Float y, Float z);
    Vector3f(Float* xyz);
    Vector3f(const Vector3f& that);
    explicit Vector3f(const Iso3f& that);
    explicit Vector3f(const Point3f& that);
    explicit Vector3f(const Normal3f& that);

    // Constants
    inline static Vector3f Zero() { return Vector3f(Iso3f::Zero()); }
    inline static Vector3f One()  { return Vector3f(Iso3f::One()); }
};

class Point3f : public Iso3f
{
public:
    // Constructors
    Point3f();
    Point3f(Float x, Float y, Float z);
    Point3f(Float* xyz);
    Point3f(const Point3f& that);
    explicit Point3f(const Iso3f& that);
    explicit Point3f(const Vector3f& that);
    explicit Point3f(const Normal3f& that);

    // Constants
    inline static Point3f Zero() { return Point3f(Iso3f::Zero()); }
    inline static Point3f One()  { return Point3f(Iso3f::One()); }
};

class Normal3f : public Iso3f
{
public:
    // Constructors
    Normal3f();
    Normal3f(Float x, Float y, Float z);
    Normal3f(Float* xyz);
    Normal3f(const Normal3f& that);
    explicit Normal3f(const Iso3f& that);
    explicit Normal3f(const Vector3f& that);
    explicit Normal3f(const Point3f& that);

    // Constants
    inline static Normal3f Zero() { return Normal3f(Iso3f::Zero()); }
    inline static Normal3f One()  { return Normal3f(Iso3f::One()); }
};



//* Iso2f ------------------------------------------------------------------------------------------------------*//

class Iso2f
{
public:
    // Fields
    Float x, y;

    // Constructors
    Iso2f();
    Iso2f(Float x, Float y);
    Iso2f(const Iso2f& that);

    // Methods
    static Float Dot(const Iso2f& l, const Iso2f& r);
    Float Norm() const;
    Float NormSquared() const;
    Iso2f Normalized() const;
    void Normalize();
    static Float Distance(const Iso2f& a, const Iso2f& b);
    static Float DistanceSquared(const Iso2f& a, const Iso2f& b);
    static Iso2f Lerp(const Iso2f& v1, const Iso2f& v2, Float t);
    bool HasNan() const;
    bool IsNearZero() const;

    // Operators
    Iso2f& operator=(const Iso2f& that);
    friend std::ostream& operator<<(std::ostream& out, const Iso2f v);
    Float operator[] (int i) const;
    Float& operator[] (int i);
    Iso2f operator+(const Iso2f& that) const;
    Iso2f& operator+=(const Iso2f& that);
    Iso2f operator-() const;
    Iso2f operator-(const Iso2f& that) const;
    Iso2f& operator-=(const Iso2f& that);
    Iso2f operator*(Float scalar) const;
    Iso2f& operator*=(Float scalar);
    friend Iso2f operator*(Float scalar, const Iso2f& v);
    Iso2f operator*(const Iso2f& that) const;
    Iso2f& operator*=(const Iso2f& that);
    Iso2f operator/(Float divide) const;
    Iso2f& operator/=(Float divide);
    bool operator==(const Iso2f& that) const;

    // Constants
    inline static Iso2f Zero() { return Iso2f(0, 0); }
    inline static Iso2f One()  { return Iso2f(1, 1); }
};

//* Vector2f / Point2f / ---------------------------------------------------------------------------------------*//

class Vector2f : public Iso2f
{
public:
    // Constructors
    Vector2f();
    Vector2f(Float x, Float y);
    Vector2f(const Vector2f& that);
    explicit Vector2f(const Iso2f& that);

    // Constants
    inline static Vector2f Zero() { return Vector2f(Iso2f::Zero()); }
    inline static Vector2f One()  { return Vector2f(Iso2f::One()); }
};

class Point2f : public Iso2f
{
public:
    // Constructors
    Point2f();
    Point2f(Float x, Float y);
    Point2f(const Point2f& that);
    explicit Point2f(const Iso2f& that);

    // Constants
    inline static Point2f Zero() { return Point2f(Iso2f::Zero()); }
    inline static Point2f One()  { return Point2f(Iso2f::One()); }
};



//* Vector4f ---------------------------------------------------------------------------------------------------*//

class Vector4f
{
public:
    // Fields
    Float x, y, z, w;

    // Constructors
    Vector4f();
    Vector4f(Float x, Float y, Float z, Float w);
    Vector4f(const Vector4f& that);
};

//* Point3i  ---------------------------------------------------------------------------------------------------*//

class Point3i
{
public:
    // Fields
    int x, y, z;

    // Constructors
    Point3i();
    Point3i(int x, int y, int z);
    Point3i(const Point3i& that);

    // Operators
    friend std::ostream& operator<<(std::ostream& out, const Point3i v);
    int operator[] (int i) const;
    int& operator[] (int i);
    bool operator==(const Point3i& that) const;
};

//* Point3u  ---------------------------------------------------------------------------------------------------*//

class Point3u
{
public:
    // Fields
    unsigned x, y, z;

    // Constructors
    Point3u();
    Point3u(unsigned x, unsigned y, unsigned z);
    Point3u(const Point3u& that);

    // Operators
    friend std::ostream& operator<<(std::ostream& out, const Point3u v);
    unsigned operator[] (int i) const;
    unsigned& operator[] (int i);
    bool operator==(const Point3u& that) const;
};



//* ////////////////////////////////////////////////////////////////////////////////////////////////////////////*//
//* SIMD -------------------------------------------------------------------------------------------------------*//
//* ////////////////////////////////////////////////////////////////////////////////////////////////////////////*//

#ifdef INTEL

// TODO: Implement packet tracing

class bool4
{
public:
    // Fields
    __m128i v;

    // Constructors
};

class float4
{
public:
    // Fields
    __m128 v;

    // Constructors
    float4(float v);
    float4(float v0, float v1, float v2, float v3);
    float4(const float4& v);

    // Operators
    float4& operator=(const float4& that);
    friend std::ostream& operator<<(std::ostream& out, const float4 v);
    float4 operator+(const float4& that) const;
    float4& operator+=(const float4& that);
    float4 operator-() const;
    float4 operator-(const float4& that) const;
    float4& operator-=(const float4& that);
    float4 operator*(Float scalar) const;
    float4& operator*=(Float scalar);
    friend float4 operator*(Float scalar, const float4& v);
    float4 operator*(const float4& that) const;
    float4& operator*=(const float4& that);
    float4 operator/(Float divide) const;
    float4& operator/=(Float divide);
    float4 operator/(const float4& that) const;
    float4& operator/=(const float4& that);
    bool4 operator==(const float4& that) const;
    bool4 operator!=(const float4& that) const;
    bool4 operator<(const float4& that) const;
    bool4 operator<=(const float4& that) const;
    bool4 operator>(const float4& that) const;
    bool4 operator>=(const float4& that) const;
};

#endif

#endif