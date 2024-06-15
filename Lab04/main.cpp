#pragma warning(disable : 4305)
#pragma warning(disable : 4244)
#include "model.h"
#include <iostream>
#include <ctime>
using namespace std;


// Macro for indexing vertex buffer
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

#define PLANE "planeLarge.dae"
#define SPHERE "sphere.dae"
#define FOAM "foamTexture3.jpg"


#define RIGHT "s_right.jpg"
#define LEFT "s_left.jpg"
#define TOP "s_top.jpg"
#define BOTTOM "s_bottom.jpg"
#define FRONT "s_front.jpg"
#define BACK "s_back.jpg"


using namespace std;

int width = 1536.0;
int height = 960.0;
int tol = 100;

bool orthopersp;
bool keyStates[256];

float lastX = width / 2;
float lastY = height / 2;
float yaw = -90.0f;
float pitch = 0.0f;
glm::vec3 cameraPos = glm::vec3(0.0f, 3.0f, 0.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.2f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

//lightsource location
GLfloat x = 150.0f;
GLfloat y = 100.0f;
GLfloat z = -90.0f;

//Matrices
mat4 movement_matrix = identity_mat4();
mat4 camera_matrix = identity_mat4();
mat4 temp_matrix = identity_mat4();
mat4 scale_matrix = identity_mat4();

//for animation of the head and tail
mat4 animation_matrix = identity_mat4();

//animation for going around the scene
mat4 animation_matrix2 = identity_mat4(); //translate(identity_mat4(), vec3(15.0f, 1.0f, 0.0f));
mat4 temp_anim = identity_mat4();
mat4 temp_anim2 = identity_mat4();

//scaling matrices
vec3 scale_vec = vec3(1.2f, 1.2f, 1.2f); //scale vector
vec3 scale_vec_down = vec3(0.7f, 0.7f, 0.7f); //scale vector
vec3 scale_vec_non_uni = vec3(1.0f, 1.5f, 1.0f); //scale vector
vec3 scale_vec_non_uni_down = vec3(1.0f, 0.7f, 1.0f); //scale vector

//initialise the shader object
Shader shader_object3;
Shader shader_skybox;
Shader shader_light;
int shaderSpecifier;

//initialise the model objects
Model light1;
Model oceanmodel;

glm::vec3 lightColor;
glm::vec3 objectColor;

unsigned int skyboxVAO, skyboxVBO;
unsigned int cubemapTexture;


const unsigned GL_BASE_VERSION = 4;
const unsigned GL_SUB_VERSION = 1;
const unsigned WINDOW_WIDTH = 1280;
const unsigned WINDOW_HEIGHT = 720;
const float NEAR_PLANE = 0.1f;
const float FAR_PLANE = 500.0f;
const float CAM_FOV = 45.0f;
const unsigned SAMPLES = 8;
const unsigned TEXTURES_AMOUNT = 13;
const unsigned TESS_LEVEL = 1;
const float DEPTH = 0.11f;

unsigned int OCEANVAO;
float interpolateFactor = 0.0f;
double deltaTime = 0.0;
unsigned heightMap[TEXTURES_AMOUNT];
unsigned normalMap[TEXTURES_AMOUNT];
unsigned waterTex;
unsigned wavesNormalMap;
unsigned wavesHeightMap;

unsigned firstIndex = 0;
unsigned lastIndex = 1;
bool rotate = false;

float timeIt = 0.1;

//FPS Counting
std::time_t lastTime = std::time(0);
int nbFrames = 0;



float skyboxVertices[108] = {
	// positions          
	-1.0f,  1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	-1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f
};
vector<std::string> faces{
	RIGHT,
	LEFT,
	TOP,
	BOTTOM,
	FRONT,
	BACK
};

void setupgeneric(GLuint& shaderProgramID, glm::mat4 model, glm::mat4 view, glm::mat4 persp_proj, GLfloat spec_coef, int& matrix_location) {
	//sets up some basic values in a generic shader passed in

	matrix_location = glGetUniformLocation(shaderProgramID, "model");
	glUniformMatrix4fv(glGetUniformLocation(shaderProgramID, "model"), 1, GL_FALSE, &model[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shaderProgramID, "view"), 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shaderProgramID, "proj"), 1, GL_FALSE, &persp_proj[0][0]);

	//setting location of the lightsource 
	GLint change1 = glGetUniformLocation(shaderProgramID, "light_position");
	GLfloat light1[3] = { x, y, z };
	glUniform3fv(change1, 1, &light1[0]);

	//change the specular coefficient
	GLint change3 = glGetUniformLocation(shaderProgramID, "specular_coef");
	glUniform1f(change3, spec_coef);

	GLint change4 = glGetUniformLocation(shaderProgramID, "cameraPos");
	glUniform3fv(change4, 1, &cameraPos[0]);


}

void materialproperties(GLuint& shaderProgramID) {
	//changes material properties.
	GLint change11 = glGetUniformLocation(shaderProgramID, "lightColor");
	GLfloat vec11[3] = { lightColor.x, lightColor.y, lightColor.z };
	glUniform3fv(change11, 1, &vec11[0]);

	GLint change12 = glGetUniformLocation(shaderProgramID, "objectColor");
	GLfloat vec12[3] = { objectColor.x, objectColor.y, objectColor.z };
	glUniform3fv(change12, 1, &vec12[0]);
}
void displayObject(Shader shader_object, Model modelObject, mat4 model, glm::mat4 model_shader, glm::mat4 view, glm::mat4 persp_proj, int matrix_location) {

	glActiveTexture(GL_TEXTURE0);
	glUseProgram(shader_object.shaderProgramID);
	setupgeneric(shader_object.shaderProgramID, model_shader, view, persp_proj, 90.0, matrix_location);
	materialproperties(shader_object.shaderProgramID);
	glBindTexture(GL_TEXTURE_2D, modelObject.texture);
	glUseProgram(shader_object.shaderProgramID);
	glBindVertexArray(modelObject.modelvao);
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, model.m);
	glDrawArrays(GL_TRIANGLES, 0, modelObject.mesh_data.mPointCount);
	glBindVertexArray(0);

}

void loadTexture(unsigned& textureId, const std::string& fileName)
{
	int width, height, nrChannels;
	unsigned char* data = stbi_load(fileName.c_str(), &width, &height, &nrChannels, 0);
	if (!data)
	{
		stbi_image_free(data);
		throw std::runtime_error(std::string("Failed to load texture from file: " + fileName + "."));
	}

	GLenum format = GL_RGB;
	if (nrChannels == 1)
		format = GL_RED;
	else if (nrChannels == 3)
		format = GL_RGB;
	else if (nrChannels == 4)
		format = GL_RGBA;

	glBindTexture(GL_TEXTURE_2D, textureId);
	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);

	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 8);
}
void renderSkybox(glm::mat4 view, glm::mat4 persp_proj){
	//Skybox
	glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content

	glUseProgram(shader_skybox.shaderProgramID);//use the skybox shader
	view = glm::mat4(glm::mat3(glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp))); // undo translation from view matrix (otherwise its just a box around origin)
	glUniformMatrix4fv(glGetUniformLocation(shader_skybox.shaderProgramID, "view"), 1, GL_FALSE, &view[0][0]); //similar to the uniformmatrix4fv above, but taking into account the view and projection
	glUniformMatrix4fv(glGetUniformLocation(shader_skybox.shaderProgramID, "projection"), 1, GL_FALSE, &persp_proj[0][0]);

	// This bit is the part that actually renders the skybox cube
	glBindVertexArray(skyboxVAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0); //breaks existing vertex array binding

	glDepthFunc(GL_LESS); // set depth function back to default

}

void renderOcean(Shader shader_object, Model modelObject, mat4 model, glm::mat4 model_shader, glm::mat4 view, glm::mat4 persp_proj, int matrix_location, int interpolate) {

	//Set the variables for the various uniform values in the shaders
	glActiveTexture(GL_TEXTURE0);
	glUseProgram(shader_object.shaderProgramID);
	setupgeneric(shader_object.shaderProgramID, model_shader, view, persp_proj, 90.0, matrix_location);
	materialproperties(shader_object.shaderProgramID);

	glm::mat4 modelOCEAN = glm::scale(glm::mat4(10.0f), glm::vec3(1000.0f));
	glUniformMatrix4fv(glGetUniformLocation(shader_object.shaderProgramID, "model"), 1, GL_FALSE, &modelOCEAN[0][0]);

	GLint change = glGetUniformLocation(shader_object.shaderProgramID, "heightMap1");
	glUniform1i(change, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, heightMap[firstIndex]);

	change = glGetUniformLocation(shader_object.shaderProgramID, "heightMap2");
	glUniform1i(change, 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, heightMap[lastIndex]);

	change = glGetUniformLocation(shader_object.shaderProgramID, "normalMap1");
	glUniform1i(change, 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, normalMap[firstIndex]);

	change = glGetUniformLocation(shader_object.shaderProgramID, "normalMap2");
	glUniform1i(change, 3);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, normalMap[lastIndex]);

	change = glGetUniformLocation(shader_object.shaderProgramID, "normalMap2");
	glUniform1i(change, 3);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, normalMap[lastIndex]);

	GLint change12 = glGetUniformLocation(shader_object.shaderProgramID, "direction");
	GLfloat vec12[3] = { x, y, z };
	glUniform3fv(change12, 1, &vec12[0]);


	change = glGetUniformLocation(shader_object.shaderProgramID, "water");
	glUniform1i(change, 4);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, waterTex);

	change = glGetUniformLocation(shader_object.shaderProgramID, "wavesHeightMap");
	glUniform1i(change, 5);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, wavesHeightMap);

	change = glGetUniformLocation(shader_object.shaderProgramID, "wavesNormalMap");
	glUniform1i(change, 6);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, wavesNormalMap);


	GLint change19 = glGetUniformLocation(shader_object.shaderProgramID, "interpolateFactor");
	glUniform1f(change19, interpolateFactor);


	// If interpolate is enabled, update the interpolateFactor and wavesOffset uniforms in the shader
	if (interpolate == 1) { //allows for updates to the wave normals and geometry
		if (interpolateFactor >= 1)
		{
			// When the interpolation is complete, reset the interpolateFactor and update the texture indices
			interpolateFactor = 0.0f;
			if (lastIndex == TEXTURES_AMOUNT - 1)
			{
				firstIndex = 0;
				lastIndex = 1;
			}
			else
			{
				++firstIndex;
				++lastIndex;
			}
		}
		else
		{
			// Increase the interpolateFactor and update the interpolateFactor uniform in the shader
			interpolateFactor += 0.04;// *deltaTime;
			change = glGetUniformLocation(shader_object.shaderProgramID, "interpolateFactor");
			glUniform1f(change, interpolateFactor);
		}

		// Increase the wavesOffset and update the wavesOffset uniform in the shader
		static float offset = 0.0f;
		if (offset >= INT_MAX - 2) {
			offset = 0;
			deltaTime = 0.0;
		}
		offset += 0.002;
		GLint change2 = glGetUniformLocation(shader_object.shaderProgramID, "wavesOffset");
		glUniform1f(change2, offset);
	}

	glUseProgram(shader_object.shaderProgramID);
	glBindVertexArray(modelObject.modelvao);
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, model.m);
	glDrawArrays(GL_TRIANGLES, 0, modelObject.mesh_data.mPointCount);
	glBindVertexArray(0);
}

void display() {
	std::time_t currentTime = std::time(0);
	if (currentTime - lastTime >= 1.0) { // If last prinf() was more than 1 sec ago
		// printf and reset timer
		printf("%f ms/frame\n", 1000.0 / double(nbFrames));
		nbFrames = 0;
		lastTime += 1.0;
	}
	nbFrames++;
	glEnable(GL_DEPTH_CLAMP);
	// tell GL to only draw onto a pixel if the shape is closer to the viewer
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//matrices
	mat4 model = identity_mat4();
	mat4 seabed_matrix = identity_mat4();
	mat4 light_matrix = identity_mat4();
	mat4 planeMatrix = identity_mat4();
	light_matrix = translate(light_matrix, vec3(x, y, z));
	seabed_matrix = translate(seabed_matrix, vec3(0.0f, -2.0f, 0.0f));
	glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
	glm::mat4 persp_proj = glm::perspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 1000.0f);
	if (orthopersp) { persp_proj = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, -100.0f, 100.0f); };

	GLfloat light_vec[3] = { x, y, z }; //setting location of the lightsource
	int matrix_location;

	//setting location of the lightsourc
	//Lightsources
	glActiveTexture(GL_TEXTURE0);
	glUseProgram(shader_light.shaderProgramID);
	glm::mat4 model_light = glm::mat4(1.0f);
	model_light = glm::translate(model_light, glm::vec3(x, y, z));
	matrix_location = glGetUniformLocation(shader_light.shaderProgramID, "model");
	glUniformMatrix4fv(glGetUniformLocation(shader_light.shaderProgramID, "model"), 1, GL_FALSE, &model_light[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shader_light.shaderProgramID, "view"), 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shader_light.shaderProgramID, "proj"), 1, GL_FALSE, &persp_proj[0][0]);
	GLfloat light_vec3[3] = { 1.0, 1.0, 1.0 }; //colour
	glUniform3fv(glGetUniformLocation(shader_light.shaderProgramID, "lightcolour"), 1, &light_vec3[0]);
	glBindVertexArray(light1.modelvao);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, light1.texture);
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, light_matrix.m);
	glDrawArrays(GL_TRIANGLES, 0, light1.mesh_data.mPointCount);
	glBindVertexArray(0);

	//setting up
	temp_anim2 = model;
	model = model * movement_matrix * scale_matrix;
	mat4 modelOCEAN = scale(model, vec3(10.0f, 10.0f, 10.0f));
	model = model * animation_matrix2;
	model = model * temp_anim;
	model = translate(model, vec3(2.0f, 0.0f, 2.0f));
	lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
	glm::mat4 model_shader = glm::mat4(1.0f);
	objectColor = glm::vec3(0.1f, 0.4f, 0.49f);

	//render the ocean
	renderOcean(shader_object3, oceanmodel, translate(modelOCEAN, vec3(0.0f, 0.0f, 0.0f)), model_shader, view, persp_proj, matrix_location, 1);

	//resetting the matrices for the next fish
	temp_anim = identity_mat4();
	model = temp_anim2;

	//Skybox
	renderSkybox(view, persp_proj);

	glutSwapBuffers();
}

void MoveCamera(float delta) {
	float speed = 5.0f * delta;

	if (keyStates['i']) {
		cameraPos += speed * cameraFront;
	}
	if (keyStates['j']) {
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * speed;
	}
	if (keyStates['k']) {
		cameraPos -= speed * cameraFront;
	}
	if (keyStates['l']) {
		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * speed;
	}
	if (keyStates['f']) {
		yaw += 0.5;
	}
	if (keyStates['c']) {
		yaw -= 0.5;
	}
	if (keyStates['v']) {
		pitch += 0.5;
	}
	if (keyStates['b']) {
		pitch -= 0.5;
	}
	if (keyStates['n']) {
		cameraPos.y += 0.1;
	}
	if (keyStates['m']) {
		cameraPos.y -= 0.1;
	}
}

void movement() {

	//model controls
	if (keyStates['w']) {
		movement_matrix = translate(movement_matrix, vec3(0.0f, 0.0f, -0.5f));
	}
	if (keyStates['a']) {
		movement_matrix = translate(movement_matrix, vec3(-0.5f, 0.0f, 0.0f));
	}
	if (keyStates['s']) {
		movement_matrix = translate(movement_matrix, vec3(0.0f, 0.0f, 0.5f));
	}
	if (keyStates['d']) {
		movement_matrix = translate(movement_matrix, vec3(0.5f, 0.0f, 0.0f));
	}
	if (keyStates['q']) {
		movement_matrix = translate(movement_matrix, vec3(0.0f, -0.5f, 0.0f));
	}
	if (keyStates['e']) {
		movement_matrix = translate(movement_matrix, vec3(0.0f, 0.5f, 0.0f));
	}

	//object rotation
	if (keyStates['x']) {
		temp_matrix = movement_matrix;
		movement_matrix = translate(identity_mat4(), vec3(0.0f, 0.0f, 0.0f));
		movement_matrix = rotate_x_deg(movement_matrix, 10.0f);
		movement_matrix = temp_matrix * movement_matrix;
	}
	if (keyStates['y']) {
		temp_matrix = movement_matrix;
		movement_matrix = translate(identity_mat4(), vec3(0.0f, 0.0f, 0.0f));
		movement_matrix = rotate_y_deg(movement_matrix, 10.0f);
		movement_matrix = temp_matrix * movement_matrix;
	}
	if (keyStates['z']) {
		temp_matrix = movement_matrix;
		movement_matrix = translate(identity_mat4(), vec3(0.0f, 0.0f, 0.0f));
		movement_matrix = rotate_z_deg(movement_matrix, 10.0f);
		movement_matrix = temp_matrix * movement_matrix;
	}

	//light control
	if (keyStates['=']) {
		x = x + 0.5f;
	}
	if (keyStates['-']) {
		x = x - 0.5f;
	}
	if (keyStates['0']) {
		z = z - 0.5f;
	}
	if (keyStates['9']) {
		z = z + 0.5f;
	}

	//scaling
	if (keyStates['3']) {
		orthopersp = false;
	}
	if (keyStates['4']) {
		orthopersp = true;
	}

}

void updateScene() {

	// Wait until at least 16ms passed since start of last frame (Effectively caps framerate at ~60fps)
	static DWORD  last_time = 0;
	DWORD  curr_time = timeGetTime();
	float  delta = (curr_time - last_time) * 0.001f;
	if (delta > 0.03f)
		delta = 0.03f;
	last_time = curr_time;

	// Draw the next frame
	glutPostRedisplay();

	MoveCamera(delta);
	movement();

	animation_matrix2 = rotate_y_deg(animation_matrix2, -0.1f);
	timeIt = timeIt + 0.01;
	deltaTime = deltaTime + 0.001;
}

//from:https://github.com/JoeyDeVries/LearnOpenGL/blob/master/src/4.advanced_opengl/6.1.cubemaps_skybox/cubemaps_skybox.cpp
unsigned int loadCubemap(vector<std::string> faces) {
	//creates and binds textureID as a cubemap
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++) //loads in the vectors passed in similar to my model texture function
	{
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	//parameters
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}

void init()
{
	// Set up the shaders
	shader_object3.shaderProgramID = shader_object3.CompileShaders(0);
	shader_skybox.shaderProgramID = shader_skybox.CompileShaders(1);
	shader_light.shaderProgramID = shader_light.CompileShaders(2);

	//set up models
	oceanmodel.setup(PLANE, FOAM, FOAM, shader_object3.shaderProgramID);
	light1.setup(SPHERE, FOAM, FOAM, shader_light.shaderProgramID);

	// skybox VAO
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	cubemapTexture = loadCubemap(faces);
	shaderSpecifier = 7;

	//OCEAN INITIATION
	glGenVertexArrays(1, &OCEANVAO);
	glBindVertexArray(OCEANVAO);
	glGenTextures(TEXTURES_AMOUNT, heightMap);
	glGenTextures(TEXTURES_AMOUNT, normalMap);
	glGenTextures(1, &waterTex);

	// Loop through each corresponding height and normal map and load them
	for (unsigned i = 0; i < TEXTURES_AMOUNT; ++i)
	{
		std::string num = std::to_string(i + 1);

		loadTexture(heightMap[i], "Shaders/textures/heights/" + num + ".png");
		loadTexture(normalMap[i], "Shaders/textures/normals/" + num + ".png");
	}

	// Load additional textures for water and waves
	loadTexture(waterTex, FOAM);
	loadTexture(wavesNormalMap, "Shaders/textures/wavesNormal.jpg");
	loadTexture(wavesHeightMap, "Shaders/textures/wavesHeight.jpg");

	glm::mat4 modelOCEAN = glm::scale(glm::mat4(1.0f), glm::vec3(100.0f));
	glm::vec3 viewPos = glm::vec3(1.0f);
	glm::mat4 view = glm::lookAt(
		viewPos,
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f)
	);

	double lastFrame = 0;
	double actualFrame = 0;
	float radius = 75.0f;

	float camX = sin(timeIt * 0.5) * radius;
	float camZ = cos(timeIt * 0.5) * radius;
	viewPos = glm::vec3(camX, 50.0f, camZ);

}

void KeyDown(unsigned char key, int x, int y) {
	keyStates[key] = true;
}
void KeyUp(unsigned char key, int x, int y) {
	keyStates[key] = false;
}

void MouseMovement(int x, int y) {

	if (x < tol || x > width - tol || y < tol || y > height - tol) {
		glutWarpPointer(width / 2, height / 2);
	}
	else {
		float xoffset = x - lastX;
		float yoffset = lastY - y;

		glutWarpPointer(lastX, lastY);

		float sensitivity = 0.1f;
		xoffset *= sensitivity;
		yoffset *= sensitivity;

		yaw += xoffset;
		pitch += yoffset;

		if (pitch > 89.0f) {
			pitch = 89.0f;
		}
		else if (pitch < -89.0f) {
			pitch = -89.0f;
		}

		glm::vec3 front;
		front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		front.y = sin(glm::radians(pitch));
		front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		cameraFront = glm::normalize(front);
	}
}


int main(int argc, char** argv) {

	// Set up the window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(width, height);
	glutCreateWindow("Viewport Teapots");
	// Tell glut where the display function is
	glutDisplayFunc(display);
	glutIdleFunc(updateScene);
	//glutKeyboardFunc(keypress);
	glutKeyboardFunc(KeyDown);
	glutKeyboardUpFunc(KeyUp);
	glutPassiveMotionFunc(MouseMovement);
	glutSetCursor(GLUT_CURSOR_NONE);

	// A call to glewInit() must be done after glut is initialized!
	glewExperimental = GL_TRUE; //for non-lab machines, this line gives better modern GL support
	GLenum res = glewInit();
	// Check for any errors
	if (res != GLEW_OK) {
		fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
		return 1;
	}
	// Set up your objects and shaders
	init();
	// Begin infinite event loop
	glutMainLoop();
	return 0;
}




