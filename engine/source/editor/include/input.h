#ifndef _INPUT_H__
#define _INPUT_H__

#define GLFW_INCLUDE_NONE
#include <glfw/glfw3.h>

#include "editor/include/camera.h"

namespace Hd2d {
    class Input {
    public:
        explicit Input(Camera* camera, const unsigned int scr_width, const unsigned int scr_height);
        void processInput(GLFWwindow* window, float delta_time);

        void mouseCallback(GLFWwindow* window, double xposIn, double yposIn);
        void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);

    private:
        Camera* camera_;

        bool first_mouse_;

        float last_x_;
        float last_y_;    
    };
}

#endif // _INPUT_H__