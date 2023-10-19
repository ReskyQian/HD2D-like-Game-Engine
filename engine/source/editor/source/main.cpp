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

bool isNormalShow = false;
bool isShadowShow = true;

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

std::shared_ptr<ShaderProgram> loadShader(Hd2d::ConfigManager& config_manager, std::string shader_name) {
    std::string vs_path = (config_manager.getShaderPath() / (shader_name + ".vs") ).generic_string();
    std::string fs_path = (config_manager.getShaderPath() / (shader_name + ".fs")).generic_string();
    std::shared_ptr<ShaderProgram> shader = std::make_shared<ShaderProgram>(vs_path, fs_path);
    return shader;
}

std::shared_ptr<Hd2d::Texture2D> initGrass(Hd2d::ConfigManager& config_manager,
                                           unsigned int& VAO, 
                                           unsigned int& VBO, 
                                           int amount) 
{
    // load grass texture
    std::string grass_path = (config_manager.getTexturePath() / "grass.png").generic_string();
    std::shared_ptr<Hd2d::Texture2D> grass_texture = Hd2d::Texture2D::loadFromFile(grass_path);
    Hd2d::Texture2D::configClampWrapper();

    // set position attrs
    float grass_vertices[] = {
        // positions        // texture Coords (swapped y coordinates because texture is flipped upside down)
        -0.5f, 1.0f, 0.0f,  0.0f,  0.0f,
        -0.5f, 0.0f, 0.0f,  0.0f,  1.0f,
         0.5f, 0.0f, 0.0f,  1.0f,  1.0f,

        -0.5f, 1.0f, 0.0f,  0.0f,  0.0f,
         0.5f, 0.0f, 0.0f,  1.0f,  1.0f,
         0.5f, 1.0f, 0.0f,  1.0f,  0.0f
    };

    // grass VAO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(grass_vertices), &grass_vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    // generate a large list of semi-random model transformation matrices
    // ------------------------------------------------------------------
    glm::mat4* modelMatrices;
    modelMatrices = new glm::mat4[amount];
    srand(static_cast<unsigned int>(glfwGetTime())); // initialize random seed
    float offset = 4.5f;
    for (unsigned int i = 0; i < amount; i++)
    {
        glm::mat4 model = glm::mat4(1.0f);
        // 1. translation: displace in range [-offset, offset]
        float x = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
        float z = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
        model = glm::translate(model, glm::vec3(x, 0, z));

        // 2. scale: Scale between 0.1f and 0.6f
        float scale = static_cast<float>((rand() % 50) / 100.0 + 0.1);
        model = glm::scale(model, glm::vec3(scale));

        // 3. rotation: add random rotation around a (semi)randomly picked rotation axis vector
        float rotAngle = static_cast<float>((rand() % 360));
        model = glm::rotate(model, rotAngle, glm::vec3(0, 1.0f, 0));

        // 4. now add to list of matrices
        modelMatrices[i] = model;
    }

    // configure instanced array
    // -------------------------
    unsigned int buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, amount * sizeof(glm::mat4), &modelMatrices[0], GL_STATIC_DRAW);

    // set transformation matrices as an instance vertex attribute (with divisor 1)
    // note: we're cheating a little by taking the, now publicly declared, VAO of the model's mesh(es) and adding new vertexAttribPointers
    // normally you'd want to do this in a more organized fashion, but for learning purposes this will do.
    // -----------------------------------------------------------------------------------------------------------------------------------
    // set attribute pointers for matrix (4 times vec4)
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));

    glVertexAttribDivisor(3, 1);
    glVertexAttribDivisor(4, 1);
    glVertexAttribDivisor(5, 1);
    glVertexAttribDivisor(6, 1);

    glBindVertexArray(0);

    return grass_texture;  
}

std::shared_ptr<Hd2d::Texture2D>  initPlane(Hd2d::ConfigManager& config_manager,
                                            unsigned int& VAO, 
                                            unsigned int& VBO) 
{
    std::string floor_path = (config_manager.getTexturePath() / "terrain.jpg").generic_string();
    std::shared_ptr<Hd2d::Texture2D> floor_texture = Hd2d::Texture2D::loadFromFile(floor_path);
    // Hd2d::Texture2D::configClampWrapper();

    float plane_vertices[] = {
        // positions          // normals 
         5.0f, 0.0f,  5.0f,  0.0f,  1.0f,  0.0f,
        -5.0f, 0.0f,  5.0f,  0.0f,  1.0f,  0.0f,
        -5.0f, 0.0f, -5.0f,  0.0f,  1.0f,  0.0f,

         5.0f, 0.0f,  5.0f,  0.0f,  1.0f,  0.0f,
        -5.0f, 0.0f, -5.0f,  0.0f,  1.0f,  0.0f,
         5.0f, 0.0f, -5.0f,  0.0f,  1.0f,  0.0f
    };    

    // plane VAO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(plane_vertices), &plane_vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);

    return floor_texture;
} 

std::shared_ptr<Hd2d::Texture2D> initSkybox(Hd2d::ConfigManager& config_manager,
                                           unsigned int& VAO, 
                                           unsigned int& VBO) 
{
    // load skybox texture
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

    // skybox VAO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skybox_vertices), &skybox_vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    return skybox_texture;
}

std::shared_ptr<Hd2d::Texture2D> initWindow(Hd2d::ConfigManager& config_manager,
                                           unsigned int& VAO, 
                                           unsigned int& VBO,
                                           std::vector<glm::vec3>& windows) 
{
    // load window texture
    std::string window_path = (config_manager.getTexturePath() / "blending_transparent_window.png").generic_string();
    std::shared_ptr<Hd2d::Texture2D> window_texture = Hd2d::Texture2D::loadFromFile(window_path);
    Hd2d::Texture2D::configClampWrapper();

    float window_vertices[] = {
        // positions         // texture Coords (swapped y coordinates because texture is flipped upside down)
        0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
        0.0f, -0.5f,  0.0f,  0.0f,  1.0f,
        1.0f, -0.5f,  0.0f,  1.0f,  1.0f,

        0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
        1.0f, -0.5f,  0.0f,  1.0f,  1.0f,
        1.0f,  0.5f,  0.0f,  1.0f,  0.0f        
    };

    windows = {
        glm::vec3(-1.5f, 0.5f, -1.0f ),
        glm::vec3( 1.8f, 0.5f,  0.6f ),
        glm::vec3( 0.3f, 0.5f,  0.9f ),
        glm::vec3(-2.8f, 0.5f, -2.7f ),
        glm::vec3( 3.5f, 0.5f, -1.6f )
    };

    // windows VAO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(window_vertices), &window_vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);

    return window_texture;
}

void initScreenQuad(Hd2d::ConfigManager& config_manager,
                    unsigned int& VAO, 
                    unsigned int& VBO,
                    unsigned int& framebuffer,
                    unsigned int& texture_colorbuffer,
                    const float buf_scale = 4.0f) 
{
    float quad_vertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };


    // screen quad VAO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), &quad_vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    // framebuffer configuration
    const int buf_width  = SCR_WIDTH / buf_scale;
    const int buf_height = SCR_HEIGHT / buf_scale;

    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    // create a color attachment texture
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
        fprintf(stderr, "Error::FRAMEBUFFER::Framebuffer is not complete!\n");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void initShadowMap( unsigned int& depthMapFBO,
                    unsigned int& depthMap)
{
    // configure depth map FBO
    const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
    glGenFramebuffers(1, &depthMapFBO);
    // create depth texture
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

int main(int argc, char** argv)
{
    initOpenGL();

    // load resource
    std::filesystem::path executable_path(argv[0]);
    std::filesystem::path config_file_path = executable_path.parent_path() / "Hd2dEditor.ini";
    Hd2d::ConfigManager config_manager;
    config_manager.initialize(config_file_path);

    std::string model_path = (config_manager.getModelPath() / "nanosuit/nanosuit.obj").generic_string();
    Hd2d::Model our_model(model_path);

    // load shaders
    std::shared_ptr<ShaderProgram> model_shader = loadShader(config_manager, "model_loading");
    std::shared_ptr<ShaderProgram> edge_shader = loadShader(config_manager, "edge");

    std::shared_ptr<ShaderProgram> screen_shader = loadShader(config_manager, "screen");
    screen_shader->use();
    screen_shader->setTexture("screenTexture", 0);
    screen_shader->setUniformBlock("Matrices", 0);

    std::shared_ptr<ShaderProgram> skybox_shader = loadShader(config_manager, "skybox");
    skybox_shader->use();
    skybox_shader->setTexture("skybox", 0);
    skybox_shader->setUniformBlock("Matrices", 0);

    std::shared_ptr<ShaderProgram> blend_shader = loadShader(config_manager, "blending");
    blend_shader->use();
    blend_shader->setTexture("texture1", 0);
    blend_shader->setUniformBlock("Matrices", 0); 

    std::shared_ptr<ShaderProgram> floor_shader = loadShader(config_manager, "floor");
    floor_shader->use();
    floor_shader->setTexture("floor_texture", 0);
    floor_shader->setUniformBlock("Matrices", 0);

    std::shared_ptr<ShaderProgram> grass_shader = loadShader(config_manager, "grass");
    grass_shader->use();
    grass_shader->setTexture("grass_texture", 0);
    grass_shader->setUniformBlock("Matrices", 0);

    std::shared_ptr<ShaderProgram> shadow_map_shader = loadShader(config_manager, "shadow_map");
    shadow_map_shader->use();
    shadow_map_shader->setUniform("depthMap", 0);

    std::string normal_vs_path = (config_manager.getShaderPath() / "normal_visualization.vs").generic_string();
    std::string normal_gs_path = (config_manager.getShaderPath() / "normal_visualization.gs").generic_string();
    std::string normal_fs_path = (config_manager.getShaderPath() / "normal_visualization.fs").generic_string();
    ShaderProgram normal_shader(normal_vs_path, normal_gs_path, normal_fs_path);
    normal_shader.use();
    normal_shader.setUniformBlock("Matrices", 0);

    unsigned int grassVAO;
    unsigned int grassVBO;
    int amount = 1000;
    std::shared_ptr<Hd2d::Texture2D> grass_texture = 
    initGrass(config_manager, grassVAO, grassVBO, amount); 

    unsigned int planeVAO;
    unsigned int planeVBO;
    std::shared_ptr<Hd2d::Texture2D> floor_texture = 
    initPlane(config_manager, planeVAO, planeVBO);

    unsigned int skyboxVAO;
    unsigned int skyboxVBO;
    std::shared_ptr<Hd2d::Texture2D> skybox_texture = 
    initSkybox(config_manager, skyboxVAO, skyboxVBO); 

    unsigned int windowVAO;
    unsigned int windowVBO;
    std::vector<glm::vec3> windows;
    std::shared_ptr<Hd2d::Texture2D> window_texture = 
    initWindow(config_manager, windowVAO, windowVBO, windows); 

    unsigned int quadVAO;
    unsigned int quadVBO;
    unsigned int framebuffer;
    unsigned int texture_colorbuffer;
    const float buf_scale = 4.0f;
    initScreenQuad(config_manager, quadVAO, quadVBO, framebuffer, texture_colorbuffer, buf_scale); 

    unsigned int depthMapFBO;
    unsigned int depthMap;
    initShadowMap(depthMapFBO, depthMap);
    
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

    glm::vec3 lightPos(-2.0f, 4.0f, -1.0f);

    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
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

        // render configuration
        glViewport(0, 0, SCR_WIDTH / buf_scale, SCR_HEIGHT / buf_scale);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glEnable(GL_DEPTH_TEST);
        glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        glStencilMask(0x00);

        // view/projection transformations
        glm::mat4 view = camera.getViewMatrix();
        glBindBuffer(GL_UNIFORM_BUFFER, ubo_matrices);
        glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        // draw grass
        grass_shader->use();
        glBindVertexArray(grassVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, grass_texture->getTextureId());
        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, amount);
        glBindVertexArray(0);

        // draw floor
        floor_shader->use();
        glm::mat4 floor_model = glm::mat4(1.0f);
        floor_shader->setUniform("model", floor_model);
        glBindVertexArray(planeVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, floor_texture->getTextureId());
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glStencilMask(0xFF);

        glEnable(GL_CULL_FACE);
        model_shader->use();
        // draw the loaded model
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); 
        model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));	
        model_shader->setUniform("model", model);
        our_model.draw(*model_shader);

        if(isNormalShow) {
            normal_shader.use();
            normal_shader.setUniform("model", model);
            our_model.draw(normal_shader);
        }

        glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
        glStencilMask(0x00);

        // draw edge of model
        edge_shader->use();
        glm::vec3 color;
        color.x = static_cast<float>(sin(glfwGetTime() * 4.0) + 1.0f);
        color.y = static_cast<float>(sin(glfwGetTime() * 1.4) + 1.0f);
        color.z = static_cast<float>(sin(glfwGetTime() * 2.6) + 1.0f);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));	// it's a bit too big for our scene, so scale it down
        edge_shader->setUniform("color", color);
        edge_shader->setUniform("model", model);
        our_model.draw(*edge_shader);

        glBindVertexArray(0);
        glStencilMask(0xFF);
        glStencilFunc(GL_ALWAYS, 0, 0xFF);

        glDisable(GL_CULL_FACE);

        // draw transparent object (windows)
        blend_shader->use();
        glBindVertexArray(windowVAO);
        glBindTexture(GL_TEXTURE_2D, window_texture->getTextureId());
        for(std::map<float, glm::vec3>::reverse_iterator it = sorted_map.rbegin(); it != sorted_map.rend(); ++it ) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, it->second);
            blend_shader->setUniform("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }

        // draw skybox
        glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
        skybox_shader->use();
        glm::mat4 view_sp = glm::mat4(glm::mat3(camera.getViewMatrix())); // remove translation from the view matrix
        skybox_shader->setUniform("view_sp", view_sp);
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

        screen_shader->use();
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