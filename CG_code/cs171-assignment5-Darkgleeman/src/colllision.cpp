#include <collision.h>
#include <utils.h>

/**
 * @brief This function checks whether the sphere collides the wall or not, and stores the collision information if necessary.
 * 
 * @param[in] wall 
 * @param[in] sphere 
 * @param[out] collision_info 
 * @return true 
 * @return false 
 */
bool CollisionHandler::Check(Wall &wall, Sphere &sphere, CollisionInfo &collision_info)
{
    return CheckParlgrmAndSphere(
        wall.GetX(), wall.GetS1(), wall.GetS2(),
        sphere.GetX(), sphere.GetRadius(), collision_info);
}

bool CollisionHandler::Sphere_Check(Sphere& sphere1, Sphere& sphere2, CollisionInfo& collision_info)
{
    return CheckSphereAndSphere(sphere1.GetX(), sphere2.GetX(), sphere1.GetRadius(), collision_info);
        
}


/**
 * @brief This function handles the collision between the sphere and the wall.
 * 
 * @param[in] wall
 * @param[in] sphere 
 */

void CollisionHandler::Handle(Wall& wall, Sphere& sphere)
{
    /// TODO: Handle the colliding contact between the sphere and the wall.
    CollisionInfo temp_info;
    if (Check(wall, sphere, temp_info))
    {
        vec3 vrel = sphere.GetV();
        vec3 surface_normal = wall.GetNormal();
        float cos_theta = dot(vrel, surface_normal) / length(vrel);
        //std::cout << "cos_theta:" << cos_theta << std::endl;
        if (cos_theta > 0)
        {
            surface_normal = -surface_normal;  //永远保证vrel和平面法向量方向相反
        }
        
       
        else cos_theta = -cos_theta;
        vec3 vrel_norm = -surface_normal * length(vrel) * cos_theta;
        vec3 vrel_tan = vrel - vrel_norm;
        //std::cout << "dot:" << dot(vrel_norm, vrel_tan);
        vec3 vout_norm = -normal_epsilon * vrel_norm;
        vec3 vout_tan = vrel_tan * tangent_epsilon;
        vec3 vout = vout_norm + vout_tan;
        vec3 delta_v = vout - vrel;
        /*std::cout << "vrel_norm:" << vrel_norm.x << "   " << vrel_norm.y << "   " << vrel_norm.z << std::endl;
        std::cout << "vrel_tan:" << vrel_tan.x << "   " << vrel_tan.y << "   " << vrel_tan.z << std::endl;
        std::cout << "vout:" << vout.x << "   " << vout.y << "   " << vout.z << std::endl;*/
        //if ()
        //{
        //    sphere.ClearConstantForces();
        //    sphere.ClearTransientForces();
        //    return;
        //}
        if (length(vrel_norm) < 0.1)
        {   
            //printf("yyy\n");
            vec3 total_force = vec3(0, 0, 0);
            for (Force& force : sphere.constant_forces)
            {
                total_force += force.F;
            }
            for (Force& force : sphere.transient_forces)
            {
                total_force += force.F;
            }

            //printf("Force:");
            //std::cout << total_force.x << "   " << total_force.y << "   " << total_force.z << std::endl;
            //printf("times:%d\n", times);
            sphere.count++;
            //total_force和surface normal反向
            float force_cos_theta = dot(total_force, wall.GetNormal()) / length(total_force);
            //vec3 N = surface_normal * length(total_force) * (-force_cos_theta);//vec3(-4.704, 6.272, 0)
            if (force_cos_theta < 0) 
            {
                vec3 N = wall.GetNormal() * length(total_force) * -force_cos_theta;//vec3(-4.704, 6.272, 0)
                //std::cout << "N:" << N.x << "   " << N.y << "   " << N.z << std::endl;
                sphere.transient_forces.push_back(Force(temp_info.collision_point, N));
                sphere.transient_forces.push_back(Force(temp_info.collision_point, (vout_tan-vrel_tan) * sphere.GetM() / dt));
                sphere.transient_forces.push_back(Force(temp_info.collision_point, vrel_norm * sphere.GetM()*(-1.0f)+ vout_norm * sphere.GetM() * (1.0f) / dt));
                //printf("eee\n");

                //printf("sphere.count:%d\n", sphere.count);
                if (length(vout) < 0.1 && sphere.count ==2)
                {   
                    //printf("yeahyahyahasd\n");
                    sphere.stop();
                    return;
                }
            }
        }
        else
        {   
            //printf("sss\n");
            sphere.transient_forces.push_back(Force(temp_info.collision_point, delta_v * sphere.GetM() / dt));
        }
        
    }
}

void CollisionHandler::Sphere_Handle(Sphere& sphere1, Sphere& sphere2)
{
    CollisionInfo temp_info;
    if (Sphere_Check(sphere1, sphere2, temp_info))
    {
        vec3 v1 = sphere1.GetV();
        vec3 v2 = sphere2.GetV();
        vec3 contact_normal = sphere1.GetX() - sphere2.GetX();
        float vrel = dot(v1 - v2, contact_normal);
        //printf("vrel:%f\n", vrel);
        if (vrel < 0)
        {   
            //printf("yes\n");
            float j = -(1 + normal_epsilon) * vrel / (1 / sphere1.GetM() + 1 / sphere2.GetM());
            //printf("j:%f\n", j);
            sphere1.transient_forces.push_back(Force(temp_info.collision_point, j * contact_normal/dt));
            sphere2.transient_forces.push_back(Force(temp_info.collision_point, -j * contact_normal/dt));
        }
    }
}

/**
 * @brief This function checks whether the parallelogram collides with the sphere.
 *        The collision information will be store if there exists the collision.
 * @param[in] parlgrm_x 
 * @param[in] parlgrm_s1 
 * @param[in] parlgrm_s2 
 * @param[in] sphere_x 
 * @param[in] sphere_r 
 * @param[out] collision_info 
 * @return true 
 * @return false 
 */
bool CollisionHandler::CheckParlgrmAndSphere(
    glm::vec3 parlgrm_x,
    glm::vec3 parlgrm_s1,
    glm::vec3 parlgrm_s2,
    glm::vec3 sphere_x,
    float sphere_r,
    CollisionInfo &collision_info)
{
    /// TODO: Check the collision between the parallelogram and the sphere.
    glm::vec3 delta = sphere_x - parlgrm_x;
    glm::vec3 normal = normalize(cross(parlgrm_s1, parlgrm_s2));
    float h = dot(delta, normal);//h>0 同侧，h<0，x在面的下面
    float degree = abs(h) - sphere_r;
    //printf("degree:%f\n", degree);
    if (degree >= 0)
    {
        collision_info.is_collided = false;
        collision_info.collision_degree = 0;
        return false;
    }
    else
    {   
        vec3 projection_center = sphere_x - h * normal;
        vec3 left_corner = parlgrm_x - parlgrm_s1 / 2.0f - parlgrm_s2 / 2.0f; // bottom-left
        vec3 c = projection_center - left_corner;
        vec3 a = parlgrm_s1; vec3 b = parlgrm_s2;
        float t1 = 0;
        float t2 = 0;
        if (a.x * b.y - a.y * b.x != 0)
        {
            t1 = (c.x * b.y - c.y * b.x) / (a.x * b.y - a.y * b.x);
            t2 = (a.y * c.x - c.y * a.x) / (b.x * a.y - a.x * b.y);
        }
        else if (a.x * b.z - a.z * b.x != 0)
        {
            t1 = (c.x * b.z - c.z * b.x) / (a.x * b.z - a.z * b.x);
            t2 = (a.z * c.x - c.z * a.x) / (b.x * a.z - a.x * b.z);
        }
        else if (a.z * b.y - a.y * b.z != 0)
        {
            t1 = (c.z * b.y - c.y * b.z) / (a.z * b.y - a.y * b.z);
            t2 = (a.y * c.z - c.y * a.z) / (b.z * a.y - a.z * b.y);
        }

        if (0 <= t1 && t1 <= 1 && 0 <= t2 && t2 <= 1)
        {
            collision_info.collision_degree = -degree;
            collision_info.collision_point = sphere_x - h * normal;
            collision_info.is_collided = true;
            return true;
        }
        else
        {
            collision_info.is_collided = false;
            collision_info.collision_degree = 0;
            return false;
        }
    }
    
}

bool CollisionHandler::CheckSphereAndSphere(
    glm::vec3 sphere1_x,
    glm::vec3 sphere2_x,
    float sphere_r,
    CollisionInfo& collision_info)
{
    /// TODO: Check the collision between the parallelogram and the sphere.
    glm::vec3 delta = sphere1_x - sphere2_x;
    float degree = length(delta) - 2*sphere_r;
    //printf("degree:%f\n", degree);
    if (degree >= 0)
    {
        collision_info.is_collided = false;
        collision_info.collision_degree = 0;
        return false;
    }
    else
    {
        collision_info.collision_degree = -degree;
        collision_info.collision_point = sphere2_x +delta /2.0f;
        collision_info.is_collided = true;
        return true;
    }

}


