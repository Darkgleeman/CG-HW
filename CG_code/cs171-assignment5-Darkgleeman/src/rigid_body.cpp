#include <rigid_body.h>

///////////////////// RigidBody /////////////////////
glm::vec3 RigidBody::GetX()
{
    return this->x;
}

glm::vec3 RigidBody::GetV()
{
    return this->v;
}

float RigidBody::GetM()
{
    return this->M;
}


void RigidBody::Draw(Shader &shader, ShaderParam &shader_param)
{
    this->opengl_object.Draw(shader, shader_param);
}

void RigidBody::ClearTransientForces()
{
    this->transient_forces.clear();
    this->count = 0;
}

void RigidBody::ClearConstantForces()
{
    this->constant_forces.clear();

}


///////////////////// Sphere /////////////////////
Sphere::Sphere(
    glm::vec3 color,
    glm::vec3 x,
    glm::vec3 v,
    glm::vec3 w,
    float M,
    float radius)
{
    this->x = x;
    this->v = v;
    this->w = w;//角速度
    this->M = M;//质量
    this->count = 0;
    this->radius = radius;
    this->is_stop = false;
    InitOpenGLObject(color);
    InitInertiaTensor();

    Force G(glm::vec3(0.0f, 0.0f, 0.0f), this->M * g);//grativity
    this->constant_forces.push_back(G);

    this->P = this->M * this->v;
    this->L = this->I * this->w; //I->转动惯量
}

/**
 * @brief Get the radius of the sphere.
 * 
 * @return float 
 */
float Sphere::GetRadius()
{
    return this->radius;
}

void Sphere::stop()
{
    this->transient_forces.clear();
    this->count = 0;
    this->constant_forces.clear();
    this -> is_stop = true;
}

/**
 * @brief This function will update the sphere's state forward for the time step dt.
 * 
 * @param[in] dt 
 */
void Sphere::Forward(float dt)
{
    /// TODO: Your code here.
    if (is_stop) return;
    for (Force& force : this->constant_forces)
    {
        this->P += force.F * dt;
    }
    for (Force& force : this->transient_forces)
    {
        this->P += force.F * dt;
    }
    //std::cout <<"P:"<< P.x << "   " << P.y << "   " << P.z << std::endl;
    this->v = this->P / this->M;
    //std::cout << "v:" << v.x << "   " << v.y << "   " << v.z << std::endl;
    this->x += this->v * dt;
    UpdateOpenGLObject();
}

/**
 * @brief This function will update the sphere's state backward for the time step dt.
 *        You may need this function to handle the collision.
 * 
 * @param[in] dt 
 */
void Sphere::Backward(float dt)
{   
    if (is_stop) return;
    /// TODO: Your code here.
    this->x -= this->v * dt;

    for (Force& force : this->constant_forces)
    {
        this->P -= force.F * dt;
    }
    for (Force& force : this->transient_forces)
    {
        this->P -= force.F * dt;
    }
    this->v = this->P / this->M;

    UpdateOpenGLObject();
}

/**
 * @brief If you consider the rotation as well, you need to handle the intertia tensor.
 * 
 */
void Sphere::InitInertiaTensor()
{
    return;
}

/**
 * @brief This function initializes the opengl object for rendering.
 *        Specifically, the vertices, indices are generated.
 * 
 * @param[in] color 
 */
void Sphere::InitOpenGLObject(glm::vec3 color)
{
    opengl_object.color = color;

    float delta = 0.01;
    int size = 2 * PI / delta + 1;

    opengl_object.vertices.resize(size * size);
    for (int t = 0; t < size; t++)
    {
        float theta = (t == size - 1) ? (2 * PI) : t * delta;
        for (int p = 0; p < size; p++)
        {
            float phi = (p == size - 1) ? (2 * PI) : p * delta;

            float x = radius * cosf(phi) * sinf(theta);
            float y = radius * sinf(phi) * sinf(theta);
            float z = radius * cosf(theta);

            opengl_object.vertices[t * size + p].position = glm::vec3(x, y, z);
            opengl_object.vertices[t * size + p].normal = normalize(glm::vec3(x, y, z));
        }
    }

    opengl_object.indices.reserve(3 * 2 * (size - 1) * (size - 1));
    for (int t = 0; t < size - 1; t++)
    {
        for (int p = 0; p < size - 1; p++)
        {
            opengl_object.indices.push_back(t * size + p);
            opengl_object.indices.push_back(t * size + p + 1);
            opengl_object.indices.push_back((t + 1) * size + p);

            opengl_object.indices.push_back(t * size + p + 1);
            opengl_object.indices.push_back((t + 1) * size + p);
            opengl_object.indices.push_back((t + 1) * size + p + 1);
        }
    }

    opengl_object.M = glm::translate(glm::mat4(1.0f), this->x);

    opengl_object.InitData();
}

/**
 * @brief This functioin updates the opengl object for rendering.
 *        We do not consider the rotation so we only need to handle the translation.
 *        
 */
void Sphere::UpdateOpenGLObject()
{
    opengl_object.M = glm::translate(glm::mat4(1.0f), this->x);//model transformation matrix ,M->model matrix
}

///////////////////// Wall /////////////////////
Wall::Wall(
    glm::vec3 color,
    glm::vec3 x,
    glm::vec3 s1,
    glm::vec3 s2)
{
    this->x = x;
    this->s1 = s1;
    this->s2 = s2;

    this->v = glm::vec3(0.0f);
    this->w = glm::vec3(0.0f);
    this->M = INFINITY;

    InitInertiaTensor();
    InitOpenGLObject(color);

    this->P = glm::vec3(0.0f);
    this->L = glm::vec3(0.0f);
}

glm::vec3 Wall::GetS1()
{
    return this->s1;
}

glm::vec3 Wall::GetS2()
{
    return this->s2;
}

glm::vec3 Wall::GetNormal()
{
    return normalize(cross(s1, s2));
}

void Wall::Forward(float dt)
{
    return;
}

void Wall::Backward(float dt)
{
    return;
}

void Wall::InitInertiaTensor()
{
    return;
}


void Wall::InitOpenGLObject(glm::vec3 color)
{
    opengl_object.color = color;

    opengl_object.vertices.resize(4);
    opengl_object.vertices[0].position = this->x - s1 / 2.0f + s2 / 2.0f; // top-left
    opengl_object.vertices[1].position = this->x + s1 / 2.0f + s2 / 2.0f; // top-right
    opengl_object.vertices[2].position = this->x - s1 / 2.0f - s2 / 2.0f; // bottom-left
    opengl_object.vertices[3].position = this->x + s1 / 2.0f - s2 / 2.0f; // bottom-right
    opengl_object.vertices[0].normal = normalize(cross(s1, s2));
    opengl_object.vertices[1].normal = opengl_object.vertices[0].normal;
    opengl_object.vertices[2].normal = opengl_object.vertices[0].normal;
    opengl_object.vertices[3].normal = opengl_object.vertices[0].normal;

    opengl_object.indices = {0, 1, 2, 1, 2, 3};

    opengl_object.M = glm::mat4(1.0f);

    opengl_object.InitData();

    return;
}

void Wall::UpdateOpenGLObject()
{
    return;
}
