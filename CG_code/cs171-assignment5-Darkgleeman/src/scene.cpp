#include <scene.h>
#include <collision.h>

/**
 * @brief Construct a new Scene:: Scene object.
 * 
 * @param[in] id The scene id: 0, 1, 2, 3, 4.
 * @param[in] dt The simulation time step.
 * @param[in] normal_epsilon The collision decay factor along the normal direction.
 * @param[in] tangent_epsilon The collision decay factor along the tangent direction.
 * @param[in] intersect_tolerance The intersection tolerance threshold.
 */
Scene::Scene(
    int id,
    float dt,
    float normal_epsilon,
    float tangent_epsilon,
    float intersect_tolerance)
{
    this->dt = dt;
    this->collision_handler.dt = dt;
    this->collision_handler.normal_epsilon = normal_epsilon;
    this->collision_handler.tangent_epsilon = tangent_epsilon;
    this->intersect_tolerance = intersect_tolerance;

    float x_span = 10;
    float y_span = 10;
    float z_span = 20;
    float z_offset = 7.5;

    this->walls.push_back(Wall( // floor
        glm::vec3(0.725000, 0.710000, 0.680000),
        glm::vec3(0, 0, 0 + z_offset),
        glm::vec3(x_span, 0, 0),
        glm::vec3(0, 0, -z_span)));

    this->walls.push_back(Wall( // ceiling
        glm::vec3(0.725000, 0.710000, 0.680000),
        glm::vec3(0, y_span, 0 + z_offset),
        glm::vec3(x_span, 0, 0),
        glm::vec3(0, 0, z_span)));
    this->walls.push_back(Wall( // back
        glm::vec3(0.725000, 0.710000, 0.680000),
        glm::vec3(0, y_span / 2.0f, -z_span / 2.0f + z_offset),
        glm::vec3(x_span, 0, 0),
        glm::vec3(0, y_span, 0)));
    this->walls.push_back(Wall( // front
        glm::vec3(0.725000, 0.710000, 0.680000),
        glm::vec3(0, y_span / 2.0f, z_span / 2.0f + z_offset),
        glm::vec3(-x_span, 0, 0),
        glm::vec3(0, y_span, 0)));
    this->walls.push_back(Wall( // right
        glm::vec3(0.140000, 0.450000, 0.091000),
        glm::vec3(x_span / 2.0f, y_span / 2.0f, 0 + z_offset),
        glm::vec3(0, 0, z_span),
        glm::vec3(0, y_span, 0)));
    this->walls.push_back(Wall( // left
        glm::vec3(0.630000, 0.065000, 0.050000),
        glm::vec3(-x_span / 2.0f, y_span / 2.0f, 0 + z_offset),
        glm::vec3(0, 0, -z_span),
        glm::vec3(0, y_span, 0)));

    if (id == 0) // One sphere
    {
        this->spheres.push_back(Sphere(
            glm::vec3(0.725000, 0.710000, 0.680000),
            glm::vec3(-4.5, 9.5, 0),
            glm::vec3(0, 0, 0),
            glm::vec3(0, 0, 0),
            1,
            0.5));
    }
    else if (id == 1)
    {
        this->walls.push_back(Wall(
            glm::vec3(0.725000, 0.710000, 0.680000),
            glm::vec3(3, 3, 0 + z_offset),
            glm::vec3(0, 0, z_span),
            glm::vec3(8, 6, 0)));
        this->spheres.push_back(Sphere(
            glm::vec3(0.725000, 0.710000, 0.680000),
            glm::vec3(4.5, 5, 5),
            glm::vec3(0, 0, -2),
            glm::vec3(0, 0, 0),
            1,
            0.5));
    }
    else if (id == 2)
    {
        this->walls.push_back(Wall(
            glm::vec3(0.725000, 0.710000, 0.680000),
            glm::vec3(0, 4, -z_span / 2 + z_offset + 2),
            glm::vec3(0, 0, 4),
            glm::vec3(10, 2, 0)));

        this->walls.push_back(Wall(
            glm::vec3(0.725000, 0.710000, 0.680000),
            glm::vec3(0, 2, -z_span / 2 + z_offset + 2),
            glm::vec3(0, 0, 4),
            glm::vec3(10, -2, 0)));

        this->spheres.push_back(Sphere(
            glm::vec3(0.725000, 0.580000, 0.710000),
            glm::vec3(-4.5, 7, -z_span / 2 + z_offset + 2),
            glm::vec3(3, 2, 0),
            glm::vec3(0, 0, 0),
            1,
            0.5));

        this->spheres.push_back(Sphere(
            glm::vec3(0.580000, 0.725000, 0.710000),
            glm::vec3(4.48, 3, -z_span / 2 + z_offset + 2),
            glm::vec3(0, 10, 0),
            glm::vec3(0, 0, 0),
            1,
            0.5));

        this->spheres.push_back(Sphere(
            glm::vec3(0.580000, 0.725000, 0.710000),
            glm::vec3(0, 1, -z_span / 2 + z_offset + 2),
            glm::vec3(0, 10, 0),
            glm::vec3(0, 0, 0),
            1,
            0.5));
    }
    else if (id == 3)
    {
        this->walls.push_back(Wall(
            glm::vec3(0.725000, 0.710000, 0.680000),
            glm::vec3(3, 3, 0 + z_offset),
            glm::vec3(0, 0, z_span),
            glm::vec3(8, 6, 0)));

        this->spheres.push_back(Sphere(
            glm::vec3(0.725000, 0.580000, 0.710000),
            glm::vec3(-4.5, 0.5, 0),
            glm::vec3(5, 0, 0),
            glm::vec3(0, 0, 0),
            1,
            0.5));

        this->spheres.push_back(Sphere(
            glm::vec3(0.580000, 0.725000, 0.710000),
            glm::vec3(4.5, 5.5, 0),
            glm::vec3(-3, 0, 0),
            glm::vec3(0, 0, 0),
            1,
            0.5));
    }
    else if (id == 4)
    {
        this->walls.push_back(Wall(
            glm::vec3(0.630000, 0.065000, 0.050000),
            glm::vec3(-2.5, 2.5, -z_span / 2 + z_offset + 2),
            glm::vec3(5, -3, 0),
            glm::vec3(0, 0, -4)));

        this->walls.push_back(Wall(
            glm::vec3(0.140000, 0.450000, 0.091000),
            glm::vec3(2.5, 2.5, -z_span / 2 + z_offset + 2),
            glm::vec3(0, 0, 4),
            glm::vec3(5, 3, 0)));

        this->intersect_tolerance = 1e-4;
        this->spheres.push_back(Sphere(
            glm::vec3(0.725000, 0.580000, 0.710000),
            glm::vec3(-4.5, 9.5, -z_span / 2 + z_offset + 2),
            glm::vec3(5, 0, 0),
            glm::vec3(0, 0, 0),
            1,
            0.5));

        this->spheres.push_back(Sphere(
            glm::vec3(0.580000, 0.725000, 0.710000),
            glm::vec3(4.5, 9.5, -z_span / 2 + z_offset + 2),
            glm::vec3(-5, 0, 0),
            glm::vec3(0, 0, 0),
            1,
            0.5));
    }
}

/**
 * @brief Update the scene.
 */
void Scene::Update()
{
    /// TODO: Specifically, this will update the whole scene forward for one time step.
    /// If the intersection is detected, firstly, the collision position will be adjusted if necessary.
    /// After that, the collision force will be applied to the objects.
    Forward(dt);
    float degree;
    if (IsCollided(degree))
    {
        AdjustCollision(degree);
        ClearTransientForces();
        ApplyCollision();
    }
    else
    {
        ClearTransientForces();
    }
}

/**
 * @brief This will update all the spheres forward for one step.
 *        But there is need to update the walls, since they are static.
 * 
 * @param[in] dt 
 */
void Scene::Forward(float dt)
{
    for (Sphere &sphere : this->spheres)
    {
        sphere.Forward(dt);
    }
}

/**
 * @brief This will update all the spheres backward for one step.
 *        But there is need to update the walls, since they are static.
 *        You may need this function to adjust the collision.
 * @param[in] dt 
 */
void Scene::Backward(float dt)
{
    for (Sphere &sphere : this->spheres)
    {
        sphere.Backward(dt);
    }
}

/**
 * @brief This will check whether any collision happens in the scene.
 *        And the intersection degree is noted down.
 * @param[out] intersect_degree The intersection degree to be updated.
 * @return true 
 * @return false 
 */
//bool Scene::IsCollided(float &intersect_degree)
//{
//    bool is_collided = false;
//    intersect_degree = 0;
//
//    // Wall-sphere intersection check
//    for (Wall &wall : this->walls)
//    {
//        for (Sphere &sphere : this->spheres)
//        {
//            CollisionInfo temp_info;
//            if (this->collision_handler.Check(wall, sphere, temp_info))
//            {
//                is_collided = true;
//                intersect_degree = fmax(intersect_degree, temp_info.collision_degree);
//            }
//        }
//    }
//
//    return is_collided;
//}

bool Scene::IsCollided(float& intersect_degree)
{
    bool is_collided = false;
    intersect_degree = 0;

    // Wall-sphere intersection check
    for (Wall& wall : this->walls)
    {
        for (Sphere& sphere : this->spheres)
        {
            CollisionInfo temp_info;
            if (this->collision_handler.Check(wall, sphere, temp_info))
            {
                is_collided = true;
                intersect_degree = fmax(intersect_degree, temp_info.collision_degree);
            }
        }
    }

    for (int i = 0;i<spheres.size();i++)
    {
        for (int j=i+1;j<spheres.size();j++)
        {
            CollisionInfo temp_info;
            if (this->collision_handler.Sphere_Check(spheres[i], spheres[j], temp_info))
            {
                is_collided = true;
                intersect_degree = fmax(intersect_degree, temp_info.collision_degree);
            }
        }
    }

    return is_collided;
}


/**
 * @brief This will adjust the collision position if necessary.
 * 
 * @param[in] intersect_degree 
 */

void Scene::AdjustCollision(float& intersect_degree)
{   
    float temp_dt = dt / 2;
    int flag = 0;//flag= 0->backward;flag =1->forward
    if (intersect_degree != 0 && intersect_degree < intersect_tolerance) return;
    while (true)
    {
        if (temp_dt < 0.0005) break;
        if (flag == 0) Backward(temp_dt);
        else Forward(temp_dt);
        if (IsCollided(intersect_degree))//改变temp_dt还是有相交
        {   
            if (intersect_degree < intersect_tolerance)
            {   
                return;
            }
            flag = 0;
            temp_dt /= 2;
        }
        else
        {
            flag = 1;
            temp_dt /= 2;
        }
    }
}
/**
 * @brief This will handle the collision between each object pair.
 * 
 */
void Scene::ApplyCollision()
{
    // Wall-sphere intersection handle
    for (int i = 0; i < spheres.size(); i++)
    {
        for (int j = i + 1; j < spheres.size(); j++)
        {
            this->collision_handler.Sphere_Handle(spheres[i], spheres[j]);
        }
    }
    for (Wall &wall : this->walls)
    {
        for (Sphere &sphere : this->spheres)
        {
            this->collision_handler.Handle(wall, sphere);
        }
    }
    
}

/**
 * @brief Clear the transient forces of each object.
 * 
 */
void Scene::ClearTransientForces()
{
    for (Sphere &sphere : this->spheres)
    {
        sphere.ClearTransientForces();
    }
}

/**
 * @brief Draw the whole scene.
 * 
 * @param[in] shader 
 * @param[in] shader_param 
 */
void Scene::Draw(Shader &shader, ShaderParam &shader_param)
{
    for (Wall &wall : this->walls)
    {
        wall.Draw(shader, shader_param);
    }

    for (Sphere &sphere : this->spheres)
    {
        sphere.Draw(shader, shader_param);
    }
}