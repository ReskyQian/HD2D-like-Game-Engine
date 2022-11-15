#include "editor/include/camera.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>
#include <algorithm>

namespace Hd2d {
    Camera::Camera() :  
        position_{glm::vec3(0.0f, 0.0f, 0.0f)}, 
        up_{glm::vec3(0.0f, 1.0f, 0.0f)},
        yaw_{YAW},
        pitch_{PITCH},
        front_{glm::vec3(0.0f, 0.0f, -1.0f)},
        move_speed_{SPEED},
        mouse_sensitivity_{SENSITIVITY},
        zoom_{ZOOM}
    {
        updateCameraVectors();
    }

    Camera::Camera(glm::vec3 position) :  
        position_{position}, 
        up_{glm::vec3(0.0f, 1.0f, 0.0f)},
        yaw_{YAW},
        pitch_{PITCH},
        front_{glm::vec3(0.0f, 0.0f, -1.0f)},
        move_speed_{SPEED},
        mouse_sensitivity_{SENSITIVITY},
        zoom_{ZOOM}
    {
        updateCameraVectors();
    }

    Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch) : 
        position_{position}, 
        up_{up},
        yaw_{yaw},
        pitch_{pitch},
        front_{glm::vec3(0.0f, 0.0f, -1.0f)},
        move_speed_{SPEED},
        mouse_sensitivity_{SENSITIVITY},
        zoom_{ZOOM}
    {
        updateCameraVectors();
    }

    Camera::~Camera() {}

    void Camera::processKeyboard(Camera_Movement direction, float delta_time) {
        float velocity = move_speed_ * delta_time;
        if (direction == Camera_Movement::FORWARD) {
            position_ += front_ * velocity;
        }
        if (direction == Camera_Movement::BACKWARD) {
            position_ -= front_ * velocity;
        }
        if (direction == Camera_Movement::LEFT) {
            position_ -= right_ * velocity;
        }
        if (direction == Camera_Movement::RIGHT) {
            position_ += right_ * velocity;
        }        
    }

    void Camera::processMouseMovement(float xoffset, float yoffset, 
                                    GLboolean constrainPitch) {
        xoffset *= mouse_sensitivity_;
        yoffset *= mouse_sensitivity_;

        yaw_ += xoffset;
        pitch_ += yoffset;

        if (constrainPitch) {
            pitch_ = std::clamp(pitch_, -89.0f, 89.0f);
        }

        updateCameraVectors();
    }

    void Camera::processMouseScroll(float yoffset) {
        zoom_ = yoffset;
        zoom_ = std::clamp(zoom_, 1.0f, 45.0f);
    }

    void Camera::updateCameraVectors() {
        glm::vec3 front;
        front.x = cos(glm::radians(yaw_)) * cos(glm::radians(pitch_));
        front.y = sin(glm::radians(pitch_));
        front.z = sin(glm::radians(yaw_)) * cos(glm::radians(pitch_));
        front_ = glm::normalize(front);

        right_ = glm::normalize(glm::cross(front_, world_up_));
        up_ = glm::normalize(glm::cross(right_, front_));
    }
};