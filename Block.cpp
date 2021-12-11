#include "Block.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <fstream>

glm::vec3 Block::TransformToDivisions(glm::vec3 pos) const
{
	return { std::round((pos.x / (double)x_size + 0.5f) * (x_divisions - 1)),std::round((pos.y / (double)y_size + 0.5f) * (y_divisions - 1)),pos.z };
}

void Block::SetViewPos(glm::vec3 view_pos)
{
	this->view_pos = view_pos;
}

void Block::Update()
{
	need_update = true;
}

float Block::GetHeight(int x, int y)
{

	if (x >= 0 && x < x_divisions && y >= 0 && y < y_divisions)
		return data[y * x_divisions + x];
	else
		return -100.0f;
}

void Block::SetHeight(int x, int y, float val)
{
	if (x >= 0 && x < x_divisions && y >= 0 && y < y_divisions)
		data[y * x_divisions + x] = val;
}

void Block::DrawFrame(float T, glm::vec3 start_pos, glm::vec3 end_pos, glm::vec3 rotation_start, glm::vec3 rotation_end, bool aproximation_is_line)
{
	rotate = glm::mat4(0.0f);

	this->MoveObjectTo(start_pos + (end_pos - start_pos) * T);

	glm::vec3 rotation_diff = rotation_end - rotation_start;

	if (abs(rotation_diff.x) > abs(rotation_end.x - (360.0f + rotation_start.x)))
		rotation_diff.x = rotation_end.x - (360.0f + rotation_start.x);
	if (abs(rotation_diff.y) > abs(rotation_end.y - (360.0f + rotation_start.y)))
		rotation_diff.y = rotation_end.y - (360.0f + rotation_start.y);
	if (abs(rotation_diff.z) > abs(rotation_end.z - (360.0f + rotation_start.z)))
		rotation_diff.z = rotation_end.z - (360.0f + rotation_start.z);

	if (abs(rotation_diff.x) > abs(360.0f + rotation_end.x -  rotation_start.x))
		rotation_diff.x = 360.0f + rotation_end.x - rotation_start.x;
	if (abs(rotation_diff.y) > abs(360.0f + rotation_end.y - rotation_start.y))
		rotation_diff.y = 360.0f + rotation_end.y - rotation_start.y;
	if (abs(rotation_diff.z) > abs(360.0f + rotation_end.z - rotation_start.z))
		rotation_diff.z = 360.0f + rotation_end.z - rotation_start.z;


	this->RotateObject(EulerToQuaternion(rotation_start + rotation_diff * T));

}

void Block::DrawFrame(float T, glm::vec3 start_pos, glm::vec3 end_pos, glm::quat rotation_start, glm::quat rotation_end, bool aproximation_is_line)
{
	rotate = glm::mat4(0.0f);
	this->MoveObjectTo(start_pos + (end_pos - start_pos) * T);
	if (aproximation_is_line) {
		if (glm::length(rotation_end - rotation_start) < glm::length(-rotation_end - rotation_start))
			this->RotateObject(glm::normalize(rotation_start + (rotation_end - rotation_start) * T));
		else
			this->RotateObject(glm::normalize(rotation_start + (-rotation_end - rotation_start) * T));
	}
	else {
		this->RotateObject(glm::slerp(rotation_start, rotation_end, T));
	}
}

void Block::create_block_points()
{
	std::ifstream input;

	input.open("resources\\duck\\duck.txt");

	int vn, in, kn;
	input >> vn;

	std::vector<glm::vec3> verts_tmp(vn);
	std::vector<glm::vec3> norms_tmp(vn);
	std::vector<glm::vec2> tex_tmp(vn);
	for (auto i = 0; i < vn; ++i)
	{
		input >> verts_tmp[i].x >> verts_tmp[i].y >> verts_tmp[i].z;
		input >> norms_tmp[i].x >> norms_tmp[i].y >> norms_tmp[i].z;
		input >> tex_tmp[i].x >> tex_tmp[i].y;

		points.push_back(verts_tmp[i].x/100.0f);
		points.push_back(verts_tmp[i].y/100.0f);
		points.push_back(verts_tmp[i].z/100.0f);

		points.push_back(norms_tmp[i].x);
		points.push_back(norms_tmp[i].y);
		points.push_back(norms_tmp[i].z);

		points.push_back(tex_tmp[i].x);
		points.push_back(tex_tmp[i].y);
	}

	input >> in;
	std::vector<unsigned short> inds(3 * in);
	for (auto i = 0; i < in; ++i) {
		input >> inds[3 * i] >> inds[3 * i + 1] >> inds[3 * i + 2];
		quads.push_back(inds[3 * i]);
		quads.push_back(inds[3 * i + 1]);
		quads.push_back(inds[3 * i + 2]);
	}
}

void Block::update_object()
{
	if (blocks_need_creation) {
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
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, description_number * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, description_number * sizeof(float), (void*)(6 * sizeof(float)));
		glEnableVertexAttribArray(2);

		// wood texture binding
		auto texLocation = glGetUniformLocation(shader.ID, "wood_texture");
		glUniform1i(texLocation, 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);



		blocks_need_creation = false;
	}
}

Block::Block(float x_size_, float y_size_, float z_size_, int x_divisions_, int y_divisions_, Shader sh) :
	Object(sh, 8)
{
	x_size = x_size_;
	y_size = y_size_;
	z_size = z_size_;
	x_divisions = x_divisions_;
	y_divisions = y_divisions_;

	shader = Shader("shader_tex.vert", "shader_tex.frag");
	update_object();
	shader.use();
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	// set the texture wrapping/filtering options (on the currently bound texture object)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load and generate the texture
	int width, height, nrChannels;
	unsigned char* data = stbi_load("resources/duck/ducktex.jpg", &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);
}

void Block::DrawObject(glm::mat4 mvp)
{
	if (need_update) {
		update_object();
		need_update = false;
	}

	Object::DrawObject(mvp);

	glm::mat4 model = translate * rotate * resize;
	glm::mat4 vp = mvp;
	shader.use();
	glm::mat4 trmodel = glm::transpose(glm::inverse(model));
	int projectionLoc = glGetUniformLocation(shader.ID, "model");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(model));

	int trmodelLoc = glGetUniformLocation(shader.ID, "trmodel");
	glUniformMatrix4fv(trmodelLoc, 1, GL_FALSE, glm::value_ptr(trmodel));

	int mvLoc = glGetUniformLocation(shader.ID, "vp");
	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(vp));

	int viewPos = glGetUniformLocation(shader.ID, "viewPos");
	glUniform3fv(viewPos, 1, &view_pos[0]);


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
