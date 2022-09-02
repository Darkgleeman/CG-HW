#include <utils.h>
#include <object.h>
#include <shader.h>
#include <bezier.h>
#include <camera.h>

const int WIDTH = 1920;
const int HEIGHT = 1080;
GLFWwindow* window;
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

// camera
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

glm::vec3 unproject_pt = glm::vec3(0.0f, 0.0f, 0.0f);
glm::mat4 proj = glm::mat4(1.0f);
glm::mat4 view_mat = glm::mat4(1.0f);
int control_index = 100;
int has_changed = 0;
//glm::vec4 control_2d[] = {
//        glm::vec4(0.0f, 0.0f, 0.0f, 0.0f),
//glm::vec4(0.0f, 0.0f, 0.0f, 0.0f),
//glm::vec4(0.0f, 0.0f, 0.0f, 0.0f),
//glm::vec4(0.0f, 0.0f, 0.0f, 0.0f),
//glm::vec4(0.0f, 0.0f, 0.0f, 0.0f),
//glm::vec4(0.0f, 0.0f, 0.0f, 0.0f),
//glm::vec4(0.0f, 0.0f, 0.0f, 0.0f),
//glm::vec4(0.0f, 0.0f, 0.0f, 0.0f),
//glm::vec4(0.0f, 0.0f, 0.0f, 0.0f),
//glm::vec4(0.0f, 0.0f, 0.0f, 0.0f),
//glm::vec4(0.0f, 0.0f, 0.0f, 0.0f),
//glm::vec4(0.0f, 0.0f, 0.0f, 0.0f),
//};

bool firstMouse = true;
float yaw = -90.0f;
float pitch = 0.0f;
float lastX = 1920.0f / 2.0;
float lastY = 1080.0f / 2.0;
float hitsX = 1920.0f / 2.0;
float hitsY = 1080.0f / 2.0;
float fov = 45.0f;
// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

glm::vec3 control_pts[] = {
glm::vec3(-0.5f, 0.0f, 0.5f),
    glm::vec3(-0.5f, 0.5f, 0.5f),
    glm::vec3(0.5f, 0.5f, 0.5f),
    glm::vec3(0.5f, 0.0f, 0.5f),
    glm::vec3(-0.3f, 0.0f, 0.0f),
    glm::vec3(-0.3f, 0.3f, 0.0f),
    glm::vec3(0.3f, 0.3f, 0.0f),
    glm::vec3(0.3f, 0.0f, 0.0f),
    glm::vec3(-0.5f, 0.0f, 1.0f),
    glm::vec3(-0.5f, 0.5f, 1.0f),
    glm::vec3(0.5f, 0.5f, 1.0f),
    glm::vec3(0.5f, 0.0f, 1.0f),

};

//glm::vec3 control_pts[] = {
//    glm::vec3(-1.0f,1.0f,0.0f),
//    glm::vec3(-0.5f, 1.0f, 0.0f),
//    glm::vec3(0.0f, 1.0f, 0.0f),
//    glm::vec3(0.5f, 1.0f, 0.0f),
//    glm::vec3(1.0f, 1.0f, 0.0f),
//    glm::vec3(-1.0f, 0.5f, 0.0f),
//    glm::vec3(-0.5f, 0.5f, 0.0f),
//    glm::vec3(0.0f, 0.5f, 0.0f),
//    glm::vec3(0.5f, 0.5f, 0.0f),
//    glm::vec3(1.0f, 0.5f, 0.0f),
//    glm::vec3(-1.0f, 0.0f, 0.0f),
//    glm::vec3(-0.5f, 0.0f, 0.0f),
//    glm::vec3(0.0f, 0.0f, 2.0f),
//    glm::vec3(0.5f, 0.0f, 0.0f),
//    glm::vec3(1.0f, 0.0f, 0.0f),
//    glm::vec3(-1.0f, -0.5f, 0.0f),
//    glm::vec3(-0.5f, -0.5f, 0.0f),
//    glm::vec3(0.0f, -0.5f, 0.0f),
//    glm::vec3(0.5f, -0.5f, 0.0f),
//    glm::vec3(1.0f, -0.5f, 0.0f),
//    glm::vec3(-1.0f,-1.0f,0.0f),
//    glm::vec3(-0.5f, -1.0f, 0.0f),
//    glm::vec3(0.0f, -1.0f, 0.0f),
//    glm::vec3(0.5f, -1.0f, 0.0f),
//    glm::vec3(1.0f, -1.0f, 0.0f),
//
//
//};


glm::vec3 control_pts_copy[] = {
glm::vec3(-0.5f, 0.0f, 0.5f),
    glm::vec3(-0.5f, -0.5f, 0.5f),
    glm::vec3(0.5f, -0.5f, 0.5f),
    glm::vec3(0.5f, 0.0f, 0.5f),
    glm::vec3(-0.3f, 0.0f, 0.0f),
    glm::vec3(-0.3f, -0.3f, 0.0f),
    glm::vec3(0.3f, -0.3f, 0.0f),
    glm::vec3(0.3f, 0.0f, 0.0f),
    glm::vec3(-0.5f, 0.0f, 1.0f),
    glm::vec3(-0.5f, -0.5f, 1.0f),
    glm::vec3(0.5f, -0.5f, 1.0f),
    glm::vec3(0.5f, 0.0f, 1.0f),

};


int main() {
  WindowGuard windowGuard(window, WIDTH, HEIGHT, "CS171 hw2");
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetCursorPosCallback(window, mouse_callback);
  glEnable(GL_DEPTH_TEST);
  Shader easyShader(getPath("src/curve_model.vs"), getPath("src/curve_model.fs"));
  Shader curveShader(getPath("src/curve_model.vs"), getPath("src/curve_model.fs"));
  Shader control_ptsShader(getPath("src/control_pts.vs"), getPath("src/control_pts.fs"));

  float vertices[] = {
      // positions          // normals        
      -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  
       0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  
       0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  
       0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  
      -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  
      -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  

      -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  
       0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  
       0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  
       0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  
      -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  
      -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  

      -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  
      -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  
      -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  
      -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  
      -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  
      -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  

       0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  
       0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  
       0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  
       0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  
       0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  
       0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  

      -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  
       0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  
       0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  
       0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  
      -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  
      -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  

      -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  
       0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  
       0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  
       0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  
      -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  
      -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  
  };

  int m = 3;
  int n = 4;

  int control_num = m * n;
  BezierSurface surface(m, n);
  BezierSurface surface_copy(m, n);
  for (int i = 0; i < m; i++)
  {
      for (int j = 0; j < n; j++)
      {
          surface.setControlPoint(i, j, control_pts[i * n + j]);
      }
  }
  Object surface_obj = surface.generateObject();
  surface_obj.init();


  for (int i = 0; i < m; i++)
  {
      for (int j = 0; j < n; j++)
      {
          surface_copy.setControlPoint(i, j, control_pts_copy[i * n + j]);
      }
  }
  Object surface_obj_copy = surface_copy.generateObject();
  surface_obj_copy.init();

  //m = 5;
  //n = 5;
  //control_num = m * n;
  //B_splinesurface b_surface(m, n);
  //for (int i = 0; i < m; i++)
  //{
  //    for (int j = 0; j < n; j++)
  //    {
  //        b_surface.setControlPoint(i, j, control_pts[i * n + j]);
  //    }
  //}
  //Object b_surface_obj = b_surface.generateObject();
  //b_surface_obj.init();
  


  glm::vec3 pointLightPositions[] = {
      glm::vec3(0.0f,  0.0f,  0.0f),
  };
  //BezierCurve curve(control_num);
  //for (int i = 0; i < control_num; i++)
  //{
  //    curve.setControlPoint(i, control_pts[i]);
  //}
  //Object curve_obj = curve.generateObject();
  //curve_obj.init();
 

  //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  while (!glfwWindowShouldClose(window)) {

      unsigned int ctrl_pts_VAO, VBO;
      unsigned int ctrl_pts_VAO_copy;
      glGenVertexArrays(1, &ctrl_pts_VAO);
      glBindVertexArray(ctrl_pts_VAO);
      glGenBuffers(1, &VBO);
      glBindBuffer(GL_ARRAY_BUFFER, VBO);
      glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
      glEnableVertexAttribArray(0);

      glGenVertexArrays(1, &ctrl_pts_VAO_copy);
      glBindVertexArray(ctrl_pts_VAO_copy);
      glBindBuffer(GL_ARRAY_BUFFER, VBO);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
      glEnableVertexAttribArray(0);

    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    processInput(window);
    if (has_changed != 0)
    {   

        surface.setControlPoint(int(control_index/n), control_index % n, control_pts[control_index]);
        surface_obj = surface.generateObject();
        surface_obj.init();
        has_changed = 0;
    }

    glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
      glfwSetWindowShouldClose(window, true);

    curveShader.use();
    curveShader.setVec3("viewPos", cameraPos);
    curveShader.setVec3("objectColor", glm::vec3(1.0f, 1.0f, 1.0f));

    curveShader.setVec3("pointLights[0].position", pointLightPositions[0]);
    curveShader.setVec3("pointLights[0].lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
    curveShader.setFloat("pointLights[0].constant", 1.0f);
    curveShader.setFloat("pointLights[0].linear", 0.09);
    curveShader.setFloat("pointLights[0].quadratic", 0.032);

    // view/projection transformations
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    proj = projection;
    view_mat = view;
    glm::mat4 model = glm::mat4(1.0f);
    curveShader.setMat4("projection", projection);
    curveShader.setMat4("view", view);
    curveShader.setMat4("model", model);
    surface_obj.drawElements();
    curveShader.setVec3("objectColor", glm::vec3(0.0f, 0.0f, 1.0f));
    surface_obj_copy.drawElements();

    //easyShader.use();
    //easyShader.setVec3("viewPos", cameraPos);
    //easyShader.setVec3("objectColor", glm::vec3(1.0f, 1.0f, 1.0f));
    //easyShader.setMat4("projection", projection);
    //easyShader.setMat4("view", view);
    //easyShader.setMat4("model", model);


    //b_surface_obj.drawElements();

    control_ptsShader.use();
    control_ptsShader.setMat4("projection", projection);
    control_ptsShader.setMat4("view", view);

    glBindVertexArray(ctrl_pts_VAO);
    for (unsigned int i = 0; i < control_num; i++)
    {
        model = glm::mat4(1.0f);
        model = glm::translate(model, control_pts[i]);
        model = glm::scale(model, glm::vec3(0.01f)); 
        control_ptsShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    glBindVertexArray(ctrl_pts_VAO_copy);
    for (unsigned int i = 0; i < control_num; i++)
    {
        model = glm::mat4(1.0f);
        model = glm::translate(model, control_pts_copy[i]);
        model = glm::scale(model, glm::vec3(0.01f)); 
        control_ptsShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    glfwPollEvents();
    glfwSwapBuffers(window);
  }

  return 0;
}

int search_control_points(GLFWwindow* window)
{   
    //printf("hitx:%f     ", hitsX);
    //printf("hity:%f\n", hitsY);
    glm::vec3 screenpos = glm::unProject(glm::vec3(hitsX, hitsY, 0), view_mat, proj, glm::ivec4(0, 0, WIDTH, HEIGHT));
    //printf("screenpos.x:%f, y:%f, z:%f\n", screenpos.x, screenpos.y, screenpos.z);
    double min_dis = 100000000;
    int index = 1000;
    for (int i = 0; i < 12; i++)
    {
        //glm::vec3 screenp = control_2d[i].xyx;
        //printf("control_2d[i].x:%f, y:%f, z:%f\n", control_2d[i].x, control_2d[i].y, control_2d[i].z);
        float temp = glm::length(screenpos - glm::vec3(control_pts[i].x, control_pts[i].y, control_pts[i].z));
        if (min_dis > temp)
        {
            min_dis = temp;
            index = i;
        }
    }
    printf("index:%d", index);
    return index;
}

void set_pos(GLFWwindow* window)
{
    //float mouseX = hitsX / (WIDTH * 0.5) - 1;
    //float mouseY = hitsY / (HEIGHT * 0.5) - 1;
    //glm::vec4 screenpos = glm::vec4(mouseX, -mouseY, 1.0f, 1.0f);
    //glm::mat4 view_mat = proj * view_mat * control_2d_model[control_index];
    //glm::mat4 inv = glm::inverse(view_mat);
    //glm::vec4 world_pos = inv * screenpos;
    //control_pts[control_index] = glm::vec3(world_pos.x, world_pos.y, world_pos.z);
    control_pts[control_index] = glm::unProject(glm::vec3(hitsX, hitsY, 0), view_mat, proj, glm::ivec4(0, 0, WIDTH, HEIGHT));
    has_changed = 1;
}


void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float cameraSpeed = 2.5 * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    //if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    //{
    //    control_index = search_control_points(window);
    //}
    //if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
    //{
    //    set_pos(window);
    //}
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        control_index = search_control_points(window);
        set_pos(window);
    }
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
    {
        set_pos(window);
    }
}


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    hitsX = xpos;
    hitsY = ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    fov -= (float)yoffset;
    if (fov < 1.0f)
        fov = 1.0f;
    if (fov > 45.0f)
        fov = 45.0f;
}
