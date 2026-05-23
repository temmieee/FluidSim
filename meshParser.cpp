#include "meshParser.h"
const unsigned int triangle = 3;
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
	char begin = tempString.find(' ');
	if (begin !=std::string::npos)
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
Face::Face() 
{
	averagePosition[0] = averagePosition[1] = averagePosition[2] = 0.0f;
	maxPosition[0] = maxPosition[1] = maxPosition[2] = 0.0f;
	minPosition[0] = minPosition[1] = minPosition[2] = 0.0f;
}
Face::Face(std::string line)
	: averagePosition{ 0.0f, 0.0f, 0.0f }, maxPosition{ 0.0f, 0.0f, 0.0f }, minPosition{ 0.0f, 0.0f, 0.0f }
{
	for (std::size_t i = 0; (i < 10) && (line.find(' ') != std::string::npos); ++i) {
		indicesGroups.push_back(ReadNextIndexGroup(&line));
	}
}
Face::Face(IndicesGroup first, IndicesGroup second, IndicesGroup third)
	: averagePosition{ 0.0f, 0.0f, 0.0f }, maxPosition{ 0.0f, 0.0f, 0.0f }, minPosition{ 0.0f, 0.0f, 0.0f }
{
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
BatchedInfo::BatchedInfo()
	: startFace(0),
	facesAmount(0),
	materialIndex(0),
	priorityIndex(0),
	bvhIndex(0),
	position{ 0.0f, 0.0f, 0.0f },
	rotation{ 0.0f, 0.0f, 0.0f },
	scale{ 0.0f, 0.0f, 0.0f },
	padding{ 0.0f, 0.0f },
	modelMatrix(Mat4()),
	inverseModelMatrix(Mat4())
{
}
BatchedInfo::BatchedInfo(unsigned int sFace, unsigned int fAmount, int mIndex, unsigned int prioIndex, unsigned int bIndex, float pos[], float rot[], float s[])
	: startFace(sFace),
	facesAmount(fAmount),
	materialIndex(mIndex),
	priorityIndex(prioIndex),
	bvhIndex(bIndex),
	modelMatrix(Mat4()),
	inverseModelMatrix(Mat4())
{
	for (int i = 0; i < 3; i++) {
		position[i] = pos[i];
		rotation[i] = rot[i];
		scale[i] = s[i];
	}
	padding[0] = 0.0f;
	padding[1] = 0.0f;
}
BVHnode::BVHnode() {
	for (char i = 0; i < 3; i++) {
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
	//safeguard
    if (facesAmount <= triangle) {
        inMesh->bvh[parent].amount = facesAmount;
        return;
    }

    float inMaxBound[3];
    float inMinBound[3];
    for (int i = 0; i < 3; ++i) {
        inMaxBound[i] = inMesh->bvh[parent].maxBound[i];
        inMinBound[i] = inMesh->bvh[parent].minBound[i];
    }

    const unsigned int division = 20;
    float lowestCost = INFINITY;
    unsigned int bestDimension = 0;
    unsigned int bestCostIndex = 0;

    for (unsigned int dimensionPartition = 0; dimensionPartition < 3; ++dimensionPartition) {
        for (unsigned int iterateSAH = 0; iterateSAH < division; ++iterateSAH) {
            float mid = inMinBound[dimensionPartition] + (iterateSAH + 1u) * (inMaxBound[dimensionPartition] - inMinBound[dimensionPartition]) / (division + 1u);
            unsigned int midIndex = firstFace;

            for (unsigned int faceIndex = firstFace; faceIndex < firstFace + facesAmount; ++faceIndex) {
                if (inMesh->faces[faceIndex].averagePosition[dimensionPartition] < mid) {
                    std::swap(inMesh->faces[faceIndex], inMesh->faces[midIndex]);
                    ++midIndex;
                }
            }

            unsigned int countFirst = (midIndex > firstFace) ? (midIndex - firstFace) : 0u;
            unsigned int countSecond = ((firstFace + facesAmount) > midIndex) ? ((firstFace + facesAmount) - midIndex) : 0u;

            if (countFirst == 0 || countSecond == 0) {
                continue;
            }

            float tempMaxBoundFirst[3] = { -INFINITY, -INFINITY, -INFINITY };
            float tempMinBoundFirst[3] = { INFINITY, INFINITY, INFINITY };
            float tempMaxBoundSecond[3] = { -INFINITY, -INFINITY, -INFINITY };
            float tempMinBoundSecond[3] = { INFINITY, INFINITY, INFINITY };

            for (unsigned int i = firstFace; i < midIndex; ++i) {
                for (int d = 0; d < 3; ++d) {
                    tempMaxBoundFirst[d] = std::max(tempMaxBoundFirst[d], inMesh->faces[i].maxPosition[d]);
                    tempMinBoundFirst[d] = std::min(tempMinBoundFirst[d], inMesh->faces[i].minPosition[d]);
                }
            }
            for (unsigned int i = midIndex; i < firstFace + facesAmount; ++i) {
                for (int d = 0; d < 3; ++d) {
                    tempMaxBoundSecond[d] = std::max(tempMaxBoundSecond[d], inMesh->faces[i].maxPosition[d]);
                    tempMinBoundSecond[d] = std::min(tempMinBoundSecond[d], inMesh->faces[i].minPosition[d]);
                }
            }

            float firstDiag[3], secondDiag[3];
            for (int d = 0; d < 3; ++d) {
                firstDiag[d] = tempMaxBoundFirst[d] - tempMinBoundFirst[d];
                secondDiag[d] = tempMaxBoundSecond[d] - tempMinBoundSecond[d];
            }

            float saFirst = firstDiag[0]*firstDiag[1] + firstDiag[1]*firstDiag[2] + firstDiag[2]*firstDiag[0];
            float saSecond = secondDiag[0]*secondDiag[1] + secondDiag[1]*secondDiag[2] + secondDiag[2]*secondDiag[0];

            float currCost = saFirst * countFirst + saSecond * countSecond;

            if (currCost < lowestCost) {
                lowestCost = currCost;
                bestDimension = dimensionPartition;
                bestCostIndex = iterateSAH;
            }
        }
    }

    //make this node a leaf again if no valid split midpoint
    if (lowestCost == INFINITY) {
        inMesh->bvh[parent].amount = facesAmount;
        return;
    }

    // perform actual split
    float mid = inMinBound[bestDimension] + (bestCostIndex + 1u) * (inMaxBound[bestDimension] - inMinBound[bestDimension]) / (division + 1u);
    unsigned int actualMidIndex = firstFace;
    for (unsigned int faceIndex = firstFace; faceIndex < firstFace + facesAmount; ++faceIndex) {
        if (inMesh->faces[faceIndex].averagePosition[bestDimension] < mid) {
            std::swap(inMesh->faces[faceIndex], inMesh->faces[actualMidIndex]);
            ++actualMidIndex;
        }
    }

    unsigned int nextAmount = actualMidIndex - firstFace;

    // another guard
    if (nextAmount == 0 || nextAmount == facesAmount) {
        inMesh->bvh[parent].amount = facesAmount;
        return;
    }

    unsigned int currIndex = inMesh->bvh.size();
    ConstructChildBVH(inMesh, firstFace, nextAmount);
    ConstructChildBVH(inMesh, actualMidIndex, facesAmount - nextAmount);

    if (triangle < static_cast<int>(nextAmount)) {
        inMesh->bvh[currIndex].index = static_cast<int>(currIndex + 2);
        ConstructBVH(inMesh, currIndex, firstFace, nextAmount);
    } else {
        inMesh->bvh[currIndex].amount = static_cast<int>(nextAmount);
    }

    if (triangle < static_cast<int>(facesAmount - nextAmount)) {
        unsigned int secondIndex = inMesh->bvh.size();
        inMesh->bvh[currIndex + 1].index = static_cast<int>(secondIndex);
        ConstructBVH(inMesh, currIndex + 1, actualMidIndex, facesAmount - nextAmount);
    } else {
        inMesh->bvh[currIndex + 1].amount = static_cast<int>(facesAmount - nextAmount);
    }
}
void ConstructBVHFromMesh(Mesh* inMesh) {
	//check for lib
	std::string filename = std::string(inMesh->name) + ".bvh";
	std::ifstream infile(filename);
	if (infile.good()) {
		std::cout << "BVH file already exists for mesh: " << inMesh->name << std::endl;
		ReadBVHNodesFromCSV(inMesh,inMesh->name);
		RemapFacesFromCSV(inMesh->faces, inMesh->name);
		return; 
	}
	unsigned int faceIndex = 0;
	for (unsigned int i = 0; i < inMesh->faces.size(); i++) {
		inMesh->faces[i].originalIndex = faceIndex++;
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
	WriteBVHNodesToCSV(inMesh, inMesh->name);
	WriteFaceIndexMappingToCSV(inMesh->faces,inMesh->name);
	//std::cout << total[0] << " " << total[1] << " " << total[2] << std::endl;
}

Mesh BatchMesh(std::vector<Mesh>& meshes) {
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
	std::string mesh = std::string(meshFile)+ ".mesh";
	std::ifstream in(mesh, std::ios::binary);
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
				mesh.name = line.substr(2);
				meshes.push_back(mesh);
				meshIndex++;
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
	
}
void WriteBVHNodesToCSV(const Mesh* mesh, const std::string& filename) {
	const std::vector<BVHnode>& nodes = mesh->bvh;
	std::string inMap = filename + ".bvh";
	std::ofstream file(inMap);
	if (!file.is_open()) {
		std::cerr << "Error opening file\n";
		return;
	}

	file << "objectIndex,rootNode\n";
	for (unsigned int i = 0; i < mesh->batchedInfos.size(); i++) {
		file << i << "," << mesh->batchedInfos[i].bvhIndex << "\n";
	}

	file << "maxX,maxY,maxZ,index,minX,minY,minZ,amount\n";

	for (const auto& node : nodes) {
		file << node.maxBound[0] << ","
			<< node.maxBound[1] << ","
			<< node.maxBound[2] << ","
			<< node.index << ","
			<< node.minBound[0] << ","
			<< node.minBound[1] << ","
			<< node.minBound[2] << ","
			<< node.amount << "\n";
	}

}
void ReadBVHNodesFromCSV(Mesh* mesh, const std::string& filename) {
	std::string inMap = filename + ".bvh";
	std::vector<BVHnode> nodes;
	std::ifstream file(inMap);
	if (!file.is_open()) {
		std::cerr << "Error opening file\n";
		return;
	}
	std::string line;
	std::getline(file, line);
	while (std::getline(file, line)) {
		if (line.rfind("maxX", 0) == 0) {
			break;
		}
		std::stringstream ss(line);
		std::string value;
		unsigned int objectIndex, rootNode;
		std::getline(ss, value, ','); objectIndex = std::stoul(value);
		std::getline(ss, value, ','); rootNode = std::stoul(value);

		// store back into mesh
		if (objectIndex < mesh->batchedInfos.size()) {
			mesh->batchedInfos[objectIndex].bvhIndex = rootNode;
		}
	}
	while (std::getline(file, line)) {
		std::stringstream ss(line);
		std::string value;

		float maxBound[3];
		unsigned int index;
		float minBound[3];
		unsigned int amount;

		// Parse CSV values
		std::getline(ss, value, ','); maxBound[0] = std::stof(value);
		std::getline(ss, value, ','); maxBound[1] = std::stof(value);
		std::getline(ss, value, ','); maxBound[2] = std::stof(value);

		std::getline(ss, value, ','); index = std::stoul(value);

		std::getline(ss, value, ','); minBound[0] = std::stof(value);
		std::getline(ss, value, ','); minBound[1] = std::stof(value);
		std::getline(ss, value, ','); minBound[2] = std::stof(value);

		std::getline(ss, value, ','); amount = std::stoul(value);

		nodes.emplace_back(maxBound, minBound, index, amount);
	}
	mesh->bvh.insert(mesh->bvh.begin(),nodes.begin(), nodes.end());
}
void WriteFaceIndexMappingToCSV(const std::vector<Face>& faces,	const std::string& filename) {
	std::vector<unsigned int> map;
	for (size_t i = 0; i < faces.size(); i++) {
		map.push_back(faces[i].originalIndex);
	}
	std::string newMap = filename + ".indexmap";
	std::ofstream file(newMap);
	if (!file.is_open()) {
		std::cerr << "Error opening file\n";
		return;
	}
	file << "OriginalIndex,CurrentIndex\n";
	for (unsigned int i = 0; i < map.size(); i++) {
		file << map[i] << "," << i << "\n";
	}
	file.close();
}
void RemapFacesFromCSV(std::vector<Face>& faces,	const std::string& filename) {

	std::vector<unsigned int> map;
	std::string inMap = filename + ".indexmap";
	std::ifstream file(inMap);
	if (!file.is_open()) {
		std::cerr << "Error opening mapping file\n";
		return;
	}
	std::string line;
	std::getline(file, line);
	while (std::getline(file, line)) {
		std::stringstream ss(line);
		std::string value;

		unsigned int original, current;
		std::getline(ss, value, ','); original = std::stoul(value);
		std::getline(ss, value, ','); current = std::stoul(value);

		map.push_back(original);
	}
	file.close();
	std::vector<Face> rearranged(faces.size());
	for (unsigned int i = 0; i < faces.size(); i++) {
		rearranged[i] = faces[map[i]];
	}
	faces.swap(rearranged);
}