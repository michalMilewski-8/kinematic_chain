#include <stdio.h>
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <math.h>
#include <vector>

#include "Block.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "Dependencies/include/rapidxml-1.13/rapidxml.hpp"
#include "Dependencies/include/rapidxml-1.13/rapidxml_print.hpp"
#include "Dependencies/include/rapidxml-1.13/rapidxml_utils.hpp"
#include "./Dependencies/include/ImGuiFileDialog-Lib_Only/ImGuiFileDialog.h"

#define DEFAULT_WIDTH 1280
#define DEFAULT_HEIGHT 720
#define EPS 0.1
#define PRECISION 1.0f
using namespace rapidxml;
using namespace std;

glm::vec3 cameraPos, cameraFront, cameraUp, lookAt, moving_up;
unsigned int width_, height_;

int e = 0;
glm::mat4 projection, view, model, mvp;
glm::mat4 projection_i, view_i, model_i, mvp_i;
glm::vec2 mousePosOld, angle;
float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame
float ipd = 0.01f;
float d = 0.1f;
float near = 0.01f;
float far = 20.0f;
bool animate = false;
float T = 0.0f;
float animation_time = 1.0f;
bool show_frames = false;
int number_of_frames_shown = 15;

float distance_d = 1.0f;
float speed = 0.2f; // speed of milling tool in milimeters per second;
float size_x = 1;
float size_y = 1;
float size_z = 1;
int divisions_x = 400;
int divisions_y = 400;
glm::vec3 translation_s;
glm::vec3 translation_e;
glm::vec3 rot_euler_s;
glm::vec3 rot_euler_e;
glm::quat quaternion_s = { 1,0,0,0 };
glm::quat quaternion_e = { 1,0,0,0 };
bool is_linear_aprox = true;

Camera cam;
Shader ourShader;
GLFWwindow* window;
//std::unique_ptr<Cursor> cursor, center;

std::vector<std::shared_ptr<Object>> objects_list = {};
std::unique_ptr<Block> block;

void draw_scene();
void framebuffer_size_callback(GLFWwindow* window_1, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void create_gui();

int main() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	window = glfwCreateWindow(DEFAULT_WIDTH, DEFAULT_HEIGHT, "SimpleCAD 1", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	model = glm::mat4(1.0f);
	view = glm::mat4(1.0f);

	cameraPos = { 0,0,5 };
	cameraFront = { 0,0,-1 };
	lookAt = { 0,0,0 };
	cameraUp = { 0,1,0 };

	angle = { -90.0f, 0.0f };
	width_ = DEFAULT_WIDTH;
	height_ = DEFAULT_HEIGHT;

	cam = Camera(cameraPos, cameraFront, cameraUp);
	cam.SetPerspective(glm::radians(45.0f), DEFAULT_WIDTH / (float)DEFAULT_HEIGHT, near, far);


	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	glViewport(0, 0, DEFAULT_WIDTH, DEFAULT_HEIGHT);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	const char* glsl_version = "#version 330";
	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	// build and compile our shader program
	ourShader = Shader("shader.vs", "shader.fs"); // you can name your shader files however you like

	glEnable(GL_DEPTH_TEST);

	block = std::make_unique<Block>(size_x, size_y, size_z, divisions_x, divisions_y, ourShader);

	// render loop
	while (!glfwWindowShouldClose(window))
	{
		glfwMakeContextCurrent(window);
		// cleaning frame
		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		projection = cam.GetProjectionMatrix();
		projection_i = glm::inverse(projection);
		view = cam.GetViewMatrix();
		view_i = glm::inverse(view);

		mvp = projection * view;

		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processInput(window);
		create_gui();

		draw_scene();

		// Render dear imgui into screen
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// check and call events and swap the buffers
		glfwPollEvents();
		glfwSwapBuffers(window);

		if (animate) {
			if (T < 1.0f) {
				T += deltaTime / animation_time;
			}
		}
	}

	// cleanup stuff
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();
	objects_list.clear();
	ourShader.deleteShader();
	return 0;
}

void draw_scene() {
	block->DrawFrame(T, translation_s, translation_e, quaternion_s, quaternion_e, is_linear_aprox);
	block->DrawObject(mvp);
}

#pragma region  boilerCodeOpenGL

void framebuffer_size_callback(GLFWwindow* window_1, int width, int height)
{
	glfwMakeContextCurrent(window_1);
	glViewport(0, 0, width, height);

	cam.SetPerspective(glm::radians(45.0f), width / (float)height, near, far);
	width_ = width;
	height_ = height;
}



void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (ImGui::GetIO().WantCaptureMouse)
		return;
	glm::vec2 mousePos = { xpos,ypos };
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
	{
		glm::vec2 diff = (mousePosOld - mousePos) * PRECISION;
		float cameraSpeed = speed * deltaTime;

		glm::vec2 movement = diff * cameraSpeed;

		glm::vec3 right_movement = cam.GetRightVector() * movement.x;
		glm::vec3 up_movement = cam.GetUpVector() * -movement.y;
		cameraPos += right_movement + up_movement;
		lookAt += right_movement + up_movement;

		cam.LookAt(cameraPos, cameraFront, cameraUp);
	}
	mousePosOld = mousePos;
}
#pragma endregion

void create_gui() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	//ImGui::ShowDemoWindow();
	bool open;
	ImGuiWindowFlags flags = 0;
	//flags |= ImGuiWindowFlags_MenuBar;
	ImGui::Begin("Main Menu##uu", &open, flags);

	ImGui::InputFloat3("Start Position", (float*)&translation_s);
	ImGui::InputFloat3("End Position", (float*)&translation_e);
	if (ImGui::InputFloat3("Euler rotation start", (float*)&rot_euler_s)) quaternion_s = Block::EulerToQuaternion(rot_euler_s);
	if (ImGui::InputFloat3("Euler rotation end", (float*)&rot_euler_e)) quaternion_e = Block::EulerToQuaternion(rot_euler_e);
	if (ImGui::InputFloat4("Quaternion start", (float*)&quaternion_s)) {
		
	}
	if (ImGui::InputFloat4("Quaternion end", (float*)&quaternion_e)) {
	};

	ImGui::SliderFloat("Animation time", &animation_time, 0.1f, 100.0f, "%.3f", 1.0f);
	ImGui::Checkbox("Linear aproximation (true) or spherical (false)", &is_linear_aprox);

	ImGui::Checkbox("Show frame by frame", &show_frames);
	if (show_frames)
		ImGui::SliderInt("Number of frames shown", &number_of_frames_shown, 1, 200);

	if (ImGui::Button("Start Animation")) animate = true;
	if (ImGui::Button("Normalize quaternion")) {
		quaternion_s = glm::normalize(quaternion_s);
		quaternion_e = glm::normalize(quaternion_e);
		rot_euler_e = Block::QuaternionToEuler(quaternion_e);
		rot_euler_s = Block::QuaternionToEuler(quaternion_s);
	}


	if (ImGui::Button("Stop Animation")) animate = false;
	if (ImGui::Button("Restart Animation")) T = 0.0f;

	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::End();
}
