#include <camera.h>

// Constructors
Camera::Camera(const Point3f& position, const Vector3f& forward_nonNormalized, const Vector3f& vup, 
               Float vfov_deg, Float aspectRatio, 
               Float aperture, Float focusDistance, 
               const Medium* medium) : 
    position(position), 
    forward(forward_nonNormalized.Normalized()), 
    right(Vector3f::Cross(vup, forward).Normalized()), //! 采用左手系，与 Unity 一致的世界坐标系
    up(Vector3f::Cross(forward, right)), 
    medium(medium)
{
    Float vfov_rad = Radians(vfov_deg);
    Float viewportHeigh = 2 * tan(vfov_rad / 2);
    Float viewportWidth = aspectRatio * viewportHeigh;

    viewportHorizont = Vector3f(focusDistance * viewportWidth * right);
    viewportVertical = Vector3f(focusDistance * viewportHeigh * up);
    lowerLeftCorner  = Point3f(position - viewportHorizont / 2 - viewportVertical / 2 + focusDistance * forward);

    lensRadius = aperture / 2;

    // LookAt & Perspective
    lookAt = Matrix4x4(
        right.x,   right.y,   right.z,   -Vector3f::Dot(position, right), 
        up.x,      up.y,      up.z,      -Vector3f::Dot(position, up), 
        forward.x, forward.y, forward.z, -Vector3f::Dot(position, forward), 
        0, 0, 0, 1);
    perspective = Matrix4x4(
        2 / (aspectRatio * viewportHeigh), 0, 0, 0, 
        0, 2 / viewportHeigh, 0, 0, 
        0, 0, 1, -2 * focusDistance, 
        0, 0, 1, 0);
    perspectiveLookAt = perspective * lookAt;

    // Near plane
    nearPlane = Vector4f(forward.x, forward.y, forward.z, focusDistance);
}

// Methods
Ray Camera::GenerateRay(Float s, Float t) const
{
    // Vector3f rd = lensRadius * Sampler::RandomInUnitCircle_XY();
    // Vector3f offset = right * rd.x + up * rd.y;
    return Ray(
        position/* + offset*/, 
        Vector3f(lowerLeftCorner + s * viewportHorizont + t * viewportVertical - position/* - offset*/), 
        Infinity
    );
}

Point3f Camera::Project(const Point3f& pWorld) const
{
    Float x = pWorld.x, y = pWorld.y, z = pWorld.z;
    Float xp = perspectiveLookAt[0][0] * x + perspectiveLookAt[0][1] * y + perspectiveLookAt[0][2] * z + perspectiveLookAt[0][3];
    Float yp = perspectiveLookAt[1][0] * x + perspectiveLookAt[1][1] * y + perspectiveLookAt[1][2] * z + perspectiveLookAt[1][3];
    Float zp = perspectiveLookAt[2][0] * x + perspectiveLookAt[2][1] * y + perspectiveLookAt[2][2] * z + perspectiveLookAt[2][3];
    Float wp = perspectiveLookAt[3][0] * x + perspectiveLookAt[3][1] * y + perspectiveLookAt[3][2] * z + perspectiveLookAt[3][3];
    return Point3f(xp / wp, yp / wp, zp / wp);
}
