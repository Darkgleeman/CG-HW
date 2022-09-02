#include <camera.h>
#include <ray.h>

Camera::Camera(const vec3 &pos, Float vFov, const vec2i &resolution,
               Float focalLen)
    : film(resolution),
      verticalFov(vFov),
      position(pos),
      focalLength(focalLen),
      forward(0, 0, 1),
      right(1, 0, 0),
      up(0, 1, 0) {}

/**
 * do camera look at
 * @param[in] lookAt look-at point
 * @param[in] refUp reference up vector
 */
void Camera::lookAt(const vec3 &lookAt, const vec3 &refUp) {
  forward = (lookAt - position).normalized();
  right = forward.cross(refUp).normalized();
  up = right.cross(forward);
  Float halfFov = radians(verticalFov / 2);
  Float yLen = tan(halfFov) * focalLength;
  up *= yLen;
  Float xLen = yLen * film.getAspectRatio();
  right *= xLen;
}

/**
 * generate a ray in the world space with given film sample (dx, dy)
 * @param[in] dx x in the film
 * @param[in] dy y in the film
 */
Ray Camera::generateRay(Float dx, Float dy) const {
  dx = dx / static_cast<Float>(film.resolution.x()) * 2 - 1;
  dy = dy / static_cast<Float>(film.resolution.y()) * 2 - 1;
  return Ray{position, dx * right + dy * up + forward};
}

void Camera::setPixel(int dx, int dy, const vec3 &value) {
  film.pixels[dy * film.resolution.x() + dx] = value;
}

const Film &Camera::getFilm() const { return film; }

std::shared_ptr<Camera> makeCamera(const vec3 &pos, Float vFov,
                                   const vec2i &resolution, Float focalLen) {
  return std::make_shared<Camera>(pos, vFov, resolution, focalLen);
}