#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <filesystem>
#include <iostream>
#include <vector>
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

    std::string model_vs_path = (config_manager.getShaderPath() / "model_loading.vs").generic_string();
    std::string model_fs_path = (config_manager.getShaderPath() / "model_loading.fs").generic_string();
    ShaderProgram model_shader(model_vs_path, model_fs_path);

    std::string edge_vs_path = (config_manager.getShaderPath() / "edge.vs").generic_string();
    std::string edge_fs_path = (config_manager.getShaderPath() / "edge.fs").generic_string();
    ShaderProgram edge_shader(edge_vs_path, edge_fs_path);

    std::string screen_vs_path = (config_manager.getShaderPath() / "screen_shader.vs").generic_string();
    std::string screen_fs_path = (config_manager.getShaderPath() / "screen_shader.fs").generic_string();
    ShaderProgram screen_shader(screen_vs_path, screen_fs_path);

    std::string skybox_vs_path = (config_manager.getShaderPath() / "skybox.vs").generic_string();
    std::string skybox_fs_path = (config_manager.getShaderPath() / "skybox.fs").generic_string();
    ShaderProgram skybox_shader(skybox_vs_path, skybox_fs_path);

    std::string blend_vs_path = (config_manager.getShaderPath() / "blending.vs").generic_string();
    std::string blend_fs_path = (config_manager.getShaderPath() / "blending.fs").generic_string();
    ShaderProgram blend_shader(blend_vs_path, blend_fs_path);

    std::string mirror_vs_path = (config_manager.getShaderPath() / "mirror.vs").generic_string();
    std::string mirror_fs_path = (config_manager.getShaderPath() / "mirror.fs").generic_string();
    ShaderProgram mirror_shader(mirror_vs_path, mirror_fs_path);

    std::string normal_vs_path = (config_manager.getShaderPath() / "normal_visualization.vs").generic_string();
    std::string normal_gs_path = (config_manager.getShaderPath() / "normal_visualization.gs").generic_string();
    std::string normal_fs_path = (config_manager.getShaderPath() / "normal_visualization.fs").generic_string();
    ShaderProgram normal_shader(normal_vs_path, normal_gs_path, normal_fs_path);

    // draw grass
    std::string grass_path = (config_manager.getTexturePath() / "grass.png").generic_string();
    std::shared_ptr<Hd2d::Texture2D> grass_texture = Hd2d::Texture2D::loadFromFile(grass_path);
    Hd2d::Texture2D::configClampWrapper();

    // draw windows
    std::string window_path = (config_manager.getTexturePath() / "blending_transparent_window.png").generic_string();
    std::shared_ptr<Hd2d::Texture2D> window_texture = Hd2d::Texture2D::loadFromFile(window_path);
    Hd2d::Texture2D::configClampWrapper();

    // draw skybox
    std::vector<std::string> faces
    {
        (config_manager.getTexturePath() /"skybox/right.jpg").generic_string(),
        (config_manager.getTexturePath() /"skybox/left.jpg").generic_string(),
        (config_manager.getTexturePath() /"skybox/top.jpg").generic_string(),
        (config_manager.getTexturePath() /"skybox/bottom.jpg").generic_string(),
        (config_manager.getTexturePath() /"skybox/front.jpg").generic_string(),
        (config_manager.getTexturePath() /"skybox/back.jpg").generic_string()
    };
    std::shared_ptr<Hd2d::Texture2D> skybox_texture = Hd2d::Texture2D::loadCubemap(faces);

    // set position attrs
    float grass_vertices[] = {
        // positions         // texture Coords (swapped y coordinates because texture is flipped upside down)
        0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
        0.0f, -0.5f,  0.0f,  0.0f,  1.0f,
        1.0f, -0.5f,  0.0f,  1.0f,  1.0f,

        0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
        1.0f, -0.5f,  0.0f,  1.0f,  1.0f,
        1.0f,  0.5f,  0.0f,  1.0f,  0.0f
    };

    float plane_vertices[] = {
        // positions          // normals 
         5.0f, -0.5f,  5.0f,  0.0f,  1.0f,  0.0f,
        -5.0f, -0.5f,  5.0f,  0.0f,  1.0f,  0.0f,
        -5.0f, -0.5f, -5.0f,  0.0f,  1.0f,  0.0f,

         5.0f, -0.5f,  5.0f,  0.0f,  1.0f,  0.0f,
        -5.0f, -0.5f, -5.0f,  0.0f,  1.0f,  0.0f,
         5.0f, -0.5f, -5.0f,  0.0f,  1.0f,  0.0f
    };

    float quad_vertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
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

    float skybox_vertices[] = {
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

    // set VAOs
    // plane VAO
    unsigned int planeVAO, planeVBO;
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(plane_vertices), &plane_vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

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

    // screen quad VAO
    unsigned int quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), &quad_vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    // skybox VAO
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skybox_vertices), &skybox_vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    // set shaders
    blend_shader.use();
    blend_shader.setTexture("texture1", 0);
    blend_shader.setUniformBlock("Matrices", 0);

    screen_shader.use();
    screen_shader.setTexture("screenTexture", 0);
    screen_shader.setUniformBlock("Matrices", 0);

    skybox_shader.use();
    skybox_shader.setTexture("skybox", 0);
    skybox_shader.setUniformBlock("Matrices", 0);

    mirror_shader.use();
    mirror_shader.setTexture("skybox", 0);
    mirror_shader.setUniformBlock("Matrices", 0);

    normal_shader.use();
    normal_shader.setUniformBlock("Matrices", 0);

    // set a uniform buffer object
    unsigned int ubo_matrices;
    glGenBuffers(1, &ubo_matrices);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo_matrices);
    glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    // define the range of the buffer that links to a uniform binding point
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, ubo_matrices, 0, 2 * sizeof(glm::mat4));

    // store the projection matrix (we only do this once now) (note: we're not using zoom anymore by changing the FoV)
    glm::mat4 projection = glm::perspective(glm::radians(camera.getZoom()), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo_matrices);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // framebuffer configuration
    const float buf_scale = 4.0f;
    const int buf_width  = SCR_WIDTH / buf_scale;
    const int buf_height = SCR_HEIGHT / buf_scale;

    unsigned int framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    // create a color attachment texture
    unsigned int texture_colorbuffer;
    glGenTextures(1, &texture_colorbuffer);
    glBindTexture(GL_TEXTURE_2D, texture_colorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, buf_width, buf_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_colorbuffer, 0);
    // create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, buf_width, buf_height); // use a single renderbuffer object for both a depth AND stencil buffer.
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); // now actually attach it
    // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

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
        glViewport(0, 0, buf_width, buf_height);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glEnable(GL_DEPTH_TEST);
        glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        // view/projection transformations
        glm::mat4 view = camera.getViewMatrix();
        glBindBuffer(GL_UNIFORM_BUFFER, ubo_matrices);
        glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        glStencilMask(0x00);

        blend_shader.use();
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

        // draw floor
        mirror_shader.use();
        glBindVertexArray(planeVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_texture->getTextureId());
        mirror_shader.setUniform("model", glm::mat4(1.0f));
        mirror_shader.setUniform("cameraPos", camera.getPosition());
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glStencilMask(0xFF);

        glEnable(GL_CULL_FACE);
        model_shader.use();
        // draw the loaded model
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -0.5f, 0.0f)); 
        model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));	// it's a bit too big for our scene, so scale it down
        model_shader.setUniform("model", model);
        our_model.draw(model_shader);

        normal_shader.use();
        normal_shader.setUniform("model", model);
        our_model.draw(normal_shader);

        glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
        glStencilMask(0x00);

        // draw edge of model
        edge_shader.use();
        glm::vec3 color;
        color.x = static_cast<float>(sin(glfwGetTime() * 4.0) + 1.0f);
        color.y = static_cast<float>(sin(glfwGetTime() * 1.4) + 1.0f);
        color.z = static_cast<float>(sin(glfwGetTime() * 2.6) + 1.0f);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -0.5f, 0.0f)); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));	// it's a bit too big for our scene, so scale it down
        edge_shader.setUniform("color", color);
        edge_shader.setUniform("model", model);
        our_model.draw(edge_shader);

        glBindVertexArray(0);
        glStencilMask(0xFF);
        glStencilFunc(GL_ALWAYS, 0, 0xFF);

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

        // draw skybox
        glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
        skybox_shader.use();
        glm::mat4 view_sp = glm::mat4(glm::mat3(camera.getViewMatrix())); // remove translation from the view matrix
        skybox_shader.setUniform("view_sp", view_sp);
        // skybox cube
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_texture->getTextureId());
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); // set depth function back to default

        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        // now bind back to default framebuffer and draw a quad plane with the attached framebuffer color texture
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDisable(GL_DEPTH_TEST); // disable depth test so screen-space quad isn't discarded due to depth test.
        // clear all relevant buffers
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // set clear color to white (not really necessary actually, since we won't be able to see behind the quad anyways)
        glClear(GL_COLOR_BUFFER_BIT);

        screen_shader.use();
        glBindVertexArray(quadVAO);
        glBindTexture(GL_TEXTURE_2D, texture_colorbuffer);	// use the color attachment texture as the texture of the quad plane
        glDrawArrays(GL_TRIANGLES, 0, 6);

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
    glDeleteVertexArrays(1, &planeVAO);
    glDeleteBuffers(1, &planeVBO);
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &quadVBO);
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &skyboxVBO);
    glfwTerminate();
    exit(EXIT_SUCCESS);
    return 0;
}