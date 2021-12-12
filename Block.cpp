#include "Block.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <fstream>
#include <queue>

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
	UpdateTexture({});
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

	angles_1 = { a, b };
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
		if (ImGui::RadioButton("Show first option", &option, 0)) Update();
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
	int n = 360;
	ImGui::Checkbox("show texture", &show_textures);
	if (show_textures)
	{
		ImGui::Begin("Object parametrisation##uu", &show_textures);
		ImGuiIO& io = ImGui::GetIO();
		ImTextureID my_tex_id = (void*)texture_left_ID;
		float my_tex_w = n;
		float my_tex_h = n;
		{
			ImGui::Text("%.0fx%.0f", my_tex_w, my_tex_h);
			ImVec2 pos = ImGui::GetCursorScreenPos();
			ImVec2 uv_min = ImVec2(0.0f, 0.0f);                 // Top-left
			ImVec2 uv_max = ImVec2(1.0f, 1.0f);                 // Lower-right
			ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);   // No tint
			ImVec4 border_col = ImVec4(1.0f, 1.0f, 1.0f, 0.5f); // 50% opaque white
			ImGui::Image(my_tex_id, ImVec2(my_tex_w, my_tex_h), uv_min, uv_max, tint_col, border_col);
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				float region_sz = 32.0f;
				float region_x = io.MousePos.x - pos.x - region_sz * 0.5f;
				float region_y = io.MousePos.y - pos.y - region_sz * 0.5f;
				float zoom = 4.0f;
				if (region_x < 0.0f) { region_x = 0.0f; }
				else if (region_x > my_tex_w - region_sz) { region_x = my_tex_w - region_sz; }
				if (region_y < 0.0f) { region_y = 0.0f; }
				else if (region_y > my_tex_h - region_sz) { region_y = my_tex_h - region_sz; }
				ImGui::Text("Min: (%.2f, %.2f)", region_x, region_y);
				ImGui::Text("Max: (%.2f, %.2f)", region_x + region_sz, region_y + region_sz);
				ImVec2 uv0 = ImVec2((region_x) / my_tex_w, (region_y) / my_tex_h);
				ImVec2 uv1 = ImVec2((region_x + region_sz) / my_tex_w, (region_y + region_sz) / my_tex_h);
				ImGui::Image(my_tex_id, ImVec2(region_sz * zoom, region_sz * zoom), uv0, uv1, tint_col, border_col);
				ImGui::EndTooltip();
			}
		}
		ImGui::End();
	}

	if (need_update) {
		if (show_pos == 0) {
			if (option == 0 && start_op_a.x >= 0)
				start_point = start_op_a;
			if (option == 1 && start_op_b.x >= 0)
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

bool Arm::linesIntersect(double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4) {
	// https://jvm-gaming.org/t/fastest-linesintersect-method/35387
	// Return false if either of the lines have zero length
	if (x1 == x2 && y1 == y2 ||
		x3 == x4 && y3 == y4) {
		return false;
	}
	// Fastest method, based on Franklin Antonio's "Faster Line Segment Intersection" topic "in Graphics Gems III" book (http://www.graphicsgems.org/)
	double ax = x2 - x1;
	double ay = y2 - y1;
	double bx = x3 - x4;
	double by = y3 - y4;
	double cx = x1 - x3;
	double cy = y1 - y3;

	double alphaNumerator = by * cx - bx * cy;
	double commonDenominator = ay * bx - ax * by;
	if (commonDenominator > 0) {
		if (alphaNumerator < 0 || alphaNumerator > commonDenominator) {
			return false;
		}
	}
	else if (commonDenominator < 0) {
		if (alphaNumerator > 0 || alphaNumerator < commonDenominator) {
			return false;
		}
	}
	double betaNumerator = ax * cy - ay * cx;
	if (commonDenominator > 0) {
		if (betaNumerator < 0 || betaNumerator > commonDenominator) {
			return false;
		}
	}
	else if (commonDenominator < 0) {
		if (betaNumerator > 0 || betaNumerator < commonDenominator) {
			return false;
		}
	}
	if (commonDenominator == 0) {
		// This code wasn't in Franklin Antonio's method. It was added by Keith Woodward.
		// The lines are parallel.
		// Check if they're collinear.
		double y3LessY1 = y3 - y1;
		double collinearityTestForP3 = x1 * (y2 - y3) + x2 * (y3LessY1)+x3 * (y1 - y2);	// see http://mathworld.wolfram.com/Collinear.html
		// If p3 is collinear with p1 and p2 then p4 will also be collinear, since p1-p2 is parallel with p3-p4
		if (collinearityTestForP3 == 0) {
			// The lines are collinear. Now check if they overlap.
			if (x1 >= x3 && x1 <= x4 || x1 <= x3 && x1 >= x4 ||
				x2 >= x3 && x2 <= x4 || x2 <= x3 && x2 >= x4 ||
				x3 >= x1 && x3 <= x2 || x3 <= x1 && x3 >= x2) {
				if (y1 >= y3 && y1 <= y4 || y1 <= y3 && y1 >= y4 ||
					y2 >= y3 && y2 <= y4 || y2 <= y3 && y2 >= y4 ||
					y3 >= y1 && y3 <= y2 || y3 <= y1 && y3 >= y2) {
					return true;
				}
			}
		}
		return false;
	}
	return true;
}

void Arm::UpdateTexture(const std::vector<std::shared_ptr<Block>>& constraints)
{
	int n = 360;

	float max_val = 0;

	std::vector<std::vector<int>> values_left = std::vector<std::vector<int>>(n, { std::vector<int>(n,-1) });

	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			bool constra = false;
			for (const auto& bl : constraints) {
				auto p_1 = p1({ i, j });
				auto p_2 = p2({ i, j });

				for (int z = 0; z < bl->corners.size(); z++) {
					auto poc = bl->corners[z];
					auto end = bl->corners[(z + 1) % bl->corners.size()];

					if (linesIntersect(0, 0, p_1.x, p_1.y, poc.x, poc.y, end.x, end.y)) {
						constra = true;
						break;
					}
					if (linesIntersect(p_1.x, p_1.y, p_2.x, p_2.y, poc.x, poc.y, end.x, end.y)) {
						constra = true;
						break;
					}
				}
				if (constra)
					values_left[i][j] = -5;
				else
					values_left[i][j] = -1;
			}
		}
	}

	flood_fill(values_left);

	glm::ivec2 current_pt = { std::roundf(end_point.x),std::roundf(end_point.y) };
	glm::ivec2 start_pt = { std::roundf(start_point.x),std::roundf(start_point.y) };
	int point = values_left[current_pt.x % 360][current_pt.y % 360];

	if (point < 0 || values_left[start_pt.x % 360][start_pt.y % 360] < 0) {
		can_not_draw = true;
	}

	for (const auto& val : values_left) {
		for (const auto& val2 : val) {
			if (val2 > max_val) max_val = val2;
		}
	}

	path = {};
	path.push_back(current_pt);

	while (point > 0 && !can_not_draw) {
		point--;
		std::cout << "x: " << current_pt.x << " y: " << current_pt.y << " values: " << values_left.size()<< std::endl;
		if (values_left[(current_pt.x + 1) % n][current_pt.y] == point) {
			std::cout << "1 x: " << current_pt.x << " y: " << current_pt.y << std::endl;
			current_pt = { (current_pt.x + 1) % n,current_pt.y };
		}
		else if (values_left[current_pt.x - 1 < 0 ? n - 1 : current_pt.x - 1][current_pt.y] == point) {
			std::cout << "2 x: " << current_pt.x << " y: " << current_pt.y << std::endl;
			current_pt = { current_pt.x - 1 < 0 ? n - 1 : current_pt.x - 1,current_pt.y };
		}
		else if (values_left[current_pt.x][(current_pt.y + 1) % n] == point) {
			std::cout << "3 x: " << current_pt.x << " y: " << current_pt.y << std::endl;
			current_pt = { current_pt.x,(current_pt.y + 1) % n };
		}
		else if (values_left[current_pt.x][current_pt.y - 1 < 0 ? n - 1 : current_pt.y - 1] == point) {
			std::cout << "4 x: " << current_pt.x << " y: " << current_pt.y << std::endl;
			current_pt = { current_pt.x,current_pt.y - 1 < 0 ? n - 1 : current_pt.y - 1 };
		}
		else
			can_not_draw = true;
		std::cout << "end x: " << current_pt.x << " y: " << current_pt.y << std::endl;
		path.push_back(current_pt);
	}

	for (const auto& cp : path) {
		values_left[cp.x][cp.y] = -4;
	}

	std::reverse(path.begin(), path.end());

	glGenTextures(1, &texture_left_ID);
	glBindTexture(GL_TEXTURE_2D, texture_left_ID);
	// set the texture wrapping/filtering options (on the currently bound texture object)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	// load and generate the texture
	std::vector<unsigned char>data = {};
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			if (values_left[i][j] >= 0) {
				data.push_back(255 * values_left[i][j] / max_val);
				data.push_back(255 * values_left[i][j] / max_val);
				data.push_back(255 * values_left[i][j] / max_val);
			}
			else if (values_left[i][j] == -5) {
				data.push_back(255);
				data.push_back(0);
				data.push_back(0);
			}
			else if (values_left[i][j] == -2) {
				data.push_back(0);
				data.push_back(255);
				data.push_back(0);
			}
			else if (values_left[i][j] == -3) {
				data.push_back(0);
				data.push_back(0);
				data.push_back(255);
			}
			else if (values_left[i][j] == -4) {
				data.push_back(255);
				data.push_back(128);
				data.push_back(0);
			}
			else {
				data.push_back(0);
				data.push_back(0);
				data.push_back(0);
			}
		}
	}
	if (data.size() > 0)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, n, n, 0, GL_RGB, GL_UNSIGNED_BYTE, data.data());
		glGenerateMipmap(GL_TEXTURE_2D);
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

	if (T_ >= 0.0f && path.size() > 0) {
		int pp = ((int)std::roundf((path.size() - 1) * std::min(T_, 1.0f))) % path.size();
		angles = path[pp];
	}

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

void Arm::flood_fill(std::vector<std::vector<int>>& data)
{
	std::queue<glm::ivec3> to_fill;
	to_fill.push({ std::roundf(start_point.x),std::roundf(start_point.y),0 });

	glm::ivec3 current;
	while (!to_fill.empty()) {
		current = to_fill.front();
		to_fill.pop();
		if (data[current.x][current.y] == -5)
			continue;
		if (data[current.x][current.y] >= 0 && data[current.x][current.y] <= current.z)
			continue;
		data[current.x][current.y] = current.z;

		to_fill.push({ (current.x + 1) % 360,current.y,current.z + 1 });
		to_fill.push({ current.x - 1 < 0 ? 360 - 1 : current.x - 1,current.y,current.z + 1 });
		to_fill.push({ current.x,(current.y + 1) % 360,current.z + 1 });
		to_fill.push({ current.x,current.y - 1 < 0 ? 360 - 1 : current.y - 1,current.z + 1 });
	}
}
