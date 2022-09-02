#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>

const int WIDTH = 1920;
const int HEIGHT = 1080;

GLFWwindow *window;

bool init();

const char* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec3 inColor;\n"
"out vec3 outColor;\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
"   outColor = inColor;\n"
"}\0";
const char* fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"in vec3 outColor;\n"
"void main()\n"
"{\n"
"   FragColor = vec4(outColor, 1.0f);\n"
"}\n\0";


int main() {
  if (!init()) {
    glfwTerminate();
    return -1;
  }

  // vertex shader
  unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
  glCompileShader(vertexShader);

  // fragment shader
  unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
  glCompileShader(fragmentShader);

  // link shaders
  unsigned int shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  float vertices[] = {
   -0.25f, -0.25f, 0.0f, 0.25f, 0.25f, 0.75f,
    -0.25f, 0.25f, 0.0f, 0.25f, 0.75f, 0.25f,
    0.25f, -0.25f, 0.0f, 0.75f, 0.25f, 0.25f,
    0.25f, -0.25f, 0.0f, 0.75f, 0.25f, 0.25f,
    0.25f, 0.25f, 0.0f,  0.25f, 0.25f, 0.75f,
    -0.25f, 0.25f, 0.0f, 0.25f, 0.75f, 0.25f,
  };
  unsigned int VBO, VAO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  
  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);


  while (!glfwWindowShouldClose(window)) {
    glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
      glfwSetWindowShouldClose(window, true);

    // TODO: add your render code here
    glUseProgram(shaderProgram);
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glfwPollEvents();
    glfwSwapBuffers(window);
  }

  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
  glDeleteProgram(shaderProgram);

  glfwTerminate();
  return 0;
}

bool init() {
  // init glfw
  if (!glfwInit()) {
    std::cerr << "Failed to init glfw" << std::endl;
    return false;
  }
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#else
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
#endif
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

  // create window
  window = glfwCreateWindow(WIDTH, HEIGHT, "CS171 hw0", nullptr, nullptr);
  if (!window) {
    std::cerr << "failed to create window" << std::endl;
    return false;
  }

  glfwMakeContextCurrent(window);
  // enable vsync
  glfwSwapInterval(1);
  // init glew
  if (glewInit() != GLEW_OK) {
    std::cerr << "failed to init glew" << std::endl;
    return false;
  }
  return true;
}