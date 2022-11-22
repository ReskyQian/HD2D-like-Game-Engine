#ifndef _CAMERA_H__
#define _CAMERA_H__

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glad/glad.h>

#include <vector>

namespace Hd2d {
    enum class Camera_Movement : int {
        FORWARD,
        BACKWARD,
        LEFT,
        RIGHT
    };

    const float YAW = -90.0f;
    const float PITCH = -10.0f;
    const float SPEED = 2.0f;
    const float SENSITIVITY = 0.05f;
    const float ZOOM = 45.0f;

    class Camera {
    public:
        explicit Camera();
        explicit Camera(glm::vec3 position);
        explicit Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch);
        ~Camera();

        constexpr glm::vec3 getPosition() const noexcept {
            return position_;
        }

        constexpr glm::vec3 getFront() const noexcept {
            return front_;
        }

        glm::mat4 getViewMatrix() const noexcept {
            return glm::lookAt(position_, position_ + front_, up_);
        }

        constexpr float getZoom() const noexcept {
            return zoom_;
        }

        void processKeyboard(Camera_Movement direction, float delta_time);
        void processMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);
        void processMouseScroll(float yoffset);

    protected:
        // camera attributes
        glm::vec3 position_;
        glm::vec3 front_;
        glm::vec3 up_;
        glm::vec3 right_;
        glm::vec3 world_up_;

        // euler angles
        float yaw_;
        float pitch_;
        float move_speed_;
        float mouse_sensitivity_;
        float zoom_;

    private:
        void updateCameraVectors();
    };

} // namespace Hd2d




#endif // _CAMERA_H__