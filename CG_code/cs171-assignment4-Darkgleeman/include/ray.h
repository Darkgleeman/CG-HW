#ifndef CS171_HW3_INCLUDE_RAY_H_
#define CS171_HW3_INCLUDE_RAY_H_
#include <core.h>
/**
 * Data structure representing a ray
 */
struct Ray {
  /* Original point of the ray */
  vec3 origin;
  /* Direction of the ray */
  vec3 direction;
  /* The maximum and minimum value of the parameter t */
  Float tMin;
  Float tMax;
  Float eta;
  explicit Ray(const vec3 &origin, const vec3 &direction, Float tMin = 0.000001,
               Float tMax = INF,Float eta = 1)
      : origin(origin),
        direction(direction.normalized()),
        tMin(tMin),
        tMax(tMax),eta(eta) {}

  [[nodiscard]] vec3 operator()(Float t) const { return getPoint(t); }
  /**
   * get the point at t in the ray
   * @param[in] t a parameter t
   * @return the point
   */
  [[nodiscard]] vec3 getPoint(Float t) const { return origin + t * direction; }
  [[nodiscard]] void print() {
      std::cout << "Ray_origin:" << origin[0] << " " << origin[1] << " " << origin[2] << " " << " direction:" << direction[0] << " " << direction[1] << " " << direction[2] << std::endl;
  }
};

#endif  // CS171_HW3_INCLUDE_RAY_H_
