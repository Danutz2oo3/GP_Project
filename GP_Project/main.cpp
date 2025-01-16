#if defined (__APPLE__)
    #define GLFW_INCLUDE_GLCOREARB
    #define GL_SILENCE_DEPRECATION
#else
    #define GLEW_STATIC
    #include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>

#include <glm/glm.hpp> //core glm functionality
#include <glm/gtc/matrix_transform.hpp> //glm extension for generating common transformation matrices
#include <glm/gtc/matrix_inverse.hpp> //glm extension for computing inverse matrices
#include <glm/gtc/type_ptr.hpp> //glm extension for accessing the internal data structure of glm types

#include "Window.h"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"
#include <iostream>
#include "SkyBox.hpp"

// window
gps::Window myWindow;

// matrices for motorcycle
glm::mat4 motorcycle_model;
glm::mat3 motorcycle_normalMatrix;

// matrices for parking_lot
glm::mat4 parking_lot_model;
glm::mat3 parking_lot_normalMatrix;

glm::mat4 view;
glm::mat4 projection;

// light parameters
glm::vec3 lightDir;
glm::vec3 lightColor;
glm::vec3 lightPos;

// shader uniform locations
GLint motorcycle_modelLoc;
GLint motorcycle_normalMatrixLoc;

GLint parking_lot_modelLoc;
GLint parking_lot_normalMatrixLoc;


GLint viewLoc;
GLint lightDirLoc;
GLint lightColorLoc;
GLint projectionLoc;
GLint lightPosLoc;
GLint moonColorLoc;
GLint sunDirLoc;
GLint sunColorLoc;
GLint moonDirLoc;
GLint timeOfDayLoc;

// camera
gps::Camera myCamera(
    glm::vec3(3.0f, 1.0f, 2.0f),   // Updated position: Closer and to the left
    glm::vec3(0.0f, 1.0f, 0.0f),   // Updated target: Focuses directly on the motorcycle
    glm::vec3(0.0f, 1.0f, 0.0f)    // Up vector remains the same
);

GLfloat cameraSpeed = 0.1f;

GLboolean pressedKeys[1024];

bool cameraLock = true;
bool isDay = true;
bool autoDayCycle = false;

// models
gps::Model3D motorcycle;
GLfloat angle;

gps::Model3D parking_lot;

// shaders
gps::Shader myBasicShader;

gps::SkyBox skyBox;

gps::Shader skyboxShader;


GLenum glCheckError_(const char *file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR) {
		std::string error;
		switch (errorCode) {
            case GL_INVALID_ENUM:
                error = "INVALID_ENUM";
                break;
            case GL_INVALID_VALUE:
                error = "INVALID_VALUE";
                break;
            case GL_INVALID_OPERATION:
                error = "INVALID_OPERATION";
                break;
            case GL_OUT_OF_MEMORY:
                error = "OUT_OF_MEMORY";
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                error = "INVALID_FRAMEBUFFER_OPERATION";
                break;
        }
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

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

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	fprintf(stdout, "Window resized! New width: %d , and height: %d\n", width, height);
	//TODO
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

	if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            pressedKeys[key] = true;
        } else if (action == GLFW_RELEASE) {
            pressedKeys[key] = false;
        }
    }
    if (pressedKeys[GLFW_KEY_N] && action == GLFW_PRESS) {
        isDay = !isDay;
        initSkyBox(isDay);
		myBasicShader.useShaderProgram();
    }
    if (pressedKeys[GLFW_KEY_L] && action == GLFW_PRESS) {
        cameraLock = !cameraLock;
    }
	if (pressedKeys[GLFW_KEY_C] && action == GLFW_PRESS) {
		autoDayCycle = !autoDayCycle;
	}
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    //TODO
}

void processMovement() {
	if (pressedKeys[GLFW_KEY_W] && !cameraLock) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
		//update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for motorcycle
        motorcycle_normalMatrix = glm::mat3(glm::inverseTranspose(view*motorcycle_model));
	}

	if (pressedKeys[GLFW_KEY_S] && !cameraLock) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for motorcycle
        motorcycle_normalMatrix = glm::mat3(glm::inverseTranspose(view* motorcycle_model));
	}

	if (pressedKeys[GLFW_KEY_A] && !cameraLock) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for motorcycle
        motorcycle_normalMatrix = glm::mat3(glm::inverseTranspose(view* motorcycle_model));
	}

	if (pressedKeys[GLFW_KEY_D] && !cameraLock) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for motorcycle
        motorcycle_normalMatrix = glm::mat3(glm::inverseTranspose(view* motorcycle_model));
	}

    if (pressedKeys[GLFW_KEY_Q]) {
        angle -= 1.0f;
        // update model matrix for motorcycle
        motorcycle_model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
		parking_lot_model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
        // update normal matrix for motorcycle
        motorcycle_normalMatrix = glm::mat3(glm::inverseTranspose(view* motorcycle_model));
		parking_lot_normalMatrix = glm::mat3(glm::inverseTranspose(view * parking_lot_model));
        //glUniformMatrix4fv(motorcycle_modelLoc, 1, GL_FALSE, glm::value_ptr(motorcycle_model));
    }

    if (pressedKeys[GLFW_KEY_E]) {
        angle += 1.0f;
        // update model matrix for motorcycle
        motorcycle_model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
		parking_lot_model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
        // update normal matrix for motorcycle
        motorcycle_normalMatrix = glm::mat3(glm::inverseTranspose(view* motorcycle_model));
		parking_lot_normalMatrix = glm::mat3(glm::inverseTranspose(view * parking_lot_model));
        //glUniformMatrix4fv(motorcycle_modelLoc, 1, GL_FALSE, glm::value_ptr(motorcycle_model));
    }
	
	
	//std::cout << "Camera position: " << myCamera.getPosition().x << " " << myCamera.getPosition().y << " " << myCamera.getPosition().z << "\n";
}

void initOpenGLWindow() {
    myWindow.Create(1024, 768, "MOTO SHOWCASE");
}

void setWindowCallbacks() {
	glfwSetWindowSizeCallback(myWindow.getWindow(), windowResizeCallback);
    glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
    glfwSetCursorPosCallback(myWindow.getWindow(), mouseCallback);
}

void initOpenGLState() {
	glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
	glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
    glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
}

void initModels() {
    motorcycle.LoadModel("models/Motorcycle/ImageToStl.com_honda_nr750_1994.obj", "models/Motorcycle/");
	parking_lot.LoadModel("models/parking_lot/ImageToStl.com_parking_lot.obj", "models/parking_lot/");
}

void initShaders() {
	myBasicShader.loadShader(
        "shaders/basic.vert",
        "shaders/basic.frag");
	skyboxShader.loadShader(
		"shaders/skyboxShader.vert",
		"shaders/skyboxShader.frag");
	skyboxShader.useShaderProgram();
}

void updateUniforms() {
    myBasicShader.useShaderProgram();

    // Compute timeOfDay (0.0 = midnight, 1.0 = noon)
    float timeOfDay = 0.0f;

    if (autoDayCycle) {
        timeOfDay = (sin(glfwGetTime() * 0.1f) + 1.0f) / 2.0f; // Cycles every ~62s
    }
    else {
        timeOfDay = isDay ? 1.0f : 0.0f; // Manual toggle
    }

    glUniform1f(timeOfDayLoc, timeOfDay);

    // Sunlight properties
    glm::vec3 sunDir = glm::normalize(glm::vec3(-0.3f, -1.0f, -0.2f));
    glm::vec3 sunColor = glm::vec3(1.0f, 0.95f, 0.85f); // Warm sunlight

    // Moonlight properties
    glm::vec3 moonDir = glm::normalize(glm::vec3(0.1f, -1.0f, 0.2f));
    glm::vec3 moonColor = glm::vec3(0.3f, 0.3f, 0.6f); // Cool bluish moonlight

    // Send updated light values to shader
    glUniform3fv(sunDirLoc, 1, glm::value_ptr(sunDir));
    glUniform3fv(sunColorLoc, 1, glm::value_ptr(sunColor));
    glUniform3fv(moonDirLoc, 1, glm::value_ptr(moonDir));
    glUniform3fv(moonColorLoc, 1, glm::value_ptr(moonColor));

    // Detect day/night transition and update skybox
    static bool lastDayState = true; // Store previous state
    bool currentDayState = (timeOfDay > 0.3f); // Consider it "day" when timeOfDay is above 0.3

    if (currentDayState != lastDayState) { // If transitioning day/night
        initSkyBox(currentDayState);
        lastDayState = currentDayState;
    }
}

void initUniforms() {
    myBasicShader.useShaderProgram();

    // Create model matrices
    motorcycle_model = glm::mat4(1.0f);
    motorcycle_modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");

    parking_lot_model = glm::mat4(1.0f);
    parking_lot_modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");

    // Get view matrix from camera
    view = myCamera.getViewMatrix();
    viewLoc = glGetUniformLocation(myBasicShader.shaderProgram, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    // Compute normal matrices
    motorcycle_normalMatrix = glm::mat3(glm::inverseTranspose(view * motorcycle_model));
    motorcycle_normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix");

    parking_lot_normalMatrix = glm::mat3(glm::inverseTranspose(view * parking_lot_model));
    parking_lot_normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix");

    // Create projection matrix
    projection = glm::perspective(glm::radians(45.0f),
        (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
        0.1f, 1000.0f);
    projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Get uniform locations for time-based lighting (does not update here!)
    timeOfDayLoc = glGetUniformLocation(myBasicShader.shaderProgram, "timeOfDay");

    sunDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "sunDir");
    sunColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "sunColor");

    moonDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "moonDir");
    moonColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "moonColor");
   

    // Add this line in the initUniforms function
    moonColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "moonColor");
}


void renderMotorcycle(gps::Shader shader) {
    // select active shader program
    shader.useShaderProgram();

	//motorcycle_model = glm::mat4(1.0f);

    //send motorcycle model matrix data to shader
    glUniformMatrix4fv(motorcycle_modelLoc, 1, GL_FALSE, glm::value_ptr(motorcycle_model));

    //send motorcycle normal matrix data to shader
    glUniformMatrix3fv(motorcycle_normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(motorcycle_normalMatrix));


    // draw motorcycle
    motorcycle.Draw(shader);
}

void renderParkingLot(gps::Shader shader) {
	shader.useShaderProgram();
    //parking_lot_model = glm::mat4(1.0f);
    glUniformMatrix4fv(parking_lot_modelLoc, 1, GL_FALSE, glm::value_ptr(parking_lot_model));
    glUniformMatrix3fv(parking_lot_normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(parking_lot_normalMatrix));
    parking_lot.Draw(shader);
}

void renderScene() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    skyBox.Draw(skyboxShader, view, projection);
	//render the sccene
    renderMotorcycle(myBasicShader);
	renderParkingLot(myBasicShader);
}

void cleanup() {
    myWindow.Delete();
    //cleanup code for your own data
}

int main(int argc, const char * argv[]) {

    try {
        initOpenGLWindow();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    initOpenGLState();
	initModels();
	initShaders();
	initUniforms();
    initSkyBox(true);
    setWindowCallbacks();

	glCheckError();
	// application loop
	while (!glfwWindowShouldClose(myWindow.getWindow())) {
        processMovement();
        updateUniforms();
	    renderScene();

		glfwPollEvents();
		glfwSwapBuffers(myWindow.getWindow());

		glCheckError();
	}

	cleanup();

    return EXIT_SUCCESS;
}
