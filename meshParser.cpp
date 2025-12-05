#include "meshParser.h"
Vector3::Vector3() {
	x = 0;
	y = 0;
	z = 0;
}
Vector3::Vector3(float inX, float inY, float inZ) {
	x = inX;
	y = inY;
	z = inZ;
}
float ReadNextNumber(std::string* stream)
{
	std::string tempString = *stream;
	if (tempString.find(' ')!=std::string::npos)
	{
		char begin = tempString.find(' ');
		if (tempString.find(' ', begin + 1)!= std::string::npos)
		{
			char end = tempString.find(' ', begin + 1);
			tempString = tempString.substr(begin + 1, end-begin+1);
		}
		else
		{
			tempString = tempString.substr(begin+1);
		}
		*stream = stream->substr(begin+1);
	}
	return std::stof(tempString);
}
unsigned int GetIndex(std::string* stream)
{
	std::string tempString = *stream;
	if (char end = tempString.find('/'))
	{
		tempString = tempString.substr(0, end);
		*stream = stream->substr(end + 1);
	}
	return std::stoi(tempString)-1;
}
IndiciesGroup ReadNextIndexGroup(std::string* stream)
{
	std::string tempString = *stream;
	if (tempString.find(' ')!= std::string::npos)
	{
		char begin = tempString.find(' ');
		if (tempString.find(' ', begin + 1)!= std::string::npos)
		{
			char end = tempString.find(' ', begin + 1);
			tempString = tempString.substr(begin + 1, end - begin+1);
		}
		else
		{
			tempString = tempString.substr(begin + 1);
		}
		*stream = stream->substr(begin + 1);
	}
	IndiciesGroup indiciesGroup;
	for (char i = 0; i < 3; i++) {
		indiciesGroup.indicies[i] = GetIndex(&tempString);
	}
	return indiciesGroup;
}
Vertex::Vertex(std::string line)
{
	for (char i = 0; i < 3; i++) {
		position[i] = ReadNextNumber(&line);
	}
}
Normal::Normal(std::string line)
{
	for (char i = 0; i < 3; i++) {
		normal[i] = ReadNextNumber(&line);
	}
}
UV::UV(std::string line)
{
	for (char i = 0; i < 2; i++) {
		uv[i] = ReadNextNumber(&line);
	}
}
Face::Face(std::string line)
{
	for (char i=0;(i < 10) && (line.find(' ')!= std::string::npos); i++) {
		indicesGroups.push_back(ReadNextIndexGroup(&line));
	}
}
Face::Face(IndiciesGroup first, IndiciesGroup second, IndiciesGroup third) {
	indicesGroups.push_back(first);
	indicesGroups.push_back(second);
	indicesGroups.push_back(third);
}
std::vector<Face> TriangulateFace(Face face) 
{
	std::vector<Face> faces;
	for (char i = 0; i < (face.indicesGroups.size() - 2); i++) {
		Face tempFace(face.indicesGroups[0], face.indicesGroups[i+1], face.indicesGroups[i + 2]);
		faces.push_back(tempFace);
	}
	return faces;
}
BatchedInfo::BatchedInfo() {
	startFace = 0;
	facesAmount = 0;
	materialIndex = 0;
	priorityIndex = 0;
	Vector3 vec0(0.f, 0.f, 0.f);
	position = vec0;
	rotation = vec0;
	scale = vec0;
	for (float f : padding) {
		f = 0;
	}
}
BatchedInfo::BatchedInfo(unsigned int sFace, unsigned int fAmount, unsigned int mIndex, unsigned int prioIndex, Vector3 pos, Vector3 rot, Vector3 s) {
	startFace = sFace;
	facesAmount = fAmount;
	materialIndex = mIndex;
	priorityIndex = prioIndex;
	position = pos;
	rotation = rot;
	scale = s;
	for (float f : padding) {
		f = 0;
	}
}
Mesh BatchMesh(std::vector<Mesh> meshes) {
	Mesh batchedMesh;
	unsigned int curFaceIndex=0;
	for (unsigned int meshIndex = 0; meshIndex < meshes.size(); meshIndex++) {
		Vector3 vec0(0.f, 0.f, 0.f);
		BatchedInfo batchedInfo(0,0,0,0,vec0,vec0,vec0);
		batchedInfo.startFace = curFaceIndex;
		for (unsigned int i=0; i < meshes[meshIndex].vertices.size(); i++) {
			batchedMesh.vertices.push_back(meshes[meshIndex].vertices[i]);
		}
		for (unsigned int i = 0; i < meshes[meshIndex].normals.size(); i++) {
			batchedMesh.normals.push_back(meshes[meshIndex].normals[i]);
		}
		for (unsigned int i = 0; i < meshes[meshIndex].uvs.size(); i++) {
			batchedMesh.uvs.push_back(meshes[meshIndex].uvs[i]);
		}
		for (unsigned int i = 0; i < meshes[meshIndex].faces.size(); i++) {
			batchedMesh.faces.push_back(meshes[meshIndex].faces[i]);
			curFaceIndex++;
		}
		batchedInfo.facesAmount = curFaceIndex-batchedInfo.startFace;
		batchedMesh.batchedInfos.push_back(batchedInfo);
	}
	return batchedMesh;
}
std::vector<Mesh> ScanForMesh(const char* meshFile)
{
	std::ifstream in(meshFile, std::ios::binary);
	std::string line;
	if (!in)
	{
		throw(errno);
	}
	std::string contents;
	std::vector<Mesh> meshes;
	int meshIndex = -1;
	for (unsigned int i = 0; std::getline(in, line); i++)
	{
			if (line[0] == 'o') 
			{
				Mesh mesh;
				meshes.push_back(mesh);
				meshIndex++;
				mesh.name = line.substr(2);
			}
			else if (line[0] == 'v' && line[1] == ' ') 
			{
				Vertex vertex(line);
				meshes[meshIndex].vertices.push_back(vertex);
			}
			else if (line[0] == 'v' && line[1] == 'n') 
			{
				Normal normal(line);
				meshes[meshIndex].normals.push_back(normal);
			}
			else if (line[0] == 'v' && line[1] == 't') 
			{
				UV uv(line);
				meshes[meshIndex].uvs.push_back(uv);
			}
			else if (line[0] == 'f') {

				Face face(line);
				std::vector<Face> tempFaces=TriangulateFace(face);
				for (char i = 0; i < tempFaces.size(); i++) {
					meshes[meshIndex].faces.push_back(tempFaces[i]);
				}
			}
		
	}
	return meshes;
	in.close();
};