#include <camera.h>
#include <ray.h>

#define PI acos(-1)

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

    forward = (position-lookAt).normalized();
    right = refUp.cross(forward).normalized();
    up = forward.cross(right);
    float height = 2*focalLength * tan(radians(verticalFov) / 2.0);
    horizontal = height * film.getAspectRatio() * right;
    vertical = height * up;
    low_left = position - horizontal / 2 - vertical / 2 - focalLength * forward;

}

/**
 * generate a ray in the world space with given film sample (dx, dy)
 * @param[in] dx x in the film
 * @param[in] dy y in the film
 */
Ray Camera::generateRay(Float dx, Float dy) const {
  //vec3 camera_position;
  //vec3 dir;
  //Matrix4x4 lookat_matrix;
  //lookat_matrix << right(0), right(1), right(2), -position.dot(right),
  //  up(0), up(1), up(2), -position.dot(up), 
  //  forward(0), forward(1), forward(2), -position.dot(forward), 
  //  0, 0, 0, 1;
  //Matrix4x4 camera2world = lookat_matrix.inverse();
  //float imageAspectRatio = film.getAspectRatio();
  //float scale = tan(verticalFov / 2 * PI / 180);
  //float x = (2 * (dx ) / (float)film.resolution.x() - 1) *imageAspectRatio * scale;
  //float y = (1 - 2 * (dy) / (float)film.resolution.y()) * scale; 
  //camera_position = position;
  //vec4 dir_temp;
  //dir_temp<< x, y, -1,0;
  //dir_temp = camera2world * dir_temp;
  //dir[0] = dir_temp[0];
  //dir[1] = dir_temp[1];
  //dir[2] = dir_temp[2];
  //
  //dir.normalize();
  //std::cout << "dir: " << dir[0] << "  " << dir[1] << "  " << dir[2]
  //          << std::endl;
  //return Ray{camera_position, dir};

    vec3 pos = position;
    vec3 temp_dir = low_left + dx * horizontal + dy * vertical - position;
    return Ray(pos, temp_dir);
}

void Camera::setPixel(int dx, int dy, const vec3 &value) {
  film.pixels[dy * film.resolution.x() + dx] = value;
}

const Film &Camera::getFilm() const { return film; }

std::shared_ptr<Camera> makeCamera(const vec3 &pos, Float vFov,
                                   const vec2i &resolution, Float focalLen) {
  return std::make_shared<Camera>(pos, vFov, resolution, focalLen);
}