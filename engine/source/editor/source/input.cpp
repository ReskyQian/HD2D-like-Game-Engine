#include "editor/include/input.h"

namespace Hd2d {
    Input::Input(Camera* camera, const unsigned int scr_width, const unsigned int scr_height) {
        camera_ = camera;
        first_mouse_ = true;
        last_x_ = scr_width / 2.0f;
        last_y_ = scr_height / 2.0f;
    }

    void Input::processInput(GLFWwindow* window, float delta_time)
    {
        if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera_->processKeyboard(Hd2d::Camera_Movement::FORWARD, delta_time);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera_->processKeyboard(Hd2d::Camera_Movement::BACKWARD, delta_time);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera_->processKeyboard(Hd2d::Camera_Movement::LEFT, delta_time);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera_->processKeyboard(Hd2d::Camera_Movement::RIGHT, delta_time);
    }

    void Input::mouseCallback(GLFWwindow* window, double xposIn, double yposIn) {
        float xpos = static_cast<float>(xposIn);
        float ypos = static_cast<float>(yposIn);

        if (first_mouse_)
        {
            last_x_ = xpos;
            last_y_ = ypos;
            first_mouse_ = false;
        }

        float xoffset = xpos - last_x_;
        float yoffset = last_y_ - ypos; // reversed since y-coordinates go from bottom to top

        last_x_ = xpos;
        last_y_ = ypos;

        camera_->processMouseMovement(xoffset, yoffset);
    }

    // glfw: whenever the mouse scroll wheel scrolls, this callback is called
    // ----------------------------------------------------------------------
    void Input::scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
    {
        camera_->processMouseScroll(static_cast<float>(yoffset));
    }
}