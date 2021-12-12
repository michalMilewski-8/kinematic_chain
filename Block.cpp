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

Arm::Arm(Shader sh) :
	Object(sh, 7)
{
	start_point = { 0,0 };
	color = { 1, 0.05f, 0, 1.0f };
	end_point = start_point;
	update_object();
}

void Arm::DrawObject(glm::mat4 mvp)
{
	if (need_update) {
		update_object();
		need_update = false;
	}

	Object::DrawObject(mvp);

	glDrawElements(GL_LINES, quads.size(), GL_UNSIGNED_INT, 0);
	glPointSize(5);
	glDrawElements(GL_POINTS, quads.size(), GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);
}

glm::quat Arm::EulerToQuaternion(glm::vec3 rot)
{
	rot = (rot / 180.0f) * (float)M_PI;
	return  glm::quat(rot);
}

glm::vec3 Arm::QuaternionToEuler(glm::quat quat)
{
	return (glm::eulerAngles(quat) / (float)M_PI) * 180.0f;
}

void Arm::SetPoint(glm::vec2 lr)
{
	glm::vec2 angles_1 = { -1,-1 }, angles_2 = { -1,-1 };

	float d = glm::length(lr);

	if (d > L1 + L2 || d < L1 - L2) {
		can_not_draw = true;
		return;
	}

	float b = glm::degrees(std::acos((-d * d + L1 * L1 + L2 * L2) / (2 * L1 * L2)));
	float b2 = 360 - b;
	b = 180 - b;
	b2 = 180 - b2;
	
	float a = glm::degrees(std::acos((L1 * L1 + d * d - L2 * L2) / (2 * d * L1)));
	float a2 = 360 - a;

	float angle = glm::degrees(atan(lr.y / lr.x));
	if (lr.x < 0) angle += 180;
	a = angle - a;
	a2 = angle - a2;

	a = a < 0 ? 360 + a : a;
	a2 = a2 < 0 ? 360 + a2 : a2;

	b = b < 0 ? 360 + b : b;
	b2 = b2 < 0 ? 360 + b2 : b2;

	angles_1 = {  a, b };
	angles_2 = { a2, b2 };

	if (show_pos == 0) {
		start_op_a = angles_1;
		start_op_b = angles_2;
		start_point = angles_1;
	}
	else {
		end_op_a = angles_1;
		end_op_b = angles_2;
		end_point = angles_1;
	}
	option = 0;
	Update();
}

void Arm::SetFinalPos()
{
	if (show_pos == 0) {
		start_op_a = { -1,-1 };
		start_op_b = { -1,-1 };
	}
	else if (show_pos == 1) {
		end_op_a = { -1,-1 };
		end_op_b = { -1,-1 };
	}
}

void Arm::Menu()
{
	ImGui::InputFloat("L1", &L1);
	ImGui::InputFloat("L2", &L2);

	if (ImGui::RadioButton("Show start position", &show_pos, 0)) Update();
		if (ImGui::RadioButton("Show end position", &show_pos, 1)) Update();

	if ((show_pos == 0 && start_op_a.x >= 0) || (show_pos == 1 && end_op_a.x >= 0))
		if(ImGui::RadioButton("Show first option", &option, 0)) Update();
	if ((show_pos == 0 && start_op_b.x >= 0) || (show_pos == 1 && end_op_b.x >= 0))
		if (ImGui::RadioButton("Show second option", &option, 1)) Update();

	if (ImGui::Button("Set Position as final")) SetFinalPos();

	if (can_not_draw)
		ImGui::OpenPopup("my_select_popup");

	if (ImGui::BeginPopup("my_select_popup"))
	{
		ImGui::Text("Selected point is outside of current arm configuration");
		ImGui::Separator();
		if (ImGui::Button("Close")) {
			can_not_draw = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	if (need_update) {
		if (show_pos == 0) {
			if (option == 0 && start_op_a.x >= 0)
				start_point = start_op_a;
			if(option == 1 && start_op_b.x >= 0)
				start_point = start_op_b;
		}
		else {
			if (option == 0 && end_op_a.x >= 0)
				end_point = end_op_a;
			if (option == 1 && end_op_b.x >= 0)
				end_point = end_op_b;
		}
	}
}

void Arm::create_block_points()
{
	points.push_back(0.0f);
	points.push_back(0.0f);
	points.push_back(0.0f);

	points.push_back(color.x);
	points.push_back(color.y);
	points.push_back(color.z);
	points.push_back(color.w);

	glm::vec2 angles;
	if (show_pos == 0)
		angles = start_point;
	else
		angles = end_point;

	glm::vec2 pt1 = p1(angles);
	glm::vec2 pt2 = p2(angles);

	points.push_back(pt1.x);
	points.push_back(pt1.y);
	points.push_back(0.0f);

	points.push_back(color.x);
	points.push_back(color.y);
	points.push_back(color.z);
	points.push_back(color.w);

	points.push_back(pt2.x);
	points.push_back(pt2.y);
	points.push_back(0.0f);

	points.push_back(color.x);
	points.push_back(color.y);
	points.push_back(color.z);
	points.push_back(color.w);

	quads.push_back(0);
	quads.push_back(1);
	quads.push_back(1);
	quads.push_back(2);
}

void Arm::update_object()
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

glm::vec2 Arm::p1(glm::vec2 angles)
{
	return { glm::cos(glm::radians(angles.x)) * L1 , glm::sin(glm::radians(angles.x)) * L1 };
}

glm::vec2 Arm::p2(glm::vec2 angles)
{
	return p1(angles) + glm::vec2{ glm::cos(glm::radians(angles.x + angles.y)) * L2 , glm::sin(glm::radians(angles.x + angles.y)) * L2 };
}
