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

// shader uniform locations
GLint motorcycle_modelLoc;
GLint motorcycle_normalMatrixLoc;

GLint parking_lot_modelLoc;
GLint parking_lot_normalMatrixLoc;


GLint viewLoc;
GLint lightDirLoc;
GLint lightColorLoc;
GLint projectionLoc;


// camera
gps::Camera myCamera(
    glm::vec3(5.0f, 1.0f, 0.0f),
    glm::vec3(0.0f, 1.0f, -1.0f),
    glm::vec3(0.0f, 1.0f, 0.0f));

GLfloat cameraSpeed = 0.1f;

GLboolean pressedKeys[1024];

bool cameraLock = true;

// models
gps::Model3D motorcycle;
GLfloat angle;

gps::Model3D parking_lot;

// shaders
gps::Shader myBasicShader;



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
        // update normal matrix for motorcycle
        motorcycle_normalMatrix = glm::mat3(glm::inverseTranspose(view* motorcycle_model));
        //glUniformMatrix4fv(motorcycle_modelLoc, 1, GL_FALSE, glm::value_ptr(motorcycle_model));
    }

    if (pressedKeys[GLFW_KEY_E]) {
        angle += 1.0f;
        // update model matrix for motorcycle
        motorcycle_model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
        // update normal matrix for motorcycle
        motorcycle_normalMatrix = glm::mat3(glm::inverseTranspose(view* motorcycle_model));
        //glUniformMatrix4fv(motorcycle_modelLoc, 1, GL_FALSE, glm::value_ptr(motorcycle_model));
    }
	if (pressedKeys[GLFW_KEY_L]) {
		cameraLock = !cameraLock;
	}
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

}

void initUniforms() {
	myBasicShader.useShaderProgram();

    // create model matrix for teapot
	motorcycle_model = glm::mat4(1.0f);
	motorcycle_modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "motorcycle_model");
	parking_lot_model = glm::mat4(1.0f);
	parking_lot_modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "parking_lot_model");
	// get view matrix for current camera
	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myBasicShader.shaderProgram, "view");
	// send view matrix to shader
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    // compute normal matrix for motorcycle
    motorcycle_normalMatrix = glm::mat3(glm::inverseTranspose(view*motorcycle_model));
	motorcycle_normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "motorcyle_normalMatrix");


	// compute normal matrix for parking_lot
	parking_lot_normalMatrix = glm::mat3(glm::inverseTranspose(view * parking_lot_model));
	parking_lot_normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "parking_lot_normalMatrix");
	// create projection matrix
	projection = glm::perspective(glm::radians(45.0f),
                               (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
                               0.1f, 1000.0f);
	projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
	// send projection matrix to shader
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));	

	//set the light direction (direction towards the light)
	lightDir = glm::vec3(0.0f, 1.0f, 1.0f);
	lightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDir");
	// send light dir to shader
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

	//set light color
	lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
	lightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightColor");
	// send light color to shader
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
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
    setWindowCallbacks();

	glCheckError();
	// application loop
	while (!glfwWindowShouldClose(myWindow.getWindow())) {
        processMovement();
	    renderScene();

		glfwPollEvents();
		glfwSwapBuffers(myWindow.getWindow());

		glCheckError();
	}

	cleanup();

    return EXIT_SUCCESS;
}
