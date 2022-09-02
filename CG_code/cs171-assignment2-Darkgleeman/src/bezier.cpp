#include <bezier.h>
#include <utils.h>
#include <vector>
using  namespace std;
BezierCurve::BezierCurve(int m) { control_points_.resize(m); }

BezierCurve::BezierCurve(std::vector<vec3>& control_points) {
  control_points_ = control_points;
}

void BezierCurve::setControlPoint(int i, vec3 point) {
  control_points_[i] = point;
}

/**
 * TODO: evaluate the point at t with the given control_points
 */
Vertex BezierCurve::evaluate(std::vector<vec3>& control_points, float t) {
    int n = control_points.size();
    std::vector<vec3> Q;
    vec3 normal = vec3(0, 0, 0);
    for (int i = 0; i < n; i++)
    {
        Q.push_back(control_points[i]);
    }
    for (int k = 1; k < n; k++)
    {   
        if (k == n - 1)
        {
            normal = normalize(Q[1] - Q[0]);
        }
        for (int i = 0; i < n - k; k++)
        {
            Q[i] = (1 - t) * Q[i] + t * Q[i + 1];
        }
    }
    vec3 position = Q[0];
    struct Vertex point = { position,normal };
    return point;
}

Vertex BezierCurve::evaluate(float t) { return evaluate(control_points_, t); }



/**
 * TODO: generate an Object of the current Bezier curve
 */
Object BezierCurve::generateObject() {
  // !DELETE THIS WHEN YOU FINISH THIS FUNCTION
    float sample_num = 64;
    float gap = 1.0 / sample_num;
    std::vector<Vertex> curve_points;
    std::vector<GLuint> indices;
    for (int i = 0; i < 64; i++)
    {
        float t = i * gap;
        Vertex point = evaluate(t);
        curve_points.push_back(point);
        if (i != 63)
        {
            indices.push_back(i);
            indices.push_back(i+1);
        }
    }
    Object curve_object;
    curve_object.vertices = curve_points;
    curve_object.indices = indices;
    curve_object.draw_mode.primitive_mode = GL_LINES;
    return curve_object;
}

//BezierSurface::BezierSurface(int m, int n) {
//  control_points_m_.resize(m);
//  for (auto& sub_vec : control_points_m_) {
//    sub_vec.resize(n);
//  }
//  control_points_n_.resize(n);
//  for (auto& sub_vec : control_points_n_) {
//    sub_vec.resize(m);
//  }
//}
BezierSurface::BezierSurface(int m, int n) {
    control_points_.resize(m);
    for (auto& sub_vec : control_points_) {
        sub_vec.resize(n);
    }
}

/**
 * @param[in] i: index (i < m)
 * @param[in] j: index (j < n)
 * @param[in] point: the control point with index i, j
 */
void BezierSurface::setControlPoint(int i, int j, vec3 point) {
  control_points_[i][j] = point;
  //printf("i:%d ", i);
  //printf("j:%d ", j);
  //printf("\n");
  //control_points_n_[j][i] = point;
}

Vertex BezierSurface::evaluate_curve_point(std::vector<vec3>& control_points, float t) {
    int n = control_points.size()-1;
    //printf("n:%d ", n);
    std::vector<vec3> Q;
    vec3 normal = vec3(0, 0, 0);
    for (int i = 0; i <= n; i++)
    {   
        //printf("control_points[i].x:%f, y:%f, z:%f\n", control_points[i].x, control_points[i].y, control_points[i].z);
        Q.push_back(control_points[i]);
    }
    for (int k = 1; k <= n; k++)
    {   
        //printf("before,k:%d\n", k);
        if (k == n)
        {
            normal = normalize(Q[1] - Q[0]);
        }
        for (int i = 0; i <= n - k; i++)
        {
            Q[i] = (1 - t) * Q[i] + t * Q[i + 1];
        }
        //printf("k:%d ", k);
        //printf("n:%d ", n);
        //printf("\n");
        //printf("Q[0].x:%f, y:%f, z:%f\n", Q[0].x, Q[0].y, Q[0].z);
    }
    vec3 position = Q[0];
    //printf("position.x:%f, y:%f, z:%f\n", position.x,position.y, position.z);
    struct Vertex point = { position,normal };
    return point;
}

/**
 * TODO: evaluate the point at (u, v) with the given control points
 */
Vertex BezierSurface::evaluate(std::vector<std::vector<vec3>>& control_points,
                               float u, float v) {
    int m = control_points.size();
    int n = control_points[0].size();
    //printf("u:%f ", u);
    //printf("v:%f ", v);
    //printf("\n");
    std::vector<vec3> line1_control_points;
    std::vector<vec3> line2_control_points;
    //printf("line1\n");
    for (int i = 0; i < m; i++)
    {
        struct Vertex temp_control_point = evaluate_curve_point(control_points[i], v);
        line1_control_points.push_back(temp_control_point.position);
    }
    //printf("evaluate1\n");
    struct Vertex temp_point1 = evaluate_curve_point(line1_control_points, u);
    //printf("line2\n");
    for (int j = 0; j < n; j++)
    {   
        std::vector<vec3> column_points;
        for (int i = 0; i < m; i++)
        {
            column_points.push_back(control_points[i][j]);
        }
        struct Vertex temp_control_point = evaluate_curve_point(column_points, u);
        line2_control_points.push_back(temp_control_point.position);
    }
    //printf("evaluate2\n");
    //printf("line2_control_points.size():%d\n", line2_control_points.size());
    struct Vertex temp_point2 = evaluate_curve_point(line2_control_points, v);
    vec3 normal = normalize(cross(temp_point1.normal, temp_point2.normal));
    struct Vertex point = { temp_point2.position,normal };
    //printf("temp1.position.x:%f, y:%f, z:%f\n", temp_point1.position.x, temp_point1.position.y, temp_point1.position.z);
    //printf("temp2.position.x:%f, y:%f, z:%f\n", temp_point2.position.x, temp_point2.position.y, temp_point2.position.z);
    
    return point;
}


/**
 * TODO: generate an Object of the current Bezier surface
 */
Object BezierSurface::generateObject() {
    float sample_num = 64;
    float gap = 1.0 / sample_num;
    std::vector<Vertex> surface_points;
    std::vector<GLuint> indices;
    sample_num++;
    //int num = 0;
    for (int i = 0; i < sample_num; i++) //store the points into surface_points in row layout
    {
        for (int j = 0; j < sample_num; j++)
        {   
            //num++;
            //printf("num:%d:",num);
            //printf("\n");
            float u = i * gap;
            float v = j * gap;
            Vertex point = evaluate(control_points_, u, v);
            surface_points.push_back(point);
        }
    }
    
    for (int i = 0; i < sample_num; i++) //set up indices
    {      
        for (int j = 0; j < sample_num -1; j++)
        {   
            if (i % 2 == 0)
            {   
                if (i == sample_num-1) break;
                ////printf("Here1\n");
                ////int A = i * sample_num + j;
                ////int B = i * sample_num + j + 1;
                ////int C = i * sample_num + j + sample_num;
                ////printf("A:%d:", A);
                ////printf("B:%d:", B);
                ////printf("C:%d:", C);
                ////printf("\n");
                ////num++;
                indices.push_back(i * sample_num + j);
                indices.push_back(i * sample_num + j+1);
                indices.push_back(i * sample_num + j + sample_num);


            }
            else
            {   
                if (i == sample_num) break;
                //printf("Here2\n");
                //num++;
                //printf("A:%d:", A);
                //printf("B:%d:", B);
                //printf("C:%d:", C);
                //printf("\n");
                int A = i * sample_num + j;
                int B = i * sample_num + j + 1;
                int C = i * sample_num + j - sample_num + 1;
                indices.push_back(i * sample_num + j);
                indices.push_back(i * sample_num + j + 1);
                indices.push_back(i * sample_num + j -sample_num + 1);

                if (i != sample_num )
                {   
                    //printf("Here3\n");
                    //A = i * sample_num + j;
                    //B = i * sample_num + j + 1;
                    //C = i * sample_num + j + sample_num;
                    //printf("A:%d:", A);
                    //printf("B:%d:", B);
                    //printf("C:%d:", C);
                    //printf("\n");

                    indices.push_back(i * sample_num + j);
                    indices.push_back(i * sample_num + j + 1);
                    indices.push_back(i * sample_num + j + sample_num);




                    //A = i * sample_num + j + sample_num;
                    //B = i * sample_num + j + sample_num + 1;
                    //C = i * sample_num + j + 1;
                    //printf("A:%d:", A);
                    //printf("B:%d:", B);
                    //printf("C:%d:", C);
                    //printf("\n");
                    //num += 2;
                    indices.push_back(i * sample_num + j + sample_num);
                    indices.push_back(i * sample_num + j + sample_num + 1);
                    indices.push_back(i * sample_num + j + 1);

                }
               
            }

            
        }
    }
    //printf("num:%d:",num);
    Object surface_object;
    surface_object.vertices = surface_points;
    surface_object.indices = indices;
    surface_object.draw_mode.primitive_mode = GL_TRIANGLES;
    return surface_object;
}




B_splinesurface::B_splinesurface(int m, int n) {
    control_points_.resize(m);
    for (auto& sub_vec : control_points_) {
        sub_vec.resize(n);
    }
    knot_init();
}

void B_splinesurface::setControlPoint(int i, int j, vec3 point) {
    control_points_[i][j] = point;
}

void B_splinesurface::knot_init() {
    int m = control_points_.size();
    int n = control_points_[0].size();
    printf("u_degree:%d\n", this->u_degree);
    printf("m:%d\n", m);
    for (int i = 0; i < this->u_degree+1; i++)
    {
        this->knots_u.push_back(0);
    }
    if (m > this->u_degree)
    {   
        printf("yes\n");
        for (int i = 0; i < m - this->u_degree-1; i++)
        {
            this->knots_u.push_back((i + 1) * 1.0 / (m - this->u_degree));
        }
    }
    printf("yes\n");
    for (int i = 0; i < this->u_degree+1; i++)
    {   
        printf("yeah\n");
        this->knots_u.push_back(1.0);
    }


    for (int i = 0; i < this->v_degree+1; i++)
    {
        this->knots_v.push_back(0);
    }
    if (n > this->v_degree)
    {
        for (int i = 0; i < n - this->v_degree-1; i++)
        {
            this->knots_v.push_back((i + 1) * 1.0 / (n - this->v_degree));
        }
    }

   
    for (int i = 0; i < this->v_degree+1; i++)
    {
        this->knots_v.push_back(1.0);
    }



    //for (int i = 0; i < this->u_degree+m; i++)
    //{
    //    this->knots_u.push_back(i*1.0/(this->u_degree+m));
    //}
    //for (int i = 0; i < this->v_degree + n; i++)
    //{
    //    this->knots_v.push_back(i * 1.0 / (this->v_degree + n));
    //}
    for (int i = 0; i < knots_u.size(); i++)
    {
        printf("knot_u[%d]:%f\n", i, this->knots_u[i]);
    }
    for (int i = 0; i < knots_v.size(); i++)
    {
        printf("knot_v[%d]:%f\n", i, this->knots_v[i]);
    }
}
Vertex B_splinesurface::evaluate_curve_point(std::vector<vec3>& control_points, float t, int degree, std::vector<float> knots)
{
    int n = control_points.size()-1;
    int k = 0;
    if (t >= knots[n + 1])
    {
        k = n;
    }
    else if (t <= knots[degree])
    {
        k = degree;
    }
    else
    {
        for (int i = 0; i <= n + 1; i++)
        {
            if (t >= knots[i] && t < knots[i + 1])
            {
                k = i;
                break;
            }
        }
    }
    std::vector<vec3>reverse;
    for (int i = 0; i < degree + 1; i++)
    {
        reverse.push_back(control_points[i + k - degree]);
    }
    for (int r = 1; r < degree + 1; r++)
    {   
        //vec3 temp1 = reverse[k-degree+r-1];
        //vec3 temp2 = vec3(0.0, 0.0, 0.0);
        //for (int i = k - degree + r; i < k; i++)
        //{   
        //    int j = i - k + degree;
        //    temp2 = reverse[j];//P(i,r-1)
        //    float a = (t - knots[i]) / (knots[i + degree - r + 1] - knots[i]);
        //    reverse[j] = (1 - a) * temp1 + a * temp2;
        //    temp1 = temp2;
        //}
        for (int i = k; i >= k - degree + r; i--)
        {
            float a = (t - knots[i]) / (knots[i + degree - r + 1] - knots[i]);
            reverse[i - k + degree] = (1 - a) * reverse[i - k + degree - 1] + a * reverse[i - k + degree];
        }
    }
    vec3 pos = reverse.back();
    struct Vertex temp_pt = {pos, vec3(0.0, 0.0, 0.0)};
    return temp_pt;
}

Vertex B_splinesurface::evaluate(std::vector<std::vector<vec3>>& control_points,float u, float v) {
    //int m = control_points_.size();
    //int n = control_points_[0].size();
    //vec3 value = vec3(0.0, 0.0, 0.0);

    //for (int i = 0; i < m; i++)
    //{
    //    for (int j = 0; j < n; j++)
    //    {
    //        //printf("control_points_[i][j].x:%f, y:%f, z:%f\n", control_points_[i][j].x, control_points_[i][j].y, control_points_[i][j].z);
    //        value += base_function(u_degree, i, u, knots_u) * base_function(v_degree, j, v, knots_v) * control_points_[i][j];
    //    }
    //}
    //
    //vec3 normal = vec3(0, 0, 0);
    //struct Vertex point = { value,normal };
    //return point;


    int m = control_points_.size();
    int n = control_points_[0].size();
    //printf("u:%f ", u);
    //printf("v:%f ", v);
    //printf("\n");
    std::vector<vec3> line1_control_points;
    //printf("line1\n");
    for (int i = 0; i < m; i++)
    {
        struct Vertex temp_control_point = evaluate_curve_point(control_points[i], v,v_degree,knots_v);
        line1_control_points.push_back(temp_control_point.position);
    }
    //printf("evaluate1\n");
    struct Vertex temp_point1 = evaluate_curve_point(line1_control_points, u,u_degree,knots_u);
    return temp_point1;
}

float B_splinesurface::base_function(int degree,int i,float t,std::vector<float> knots)
{
    if (degree == 0)
    {
        if (t >= knots[i] && t < knots[i + 1u])
        {
            return 1.0;
        }
        else
        {
            return 0.0;
        }
    }
    float coeff1 = 0;
    float coeff2 = 0;
    float temp1 = base_function(degree - 1,i,t, knots);
    float temp2 = base_function(degree - 1, i + 1, t, knots);
    if (knots[i + degree] != knots[i])
    {
        coeff1 = (t - knots[i]) / (knots[i + degree] - knots[i]);
    }
    if (knots[i + degree + 1]!= knots[i + 1])
    {
        coeff2 = (knots[i + degree + 1] - t) / (knots[i + degree + 1] - knots[i + 1]);
    }
    //if (knots[i + degree-1] != knots[i])
    //{
    //    coeff1 = (t - knots[i]) / (knots[i + degree-1] - knots[i]);
    //}
    //if (knots[i + degree] != knots[i + 1])
    //{
    //    coeff2 = (knots[i + degree] - t) / (knots[i + degree] - knots[i + 1]);
    //}
     
    //printf("coeff1:%f\n", coeff1);
    //printf("coeff2:%f\n", coeff2);
    float value = coeff1 * temp1 + coeff2 * temp2;
    //printf("value:%f\n", value);
    return coeff1 * temp1 + coeff2 * temp2;
}


Object B_splinesurface::generateObject() {
    float sample_num = 64;
    float gap = 1.0 / sample_num;
    std::vector<Vertex> surface_points;
    std::vector<GLuint> indices;
    //int num = 0;
    sample_num++;
    for (int i = 0; i < sample_num; i++) //store the points into surface_points in row layout
    {
        for (int j = 0; j < sample_num; j++)
        {
            //num++;
            //printf("num:%d:",num);
            //printf("\n");
            float u = i * gap;
            float v = j * gap;
            Vertex point = evaluate(control_points_, u, v);
            Vertex temp_point1 = evaluate(control_points_, u, v + 1e-3f);
            Vertex temp_point2 = evaluate(control_points_, u + 1e-3f, v);
            vec3 normal = normalize(cross(temp_point1.position-point.position,temp_point2.position-point.position));
            point.normal = normal;
            surface_points.push_back(point);
            //if (point.position.z > 1.0)
            //{
            //    printf("i:%d,j:%d\n", i, j);
            //}
            if (point.position.x == 0 && point.position.y == 0 && point.position.z == 0)
            {
                printf("i:%d,j:%d\n", i, j);
            }
            if (i == 32 && j == 32)
            {
                printf("point.x:%f, y:%f, z:%f\n", point.position.x, point.position.y, point.position.z);
            }
            
        }
    }

    for (int i = 0; i < sample_num; i++) //set up indices
    {
        for (int j = 0; j < sample_num - 1; j++)
        {
            if (i % 2 == 0)
            {
                if (i == sample_num - 1) break;
                //printf("Here1\n");
                //int A = i * sample_num + j;
                //int B = i * sample_num + j + 1;
                //int C = i * sample_num + j + sample_num;
                //printf("A:%d:", A);
                //printf("B:%d:", B);
                //printf("C:%d:", C);
                //printf("\n");
                //num++;
                indices.push_back(i * sample_num + j);
                indices.push_back(i * sample_num + j + 1);
                indices.push_back(i * sample_num + j + sample_num);


            }
            else
            {
                if (i == sample_num) break;
                //printf("Here2\n");
                //int A = i * sample_num + j;
                //int B = i * sample_num + j + 1;
                //int C = i * sample_num + j - sample_num + 1;
                //num++;
                //printf("A:%d:", A);
                //printf("B:%d:", B);
                //printf("C:%d:", C);
                //printf("\n");
                indices.push_back(i * sample_num + j);
                indices.push_back(i * sample_num + j + 1);
                indices.push_back(i * sample_num + j - sample_num + 1);

                if (i != sample_num)
                {
                    //printf("Here3\n");
                    //A = i * sample_num + j;
                    //B = i * sample_num + j + 1;
                    //C = i * sample_num + j + sample_num;

                    indices.push_back(i * sample_num + j);
                    indices.push_back(i * sample_num + j + 1);
                    indices.push_back(i * sample_num + j + sample_num);

                    //printf("A:%d:", A);
                    //printf("B:%d:", B);
                    //printf("C:%d:", C);
                    //printf("\n");


                    //A = i * sample_num + j + sample_num;
                    //B = i * sample_num + j + sample_num + 1;
                    //C = i * sample_num + j + 1;
                    //printf("A:%d:", A);
                    //printf("B:%d:", B);
                    //printf("C:%d:", C);
                    //printf("\n");
                    //num += 2;
                    indices.push_back(i * sample_num + j + sample_num);
                    indices.push_back(i * sample_num + j + sample_num + 1);
                    indices.push_back(i * sample_num + j + 1);

                }

            }


        }
    }
    //printf("num:%d:",num);
    Object surface_object;
    surface_object.vertices = surface_points;
    surface_object.indices = indices;
    surface_object.draw_mode.primitive_mode = GL_TRIANGLES;
    return surface_object;

//float sample_num = 64;
//float gap = 1.0 / sample_num;
//std::vector<Vertex> surface_points;
//std::vector<GLuint> indices;
////int num = 0;
//sample_num++;
//for (int i = 0; i < sample_num; i++) //store the points into surface_points in row layout
//{
//    for (int j = 0; j < sample_num; j++)
//    {
//        //num++;
//        //printf("num:%d:",num);
//        //printf("\n");
//        float u = i * gap;
//        float v = j * gap;
//        if (u == 0) u = 0.00001;
//        if (v == 0) v = 0.00001;
//        if (u == 1) u = 0.99999;
//        if (v == 1) v = 0.99999;
//        Vertex point = evaluate(control_points_, u, v);
//        Vertex temp_point1 = evaluate(control_points_, u, v + 1e-3f);
//        Vertex temp_point2 = evaluate(control_points_, u + 1e-3f, v);
//        vec3 normal = normalize(cross(temp_point1.position - point.position, temp_point2.position - point.position));
//        point.normal = normal;
//        surface_points.push_back(point);
//        //if (point.position.z > 1.0)
//        //{
//        //    printf("i:%d,j:%d\n", i, j);
//        //}
//        if (point.position.x == 0 && point.position.y == 0 && point.position.z == 0)
//        {
//            printf("i:%d,j:%d\n", i, j);
//        }
//        if (i == 32 && j == 32)
//        {
//            printf("point.x:%f, y:%f, z:%f\n", point.position.x, point.position.y, point.position.z);
//        }
//
//    }
//}
//
//for (int i = 0; i < sample_num; i++) //set up indices
//{
//    for (int j = 0; j < sample_num - 1; j++)
//    {
//        if (i % 2 == 0)
//        {
//            if (i == sample_num - 1) break;
//            //printf("Here1\n");
//            //int A = i * sample_num + j;
//            //int B = i * sample_num + j + 1;
//            //int C = i * sample_num + j + sample_num;
//            //printf("A:%d:", A);
//            //printf("B:%d:", B);
//            //printf("C:%d:", C);
//            //printf("\n");
//            //num++;
//            indices.push_back(i * sample_num + j);
//            indices.push_back(i * sample_num + j + 1);
//            indices.push_back(i * sample_num + j + sample_num);
//
//
//        }
//        else
//        {
//            if (i == sample_num) break;
//            //printf("Here2\n");
//            //int A = i * sample_num + j;
//            //int B = i * sample_num + j + 1;
//            //int C = i * sample_num + j - sample_num + 1;
//            //num++;
//            //printf("A:%d:", A);
//            //printf("B:%d:", B);
//            //printf("C:%d:", C);
//            //printf("\n");
//            indices.push_back(i * sample_num + j);
//            indices.push_back(i * sample_num + j + 1);
//            indices.push_back(i * sample_num + j - sample_num + 1);
//
//            if (i != sample_num)
//            {
//                //printf("Here3\n");
//                //A = i * sample_num + j;
//                //B = i * sample_num + j + 1;
//                //C = i * sample_num + j + sample_num;
//
//                indices.push_back(i * sample_num + j);
//                indices.push_back(i * sample_num + j + 1);
//                indices.push_back(i * sample_num + j + sample_num);
//
//                //printf("A:%d:", A);
//                //printf("B:%d:", B);
//                //printf("C:%d:", C);
//                //printf("\n");
//
//
//                //A = i * sample_num + j + sample_num;
//                //B = i * sample_num + j + sample_num + 1;
//                //C = i * sample_num + j + 1;
//                //printf("A:%d:", A);
//                //printf("B:%d:", B);
//                //printf("C:%d:", C);
//                //printf("\n");
//                //num += 2;
//                indices.push_back(i * sample_num + j + sample_num);
//                indices.push_back(i * sample_num + j + sample_num + 1);
//                indices.push_back(i * sample_num + j + 1);
//
//            }
//
//        }
//
//
//    }
//}
////printf("num:%d:",num);
//Object surface_object;
//surface_object.vertices = surface_points;
//surface_object.indices = indices;
//surface_object.draw_mode.primitive_mode = GL_TRIANGLES;
//return surface_object;
}
