﻿#if defined (__APPLE__)
#define GLFW_INCLUDE_GLCOREARB
#define GL_SILENCE_DEPRECATION
#else
#define GLEW_STATIC
#include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.hpp"
#include "Model3D.hpp"
#include "Camera.hpp"
#include "SkyBox.hpp"
#include <iostream>

int glWindowWidth = 1024;
int glWindowHeight = 768;
int retina_width, retina_height;
GLFWwindow* glWindow = NULL;

const unsigned int SHADOW_WIDTH = 2048;
const unsigned int SHADOW_HEIGHT = 2048;

glm::mat4 model;
GLuint modelLoc;
glm::mat4 view;
GLuint viewLoc;
glm::mat4 projection;
GLuint projectionLoc;
glm::mat3 normalMatrix;
GLuint normalMatrixLoc;
glm::mat4 lightRotation;

glm::mat4 hondaModel;
GLuint hondaModelLoc;

glm::mat3 honda_normalMatrix;
GLuint honda_normalMatrixLoc;

glm::mat4 parking_lotModel;
GLuint parking_lotModelLoc;

glm::mat3 parking_lot_normalMatrix;
GLuint parking_lot_normalMatrixLoc;

struct PointLight {
	glm::vec3 position;
	glm::vec3 color;
	float constant;
	float linear;
	float quadratic;
};

std::vector<PointLight> pointLights = {
	{ glm::vec3(-17.1872f, 6.7f, -4.89938f), glm::vec3(1.0f, 1.0f, 0.8f), 1.0f, 0.09f, 0.032f }, // First lamp
	{ glm::vec3(-0.638713f, 6.7f, -5.18755f), glm::vec3(1.0f, 0.9f, 0.6f), 1.0f, 0.09f, 0.032f }, // Second lamp
	{ glm::vec3(15.7567f, 6.7f, -5.2f), glm::vec3(1.0f, 1.0f, 1.0f), 1.0f, 0.09f, 0.032f } // Third lamp
};

gps::Camera myCamera(
	glm::vec3(3.0f, 1.0f, 2.0f),   // Updated position: Closer and to the left
	glm::vec3(0.0f, 1.0f, 0.0f),   // Updated target: Focuses directly on the motorcycle
	glm::vec3(0.0f, 1.0f, 0.0f)    // Up vector remains the same
);

float cameraSpeed = 0.1f;
float angle;
float lightZ, lightY;
bool pressedKeys[1024];
float angleY = 0.0f;
GLfloat lightAngle;

bool autoDayCycle = false;  // Enables automatic day-night cycle
float timeOfDay = 8.0f;    // 0 = sunrise, 12 = noon, 24 = midnight
float daySpeed = 0.02f;    // Controls the speed of transition

gps::Model3D honda;
gps::Model3D parking_lot;
gps::Model3D lightCube;
gps::Model3D screenQuad;

gps::Shader myCustomShader;
gps::Shader lightShader;
gps::Shader screenQuadShader;
gps::Shader depthMapShader;

GLuint shadowMapFBO;
GLuint depthMapTexture;

bool showDepthMap;
bool isDay = true;
bool cameraLock = false;

gps::SkyBox skyBox;

gps::Shader skyboxShader;

glm::vec3 sunLightPosition;
glm::vec3 sunLightDir;
glm::vec3 sunLightColor;
GLint sunLightPositionLoc;
GLint sunLightDirLoc;
GLint sunLightColorLoc;

using namespace std;

GLenum glCheckError_(const char* file, int line) {
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	fprintf(stdout, "window resized to width: %d , and height: %d\n", width, height);
	//TODO	
}

void initSkyBox(bool isDay) {
	std::vector<const GLchar*> faces;

	if (!isDay) {
		// Night skybox
		faces.push_back("skybox/skyboxNight/posx.png"); // right
		faces.push_back("skybox/skyboxNight/negx.png"); // left
		faces.push_back("skybox/skyboxNight/posy.png"); // top
		faces.push_back("skybox/skyboxNight/negy.png"); // bottom
		faces.push_back("skybox/skyboxNight/posz.png"); // front
		faces.push_back("skybox/skyboxNight/negz.png"); // back
	}
	else {
		// Day skybox
		faces.push_back("skybox/skyboxDay/px.png"); // right
		faces.push_back("skybox/skyboxDay/nx.png"); // left
		faces.push_back("skybox/skyboxDay/py.png"); // top
		faces.push_back("skybox/skyboxDay/ny.png"); // bottom
		faces.push_back("skybox/skyboxDay/pz.png"); // front
		faces.push_back("skybox/skyboxDay/nz.png"); // back
	}

	skyBox.Load(faces);
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	if (key >= 0 && key < 1024) {
		if (action == GLFW_PRESS) {
			pressedKeys[key] = true;
		}
		else if (action == GLFW_RELEASE) {
			pressedKeys[key] = false;
		}
	}
	if (pressedKeys[GLFW_KEY_N] && action == GLFW_PRESS) {
		if (isDay) timeOfDay = 24.0f;  // Set time to night
		else timeOfDay = 8.0f;  // Set time to day
		
	}
	if (pressedKeys[GLFW_KEY_L] && action == GLFW_PRESS) {
		cameraLock = !cameraLock;
	}
	if (pressedKeys[GLFW_KEY_C] && action == GLFW_PRESS) {
		if (autoDayCycle) timeOfDay = 12.0f;  // Reset time to noon
		autoDayCycle = !autoDayCycle;  // Toggle automatic cycle
	}
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {

}

void processMovement() {
	cout << "Camera position: " << myCamera.getCameraPosition().x << " " << myCamera.getCameraPosition().y << " " << myCamera.getCameraPosition().z << endl;
	if (!cameraLock) {
		if (pressedKeys[GLFW_KEY_W]) {
			myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
		}
		if (pressedKeys[GLFW_KEY_S]) {
			myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
		}
		if (pressedKeys[GLFW_KEY_A]) {
			myCamera.move(gps::MOVE_LEFT, cameraSpeed);
		}
		if (pressedKeys[GLFW_KEY_D]) {
			myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
		}
		if (pressedKeys[GLFW_KEY_UP]) {
			myCamera.rotate(cameraSpeed * 10.0f, 0.0f);
		}

		if (pressedKeys[GLFW_KEY_DOWN]) {
			myCamera.rotate(-cameraSpeed * 10.0f, 0.0f);
		}

		if (pressedKeys[GLFW_KEY_LEFT]) {
			myCamera.rotate(0.0f, cameraSpeed * 10.0f);
		}

		if (pressedKeys[GLFW_KEY_RIGHT]) {
			myCamera.rotate(0.0f, -cameraSpeed * 10.0f);
		}
	}
	else {
		if (pressedKeys[GLFW_KEY_Q]) {
			angle -= 1.0f; // Rotate left
		}
		if (pressedKeys[GLFW_KEY_E]) {
			angle += 1.0f; // Rotate right
		}

		float radius = 5.0f; // Distance of camera from motorcycle
		float camX = sin(glm::radians(angle)) * radius;
		float camZ = cos(glm::radians(angle)) * radius;

		myCamera.setCameraPosition(glm::vec3(camX, 1.0f, camZ));
		myCamera.setCameraTarget(glm::vec3(0.0f, 1.0f, 0.0f)); // Focus on motorcycle
	}

	// **🔹 Always update the view matrix**
	view = myCamera.getViewMatrix();
	myCustomShader.useShaderProgram();
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	// **🔹 Update normal matrix**
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
}

void updateDayNightCycle() {
	if (autoDayCycle) {
		timeOfDay += daySpeed;
		if (timeOfDay >= 24.0f) timeOfDay = 0.0f;  // Reset at midnight
	}

	// Compute sun's position angle in a full cycle (0 to 2π radians)
	float sun_angle = glm::radians(((timeOfDay - 6.0f) / 24.0f) * 360.0f);

	if (isDay) {
		lightZ = cos(sun_angle) * 10.0f;  // Moves in a circular motion along Z
		lightY = sin(sun_angle) * 10.0f;  // Moves up/down (highest at noon, lowest at midnight)
	}
	else {
		lightZ = cos(sun_angle + 3.1416f) * 10.0f;  // Moves in a circular motion along Z
		lightY = sin(sun_angle + 3.1416f) * 10.0f;  // Moves up/down (highest at noon, lowest at midnight)
	}

	sunLightDir = glm::vec3(0.0f, lightY, lightZ);

	// Reduce overall brightness at night
	float lightIntensity = glm::clamp(sin(sun_angle) * 1.2f, 0.1f, 1.0f); // Lower min to make night darker

	glm::vec3 dayColor = glm::vec3(1.0f, 0.95f, 0.8f);  // Warm yellowish-white
	glm::vec3 nightColor = glm::vec3(0.05f, 0.05f, 0.15f); // Almost black with a slight blue tint
	sunLightColor = glm::mix(nightColor, dayColor, lightIntensity);

	// Update shaders
	myCustomShader.useShaderProgram();
	glUniform3fv(sunLightDirLoc, 1, glm::value_ptr(glm::normalize(sunLightDir)));
	glUniform3fv(sunLightColorLoc, 1, glm::value_ptr(sunLightColor));

	// **Change skybox when transitioning between day/night**
	if (timeOfDay < 6.0f || timeOfDay > 18.0f) {
		if (isDay) {
			isDay = false;
			initSkyBox(false);  // Load night skybox
		}
	}
	else {
		if (!isDay) {
			isDay = true;
			initSkyBox(true);  // Load day skybox
		}
	}
}


bool initOpenGLWindow()
{
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


	//window scaling for HiDPI displays
	glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

	//for sRBG framebuffer
	glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);

	//for antialising
	glfwWindowHint(GLFW_SAMPLES, 4);

	glWindow = glfwCreateWindow(glWindowWidth, glWindowHeight, "OpenGL Shader Example", NULL, NULL);
	if (!glWindow) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return false;
	}

	glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
	glfwSetKeyCallback(glWindow, keyboardCallback);
	glfwSetCursorPosCallback(glWindow, mouseCallback);
	//glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwMakeContextCurrent(glWindow);

	glfwSwapInterval(1);

#if not defined (__APPLE__)
	// start GLEW extension handler
	glewExperimental = GL_TRUE;
	glewInit();
#endif

	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
	const GLubyte* version = glGetString(GL_VERSION); // version as a string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	return true;
}

void initOpenGLState()
{
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	glViewport(0, 0, retina_width, retina_height);

	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise

	glEnable(GL_FRAMEBUFFER_SRGB);
}

void initObjects() {
	honda.LoadModel("models/honda/ImageToStl.com_honda_nr750_1994.obj");
	parking_lot.LoadModel("models/parking_lot/ImageToStl.com_parking_lot.obj");
	lightCube.LoadModel("models/cube/cube.obj");
	screenQuad.LoadModel("models/quad/quad.obj");
}

void initShaders() {
	myCustomShader.loadShader("shaders/basic.vert", "shaders/basic.frag");
	myCustomShader.useShaderProgram();
	lightShader.loadShader("shaders/lightCube.vert", "shaders/lightCube.frag");
	lightShader.useShaderProgram();
	screenQuadShader.loadShader("shaders/screenQuad.vert", "shaders/screenQuad.frag");
	screenQuadShader.useShaderProgram();
	depthMapShader.loadShader("shaders/depthMap.vert", "shaders/depthMap.frag");
	depthMapShader.useShaderProgram();
	skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
	skyboxShader.useShaderProgram();
}



void initUniforms() {
	myCustomShader.useShaderProgram();

	model = glm::mat4(1.0f);
	modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	hondaModel = glm::mat4(1.0f);
	hondaModelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");
	glUniformMatrix4fv(hondaModelLoc, 1, GL_FALSE, glm::value_ptr(hondaModel));

	parking_lotModel = glm::mat4(1.0f);
	parking_lotModelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");
	glUniformMatrix4fv(parking_lotModelLoc, 1, GL_FALSE, glm::value_ptr(parking_lotModel));

	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myCustomShader.shaderProgram, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	normalMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	honda_normalMatrix = glm::mat3(glm::inverseTranspose(view * hondaModel));
	honda_normalMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");
	glUniformMatrix3fv(honda_normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(honda_normalMatrix));

	parking_lot_normalMatrix = glm::mat3(glm::inverseTranspose(view * parking_lotModel));
	parking_lot_normalMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");
	glUniformMatrix3fv(parking_lot_normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(parking_lot_normalMatrix));

	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	//set the light direction (direction towards the light)
	sunLightDir = glm::vec3(0.0f, 10.0f, 1.0f);
	lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	sunLightDirLoc = glGetUniformLocation(myCustomShader.shaderProgram, "sunLightDir");
	glUniform3fv(sunLightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * sunLightDir));

	//set light color
	sunLightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
	sunLightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "sunLightColor");
	glUniform3fv(sunLightColorLoc, 1, glm::value_ptr(sunLightColor));

	for (int i = 0; i < pointLights.size(); i++) {
		std::string base = "pointLights[" + std::to_string(i) + "]";
		glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, (base + ".position").c_str()), 1, glm::value_ptr(pointLights[i].position));
		glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, (base + ".color").c_str()), 1, glm::value_ptr(pointLights[i].color));
		glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, (base + ".constant").c_str()), pointLights[i].constant);
		glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, (base + ".linear").c_str()), pointLights[i].linear);
		glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, (base + ".quadratic").c_str()), pointLights[i].quadratic);
	}

	lightShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	
}

void initFBO() {
	//TODO - Create the FBO, the depth texture and attach the depth texture to the FBO
	glGenFramebuffers(1, &shadowMapFBO);
	glGenTextures(1, &depthMapTexture);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_HEIGHT, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);

	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

glm::mat4 computeLightSpaceTrMatrix() {
	lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));

	// Adjust the light position to cover both objects
	glm::vec3 lightPosition = glm::vec3(lightRotation * glm::vec4(sunLightDir, 1.0f)) + glm::vec3(0.0f, 5.0f, 0.0f);

	// Ensure shadows are computed for both objects by adjusting light projection
	glm::mat4 lightView = glm::lookAt(lightPosition, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	const GLfloat near_plane = 0.1f, far_plane = 100.0f; // Increased far_plane to cover parking lot
	glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);

	glm::mat4 lightSpaceTrMatrix = lightProjection * lightView;
	return lightSpaceTrMatrix;
}


void drawObjects(gps::Shader shader, bool depthPass) {

	shader.useShaderProgram();

	skyBox.Draw(skyboxShader, view, projection);

	model = hondaModel;
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	// Compute normal matrix for accurate lighting and shadow calculations
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	// Draw the honda
	honda.Draw(shader);

	// Draw the parking lot (Ensure it's part of the shadow pass)
	model = parking_lotModel;
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	parking_lot.Draw(shader);
}


void renderScene() {
	// depth maps creation pass
	depthMapShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"),
		1, GL_FALSE, glm::value_ptr(computeLightSpaceTrMatrix()));

	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	drawObjects(depthMapShader, true);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Render depth map on screen (toggle with M key)
	if (showDepthMap) {
		glViewport(0, 0, retina_width, retina_height);
		glClear(GL_COLOR_BUFFER_BIT);
		screenQuadShader.useShaderProgram();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glUniform1i(glGetUniformLocation(screenQuadShader.shaderProgram, "depthMap"), 0);
		glDisable(GL_DEPTH_TEST);
		screenQuad.Draw(screenQuadShader);
		glEnable(GL_DEPTH_TEST);
	}
	else {
		// Final scene rendering pass (with shadows)
		glViewport(0, 0, retina_width, retina_height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		myCustomShader.useShaderProgram();

		view = myCamera.getViewMatrix();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

		lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniform3fv(sunLightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * sunLightDir));

		// Bind shadow map
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "shadowMap"), 3);

		glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightSpaceTrMatrix"),
			1, GL_FALSE, glm::value_ptr(computeLightSpaceTrMatrix()));

		drawObjects(myCustomShader, false);

		// **🔹 Draw a small white cube at the sun position**
		lightShader.useShaderProgram();
		glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

		model = lightRotation;
		model = glm::translate(model, sunLightDir + glm::vec3(0.5f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));
		glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		lightCube.Draw(lightShader);

		glm::vec3 boostedColor = isDay ? glm::vec3(1.0f, 1.0f, 1.0f) : glm::vec3(1.2f, 1.0f, 0.7f);  // Soft glow at night


		myCustomShader.useShaderProgram();
		for (int i = 0; i < pointLights.size(); i++) {
			std::string base = "pointLights[" + std::to_string(i) + "]";
			glm::vec3 adjustedColor = pointLights[i].color * boostedColor;
			glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, (base + ".color").c_str()), 1, glm::value_ptr(adjustedColor));
		}

		// **🔹 Draw small cubes at point light positions**
		for (int i = 0; i < pointLights.size(); i++) {
			model = glm::mat4(1.0f);
			model = glm::translate(model, pointLights[i].position);
			model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f)); // Small glowing cube for point light
			glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
			lightCube.Draw(lightShader);
		}
	}
}

void cleanup() {
	glDeleteTextures(1, &depthMapTexture);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &shadowMapFBO);
	glfwDestroyWindow(glWindow);
	//close GL context and any other GLFW resources
	glfwTerminate();
}

int main(int argc, const char* argv[]) {

	if (!initOpenGLWindow()) {
		glfwTerminate();
		return 1;
	}

	initOpenGLState();
	initObjects();
	initShaders();
	initUniforms();
	initSkyBox(true);
	initFBO();

	glCheckError();

	while (!glfwWindowShouldClose(glWindow)) {
		processMovement();
		updateDayNightCycle();
		renderScene();

		glfwPollEvents();
		glfwSwapBuffers(glWindow);
	}

	cleanup();

	return 0;
}
