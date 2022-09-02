#ifndef _BEZIER_H_
#define _BEZIER_H_
#include "defines.h"
#include <object.h>

#include <vector>

class BezierCurve {
 public:
  std::vector<vec3> control_points_;

  BezierCurve(int m);
  BezierCurve(std::vector<vec3>& control_points);

  void setControlPoint(int i, vec3 point);
  Vertex evaluate(std::vector<vec3>& control_points, float t);
  Vertex evaluate(float t);
  Object generateObject();
};

class BezierSurface {
 public:
  //std::vector<std::vector<vec3>> control_points_m_;
  //std::vector<std::vector<vec3>> control_points_n_;
  std::vector<std::vector<vec3>> control_points_;
  BezierSurface(int m, int n);
  void setControlPoint(int i, int j, vec3 point);
  Vertex evaluate(std::vector<std::vector<vec3>>& control_points, float u, float v);
  Object generateObject();
  Vertex evaluate_curve_point(std::vector<vec3>& control_points, float t);
};

class B_splinesurface {
public:
	std::vector<std::vector<vec3>> control_points_;
	B_splinesurface(int m, int n);
	void setControlPoint(int i, int j, vec3 point);
	Vertex evaluate(std::vector<std::vector<vec3>>& control_points, float u, float v);
	Vertex evaluate_curve_point(std::vector<vec3>& control_points, float t, int degree, std::vector<float> knots);
	float base_function(int degree, int i, float t, std::vector<float> knots);
	Object generateObject();
	void knot_init();
	std::vector<float> knots_u;
	std::vector<float> knots_v;
	int u_degree = 3;
	int v_degree = 3;

};

#endif