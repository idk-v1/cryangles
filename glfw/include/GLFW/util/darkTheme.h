#pragma once
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#define GLFW_THEME_AUTO -1
#define GLFW_THEME_LIGHT 0
#define GLFW_THEME_DARK 1

void glfwSetWindowTheme(GLFWwindow* window, int theme);

int glfwGetSystemTheme();