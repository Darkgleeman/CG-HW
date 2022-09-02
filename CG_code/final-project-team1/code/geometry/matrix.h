#ifndef MATRIX_H
#define MATRIX_H

#include <vector.h>

class Matrix3x3
{
public:
    // Constructors
    Matrix3x3();
    Matrix3x3(const Matrix3x3& that);
    Matrix3x3(const Vector3f& row1, const Vector3f& row2, const Vector3f& row3);
    Matrix3x3(Float m00, Float m01, Float m02, Float m10, Float m11, Float m12, Float m20, Float m21, Float m22);

    // Operators
    Matrix3x3 Transposed() const;
    Float Determinant() const;
    Matrix3x3 Inversed() const;
    Matrix3x3 Adjoint() const;

    // Operators
    friend std::ostream& operator<<(std::ostream& out, const Matrix3x3 m);
    Float* operator[](int rowIndex);
    const Float* operator[](int rowIndex) const;
    Matrix3x3 operator+(const Matrix3x3& that) const;
    Matrix3x3 operator*(const Matrix3x3& that) const;
    Matrix3x3 operator*(Float scalar) const;
    friend Matrix3x3 operator*(Float scalar, const Matrix3x3& m);
    Matrix3x3 operator/(Float divide) const;
private:
    // Fields
    Float m[3][3];
};

class Matrix4x4
{
public:
    // Constructors
    Matrix4x4();
    Matrix4x4(const Matrix4x4& that);
    Matrix4x4(const Vector4f& row1, const Vector4f& row2, const Vector4f& row3, const Vector4f& row4);
    Matrix4x4(Float m00, Float m01, Float m02, Float m03, Float m10, Float m11, Float m12, Float m13, Float m20, Float m21, Float m22, Float m23, Float m30, Float m31, Float m32, Float m33);

    // Methods
    Matrix4x4 Transposed() const;
    Float Determinant() const;
    Matrix4x4 Inversed() const;
    Matrix4x4 Adjoint() const;

    // Operators
    friend std::ostream& operator<<(std::ostream& out, const Matrix4x4 m);
    Float* operator[](int rowIndex);
    const Float* operator[](int rowIndex) const;
    Matrix4x4 operator+(const Matrix4x4& that) const;
    Matrix4x4 operator*(const Matrix4x4& that) const;
    Matrix4x4 operator*(Float scalar) const;
    friend Matrix4x4 operator*(Float scalar, const Matrix4x4& m);
    Matrix4x4 operator/(Float divide) const;
private:
    // Fields
    Float m[4][4];
};

#endif