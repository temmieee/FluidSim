#ifndef MESH_PARSER_H
#define MESH_PARSER_H

#include"glad/glad.h"
#include"GLFW/glfw3.h"
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cerrno>
#include <vector>
struct Vector3
{
	GLfloat x, y, z;
	Vector3();
	Vector3(float inX, float inY, float inZ);
};

struct Vector4
{
	GLfloat x, y, z, w;
	Vector4();
	Vector4(float inX, float inY, float inZ, float inW);
};
struct Material {
	Vector4 color;
	float roughness;
	float metallic;
	float emissive;
	float refractiveIndex;
	Material();
	Material(Vector4 inCol, float inRoughness, float inMetallic, float inEmissive, float inRefractiveIndex);
};
struct Vertex {
	float position[3];
	float padding=1.0f;
	Vertex(std::string line);
};
struct Normal {
	float normal[3];
	float padding = 1.0f;
	Normal(std::string line);
};
struct UV {
	float uv[2];
	UV(std::string line);
};
struct IndiciesGroup {
	//position,normal,uv
	int indicies[3];
	int objectIndex=0;
};
struct Face {
	std::vector<IndiciesGroup> indicesGroups;
	Face(IndiciesGroup first, IndiciesGroup second, IndiciesGroup third);
	Face(std::string line);

};
struct BatchedInfo {
	float position[3];
	unsigned int startFace;
	float rotation[3];
	unsigned int facesAmount;
	float scale[3];
	int materialIndex;
	float padding[3];
	unsigned int priorityIndex;
	BatchedInfo();
	BatchedInfo(unsigned int sFace, unsigned int fAmount, int mIndex, unsigned int prioIndex, float pos[], float rot[], float s[]);
};
struct Mesh {
public:
	std::string name;
	std::vector<Vertex> vertices;
	std::vector<Normal> normals;
	std::vector<UV> uvs;
	std::vector<Face> faces;
	std::vector<BatchedInfo> batchedInfos;
};
Mesh BatchMesh(std::vector<Mesh> meshes);
std::vector<Mesh> ScanForMesh(const char* meshFile);
float ReadNextNumber(std::string* stream);
unsigned int GetIndex(std::string* stream);
IndiciesGroup ReadNextIndexGroup(std::string* stream);
std::vector<Face> TriangulateFace(Face face);
#endif