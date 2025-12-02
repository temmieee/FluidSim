#ifndef MESH_PARSER_H
#define MESH_PARSER_H
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cerrno>
#include <vector>
struct Vertex {
	float position[3];
	Vertex(std::string line);
};
struct Normal {
	float normal[3];
	Normal(std::string line);
};
struct UV {
	float uv[2];
	UV(std::string line);
};
struct IndiciesGroup {
	//position,normal,uv
	int indicies[3];
};
struct Face {
	std::vector<IndiciesGroup> indicesGroups;
	Face(std::string line);

};
std::vector<Face> TriangulateFace(std::string line);
struct Mesh {
public:
	std::string name;
	std::string meshData;
	std::vector<Vertex> vertices;
	std::vector<Normal> normals;
	std::vector<UV> uvs;
	std::vector<Face> faces;
};
std::vector<Mesh> ScanForMesh(const char* meshFile);
#endif