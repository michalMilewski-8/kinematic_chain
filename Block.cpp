#include "Block.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <fstream>

void Block::Update()
{
	need_update = true;
}

void Block::create_block_points()
{
	corners.clear();
	corners.push_back(upper_left);
	corners.push_back(glm::vec2(lower_right.x, upper_left.y));
	corners.push_back(lower_right);
	corners.push_back(glm::vec2(upper_left.x, lower_right.y));

	for (auto& corner : corners) {
		points.push_back(corner.x);
		points.push_back(corner.y);
		points.push_back(0.0f);

		points.push_back(color.x);
		points.push_back(color.y);
		points.push_back(color.z);
		points.push_back(color.w);
	}

	quads.push_back(0);
	quads.push_back(1);
	quads.push_back(2);

	quads.push_back(2);
	quads.push_back(3);
	quads.push_back(0);

}

void Block::update_object()
{
	quads.clear();
	points.clear();

	create_block_points();

	shader.use();
	// 1. bind Vertex Array Object
	glBindVertexArray(VAO);
	// 2. copy our vertices array in a vertex buffer for OpenGL to use
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * points.size(), points.data(), GL_DYNAMIC_DRAW);
	// 3. copy our index array in a element buffer for OpenGL to use
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * quads.size(), quads.data(), GL_DYNAMIC_DRAW);
	// 4. then set the vertex attributes pointers
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, description_number * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, description_number * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
}

Block::Block(glm::vec2 lup, Shader sh) :
	Object(sh, 7)
{
	upper_left = lup;
	lower_right = lup;
	color = { rand() % 10 / 10.0f,rand() % 10 / 10.0f ,rand() % 10 / 10.0f ,1.0f };
	update_object();
}

void Block::DrawObject(glm::mat4 mvp)
{
	if (need_update) {
		update_object();
		need_update = false;
	}

	Object::DrawObject(mvp);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawElements(GL_TRIANGLES, quads.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

glm::quat Block::EulerToQuaternion(glm::vec3 rot)
{
	//glm::vec3 Ox = { 1,0,0 };
	//glm::vec3 Oy = { 0,1,0 };
	//glm::vec3 Oz = { 0,0,1 };

	//rot = rot / (float)(180.0f * M_PI);

	//glm::quat quat = {0,0,0,1};
	//auto q = glm::angleAxis(rot.x, Oz);
	//Ox = q * Ox;
	//Oy = q * Oy;
	//Oz = q * Oz;
	//quat = q * quat;

	//q = glm::angleAxis(rot.y, Ox);
	//Ox = q * Ox;
	//Oy = q * Oy;
	//Oz = q * Oz;
	//quat = q * quat;

	//q = glm::angleAxis(rot.z, Oz);
	//Ox = q * Ox;
	//Oy = q * Oy;
	//Oz = q * Oz;
	//quat = q * quat;

	//return quat;

	rot = (rot / 180.0f) * (float)M_PI;
	return  glm::quat(rot);
}

glm::vec3 Block::QuaternionToEuler(glm::quat quat)
{
	return (glm::eulerAngles(quat) / (float)M_PI) * 180.0f;
}
