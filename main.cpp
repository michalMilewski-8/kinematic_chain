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

#define DEFAULT_WIDTH 1280
#define DEFAULT_HEIGHT 720
#define EPS 0.1
#define PRECISION 1.0f

using namespace std;

glm::vec3 cameraPos, cameraFront, cameraUp, lookAt, moving_up;
unsigned int width_, height_;
float aspect_;

int e = 0;
glm::mat4 projection, view, model, mvp;
glm::mat4 projection_i, view_i, model_i, mvp_i;
glm::vec2 mousePosOld, angle;
float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame
bool animate = false;
bool editing_state = true;
bool add_boxes = true;
bool edit_arm_pos = false;
int box_index = -1;
float T = 0.0f;
float animation_time = 1.0f;
float l1 = 0.3f;
float l2 = 0.2f;


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

std::vector<std::shared_ptr<Block>> objects_list = {};
std::shared_ptr<Arm> arm = {};

void draw_scene();
void framebuffer_size_callback(GLFWwindow* window_1, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
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

	cameraPos = { 0,0,1 };
	cameraFront = { 0,0,-1 };
	cameraUp = { 0,1,0 };

	width_ = DEFAULT_WIDTH;
	height_ = DEFAULT_HEIGHT;

	cam = Camera(cameraPos, cameraFront, cameraUp);
	aspect_ = DEFAULT_WIDTH / (float)DEFAULT_HEIGHT;
	cam.SetPerspective(glm::radians(45.0f), aspect_, 1, 1);


	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	glViewport(0, 0, DEFAULT_WIDTH, DEFAULT_HEIGHT);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

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
	ourShader = Shader("shader.vert", "shader.frag"); // you can name your shader files however you like

	glEnable(GL_DEPTH_TEST);

	arm = std::make_shared<Arm>(ourShader);
	// render loop
	while (!glfwWindowShouldClose(window))
	{
		glfwMakeContextCurrent(window);
		// cleaning frame
		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processInput(window);
		create_gui();

		projection = cam.GetProjectionMatrix();
		projection_i = glm::inverse(projection);
		view = cam.GetViewMatrix();
		view_i = glm::inverse(view);

		mvp = projection * view;

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
	for (auto& obj : objects_list) {
		obj->DrawObject(mvp);
	}
	if (animate) {
		arm->SetT(T);
	}
	arm->DrawObject(mvp);
}

#pragma region  boilerCodeOpenGL

void framebuffer_size_callback(GLFWwindow* window_1, int width, int height)
{
	glfwMakeContextCurrent(window_1);
	glViewport(0, 0, width, height);

	aspect_ = width / (float)height;
	cam.SetPerspective(glm::radians(45.0f), aspect_, 1, 1);
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
	if (!editing_state || !add_boxes)
		return;
	if (box_index >= 0) {
		glm::vec2 mousePos = glm::vec2(((xpos / width_) * 2.0f - 1.0f) * aspect_, -((ypos / height_) * 2.0f - 1.0f));
		objects_list[box_index]->SetLowerRight(mousePos);
	}
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	if (ImGui::GetIO().WantCaptureMouse)
		return;
	if (!editing_state)
		return;

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		double x_pos, y_pos;
		glfwGetCursorPos(window, &x_pos, &y_pos);
		glm::vec2 mousePos = glm::vec2(((x_pos / width_) * 2.0f - 1.0f) * aspect_, -((y_pos / height_) * 2.0f - 1.0f));
		if (add_boxes) {
			if (box_index >= 0)
				box_index = -1;
			else {
				objects_list.push_back(std::make_shared<Block>(mousePos, ourShader));
				box_index = objects_list.size() - 1;
			}
		}
		else if (edit_arm_pos) {
			arm->SetPoint(mousePos);
		}
	}
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


	ImGui::SliderFloat("Animation time", &animation_time, 0.1f, 100.0f, "%.3f", 1.0f);

	arm->Menu();

	if (ImGui::Checkbox("editing state", &editing_state)) {
		if (editing_state)
			arm->SetT(-1.0f);
		else
			arm->SetT(0.0f);
	}
	ImGui::Checkbox("add new constraint boxes", &add_boxes);
	ImGui::Checkbox("edit arm position", &edit_arm_pos);

	if (ImGui::Button("Update texture")) arm->UpdateTexture(objects_list);

	int to_delete = -1;
	if (ImGui::CollapsingHeader("Objects Present on Scene")) {
		int i = 0;
		for (auto& ob : objects_list) {
			ob->CreateMenu();
			ImGui::SameLine();
			if (ImGui::Button(("Delete##" + std::to_string(i)).c_str())) {
				to_delete = i;
			}
			i++;
		}
		if (to_delete > -1) {
			objects_list.erase(objects_list.begin() + to_delete);
			to_delete = -1;
			std::for_each(objects_list.begin(), objects_list.end(), [](std::shared_ptr<Object> obj) {obj->Update(); });
		}
	}

	if (ImGui::Button("Start Animation")) {
		animate = true;
		editing_state = false;
	}

	if (ImGui::Button("Stop Animation")) {
		animate = false;
	}
	if (ImGui::Button("Restart Animation")) {
		arm->SetT(0.0f);
		T = 0.0f;
	}

	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::End();
}
