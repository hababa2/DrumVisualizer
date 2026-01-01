#include "Window.hpp"

#include "GraphicsInclude.hpp"

void Window::Create(WindowConfig config, Window* share)
{
	this->config = config;

	glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, config.transparent);
	glfwWindowHint(GLFW_DECORATED, config.menu);
	glfwWindowHint(GLFW_MOUSE_PASSTHROUGH, !config.interactable);
	glfwWindowHint(GLFW_FLOATING, config.floating);

	glfwWindowHint(GLFW_POSITION_X, config.x);
	glfwWindowHint(GLFW_POSITION_Y, config.y);

	window = glfwCreateWindow(config.width, config.height, config.name.c_str(), nullptr, share ? share->window : nullptr);

	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
}

void Window::Destroy()
{
	glfwDestroyWindow(window);
}

void Window::Update()
{
	glfwGetWindowPos(window, &config.x, &config.y);
	glfwGetWindowSize(window, &config.width, &config.height);

	glfwMakeContextCurrent(window);
	glViewport(0, 0, config.width, config.height);
	glScissor(0, 0, config.width, config.height);
	glClearColor(config.clearColor.x, config.clearColor.y, config.clearColor.z, config.clearColor.w);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Window::Render()
{
	glfwSwapBuffers(window);
}

void Window::SetMenu(bool b)
{
	config.menu = b;
	glfwSetWindowAttrib(window, GLFW_DECORATED, b);
}

void Window::SetTransparent(bool b)
{
	config.transparent = b;
	glfwSetWindowAttrib(window, GLFW_TRANSPARENT_FRAMEBUFFER, b);
}

void Window::SetInteractable(bool b)
{
	config.interactable = b;
	glfwSetWindowAttrib(window, GLFW_MOUSE_PASSTHROUGH, !b);
}

void Window::SetFloating(bool b)
{
	config.floating = b;
	glfwSetWindowAttrib(window, GLFW_FLOATING, b);
}

const WindowConfig& Window::Config() const
{
	return config;
}

Window::operator bool() const
{
	return window;
}

Window::operator GLFWwindow* () const
{
	return window;
}