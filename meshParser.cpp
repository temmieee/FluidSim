#include "meshParser.h"
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
	return std::stoi(tempString);
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
std::vector<Face> TriangulateFace(std::string line) 
{
	std::vector<Face> faces;
	return faces;
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
	Mesh mesh;
	for (unsigned int i = 0; std::getline(in, line); i++)
	{
			if (line[0] == 'o') 
			{
				mesh.name = line.substr(2);
			}
			else if (line[0] == 'v' && line[1] == ' ') 
			{
				Vertex vertex(line);
				mesh.vertices.push_back(vertex);
			}
			else if (line[0] == 'v' && line[1] == 'n') 
			{
				Normal normal(line);
				mesh.normals.push_back(normal);
			}
			else if (line[0] == 'v' && line[1] == 't') 
			{
				UV uv(line);
				mesh.uvs.push_back(uv);
			}
			else if (line[0] == 'f') {

				Face face(line);
				mesh.faces.push_back(face);
			}
		
	}
	meshes.push_back(mesh);
	return meshes;
	in.close();
};