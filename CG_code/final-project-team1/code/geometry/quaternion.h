#ifndef QUATERNION_H
#define QUATERNION_H

#include <vector.h>

class Quaternion
{
public:
    // Fields
    Float w;
    Float x, y, z;

    // Constructors
    Quaternion();
    Quaternion(Float w, Float x, Float y, Float z);
    Quaternion(Float w, const Vector3f& xyz);
    Quaternion(const Vector3f& rotateAxis, Float rotateRadians);

    // Methods
    Float Dot(const Quaternion& that) const;
    Float Norm() const;
    Quaternion Normalized() const;
    void Normalize();
    Quaternion Conjugate() const;
    Quaternion Inverse() const;
    static Quaternion Lerp(const Quaternion& q1, const Quaternion& q2, Float t);
    static Quaternion Slerp(const Quaternion& q1, const Quaternion& q2, Float t);
    /**
     * @brief 绕 axis 逆时针自转 radians
     */
    void Rotate(const Vector3f& axis, Float radians);
    // 按 axis 旋转 radians 后的四元数
    /**
     * @brief 绕 axis 逆时针自转 radians 后的四元数
     */
    Quaternion Rotated(const Vector3f& axis, Float radians) const;

    // Operators
    friend std::ostream& operator<<(std::ostream& out, const Quaternion q);
    /**
     * @brief 对应位置相加
     */
    Quaternion operator+(const Quaternion& that) const;
    /**
     * @brief 各位置数乘，旋转含义保持不变，Norm 扩大 scalar 倍
     */
    Quaternion operator*(Float scalar) const;
    /**
     * @brief 各位置数乘，旋转含义保持不变，Norm 扩大 scalar 倍
     */
    friend Quaternion operator*(Float scalar, const Quaternion& q);
    /**
     * @brief 四元数级联，旋转叠加
     */
    Quaternion operator*(const Quaternion& that) const;
    /**
     * @brief 返回旋转后的向量
     */
    Vector3f operator*(const Vector3f& v) const;
    /**
     * @brief 各位置数除，旋转含义保持不变，Norm 缩小 divide 倍
     */
    Quaternion operator/(Float divide) const;

    // Constants
    inline static Quaternion Identity() { return Quaternion(1, 0, 0, 0); }
private:
};

#endif