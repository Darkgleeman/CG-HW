#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <matrix.h>
#include <quaternion.h>
#include <ray.h>
#include <boundingBox.h>
#include <interaction.h>

class Transform
{
public:
    // Fields
    Matrix4x4 m, mInv;

    // Constructors
    Transform();
    Transform(const Matrix4x4& m);
    Transform(const Matrix4x4& m, const Matrix4x4& mInv);
    Transform(const Point3f& position, const Quaternion& rotation, const Vector3f& scale);

    // Methods
    Vector3f Scale() const;
    void Scale(const Vector3f& scale);
    Matrix3x3 Rotation() const;
    Vector3f Translation() const;
    void Translate(const Vector3f& translation);
    Transform Transposed() const;
    Transform Inversed() const;

    // Operators
    Point3f            operator()(const Point3f& point) const;
    Vector3f           operator()(const Vector3f& vector) const;
    Normal3f           operator()(const Normal3f& normal) const;
    Ray                operator()(const Ray& ray) const;
    AABound3f          operator()(const AABound3f& bound) const; // TODO: 有待优化
    SurfaceInteraction operator()(const SurfaceInteraction& si) const;
};

#endif