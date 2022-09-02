#include <utils.h>
#include <mesh.h>
#include <shader.h>
const int WIDTH = 1920;
const int HEIGHT = 1080;

GLFWwindow *window;
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

// camera
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

bool firstMouse = true;
float yaw = -90.0f;	
float pitch = 0.0f;
float lastX = 1920.0f / 2.0;
float lastY = 1080.0f / 2.0;
float fov = 45.0f;

// timing
float deltaTime = 0.0f;	
float lastFrame = 0.0f;

int main() {
  WindowGuard windowGuard(window, WIDTH, HEIGHT, "CS171 hw1");
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetScrollCallback(window, scroll_callback);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  glEnable(GL_DEPTH_TEST);
  //lightning shader->object£¬lightcubeshade->light source,NormalShder->object normal
  //Shader lightSourceShader(getPath("src/light_source.vs"), getPath("src/light_source.fs"));
  Shader lightingShader(getPath("src/lighting_model.vs"), getPath("src/lighting_model.fs"));
  Shader NormalShader(getPath("src/normal.vs"), getPath("src/normal.fs"), getPath("src/normal.gs"));
//cube settings for light
  float vertices[] = {
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
       -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
  };


  glm::vec3 pointLightPositions[] = {
        glm::vec3(4.0f,  3.0f,  2.0f),
        glm::vec3(2.0f, -3.0f, 2.0f),
        glm::vec3(-2.0f,  3.0f, -2.0f),
  };
  //set up mesh VAO and VBO and EBO
  Mesh mesh_object("assets/bunny.object");
  mesh_object.MeshVAO_set();

  //set up light source vao and vbo
  //unsigned int lightcubevao,lightcubevbo;
  //glgenvertexarrays(1, &lightcubevao);
  //glgenbuffers(1, &lightcubevbo);
  //glbindbuffer(gl_array_buffer, lightcubevbo);
  //glbufferdata(gl_array_buffer, sizeof(vertices), vertices, gl_static_draw);
  //glbindvertexarray(lightcubevao);
  //glvertexattribpointer(0, 3, gl_float, gl_false, 6 * sizeof(float), (void*)0);
  //glenablevertexattribarray(0);

  while (!glfwWindowShouldClose(window)) {
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    processInput(window);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
      glfwSetWindowShouldClose(window, true);

    lightingShader.use();
    lightingShader.setVec3("viewPos", cameraPos);
    lightingShader.setVec3("objectColor", glm::vec3(1.0f,1.0f,1.0f));

    lightingShader.setVec3("pointLights[0].position", pointLightPositions[0]);
    lightingShader.setVec3("pointLights[0].lightColor", glm::vec3(0.0f, 0.7f, 0.2f));
    lightingShader.setFloat("pointLights[0].constant", 1.0f);
    lightingShader.setFloat("pointLights[0].linear", 0.09);
    lightingShader.setFloat("pointLights[0].quadratic", 0.032);

    lightingShader.setVec3("pointLights[1].position", pointLightPositions[1]);
    lightingShader.setVec3("pointLights[1].lightColor", glm::vec3(0.7f, 0.7f, 0.7f));
    lightingShader.setFloat("pointLights[1].constant", 1.0f);
    lightingShader.setFloat("pointLights[1].linear", 0.09);
    lightingShader.setFloat("pointLights[1].quadratic", 0.032);

    lightingShader.setVec3("pointLights[2].position", pointLightPositions[2]);
    lightingShader.setVec3("pointLights[2].lightColor", glm::vec3(0.0f, 0.2f, 0.7f));
    lightingShader.setFloat("pointLights[2].constant", 1.0f);
    lightingShader.setFloat("pointLights[2].linear", 0.09);
    lightingShader.setFloat("pointLights[2].quadratic", 0.032);

    // view/projection transformations
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    lightingShader.setMat4("projection", projection);
    lightingShader.setMat4("view", view);
    glm::mat4 model = glm::mat4(1.0f);
    lightingShader.setMat4("model", model);

    //render object
    mesh_object.draw();

    NormalShader.use();
    NormalShader.setMat4("projection", projection);
    NormalShader.setMat4("view", view);
    NormalShader.setMat4("model", model);
    mesh_object.draw();
    ////render light sources
    //for (unsigned int i = 0; i < 3; i++)
    //{
    //    model = glm::mat4(1.0f);
    //    model = glm::translate(model, pointLightPositions[i]);
    //    model = glm::scale(model, glm::vec3(1.0f)); 
    //    lightSourceShader.setMat4("model", model);
    //    if (i == 0)
    //    {
    //        lightSourceShader.setVec3("light_color", glm::vec3(0.0f, 0.7f, 0.2f));
    //    }
    //    else if (i == 1)
    //    {
    //        lightSourceShader.setVec3("light_color", glm::vec3(0.0f, 0.5f, 0.4f));
    //    }
    //    else if (i == 2)
    //    {
    //        lightSourceShader.setVec3("light_color", glm::vec3(0.0f, 0.2f, 0.7f));
    //    }
    //    
    //    glDrawArrays(GL_TRIANGLES, 0, 36);
    //}


    glfwPollEvents();
    glfwSwapBuffers(window);
  }
  glfwTerminate();
  return 0;
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