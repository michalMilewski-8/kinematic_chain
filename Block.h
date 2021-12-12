#pragma once
#include "Object.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <math.h>
#define _USE_MATH_DEFINES
#include <cmath>

class Block :
	public Object
{
public:
	Block(glm::vec2 lup, Shader sh);

	void DrawObject(glm::mat4 mvp) override;

	void Update() override;
	void SetLowerRight(glm::vec2 lr) { lower_right = lr; Update(); }

	std::vector<glm::vec2> corners;

private:
	void create_block_points();
	void update_object() override;

	glm::vec2 upper_left;
	glm::vec2 lower_right;



	bool blocks_need_creation = true;

	std::vector<float> points;
	std::vector<unsigned int> quads;
};

class Arm :
	public Object
{
public:
	Arm(Shader sh);

	void DrawObject(glm::mat4 mvp) override;

	static glm::quat EulerToQuaternion(glm::vec3 rot);
	static glm::vec3 QuaternionToEuler(glm::quat quat);

	void Update() override { need_update = true; };
	void SetPoint(glm::vec2 lr);
	void SetFinalPos();
	void Menu();
	void SetT(float T) { T_ = T; Update(); };

	void UpdateTexture(const std::vector <std::shared_ptr<Block>>& constraints);

private:
	void create_block_points();
	void update_object() override;
	bool linesIntersect(double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4);
	void flood_fill(std::vector<std::vector<int>>& data);
	glm::vec2 p1(glm::vec2 angles);
	glm::vec2 p2(glm::vec2 angles);

	glm::vec2 start_point;
	glm::vec2 start_op_a;
	glm::vec2 start_op_b;

	glm::vec2 end_point;
	glm::vec2 end_op_a;
	glm::vec2 end_op_b;

	std::vector<glm::vec2> path;

	int show_pos = 0;
	int option = 0;
	bool show_textures = false;

	float L1 = 0.5f, L2 = 0.5f;
	float T_ = -1;

	bool blocks_need_creation = true;
	bool can_not_draw = false;

	unsigned int texture_left_ID;

	std::vector<float> points;
	std::vector<unsigned int> quads;
};
