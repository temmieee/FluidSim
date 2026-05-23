#include<iostream>

#include"glad/glad.h"
#include"frameTimer.h"
#include"GLFW/glfw3.h"
#include"shaderClass.h"
#include"meshParser.h"
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include <thread>
#include <chrono>
const unsigned int SCREEN_WIDTH = 2048;
const unsigned int SCREEN_HEIGHT = 1024;

const unsigned short OPENGL_MAJOR_VERSION = 4;
const unsigned short OPENGL_MINOR_VERSION = 6;

bool vSync = true;



GLfloat vertices[] =
{
	-1.0f, -1.0f , 0.0f, 0.0f, 0.0f,
	-1.0f,  1.0f , 0.0f, 0.0f, 1.0f,
	 1.0f,  1.0f , 0.0f, 1.0f, 1.0f,
	 1.0f, -1.0f , 0.0f, 1.0f, 0.0f,  
};

GLuint indices[] =
{
	0, 2, 1,
	0, 3, 2
};

struct Sphere {
	Vector3 position;
	GLfloat radius;
	Vector4 color;
	Material material;
	Sphere(float* input[]) {
		position.x = *input[0];
		position.y = *input[1];
		position.z = *input[2];
		radius = *input[3];
		color.x = *input[4];
		color.y = *input[5];
		color.z = *input[6];
		color.w = *input[7];
		material.roughness = *input[8];
		material.metallic = *input[9];
		material.emissive = *input[10];
		material.refractiveIndex = *input[11];
	}
};

void PrintSpecs() {
	int work_grp_cnt[3];
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &work_grp_cnt[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &work_grp_cnt[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &work_grp_cnt[2]);

	int work_grp_size[3];
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &work_grp_size[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &work_grp_size[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &work_grp_size[2]);

	int work_grp_inv;
	glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &work_grp_inv);
	std::cout << "Max work groups per compute shader" <<
		" x:" << work_grp_cnt[0] <<
		" y:" << work_grp_cnt[1] <<
		" z:" << work_grp_cnt[2] << "\n";
	std::cout << "Max work group sizes" <<
		" x:" << work_grp_size[0] <<
		" y:" << work_grp_size[1] <<
		" z:" << work_grp_size[2] << "\n";

	std::cout << "Max invocations count per work group: " << work_grp_inv << "\n";
}
void Draw(std::vector<Sphere> spheresArray) {
	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, OPENGL_MAJOR_VERSION);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, OPENGL_MINOR_VERSION);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Temmie", NULL, NULL);
	if (!window)
	{
		std::cout << "Failed to create the GLFW window\n";
		glfwTerminate();
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(vSync);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize OpenGL context" << std::endl;
	}
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);


	GLuint VAO, VBO, EBO;
	glCreateVertexArrays(1, &VAO);
	glCreateBuffers(1, &VBO);
	glCreateBuffers(1, &EBO);

	glNamedBufferData(VBO, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glNamedBufferData(EBO, sizeof(indices), indices, GL_STATIC_DRAW);

	glEnableVertexArrayAttrib(VAO, 0);
	glVertexArrayAttribBinding(VAO, 0, 0);
	glVertexArrayAttribFormat(VAO, 0, 3, GL_FLOAT, GL_FALSE, 0);

	glEnableVertexArrayAttrib(VAO, 1);
	glVertexArrayAttribBinding(VAO, 1, 0);
	glVertexArrayAttribFormat(VAO, 1, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat));

	glVertexArrayVertexBuffer(VAO, 0, VBO, 0, 5 * sizeof(GLfloat));
	glVertexArrayElementBuffer(VAO, EBO);


	GLuint screenTex;
	glCreateTextures(GL_TEXTURE_2D, 1, &screenTex);
	glTextureParameteri(screenTex, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(screenTex, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(screenTex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(screenTex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureStorage2D(screenTex, 1, GL_RGBA32F, SCREEN_WIDTH, SCREEN_HEIGHT);
	glBindImageTexture(0, screenTex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

	Shader screenShader("default.vert", "default.frag");
	float input1 = 0.5;
	float input2 = 0.75;
	float input3 = 1;
	Sphere sphere(new float* [16] { &input1, & input2, & input3, & input1, & input2, & input3, & input1, & input2, & input3, & input1, & input2, & input3, & input1, & input2, & input3, &input2});
	std::vector<Sphere> spheres;
	spheres.push_back(sphere);
	spheres.push_back(sphere);
	//spheresArray = spheres;
	Shader computeShader("computeShader.compute");
	GLuint spheresBuffer;
	glCreateBuffers(1, &spheresBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, spheresBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, spheresArray.size() * sizeof(Sphere), spheresArray.data(), GL_STATIC_DRAW);
	void* ptr = spheresArray.data();
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER,1, spheresBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	GLuint halffov = glGetUniformLocation(computeShader.ID, "halffov");
	std::cout << sizeof(Sphere) << "\n";
	std::cout << sizeof(spheres) << "\n";

	PrintSpecs();


	while (!glfwWindowShouldClose(window))
	{
		auto start = std::chrono::high_resolution_clock::now();
		computeShader.Activate();
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, spheresBuffer);
		glUniform1f(halffov, 30);
		glDispatchCompute(ceil(SCREEN_WIDTH / 32), ceil(SCREEN_HEIGHT / 32), 1);
		glMemoryBarrier(GL_ALL_BARRIER_BITS);
		screenShader.Activate();
		glBindTextureUnit(0, screenTex);
		const char* texture = "screen";
		screenShader.GetTexture(texture,0);
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(indices[0]), GL_UNSIGNED_INT, 0);
		auto stop = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
		std::cout << "Render time: " << duration.count() << " microseconds" << std::endl;
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glDeleteBuffers(1, &EBO);
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	computeShader.Delete();
	screenShader.Delete();

	glfwDestroyWindow(window);
	glfwTerminate();
}
void DrawDensity(std::vector<Sphere> spheresArray) {
	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, OPENGL_MAJOR_VERSION);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, OPENGL_MINOR_VERSION);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Temmie", NULL, NULL);
	if (!window)
	{
		std::cout << "Failed to create the GLFW window\n";
		glfwTerminate();
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(vSync);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize OpenGL context" << std::endl;
	}
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);


	GLuint VAO, VBO, EBO;
	glCreateVertexArrays(1, &VAO);
	glCreateBuffers(1, &VBO);
	glCreateBuffers(1, &EBO);

	glNamedBufferData(VBO, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glNamedBufferData(EBO, sizeof(indices), indices, GL_STATIC_DRAW);

	glEnableVertexArrayAttrib(VAO, 0);
	glVertexArrayAttribBinding(VAO, 0, 0);
	glVertexArrayAttribFormat(VAO, 0, 3, GL_FLOAT, GL_FALSE, 0);

	glEnableVertexArrayAttrib(VAO, 1);
	glVertexArrayAttribBinding(VAO, 1, 0);
	glVertexArrayAttribFormat(VAO, 1, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat));

	glVertexArrayVertexBuffer(VAO, 0, VBO, 0, 5 * sizeof(GLfloat));
	glVertexArrayElementBuffer(VAO, EBO);


	GLuint screenTex;
	glCreateTextures(GL_TEXTURE_2D, 1, &screenTex);
	glTextureParameteri(screenTex, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(screenTex, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(screenTex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(screenTex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureStorage2D(screenTex, 1, GL_RGBA32F, SCREEN_WIDTH, SCREEN_HEIGHT);
	glBindImageTexture(0, screenTex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

	Shader screenShader("default.vert", "default.frag");
	float input1 = 0.5;
	float input2 = 0.75;
	float input3 = 1;
	Sphere sphere(new float* [16] { &input1, & input2, & input3, & input1, & input2, & input3, & input1, & input2, & input3, & input1, & input2, & input3, & input1, & input2, & input3, & input2});
	std::vector<Sphere> spheres;
	spheres.push_back(sphere);
	spheres.push_back(sphere);
	//spheresArray = spheres;
	Shader computeShader("physicHandler.compute");
	GLuint spheresBuffer;
	glCreateBuffers(1, &spheresBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, spheresBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, spheresArray.size() * sizeof(Sphere), spheresArray.data(), GL_DYNAMIC_READ);
	void* ptr = spheresArray.data();
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, spheresBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	GLuint halffov = glGetUniformLocation(computeShader.ID, "halffov");
	GLuint bound = glGetUniformLocation(computeShader.ID, "bound");

	PrintSpecs();
	const std::chrono::milliseconds interval(100); // 100 milliseconds = 0.1 seconds
	

	while (!glfwWindowShouldClose(window))
	{
		std::thread workerThread([&]() {
			
				std::this_thread::sleep_for(interval);
		});
		workerThread.join();
		computeShader.Activate();
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, spheresBuffer);
		glUniform1f(halffov, 75);
		glUniform3f(bound, 5.0f, 5.0f, 1.0f);
		glDispatchCompute(ceil(SCREEN_WIDTH / 8), ceil(SCREEN_HEIGHT / 4), 1);
		glMemoryBarrier(GL_ALL_BARRIER_BITS);
		void* mappedBuffer = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 1, spheresArray.size() * sizeof(Sphere), GL_MAP_READ_BIT);
		if (mappedBuffer) {
			// Copy data from mappedBuffer to your C++ array
			std::memcpy(spheresArray.data(), mappedBuffer, spheresArray.size() * sizeof(Sphere));
			glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		}
		screenShader.Activate();
		glBindTextureUnit(0, screenTex);
		const char* texture = "screen";
		screenShader.GetTexture(texture,0);
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(indices[0]), GL_UNSIGNED_INT, 0);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glDeleteBuffers(1, &EBO);
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &spheresBuffer);
	computeShader.Delete();
	screenShader.Delete();

	glfwDestroyWindow(window);
	glfwTerminate();
}
std::vector<Sphere> CreateSphereArray(float bound[], int* amount) {
	float volume = bound[0] * (bound[1]) * (bound[2])/(*amount);
	float dimension = cbrt(volume);
	float inverseDimension = 1/dimension;
	std::vector<Sphere> spheres;
	int dimensions[3] = { floor(bound[0] * inverseDimension), floor(bound[1] * inverseDimension), floor(bound[2] * inverseDimension) };
	
	for (int i = 0; i < dimensions[0]; i++) {
		for (int j = 0; j < dimensions[1]; j++) {
			for (int k = 0; k< dimensions[2]; k++) {
				float positionX = (i - dimensions[0] / 2) * dimension;
				float positionY = (j - dimensions[1] / 2) * dimension;
				float positionZ = (k - dimensions[2] / 2) * dimension;
				float radius = dimension/2.25;
				float colorR = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
				float colorG = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
				float colorB = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
				float colorA = 1.f;
				float roughness = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
				roughness = (roughness>0.5) ? 1 : 0;
				float metallic = 0.0f;
				float emissive = 0.f;
				float refractiveIndex = 1.0f;
				float materialColorR = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
				float materialColorG = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
				float materialColorB = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
				float materialColorA = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
				Sphere newSphere = Sphere(new float* [16] { &positionX, & positionY, & positionZ, & radius, & colorR, & colorG, & colorB, & colorA, & roughness, & metallic, & emissive, & refractiveIndex, &materialColorR, & materialColorG,&materialColorB, &materialColorA  });
				spheres.push_back(newSphere);
			}
		}
	}
	return spheres;
}
void PrintArray(float array[], char howMany)
{
	for (char i = 0; i < howMany; i++) {
		std::cout << array[i] << " ";
	}
	std::cout << "\n";
}
void PrintArray(int array[], char howMany)
{
	for (char i = 0; i < howMany; i++) {
		if (i) {
			std::cout << "/";
		}
		std::cout  << array[i] ;
	}
	std::cout << " ";
}
void PrintMesh(Mesh m) {
	for (unsigned int i = 0; i < m.vertices.size(); i++)
	{
		std::cout << "Vertex " << (int)i << ": ";
		PrintArray(m.vertices[i].position, 3);
	}
	for (unsigned int i = 0; i < m.normals.size(); i++) {
		std::cout << "Normal" << (int)i;
		PrintArray(m.normals[i].normal, 3);
	}
	for (unsigned int i = 0; i < m.uvs.size(); i++) {
		std::cout << "UV" << (int)i << ':';
		PrintArray(m.uvs[i].uv, 2);

	}
	for (unsigned int i = 0; i < m.faces.size(); i++) {
		std::cout << "Face" << (int)i << " ";
		for (char j = 0; j < m.faces[i].indicesGroups.size(); j++) {
			PrintArray(m.faces[i].indicesGroups[j].indices, 3);
		}
		std::cout << "\n";
	}
}

void DrawBatchedMesh(Mesh m, std::vector<Material> materialArray) {

	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, OPENGL_MAJOR_VERSION);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, OPENGL_MINOR_VERSION);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Temmie", NULL, NULL);
	if (!window)
	{
		std::cout << "Failed to create the GLFW window\n";
		glfwTerminate();
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(vSync);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize OpenGL context" << std::endl;
	}
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	for (char i=0;i<m.batchedInfos.size();i++) {
		m.batchedInfos[i].modelMatrix = CalculateModelMatrix(m.batchedInfos[i].position, m.batchedInfos[i].rotation, m.batchedInfos[i].scale);
		m.batchedInfos[i].inverseModelMatrix = CalculateInverseModelMatrix(m.batchedInfos[i].position, m.batchedInfos[i].rotation, m.batchedInfos[i].scale);
	}

	GLuint VAO, VBO, EBO;
	glCreateVertexArrays(1, &VAO);
	glCreateBuffers(1, &VBO);
	glCreateBuffers(1, &EBO);

	glNamedBufferData(VBO, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glNamedBufferData(EBO, sizeof(indices), indices, GL_STATIC_DRAW);

	glEnableVertexArrayAttrib(VAO, 0);
	glVertexArrayAttribBinding(VAO, 0, 0);
	glVertexArrayAttribFormat(VAO, 0, 3, GL_FLOAT, GL_FALSE, 0);

	glEnableVertexArrayAttrib(VAO, 1);
	glVertexArrayAttribBinding(VAO, 1, 0);
	glVertexArrayAttribFormat(VAO, 1, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat));

	glVertexArrayVertexBuffer(VAO, 0, VBO, 0, 5 * sizeof(GLfloat));
	glVertexArrayElementBuffer(VAO, EBO);


	GLuint screenTex;
	glCreateTextures(GL_TEXTURE_2D, 1, &screenTex);
	glTextureParameteri(screenTex, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(screenTex, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(screenTex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(screenTex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureStorage2D(screenTex, 1, GL_RGBA32F, SCREEN_WIDTH, SCREEN_HEIGHT);
	glBindImageTexture(0, screenTex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);


	Shader screenShader("default.vert", "progressiveRender.frag");
	float input1 = 0.5;
	float input2 = 0.75;
	float input3 = 1;

	//spheresArray = spheres;
	Shader computeShader("finalComputeShader.compute");
	//meshBuffer
	std::vector<Vector4 >colorArray;
	for (unsigned int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
		colorArray.push_back(Vector4(0, 0, 0, 1));
		colorArray.push_back(Vector4(0, 0, 0, 1));
	}
	GLuint colorBuffer;
	glCreateBuffers(1, &colorBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, colorBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, colorArray.size()* sizeof(Vector4), colorArray.data(), GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, colorBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);


	GLuint vertexBuffer;
	glCreateBuffers(1, &vertexBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertexBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, m.vertices.size() * sizeof(Vertex), m.vertices.data(), GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, vertexBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	GLuint normalBuffer;
	glCreateBuffers(1, &normalBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, normalBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, m.normals.size() * sizeof(Normal), m.normals.data(), GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, normalBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);


	//convert this to something the GPU could understand aka flatten the 2D dynamic array
	std::vector<IndicesGroup> tempFace;
	for (Face faceArray : m.faces) {
		for (IndicesGroup iG : faceArray.indicesGroups) {
			tempFace.push_back(iG);
		}
	}
	GLuint faceBuffer;
	glCreateBuffers(1, &faceBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, faceBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, tempFace.size() * sizeof(IndicesGroup), tempFace.data(), GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, faceBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	//sort light emitters
	unsigned int lightSources = 0;
	for (unsigned int i = 0; i < m.batchedInfos.size(); i++) {
		if (materialArray[m.batchedInfos[i].materialIndex].emissive > 0) {
			std::rotate(m.batchedInfos.begin(), m.batchedInfos.begin()+i, m.batchedInfos.begin()+i+1);
			lightSources++;
		}
	}
	GLuint batchedInfoBuffer;
	glCreateBuffers(1, &batchedInfoBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, batchedInfoBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, m.batchedInfos.size() * sizeof(BatchedInfo), m.batchedInfos.data(), GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, batchedInfoBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	GLuint materialBuffer;
	glCreateBuffers(1, &materialBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, materialBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, materialArray.size() * sizeof(Material), materialArray.data(), GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, materialBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	GLuint bvhBuffer;
	glCreateBuffers(1, &bvhBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, bvhBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, m.bvh.size() * sizeof(BVHnode), m.bvh.data(), GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, bvhBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	GLuint halffov = glGetUniformLocation(computeShader.ID, "halffov");
	GLuint objectAmount = glGetUniformLocation(computeShader.ID, "objectAmount");
	GLuint lightAmount = glGetUniformLocation(computeShader.ID, "lightAmount");
	GLuint progress = glGetUniformLocation(computeShader.ID, "progress");

	PrintSpecs();
	
	unsigned int prog = 0;
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, colorBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, vertexBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, normalBuffer);
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, uvBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, faceBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, batchedInfoBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, materialBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, bvhBuffer);

	FrameTimer timer(60);
	timer.Start();
	float time = 0;
	while (!glfwWindowShouldClose(window))
	{
		double dt = timer.Tick();
		double fps = timer.GetFPS();
		time += dt;
		if (time>1) {
			time = 0;
			std::cout << "FPS: " << fps << " Frame Time: " << dt * 1000 << "ms\n";
		}
		if (1) { 
		


			computeShader.Activate();
			glUniform1f(halffov, 35);
			glUniform1ui(objectAmount, m.batchedInfos.size());
			glUniform1ui(lightAmount, lightSources);
			glUniform1ui(progress, prog);
			glDispatchCompute(ceil(SCREEN_WIDTH / 32), ceil(SCREEN_HEIGHT / 32), 1);
			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
			prog++;
		}
		screenShader.Activate();
		glBindTextureUnit(0, screenTex);
		const char* texture = "screen";
		screenShader.GetTexture(texture,0);
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(indices[0]), GL_UNSIGNED_INT, 0);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glDeleteBuffers(1, &EBO);
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	computeShader.Delete();
	screenShader.Delete();

	glfwDestroyWindow(window);
	glfwTerminate();

}
std::vector<Material> CreateMaterialArray(int howMany) {
	std::vector<Material> mA;
	for (char i = 0; i < howMany; i++) {
		float materialColorR = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		float materialColorG = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		float materialColorB = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		float materialColorA = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		float roughness =1.0f;
		float metallic = 0.0f;
		float emissive = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		emissive = 0;
		float refractiveIndex = 1.0f;
		Vector4 materialColor(materialColorR, materialColorG, materialColorB, materialColorA);
		Material newMat(materialColor, roughness, metallic, emissive, refractiveIndex);
		mA.push_back(newMat);
	}
	return mA;
}
void InstantiateMeshes(std::vector<BatchedInfo>* batchedInfos, float bounds[], unsigned int dimensions[], float center[], unsigned int meshIndex) {
	for (unsigned int i = 0; i < dimensions[0]; i++) {
		for (unsigned int j = 0; j < dimensions[1]; j++) {
			for (unsigned int k = 0; k < dimensions[2]; k++) {
				BatchedInfo newBatchedInfo = (*batchedInfos)[meshIndex];
				float posX = 2*((float)(i+0.5)/(float)dimensions[0]-0.5)*bounds[0];
				float posY = 2*((float)(j+0.5) / (float)dimensions[1] - 0.5) * bounds[1];
				float posZ = 2*((float)(k + 0.5) / (float)dimensions[2] - 0.5) * bounds[2];
				newBatchedInfo.position[0] =posX+center[0];
				newBatchedInfo.position[1] = posY + center[1];
				newBatchedInfo.position[2] = posZ + center[2];
				float scaleFactor=INFINITY;
				for (char s = 0; s < 3; s++) {
					float currScaleFactor = bounds[s] / dimensions[s];
					scaleFactor = scaleFactor < currScaleFactor ? scaleFactor : currScaleFactor;
					newBatchedInfo.scale[s] =  scaleFactor;
				}
				scaleFactor *= 0.5f+ 0.75*static_cast <float>(rand()) / static_cast <float>(RAND_MAX);
				for (char s = 0; s < 3; s++) {
					newBatchedInfo.scale[s] = scaleFactor;
				}
				newBatchedInfo.rotation[1] = 3.1416*static_cast <float>(rand()) / static_cast <float>(RAND_MAX);
				newBatchedInfo.materialIndex= floor(9*static_cast <float> (rand()) / static_cast <float> (RAND_MAX));
				newBatchedInfo.bvhIndex = (*batchedInfos)[meshIndex].bvhIndex;
				batchedInfos->push_back(newBatchedInfo);
			}
		}
	}
}
void InstantiateWalls(std::vector<BatchedInfo>* batchedInfos, float bounds[], unsigned int dimensions[], float center[], unsigned int meshIndex) {
	//floor and ceiling
	for (char i = 0; i < 3; i++) {
		for (char j = 0; j < 2; j++) {
			BatchedInfo newBatchedInfo = (*batchedInfos)[meshIndex];
			float pos = 2 * ((float)(j + 0.5) / 2 - 0.5) * bounds[i]+center[i];
			newBatchedInfo.position[i] = pos;
			for (char k = 0; k < 3; k++) {
				if (k != i) {
					newBatchedInfo.scale[k] = bounds[k];
				}
				else {
					newBatchedInfo.scale[k] = 0.1;
				}
			}
			newBatchedInfo.materialIndex = floor(9 * static_cast <float> (rand()) / static_cast <float> (RAND_MAX));
			newBatchedInfo.bvhIndex = (*batchedInfos)[meshIndex].bvhIndex;
			batchedInfos->push_back(newBatchedInfo);
		}

	}

}
int main()
{
	const char* file = "Dragon";
	std::vector<Mesh> mesh = ScanForMesh(file);
	std::vector<Material> materials;

	Mesh batchedMesh = BatchMesh(mesh);
	batchedMesh.name = file;
	ConstructBVHFromMesh(&batchedMesh);
	Vector3 vec3(1, 1, 1);
	float temp[3] = { 0,0,0 };

	//Instantiate meshes
	float bounds[3] = { 3.f,2.f,3.f };
	float center[3] = { 0,-1.8f,0 };
	float centerWall[3] = { 0,0,0 };
	float walls[3] = { 7.f,3.75f,7.f };
	unsigned int dimensions[3] = {4,1,4};

	batchedMesh.batchedInfos[0].position[0] = 0.f;
	batchedMesh.batchedInfos[0].position[1] = 1.7f;
	batchedMesh.batchedInfos[0].position[2] = 0.f;
	batchedMesh.batchedInfos[0].rotation[1] = -0.f;
	batchedMesh.batchedInfos[0].scale[0] = 1.f;
	batchedMesh.batchedInfos[0].scale[1] = 0.1f;
	batchedMesh.batchedInfos[0].scale[2] = 1.f;
	batchedMesh.batchedInfos[0].materialIndex = 10;
	batchedMesh.batchedInfos[1].position[0] = 0.f;
	batchedMesh.batchedInfos[1].position[1] = -1.815f;
	batchedMesh.batchedInfos[1].position[2] = -0.f;
	batchedMesh.batchedInfos[1].rotation[1] = 0.75f;
	batchedMesh.batchedInfos[1].scale[0] = 1.5f;
	batchedMesh.batchedInfos[1].scale[1] = 1.5f;
	batchedMesh.batchedInfos[1].scale[2] = 1.5f;

	batchedMesh.batchedInfos[1].materialIndex = 11;
	


	std::vector<Material> materialArray = CreateMaterialArray(13+bounds[0]*bounds[1]*bounds[2]);
	materialArray[10].emissive = 3.0f;
	materialArray[10].color = { 1,1,1,1 };
	materialArray[11].color = Vector4(1.f, 0.75f, 0.25f, 1.f);
	materialArray[11].roughness =0.0001f;
	materialArray[12].color = Vector4(1.f, 1, 1, 1.f);
	//wall
	float wallRoughness = 0.001f;
	float otherColor = wallRoughness<0.9? 1 - wallRoughness:0.1f;
	materialArray[0].color = Vector4(1.0f,otherColor,otherColor, 1.f);
	materialArray[2].color = Vector4(otherColor ,1.0f,otherColor, 1.f);
	materialArray[1].color = Vector4(otherColor,otherColor, 1.0f, 1.f);
	materialArray[3].color = Vector4(1, 1, 1, 1.f);
	materialArray[0].roughness = wallRoughness;
	materialArray[2].roughness = wallRoughness;
	materialArray[1].roughness = wallRoughness;
	materialArray[3].roughness = wallRoughness;


	//instantiate more meshes for bragging rights
	if (true){
		InstantiateWalls(&batchedMesh.batchedInfos, walls, dimensions, centerWall, 0);
		batchedMesh.batchedInfos[2].materialIndex = 0;
		batchedMesh.batchedInfos[3].materialIndex = 1;
		batchedMesh.batchedInfos[4].materialIndex = 12;
		batchedMesh.batchedInfos[5].materialIndex = 12;
		batchedMesh.batchedInfos[6].materialIndex = 3;
		batchedMesh.batchedInfos[7].materialIndex = 2;
	}
	if (false) {
		InstantiateMeshes(&batchedMesh.batchedInfos, bounds, dimensions, center, 1);
		for (unsigned int i = 8; i < batchedMesh.batchedInfos.size(); i++) {
			batchedMesh.batchedInfos[i].materialIndex = i + 5;
		}
		for (unsigned int i = 8; i < batchedMesh.batchedInfos.size(); i++) {
			materialArray[batchedMesh.batchedInfos[i].materialIndex].roughness = floor(static_cast <float>(rand()) / static_cast <float>(RAND_MAX)*1);
		}
		materialArray[13].emissive = 4.f;
		materialArray[16].emissive = 4.f;
		materialArray[24].emissive = 4.f;
		materialArray[26].emissive = 4.f;
		materialArray[13].color = { 0.1,1.0,1.,1 };
		materialArray[16].color = {1,0.3,1,1};
		materialArray[24].color = { 0.3,0.75,1,1 };
	}

	//PrintMesh(mesh[0]);
	DrawBatchedMesh(batchedMesh,materialArray);
	return 0;
}