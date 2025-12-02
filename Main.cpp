#include<iostream>

#include"glad/glad.h"
#include"GLFW/glfw3.h"
#include"shaderClass.h"
#include"meshParser.h"
#include <string>
#include <vector>
#include <cmath>
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
struct Vector3
{
	GLfloat x, y, z;
}; 
struct Vector4
{
	GLfloat x, y, z,w;
};
struct Material {
	Vector4 color;
	float roughness;
	float metallic;
	float emissive;
	float refractiveIndex;
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
		glUniform1f(halffov, 75);
		glDispatchCompute(ceil(SCREEN_WIDTH / 32), ceil(SCREEN_HEIGHT / 32), 1);
		glMemoryBarrier(GL_ALL_BARRIER_BITS);
		screenShader.Activate();
		glBindTextureUnit(0, screenTex);
		const char* texture = "screen";
		screenShader.GetTexture(texture);
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
		screenShader.GetTexture(texture);
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
void DrawMesh() {
	std::vector<Mesh> mesh=ScanForMesh("Test.mesh");
	Mesh m = mesh[0];
	for (unsigned int i = 0; i < m.vertices.size(); i++)
	{
		std::cout << "Vertex " << (int)i << ": ";
		PrintArray(m.vertices[i].position,3);
	}
	for (unsigned int i= 0; i < m.normals.size(); i++) {
		std::cout << "Normal" << (int)i;
		PrintArray(m.normals[i].normal,3);
	}
	for (unsigned int i = 0; i < m.uvs.size(); i++) {
		std::cout << "UV" << (int)i << ':';
		PrintArray(m.uvs[i].uv,2);

	}
	for (unsigned int i = 0; i < m.faces.size(); i++) {
		std::cout << "Face" << (int)i << " ";
		for (char j = 0; j < m.faces[i].indicesGroups.size(); j++) {
			PrintArray(m.faces[i].indicesGroups[j].indicies,3);
		}
		std::cout << "\n";
	}
}
int main()
{
	float bound[3] = { 10.f,3.f,10.f };
	int amount =20;
	std::vector<Sphere> spheres=CreateSphereArray(bound, &amount);
	spheres[0].position.x = -15;
	spheres[0].color = { 0.f,0.f,0.f,1.f };
	spheres[0].position.z = -3;
	spheres[0].position.y =10;
	spheres[0].radius = 10;
	spheres[0].material.emissive = 4;
	spheres[1].position.y = -100.76;
	spheres[1].position.z = 0;
	spheres[1].radius = 100;
	spheres[1].material.roughness = 1;
	spheres[9].material.emissive = 4;
	//Draw(spheres);
	DrawMesh();
	return 0;
}