#include <matrix.h>

//* Matrix3x3 --------------------------------------------------------------------------------------------------*//

// Constructors
Matrix3x3::Matrix3x3() : Matrix3x3(1, 0, 0, 0, 1, 0, 0, 0, 1) { }
Matrix3x3::Matrix3x3(const Matrix3x3& that) { m[0][0] = that[0][0], m[0][1] = that[0][1], m[0][2] = that[0][2], m[1][0] = that[1][0], m[1][1] = that[1][1], m[1][2] = that[1][2], m[2][0] = that[2][0], m[2][1] = that[2][1], m[2][2] = that[2][2]; }
Matrix3x3::Matrix3x3(const Vector3f& row1, const Vector3f& row2, const Vector3f& row3) : Matrix3x3(row1.x, row1.y, row1.z, row2.x, row2.y, row2.z, row3.x, row3.y, row3.z) { }
Matrix3x3::Matrix3x3(Float m00, Float m01, Float m02, Float m10, Float m11, Float m12, Float m20, Float m21, Float m22) { m[0][0] = m00, m[0][1] = m01, m[0][2] = m02, m[1][0] = m10, m[1][1] = m11, m[1][2] = m12, m[2][0] = m20, m[2][1] = m21, m[2][2] = m22; }

// Operators
Matrix3x3 Matrix3x3::Transposed() const { return Matrix3x3(m[0][0], m[1][0], m[2][0], m[0][1], m[1][1], m[2][1], m[0][2], m[1][2], m[2][2]); }
Float Matrix3x3::Determinant() const { return m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1]) - m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0]) + m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]); }
Matrix3x3 Matrix3x3::Inversed() const { return Adjoint() / Determinant(); }
Matrix3x3 Matrix3x3::Adjoint() const { return Matrix3x3(m[1][1] * m[2][2] - m[1][2] * m[2][1], m[0][2] * m[2][1] - m[0][1] * m[2][2], m[0][1] * m[1][2] - m[0][2] * m[1][1], m[1][2] * m[2][0] - m[1][0] * m[2][2], m[0][0] * m[2][2] - m[0][2] * m[2][0], m[0][2] * m[1][0] - m[0][0] * m[1][2], m[1][0] * m[2][1] - m[1][1] * m[2][0], m[0][1] * m[2][0] - m[0][0] * m[2][1], m[0][0] * m[1][1] - m[0][1] * m[1][0]); }

// Operators
/*friend*/ std::ostream& /*Matrix3x3::*/operator<<(std::ostream& out, const Matrix3x3 m) { out << '[' << m[0][0] << ' ' << m[0][1] << ' ' << m[0][2] << ';' << ' ' << m[1][0] << ' ' << m[1][1] << ' ' << m[1][2] << ';' << ' ' << m[2][0] << ' ' << m[2][1] << ' ' << m[2][2] << ']'; return out; }
Float* Matrix3x3::operator[](int rowIndex) { return m[rowIndex]; }
const Float* Matrix3x3::operator[](int rowIndex) const { return m[rowIndex]; }
Matrix3x3 Matrix3x3::operator+(const Matrix3x3& that) const { return Matrix3x3(m[0][0] + that.m[0][0], m[0][1] + that.m[0][1], m[0][2] + that.m[0][2], m[1][0] + that.m[1][0], m[1][1] + that.m[1][1], m[1][2] + that.m[1][2], m[2][0] + that.m[2][0], m[2][1] + that.m[2][1], m[2][2] + that.m[2][2]); }
Matrix3x3 Matrix3x3::operator*(const Matrix3x3& that) const { return Matrix3x3(m[0][0] * that.m[0][0] + m[0][1] * that.m[1][0] + m[0][2] * that.m[2][0], m[0][0] * that.m[0][1] + m[0][1] * that.m[1][1] + m[0][2] * that.m[2][1], m[0][0] * that.m[0][2] + m[0][1] * that.m[1][2] + m[0][2] * that.m[2][2], m[1][0] * that.m[0][0] + m[1][1] * that.m[1][0] + m[1][2] * that.m[2][0], m[1][0] * that.m[0][1] + m[1][1] * that.m[1][1] + m[1][2] * that.m[2][1], m[1][0] * that.m[0][2] + m[1][1] * that.m[1][2] + m[1][2] * that.m[2][2], m[2][0] * that.m[0][0] + m[2][1] * that.m[1][0] + m[2][2] * that.m[2][0], m[2][0] * that.m[0][1] + m[2][1] * that.m[1][1] + m[2][2] * that.m[2][1], m[2][0] * that.m[0][2] + m[2][1] * that.m[1][2] + m[2][2] * that.m[2][2]); }
Matrix3x3 Matrix3x3::operator*(Float scalar) const { return Matrix3x3(scalar * m[0][0], scalar * m[0][1], scalar * m[0][2], scalar * m[1][0], scalar * m[1][1], scalar * m[1][2], scalar * m[2][0], scalar * m[2][1], scalar * m[2][2]); }
/*friend*/ Matrix3x3 /*Matrix3x3::*/operator*(Float scalar, const Matrix3x3& m) { return m * scalar; }
Matrix3x3 Matrix3x3::operator/(Float divide) const { return Matrix3x3(m[0][0] / divide, m[0][1] / divide, m[0][2] / divide, m[1][0] / divide, m[1][1] / divide, m[1][2] / divide, m[2][0] / divide, m[2][1] / divide, m[2][2] / divide); }



//* Matrix4x4 --------------------------------------------------------------------------------------------------*//

// Constructors
Matrix4x4::Matrix4x4() : Matrix4x4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1) { }
Matrix4x4::Matrix4x4(const Matrix4x4& that) { m[0][0] = that[0][0], m[0][1] = that[0][1], m[0][2] = that[0][2], m[0][3] = that[0][3], m[1][0] = that[1][0], m[1][1] = that[1][1], m[1][2] = that[1][2], m[1][3] = that[1][3], m[2][0] = that[2][0], m[2][1] = that[2][1], m[2][2] = that[2][2]; m[2][3] = that[2][3], m[3][0] = that[3][0], m[3][1] = that[3][1], m[3][2] = that[3][2], m[3][3] = that[3][3]; }
Matrix4x4::Matrix4x4(const Vector4f& row1, const Vector4f& row2, const Vector4f& row3, const Vector4f& row4) : Matrix4x4(row1.x, row1.y, row1.z, row1.w, row2.x, row2.y, row2.z, row2.w, row3.x, row3.y, row3.z, row3.w, row4.x, row4.y, row4.z, row4.w) { }
Matrix4x4::Matrix4x4(Float m00, Float m01, Float m02, Float m03, Float m10, Float m11, Float m12, Float m13, Float m20, Float m21, Float m22, Float m23, Float m30, Float m31, Float m32, Float m33) { m[0][0] = m00, m[0][1] = m01, m[0][2] = m02, m[0][3] = m03, m[1][0] = m10, m[1][1] = m11, m[1][2] = m12, m[1][3] = m13, m[2][0] = m20, m[2][1] = m21, m[2][2] = m22; m[2][3] = m23, m[3][0] = m30, m[3][1] = m31, m[3][2] = m32, m[3][3] = m33; }

// Methods
Matrix4x4 Matrix4x4::Transposed() const { return Matrix4x4(m[0][0], m[1][0], m[2][0], m[3][0], m[0][1], m[1][1], m[2][1], m[3][1], m[0][2], m[1][2], m[2][2], m[3][2], m[0][3], m[1][3], m[2][3], m[3][3]); }
Float Matrix4x4::Determinant() const { return 
        m[0][0] * (m[1][1] * (m[2][2] * m[3][3] - m[2][3] * m[3][2]) + m[1][2] * (m[2][3] * m[3][1] - m[2][1] * m[3][3]) + m[1][3] * (m[2][1] * m[3][2] - m[2][2] * m[3][1])) +
        m[0][1] * (m[1][0] * (m[2][3] * m[3][2] - m[2][2] * m[3][3]) + m[1][2] * (m[2][0] * m[3][3] - m[2][3] * m[3][0]) + m[1][3] * (m[2][2] * m[3][0] - m[2][0] * m[3][2])) +
        m[0][2] * (m[1][0] * (m[2][1] * m[3][3] - m[2][3] * m[3][1]) + m[1][1] * (m[2][3] * m[3][0] - m[2][0] * m[3][3]) + m[1][3] * (m[2][0] * m[3][1] - m[2][1] * m[3][0])) +
        m[0][3] * (m[1][0] * (m[2][2] * m[3][1] - m[2][1] * m[3][2]) + m[1][1] * (m[2][0] * m[3][2] - m[2][2] * m[3][0]) + m[1][2] * (m[2][1] * m[3][0] - m[2][0] * m[3][1]));
}
Matrix4x4 Matrix4x4::Inversed() const { return Adjoint() / Determinant(); }
Matrix4x4 Matrix4x4::Adjoint() const { return Matrix4x4(
        m[1][1] * (m[2][2] * m[3][3] - m[2][3] * m[3][2]) + m[1][2] * (m[2][3] * m[3][1] - m[2][1] * m[3][3]) + m[1][3] * (m[2][1] * m[3][2] - m[2][2] * m[3][1]),
        m[0][1] * (m[2][3] * m[3][2] - m[2][2] * m[3][3]) + m[0][2] * (m[2][1] * m[3][3] - m[2][3] * m[3][1]) + m[0][3] * (m[2][2] * m[3][1] - m[2][1] * m[3][2]),
        m[0][1] * (m[1][2] * m[3][3] - m[1][3] * m[3][2]) + m[0][2] * (m[1][3] * m[3][1] - m[1][1] * m[3][3]) + m[0][3] * (m[1][1] * m[3][2] - m[1][2] * m[3][1]),
        m[0][1] * (m[1][3] * m[2][2] - m[1][2] * m[2][3]) + m[0][2] * (m[1][1] * m[2][3] - m[1][3] * m[2][1]) + m[0][3] * (m[1][2] * m[2][1] - m[1][1] * m[2][2]),
        m[1][0] * (m[2][3] * m[3][2] - m[2][2] * m[3][3]) + m[1][2] * (m[2][0] * m[3][3] - m[2][3] * m[3][0]) + m[1][3] * (m[2][2] * m[3][0] - m[2][0] * m[3][2]),
        m[0][0] * (m[2][2] * m[3][3] - m[2][3] * m[3][2]) + m[0][2] * (m[2][3] * m[3][0] - m[2][0] * m[3][3]) + m[0][3] * (m[2][0] * m[3][2] - m[2][2] * m[3][0]),
        m[0][0] * (m[1][3] * m[3][2] - m[1][2] * m[3][3]) + m[0][2] * (m[1][0] * m[3][3] - m[1][3] * m[3][0]) + m[0][3] * (m[1][2] * m[3][0] - m[1][0] * m[3][2]),
        m[0][0] * (m[1][2] * m[2][3] - m[1][3] * m[2][2]) + m[0][2] * (m[1][3] * m[2][0] - m[1][0] * m[2][3]) + m[0][3] * (m[1][0] * m[2][2] - m[1][2] * m[2][0]),
        m[1][0] * (m[2][1] * m[3][3] - m[2][3] * m[3][1]) + m[1][1] * (m[2][3] * m[3][0] - m[2][0] * m[3][3]) + m[1][3] * (m[2][0] * m[3][1] - m[2][1] * m[3][0]),
        m[0][0] * (m[2][3] * m[3][1] - m[2][1] * m[3][3]) + m[0][1] * (m[2][0] * m[3][3] - m[2][3] * m[3][0]) + m[0][3] * (m[2][1] * m[3][0] - m[2][0] * m[3][1]),
        m[0][0] * (m[1][1] * m[3][3] - m[1][3] * m[3][1]) + m[0][1] * (m[1][3] * m[3][0] - m[1][0] * m[3][3]) + m[0][3] * (m[1][0] * m[3][1] - m[1][1] * m[3][0]),
        m[0][0] * (m[1][3] * m[2][1] - m[1][1] * m[2][3]) + m[0][1] * (m[1][0] * m[2][3] - m[1][3] * m[2][0]) + m[0][3] * (m[1][1] * m[2][0] - m[1][0] * m[2][1]),
        m[1][0] * (m[2][2] * m[3][1] - m[2][1] * m[3][2]) + m[1][1] * (m[2][0] * m[3][2] - m[2][2] * m[3][0]) + m[1][2] * (m[2][1] * m[3][0] - m[2][0] * m[3][1]),
        m[0][0] * (m[2][1] * m[3][2] - m[2][2] * m[3][1]) + m[0][1] * (m[2][2] * m[3][0] - m[2][0] * m[3][2]) + m[0][2] * (m[2][0] * m[3][1] - m[2][1] * m[3][0]),
        m[0][0] * (m[1][2] * m[3][1] - m[1][1] * m[3][2]) + m[0][1] * (m[1][0] * m[3][2] - m[1][2] * m[3][0]) + m[0][2] * (m[1][1] * m[3][0] - m[1][0] * m[3][1]),
        m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1]) + m[0][1] * (m[1][2] * m[2][0] - m[1][0] * m[2][2]) + m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]));
}

// Operators
/*friend*/ std::ostream& /*Matrix4x4::*/operator<<(std::ostream& out, const Matrix4x4 m) { out << '[' << m[0][0] << ' ' << m[0][1] << ' ' << m[0][2] << ' ' << m[0][3] << ';' << ' ' << m[1][0] << ' ' << m[1][1] << ' ' << m[1][2] << ' ' << m[1][3] << ';' << ' ' << m[2][0] << ' ' << m[2][1] << ' ' << m[2][2] << ' ' << m[2][3] << ';' << ' ' << m[3][0] << ' ' << m[3][1] << ' ' << m[3][2] << ' ' << m[3][3] << ']'; return out; }
Float* Matrix4x4::operator[](int rowIndex) { return m[rowIndex]; }
const Float* Matrix4x4::operator[](int rowIndex) const { return m[rowIndex]; }
Matrix4x4 Matrix4x4::operator+(const Matrix4x4& that) const { return Matrix4x4(m[0][0] + that.m[0][0], m[0][1] + that.m[0][1], m[0][2] + that.m[0][2], m[0][3] + that.m[0][3], m[1][0] + that.m[1][0], m[1][1] + that.m[1][1], m[1][2] + that.m[1][2], m[1][3] + that.m[1][3], m[2][0] + that.m[2][0], m[2][1] + that.m[2][1], m[2][2] + that.m[2][2], m[2][3] + that.m[2][3], m[3][0] + that.m[3][0], m[3][1] + that.m[3][1], m[3][2] + that.m[3][2], m[3][3] + that.m[3][3]); }
Matrix4x4 Matrix4x4::operator*(const Matrix4x4& that) const { return Matrix4x4(
        m[0][0] * that.m[0][0] + m[0][1] * that.m[1][0] + m[0][2] * that.m[2][0] + m[0][3] * that.m[3][0], m[0][0] * that.m[0][1] + m[0][1] * that.m[1][1] + m[0][2] * that.m[2][1] + m[0][3] * that.m[3][1], m[0][0] * that.m[0][2] + m[0][1] * that.m[1][2] + m[0][2] * that.m[2][2] + m[0][3] * that.m[3][2], m[0][0] * that.m[0][3] + m[0][1] * that.m[1][3] + m[0][2] * that.m[2][3] + m[0][3] * that.m[3][3], 
        m[1][0] * that.m[0][0] + m[1][1] * that.m[1][0] + m[1][2] * that.m[2][0] + m[1][3] * that.m[3][0], m[1][0] * that.m[0][1] + m[1][1] * that.m[1][1] + m[1][2] * that.m[2][1] + m[1][3] * that.m[3][1], m[1][0] * that.m[0][2] + m[1][1] * that.m[1][2] + m[1][2] * that.m[2][2] + m[1][3] * that.m[3][2], m[1][0] * that.m[0][3] + m[1][1] * that.m[1][3] + m[1][2] * that.m[2][3] + m[1][3] * that.m[3][3], 
        m[2][0] * that.m[0][0] + m[2][1] * that.m[1][0] + m[2][2] * that.m[2][0] + m[2][3] * that.m[3][0], m[2][0] * that.m[0][1] + m[2][1] * that.m[1][1] + m[2][2] * that.m[2][1] + m[2][3] * that.m[3][1], m[2][0] * that.m[0][2] + m[2][1] * that.m[1][2] + m[2][2] * that.m[2][2] + m[2][3] * that.m[3][2], m[2][0] * that.m[0][3] + m[2][1] * that.m[1][3] + m[2][2] * that.m[2][3] + m[2][3] * that.m[3][3], 
        m[3][0] * that.m[0][0] + m[3][1] * that.m[1][0] + m[3][2] * that.m[2][0] + m[3][3] * that.m[3][0], m[3][0] * that.m[0][1] + m[3][1] * that.m[1][1] + m[3][2] * that.m[2][1] + m[3][3] * that.m[3][1], m[3][0] * that.m[0][2] + m[3][1] * that.m[1][2] + m[3][2] * that.m[2][2] + m[3][3] * that.m[3][2], m[3][0] * that.m[0][3] + m[3][1] * that.m[1][3] + m[3][2] * that.m[2][3] + m[3][3] * that.m[3][3]); }
Matrix4x4 Matrix4x4::operator*(Float scalar) const { return Matrix4x4(scalar * m[0][0], scalar * m[0][1], scalar * m[0][2], scalar * m[0][3], scalar * m[1][0], scalar * m[1][1], scalar * m[1][2], scalar * m[1][3], scalar * m[2][0], scalar * m[2][1], scalar * m[2][2], scalar * m[2][3], scalar * m[3][0], scalar * m[3][1], scalar * m[3][2], scalar * m[3][3]); }
/*friend*/ Matrix4x4 /*Matrix4x4::*/operator*(Float scalar, const Matrix4x4& m) { return m * scalar; }
Matrix4x4 Matrix4x4::operator/(Float divide) const { return Matrix4x4(m[0][0] / divide, m[0][1] / divide, m[0][2] / divide, m[0][3] / divide, m[1][0] / divide, m[1][1] / divide, m[1][2] / divide, m[1][3] / divide, m[2][0] / divide, m[2][1] / divide, m[2][2] / divide, m[2][3] / divide, m[3][0] / divide, m[3][1] / divide, m[3][2] / divide, m[3][3] / divide); }
