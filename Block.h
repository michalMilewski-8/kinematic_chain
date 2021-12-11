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

	static glm::quat EulerToQuaternion(glm::vec3 rot);
	static glm::vec3 QuaternionToEuler(glm::quat quat);

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
