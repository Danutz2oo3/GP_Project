#include "Camera.hpp"
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
namespace gps {

    //Camera constructor
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->cameraUpDirection = cameraUp;
        this->cameraFrontDirection = glm::normalize(cameraTarget - cameraPosition);
        this->cameraRightDirection = glm::normalize(glm::cross(this->cameraFrontDirection, this->cameraUpDirection));
		this->cameraUpDirection = glm::normalize(glm::cross(this->cameraRightDirection, this->cameraFrontDirection));
    }

    //return the view matrix, using the glm::lookAt() function
    glm::mat4 Camera::getViewMatrix() {
        return glm::lookAt(cameraPosition, cameraPosition + cameraFrontDirection, this->cameraUpDirection);
    }

    //update the camera internal parameters following a camera move event
    void Camera::move(MOVE_DIRECTION direction, float speed) {
        switch (direction)
        {
        case gps::MOVE_FORWARD:
			cameraPosition += cameraFrontDirection * speed;
            break;
        case gps::MOVE_BACKWARD:
			cameraPosition -= cameraFrontDirection * speed;
            break;
        case gps::MOVE_RIGHT:
            cameraPosition += cameraRightDirection * speed;
            break;
        case gps::MOVE_LEFT:
            cameraPosition -= cameraRightDirection * speed;
            break;
        default:
            break;
        }
    }

    //update the camera internal parameters following a camera rotate event
    //yaw - camera rotation around the y axis
    //pitch - camera rotation around the x axis
    void Camera::rotate(float pitch, float yaw) {
        cameraFrontDirection = glm::rotate(cameraFrontDirection, glm::radians(pitch), cameraRightDirection);
        cameraFrontDirection = glm::rotate(cameraFrontDirection, glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f));
        cameraFrontDirection = glm::normalize(cameraFrontDirection);


        cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, cameraUpDirection));
        cameraUpDirection = glm::cross(cameraRightDirection, cameraFrontDirection);
    }
}
