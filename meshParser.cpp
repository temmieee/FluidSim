#include "meshParser.h"
const int triangle = 2;
int total[3] = { 0,0,0 };
Vector3::Vector3() {
	x = 0;
	y = 0;
	z = 0;
}
Vector4::Vector4() {
	x = 0;
	y = 0;
	z = 0;
	w = 0;
}
Vector3::Vector3(float inX, float inY, float inZ) {
	x = inX;
	y = inY;
	z = inZ;
}
Vector4::Vector4(float inX, float inY, float inZ, float inW) {
	x = inX;
	y = inY;
	z = inZ;
	w = inW;
}
Material::Material() {
	Vector4 vec4;
	color = vec4;
	roughness = 1;
	metallic = 1;
	emissive = 0;
	refractiveIndex = 1;
}
Material::Material(Vector4 inCol, float inRoughness, float inMetallic, float inEmissive, float inRefractiveIndex) {
	color = inCol;
	roughness = inRoughness;
	metallic = inMetallic;
	emissive = inEmissive;
	refractiveIndex = inRefractiveIndex;
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
	if (tempString.find('/') != std::string::npos)
	{
		char end = tempString.find('/');
		tempString = tempString.substr(0, end);
		*stream = stream->substr(end + 1);
	}
	if (!tempString.empty()) {
		return std::stoi(tempString) - 1;
	}
	return 0;
}
IndicesGroup ReadNextIndexGroup(std::string* stream)
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
	IndicesGroup indicesGroup;
	for (char i = 0; i < 3; i++) {
		indicesGroup.indices[i] = GetIndex(&tempString);
	}
	return indicesGroup;
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
Face::Face(IndicesGroup first, IndicesGroup second, IndicesGroup third) {
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
	bvhIndex = 0;
	for (char i = 0; i < 3; i++) {
		position[i] = 0;
		rotation[i] = 0;
		scale[i] = 0;
		if (i < 2) padding[i] = 0;
	}
}
BatchedInfo::BatchedInfo(unsigned int sFace, unsigned int fAmount, int mIndex, unsigned int prioIndex, unsigned int bIndex, float pos[], float rot[], float s[]) {
	startFace = sFace;
	facesAmount = fAmount;
	materialIndex = mIndex;
	priorityIndex = prioIndex;
	bvhIndex =bIndex;
	for (char i = 0; i < 3; i++) {
		position[i] = pos[i];
		rotation[i] = rot[i];
		scale[i] = s[i];
		if (i<2) padding[i] = 0;
	}

}
BVHnode::BVHnode() {
	for (char i = 0; i++; i < 3) {
		maxBound[i] = 0;
		minBound[i] = 0;
		index = 0;
		amount = 0;
	}
}
BVHnode::BVHnode(float inMax[], float inMin[], unsigned int inIndex, unsigned int inAmount) {
	for (char i = 0; i < 3; i++) {
		maxBound[i] = inMax[i];
		minBound[i] = inMin[i];
	}
	index = inIndex;
	amount = inAmount;
}
void ConstructChildBVH(Mesh* inMesh, unsigned int firstFace, unsigned int facesAmount) {
	float tempMaxBound[3] = { -INFINITY,-INFINITY,-INFINITY };
	float tempMinBound[3] = { INFINITY,INFINITY,INFINITY };
	for (unsigned int faceIndex = firstFace; faceIndex < firstFace + facesAmount; faceIndex++) {
		for (char dimension = 0; dimension < 3; dimension++) {
			tempMaxBound[dimension] = tempMaxBound[dimension] > inMesh->faces[faceIndex].maxPosition[dimension] ? tempMaxBound[dimension] : inMesh->faces[faceIndex].maxPosition[dimension];
			tempMinBound[dimension] = tempMinBound[dimension] < inMesh->faces[faceIndex].minPosition[dimension] ? tempMinBound[dimension] : inMesh->faces[faceIndex].minPosition[dimension];
		}


	}

	int childrenFaces = facesAmount > triangle ? 0 : facesAmount;
	BVHnode bvhNode(tempMaxBound, tempMinBound, firstFace, childrenFaces);
	inMesh->bvh.push_back(bvhNode);
}
void ConstructBVH(Mesh* inMesh, unsigned int parent, unsigned int firstFace, unsigned int facesAmount) {
	float inMaxBound[3];
	float inMinBound[3];
	for (char i = 0; i < 3; i++) {
		inMaxBound[i] = inMesh->bvh[parent].maxBound[i];
		inMinBound[i] = inMesh->bvh[parent].minBound[i];
	}
	//split
	
	float diagonalVector[3];
	char bestDimension;
	char division = 20;
	float lowestCost = INFINITY;
	char bestCostIndex=-1;
	for (char dimensionPartition = 0; dimensionPartition < 3; dimensionPartition++) {
		for (char iterateSAH = 0; iterateSAH < division; iterateSAH++) {
			float mid = inMinBound[dimensionPartition] + (iterateSAH + 1) * (inMaxBound[dimensionPartition] - inMinBound[dimensionPartition]) / (division + 1);
			unsigned int midIndex = firstFace;
			bool second = false;
			//sort to 2 halves
			for (unsigned int faceIndex = firstFace; faceIndex < firstFace + facesAmount; faceIndex++) {
				if (inMesh->faces[faceIndex].averagePosition[dimensionPartition] < mid)
				{
					std::swap(inMesh->faces[faceIndex], inMesh->faces[midIndex]);
					midIndex++;
				}
			}
			//calculate cost
			float currCost;
			float tempMaxBoundFirst[3] = { -INFINITY ,-INFINITY ,-INFINITY };
			float tempMinBoundFirst[3] = { INFINITY ,INFINITY ,INFINITY };
			float tempMaxBoundSecond[3] = { -INFINITY ,-INFINITY ,-INFINITY };
			float tempMinBoundSecond[3] = { INFINITY ,INFINITY ,INFINITY };
			for (unsigned int i = firstFace; i < midIndex; i++) {
				for (char dimension = 0; dimension < 3; dimension++) {
					tempMaxBoundFirst[dimension] = tempMaxBoundFirst[dimension] > inMesh->faces[i].maxPosition[dimension] ? tempMaxBoundFirst[dimension] : inMesh->faces[i].maxPosition[dimension];
					tempMinBoundFirst[dimension] = tempMinBoundFirst[dimension] < inMesh->faces[i].minPosition[dimension] ? tempMinBoundFirst[dimension] : inMesh->faces[i].minPosition[dimension];
				}
			}
			for (unsigned int i = midIndex; i < firstFace + facesAmount; i++) {
				for (char dimension = 0; dimension < 3; dimension++) {
					tempMaxBoundSecond[dimension] = tempMaxBoundSecond[dimension] > inMesh->faces[i].maxPosition[dimension] ? tempMaxBoundSecond[dimension] : inMesh->faces[i].maxPosition[dimension];
					tempMinBoundSecond[dimension] = tempMinBoundSecond[dimension] < inMesh->faces[i].minPosition[dimension] ? tempMinBoundSecond[dimension] : inMesh->faces[i].minPosition[dimension];
				}
			}
			float firstDiagonalVector[3];
			float secondDiagonalVector[3];
			for (char dimension = 0; dimension < 3; dimension++) {
				firstDiagonalVector[dimension] = tempMaxBoundFirst[dimension] - tempMinBoundFirst[dimension];
				secondDiagonalVector[dimension] = tempMaxBoundSecond[dimension] - tempMinBoundSecond[dimension];
			}
			currCost = (firstDiagonalVector[0] * firstDiagonalVector[1] + firstDiagonalVector[1] * firstDiagonalVector[2] + firstDiagonalVector[2] * firstDiagonalVector[0]) * (midIndex - firstFace)
				+ (secondDiagonalVector[0] * secondDiagonalVector[1] + secondDiagonalVector[1] * secondDiagonalVector[2] + secondDiagonalVector[2] * secondDiagonalVector[0]) * (facesAmount - midIndex + firstFace);
			if (currCost < lowestCost) {
				bestCostIndex = iterateSAH;
				bestDimension = dimensionPartition;
				lowestCost = currCost;
			}
		}

	}
	total[bestDimension]++;
	//construct actual children BVH
	float mid = inMinBound[bestDimension] + (bestCostIndex + 1) * (inMaxBound[bestDimension] - inMinBound[bestDimension]) / (division + 1);
 	unsigned int actualMidIndex = firstFace;
	//sort to 2 halves
	for (unsigned int faceIndex = firstFace; faceIndex < firstFace + facesAmount; faceIndex++) {
		if (inMesh->faces[faceIndex].averagePosition[bestDimension] < mid)
		{
			std::swap(inMesh->faces[faceIndex], inMesh->faces[actualMidIndex]);
			actualMidIndex++;
		}
	}
	unsigned int nextAmount = actualMidIndex - firstFace;
	unsigned int currIndex = inMesh->bvh.size();
	ConstructChildBVH(inMesh, firstFace, nextAmount);
	ConstructChildBVH(inMesh, actualMidIndex, facesAmount - nextAmount);
	if (triangle<nextAmount) {
		inMesh->bvh[currIndex].index = currIndex + 2;
		ConstructBVH(inMesh, currIndex, firstFace, nextAmount);
	}
	if (triangle<(facesAmount - nextAmount)) {
		unsigned int secondIndex = inMesh->bvh.size();
		inMesh->bvh[currIndex+1].index = secondIndex;
		ConstructBVH(inMesh, currIndex+1, actualMidIndex, facesAmount - nextAmount);
	//	std::cout << actualMidIndex << std::endl;
	}
}
void ConstructBVHFromMesh(Mesh* inMesh) {

	for (unsigned int i = 0; i < inMesh->faces.size(); i++) {
		float tempAveragePos[3] = { 0,0,0 };
		float tempMaxPos[3] = { -INFINITY ,-INFINITY ,-INFINITY };
		float tempMinPos[3] = { INFINITY ,INFINITY ,INFINITY };
		for (char iVertex = 0; iVertex < 3; iVertex++) {
			for (char dimension = 0; dimension < 3; dimension++) {
				float curPosition = inMesh->vertices[inMesh->faces[i].indicesGroups[iVertex].indices[0]].position[dimension];
				tempAveragePos[dimension] += curPosition/3;
				tempMaxPos[dimension] = tempMaxPos[dimension] > curPosition ? tempMaxPos[dimension] : curPosition;
				tempMinPos[dimension] = tempMinPos[dimension] < curPosition ? tempMinPos[dimension] : curPosition;
			}
		}
		for (char dimension = 0; dimension < 3; dimension++) {
			inMesh->faces[i].averagePosition[dimension] = tempAveragePos[dimension];
			inMesh->faces[i].maxPosition[dimension] = tempMaxPos[dimension];
			inMesh->faces[i].minPosition[dimension] = tempMinPos[dimension];
		}
	}
	for (unsigned int i = 0; i < inMesh->batchedInfos.size(); i++) {
		unsigned int currIndex = inMesh->bvh.size();
		inMesh->batchedInfos[i].bvhIndex = currIndex;
		ConstructChildBVH(inMesh, inMesh->batchedInfos[i].startFace, inMesh->batchedInfos[i].facesAmount);
		inMesh->bvh[currIndex].index = currIndex+1;
		ConstructBVH(inMesh,currIndex, inMesh->batchedInfos[i].startFace, inMesh->batchedInfos[i].facesAmount);
	}
	std::cout << total[0] << " " << total[1] << " " << total[2] << std::endl;
}

Mesh BatchMesh(std::vector<Mesh> meshes) {
	Mesh batchedMesh;
	unsigned int curFaceIndex=0;
	for (unsigned int meshIndex = 0; meshIndex < meshes.size(); meshIndex++) {
		float temp[3] = { 0,0,0 };
		BatchedInfo batchedInfo(0,0,0,0,0,temp,temp,temp);
		batchedInfo.startFace = curFaceIndex;
		unsigned int currentVerticesSize = batchedMesh.vertices.size();
		unsigned int currentNormalSize = batchedMesh.normals.size();
		unsigned int currentUVsSize = batchedMesh.normals.size();
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
			for (char j=0;j<3;j++){
					meshes[meshIndex].faces[i].indicesGroups[j].indices[0] += currentVerticesSize;
					meshes[meshIndex].faces[i].indicesGroups[j].indices[1] += currentNormalSize;
					meshes[meshIndex].faces[i].indicesGroups[j].indices[2] += currentUVsSize;

			}
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