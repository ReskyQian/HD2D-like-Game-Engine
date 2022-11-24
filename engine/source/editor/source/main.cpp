#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <filesystem>
#include <iostream>
#include <map>

#include "editor/include/config_manager.h"
#include "editor/include/camera.h"
#include "editor/include/shader.h"
#include "editor/include/model.h"
#include "editor/include/input.h"

const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

float delta_time = 0.0f;
float last_frame = 0.0f;

GLFWwindow* window;
Hd2d::Camera camera(glm::vec3(0.0f, 2.0f, 6.0f));
Hd2d::Input input(&camera, SCR_WIDTH, SCR_HEIGHT);

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void glfwMouseCallback(GLFWwindow* window, double xpos, double ypos) {
    input.mouseCallback(window, xpos, ypos);
}

void glfwScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    input.scrollCallback(window, xoffset, yoffset);
}

void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void initOpenGL() {
    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Hd2d Game Engine", NULL, NULL);
    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetCursorPosCallback(window, glfwMouseCallback);
    glfwSetScrollCallback(window, glfwScrollCallback);

    // tell GLFW to capture our mouse
    // otherwise mouse out of screen will be lost
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    gladLoadGL();

    glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

int main(int argc, char** argv)
{
    initOpenGL();

    // load resource
    std::filesystem::path executable_path(argv[0]);
    std::filesystem::path config_file_path = executable_path.parent_path() / "Hd2dEditor.ini";
    Hd2d::ConfigManager config_manager;
    config_manager.initialize(config_file_path);

    std::string model_path = (config_manager.getModelPath() / "swordMaid/swordMaid.pmx").generic_string();
    // std::string model_path = (config_manager.getTexturePath() / "nanosuit/nanosuit.obj").generic_string();
    Hd2d::Model our_model(model_path);

    std::string color_vs_path = (config_manager.getShaderPath() / "model_loading.vs").generic_string();
    std::string color_fs_path = (config_manager.getShaderPath() / "model_loading.fs").generic_string();
    ShaderProgram color_shader(color_vs_path, color_fs_path);

    std::string edge_vs_path = (config_manager.getShaderPath() / "edge.vs").generic_string();
    std::string edge_fs_path = (config_manager.getShaderPath() / "edge.fs").generic_string();
    ShaderProgram edge_shader(edge_vs_path, edge_fs_path);

    // draw grass
    std::string grass_path = (config_manager.getTexturePath() / "grass.png").generic_string();
    std::shared_ptr<Hd2d::Texture2D> grass_texture = Hd2d::Texture2D::loadFromFile(grass_path);
    Hd2d::Texture2D::configClampWrapper();

    // draw windows
    std::string window_path = (config_manager.getTexturePath() / "blending_transparent_window.png").generic_string();
    std::shared_ptr<Hd2d::Texture2D> window_texture = Hd2d::Texture2D::loadFromFile(window_path);
    Hd2d::Texture2D::configClampWrapper();

    std::string blend_vs_path = (config_manager.getShaderPath() / "blending.vs").generic_string();
    std::string blend_fs_path = (config_manager.getShaderPath() / "blending.fs").generic_string();
    ShaderProgram blend_shader(blend_vs_path, blend_fs_path);

    float grass_vertices[] = {
        // positions         // texture Coords (swapped y coordinates because texture is flipped upside down)
        0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
        0.0f, -0.5f,  0.0f,  0.0f,  1.0f,
        1.0f, -0.5f,  0.0f,  1.0f,  1.0f,

        0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
        1.0f, -0.5f,  0.0f,  1.0f,  1.0f,
        1.0f,  0.5f,  0.0f,  1.0f,  0.0f
    };

    std::vector<glm::vec3> vegetation 
    {
        glm::vec3(-1.5f, 0.0f, -0.48f),
        glm::vec3( 1.5f, 0.0f, 0.51f ),
        glm::vec3( 0.0f, 0.0f, 0.7f  ),
        glm::vec3(-0.3f, 0.0f, -2.3f ),
        glm::vec3( 0.5f, 0.0f, -0.6f )
    };

    float window_vertices[] = {
        // positions         // texture Coords (swapped y coordinates because texture is flipped upside down)
        0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
        0.0f, -0.5f,  0.0f,  0.0f,  1.0f,
        1.0f, -0.5f,  0.0f,  1.0f,  1.0f,

        0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
        1.0f, -0.5f,  0.0f,  1.0f,  1.0f,
        1.0f,  0.5f,  0.0f,  1.0f,  0.0f        
    };

    std::vector<glm::vec3> windows
    {
        glm::vec3(-1.5f, 0.0f, -1.0f),
        glm::vec3( 1.8f, 0.0f, 0.6f ),
        glm::vec3( 0.3f, 0.0f, 0.9f  ),
        glm::vec3(-2.8f, 0.0f, -2.7f ),
        glm::vec3( 3.5f, 0.0f, -1.6f )
    };

    // grass VAO
    unsigned int grassVAO, grassVBO;
    glGenVertexArrays(1, &grassVAO);
    glGenBuffers(1, &grassVBO);
    glBindVertexArray(grassVAO);
    glBindBuffer(GL_ARRAY_BUFFER, grassVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(grass_vertices), &grass_vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);

    // windows VAO
    unsigned int windowVAO, windowVBO;
    glGenVertexArrays(1, &windowVAO);
    glGenBuffers(1, &windowVBO);
    glBindVertexArray(windowVAO);
    glBindBuffer(GL_ARRAY_BUFFER, windowVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(window_vertices), &window_vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);

    blend_shader.use();
    blend_shader.setUniform("texture1", 0);

    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        delta_time = currentFrame - last_frame;
        last_frame = currentFrame;

        // input
        input.processInput(window, delta_time);

        // sort for transparent object
        std::map<float, glm::vec3> sorted_map;
        for(unsigned int i =0; i < windows.size(); i++ ) {
            float distance = glm::length(camera.getPosition() - windows[i]);
            sorted_map[distance] = windows[i];
        }

        // render
        glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.getZoom()), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.getViewMatrix();
        edge_shader.use();
        edge_shader.setUniform("projection", projection);
        edge_shader.setUniform("view", view);

        color_shader.use();
        color_shader.setUniform("projection", projection);
        color_shader.setUniform("view", view);

        blend_shader.use();
        blend_shader.setUniform("projection", projection);
        blend_shader.setUniform("view", view);

        glStencilMask(0x00);

        glm::mat4 model = glm::mat4(1.0f);
        // draw grass
        glBindVertexArray(grassVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, grass_texture->getTextureId());
        for (unsigned int i = 0; i < vegetation.size(); i++)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, vegetation[i]);
            blend_shader.setUniform("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }

        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glStencilMask(0xFF);

        glEnable(GL_CULL_FACE);
        color_shader.use();
        // draw the loaded model
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); 
        model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));	// it's a bit too big for our scene, so scale it down
        color_shader.setUniform("model", model);
        our_model.draw(color_shader);

        glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
        glStencilMask(0x00);
        glDisable(GL_DEPTH_TEST);

        // draw edge of model
        edge_shader.use();
        glm::vec3 color;
        color.x = static_cast<float>(sin(glfwGetTime() * 4.0) + 1.0f);
        color.y = static_cast<float>(sin(glfwGetTime() * 1.4) + 1.0f);
        color.z = static_cast<float>(sin(glfwGetTime() * 2.6) + 1.0f);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));	// it's a bit too big for our scene, so scale it down
        edge_shader.setUniform("color", color);
        edge_shader.setUniform("model", model);
        our_model.draw(edge_shader);

        glBindVertexArray(0);
        glStencilMask(0xFF);
        glStencilFunc(GL_ALWAYS, 0, 0xFF);
        glEnable(GL_DEPTH_TEST);

        glDisable(GL_CULL_FACE);

        // draw transparent object (windows)
        blend_shader.use();
        glBindVertexArray(windowVAO);
        glBindTexture(GL_TEXTURE_2D, window_texture->getTextureId());
        for(std::map<float, glm::vec3>::reverse_iterator it = sorted_map.rbegin(); it != sorted_map.rend(); ++it ) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, it->second);
            blend_shader.setUniform("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    our_model.deleteBuffer();
    glDeleteVertexArrays(1, &grassVAO);
    glDeleteBuffers(1, &grassVBO);
    glDeleteVertexArrays(1, &windowVAO);
    glDeleteBuffers(1, &windowVBO);
    glfwTerminate();
    exit(EXIT_SUCCESS);
    return 0;
}