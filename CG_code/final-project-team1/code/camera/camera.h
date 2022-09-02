#ifndef CAMERA_H
#define CAMERA_H

#include <matrix.h>
#include <ray.h>
#include <interaction.h>

class Medium;

class Camera
{
public:
    // Fields
    Point3f position;
    Vector3f forward, right, up;
    /**
     * @brief Near plane 左下角的坐标
     */
    Point3f lowerLeftCorner;
    /**
     * @brief Viewport 的宽向量
     */
    Vector3f viewportHorizont;
    /**
     * @brief Viewport 的高向量
     */
    Vector3f viewportVertical;
    Float lensRadius;

    Matrix4x4 lookAt;
    Matrix4x4 perspective;
    Matrix4x4 perspectiveLookAt;
    Vector4f nearPlane; // TODO: class Plane : public Iso4f，因为平面的 transform 规则与一般向量大有不同
    const Medium* medium;

    // Constructors
    Camera(const Point3f& position, const Vector3f& forward_nonNormalized, const Vector3f& vup, 
           Float vfov_deg, Float aspectRatio, 
           Float aperture, Float focusDistance, 
           const Medium* medium);

    // Methods
    Ray GenerateRay(Float s, Float t) const;
    Point3f Project(const Point3f& pWorld) const;
private:
};

#endif