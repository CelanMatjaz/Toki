#define GLFW_INCLUDE_NONE

#include <GLFW/glfw3.h>
#include <unistd.h>

#include <print>

#include "include/glad/glad.h"

int main(){
  GLFWwindow* window;

  if(!glfwInit()) return -1;


  glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);
  glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
  glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
  window = glfwCreateWindow(640, 480, "Test", NULL, NULL);
  if(!window) {
    glfwTerminate();
    return -1;
  } 

  glfwMakeContextCurrent(window);

  gladLoadGL();
  printf("OpenGL version: %s\n", glGetString(GL_VERSION));


  while(!glfwWindowShouldClose(window)) {
    glClear(GL_COLOR_BUFFER_BIT);

    glfwSwapBuffers(window);

    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
