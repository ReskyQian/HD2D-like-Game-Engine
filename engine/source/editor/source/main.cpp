#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <filesystem>
#include <memory>

#include "editor/include/config_manager.h"
#include "editor/include/camera.h"
#include "editor/include/shader.h"
#include "editor/include/texture2d.h"
#include "editor/include/mesh.h"

const unsigned int SCR_WIDTH = 1080;
const unsigned int SCR_HEIGHT = 1080;

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool first_mouse = true;

float delta_time = 0.0f;
float last_frame = 0.0f;

Hd2d::Camera camera(glm::vec3(0.0f, 0.0f, 8.0f));

float vertices[] = {
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f,  0.0f, -1.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,  0.0f, -1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,  0.0f, -1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,  0.0f, -1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  0.0f, -1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f,  0.0f, -1.0f,

    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f,  0.0f,  1.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  0.0f,  1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,  0.0f,  1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,  0.0f,  1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f,  0.0f,  1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f,  0.0f,  1.0f,

    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, -1.0f,  0.0f,  0.0f,

     0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 1.0f,  0.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 1.0f,  0.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  0.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 1.0f,  0.0f,  0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f, -1.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f, -1.0f,  0.0f,

    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  1.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  1.0f,  0.0f
};

// world space positions of our cubes
glm::vec3 cube_positions[] = {
    glm::vec3( 0.0f,  0.0f,  0.0f),
    glm::vec3( 2.0f,  5.0f, -15.0f),
    glm::vec3(-1.5f, -2.2f, -2.5f),
    glm::vec3(-3.8f, -2.0f, -12.3f),
    glm::vec3( 2.4f, -0.4f, -3.5f),
    glm::vec3(-1.7f,  3.0f, -7.5f),
    glm::vec3( 1.3f, -2.0f, -2.5f),
    glm::vec3( 1.5f,  2.0f, -2.5f),
    glm::vec3( 1.5f,  0.2f, -1.5f),
    glm::vec3(-1.3f,  1.0f, -1.5f)
};

// positions of the point lights
glm::vec3 point_light_positions[] = {
    glm::vec3( 0.7f,  0.2f,  2.0f ),
    glm::vec3( 2.3f, -3.3f, -4.0f ),
    glm::vec3(-4.0f,  2.0f, -12.0f),
    glm::vec3( 0.0f,  0.0f, -3.0f )
};

void processInput(GLFWwindow *window);
void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void mouseCallback(GLFWwindow* window, double xpos, double ypos);
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);

int main(int argc, char** argv)
{
    std::filesystem::path executable_path(argv[0]);
    std::filesystem::path config_file_path = executable_path.parent_path() / "Hd2dEditor.ini";
    ConfigManager config_manager;
    config_manager.initialize(config_file_path);

    if (!glfwInit())
        return -1;
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Hd2d Game Engine", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetScrollCallback(window, scrollCallback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    gladLoadGL();

    glEnable(GL_DEPTH_TEST);
    glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE);

    std::string color_vs_path = (config_manager.getShaderPath() / "lighting_shader.vs").generic_string();
    std::string color_fs_path = (config_manager.getShaderPath() / "lighting_shader.fs").generic_string();
    ShaderProgram color_shader(color_vs_path, color_fs_path);

    std::string light_vs_path = (config_manager.getShaderPath() / "light_cube.vs").generic_string();
    std::string light_fs_path = (config_manager.getShaderPath() / "light_cube.fs").generic_string();
    ShaderProgram light_cube_shader(light_vs_path, light_fs_path);

    // configure the cubes
    unsigned int VBO, cube_VAO;
    glGenVertexArrays(1, &cube_VAO);
    glGenBuffers(1, &VBO);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(cube_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // normal attribute
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);

    unsigned int light_cube_VAO;
    glGenVertexArrays(1, &light_cube_VAO);
    glBindVertexArray(light_cube_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    std::string pocky1_png = (config_manager.getTexturePath() / "pocky1.png").generic_string();
    std::string pocky1_cpt = (config_manager.getTexturePath() / "pocky1.cpt").generic_string();
    std::string pocky2_png = (config_manager.getTexturePath() / "pocky2.png").generic_string();
    std::string pocky2_cpt = (config_manager.getTexturePath() / "pocky2.cpt").generic_string();

    std::shared_ptr<Hd2d::Texture2D> texture1 = Hd2d::Texture2D::loadTexture(pocky1_png, pocky1_cpt);
    std::shared_ptr<Hd2d::Texture2D> texture2 = Hd2d::Texture2D::loadTexture(pocky2_png, pocky2_cpt);
    color_shader.use();
    color_shader.setTexture("texture1", 0);
    color_shader.setTexture("texture2", 1);

    while (!glfwWindowShouldClose(window))
    {
        float current_frame = static_cast<float>(glfwGetTime());
        delta_time = current_frame - last_frame;
        last_frame = current_frame;

        // input
        processInput(window);

        // render background
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.75f, 0.82f, 0.94f, 0.6f);

        // bind textures on corresponding texture units
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1->getTextureId());
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2->getTextureId());

        color_shader.use();

        float timeSpd = 2.0f;
        float timeValue = glfwGetTime() * timeSpd;
        float mixValue = sin(timeValue - 3.14f) / 2.0f + 0.5f;
        color_shader.setUniform("mixValue", mixValue);

        color_shader.setUniform("viewPos", camera.getPosition());
        // set material attr
        color_shader.setUniform("material.specular", glm::vec3(0.6f, 0.6f, 0.6f));
        color_shader.setUniform("material.shininess", 32.0f);

        // set attenuation attr
        color_shader.setUniform("lightAtten.constant" , 1.0f  );
        color_shader.setUniform("lightAtten.linear"   , 0.09f );
        color_shader.setUniform("lightAtten.quadratic", 0.032f); 

        // set directional light attr
        color_shader.setUniform("dirLight.direction", glm::vec3(-0.2f, -1.0f, -0.3f));
        color_shader.setUniform("dirLight.ambient"  , glm::vec3(0.05f, 0.05f, 0.05f));
        color_shader.setUniform("dirLight.diffuse"  , glm::vec3( 0.4f,  0.4f,  0.4f));
        color_shader.setUniform("dirLight.specular" , glm::vec3( 0.5f,  0.5f,  0.5f));

        // set spot light attr
        color_shader.setUniform("spotLight.position"   , camera.getPosition()         );
        color_shader.setUniform("spotLight.direction"  , camera.getFront()            );
        color_shader.setUniform("spotLight.cutoff"     , glm::cos(glm::radians(12.5f)));
        color_shader.setUniform("spotLight.outerCutoff", glm::cos(glm::radians(15.0f)));
        color_shader.setUniform("spotLight.ambient"    , glm::vec3(0.0f, 0.0f, 0.0f)  );
        color_shader.setUniform("spotLight.diffuse"    , glm::vec3(1.0f, 1.0f, 1.0f)  );
        color_shader.setUniform("spotLight.specular"   , glm::vec3(1.0f, 1.0f, 1.0f)  );

        // set point light attr
        glm::vec3 light_color;
        light_color.x           = static_cast<float>(sin(glfwGetTime() * 2.0) + 1.0f);
        light_color.y           = static_cast<float>(sin(glfwGetTime() * 0.7) + 1.0f);
        light_color.z           = static_cast<float>(sin(glfwGetTime() * 1.3) + 1.0f);
        glm::vec3 diffuse_color = light_color   * glm::vec3(0.5f); // decrease the influence
        glm::vec3 ambient_color = diffuse_color * glm::vec3(0.2f); // low influence

        for (unsigned int i = 0; i < 4; i++) {
            color_shader.setUniform("pointLights[" + std::to_string(i) + "].position", point_light_positions[i]);
            color_shader.setUniform("pointLights[" + std::to_string(i) + "].ambient", ambient_color);
            color_shader.setUniform("pointLights[" + std::to_string(i) + "].diffuse", diffuse_color);
            color_shader.setUniform("pointLights[" + std::to_string(i) + "].specular", glm::vec3(1.0f, 1.0f, 1.0f));
        }

        // create transformations
        glm::mat4 view       = camera.getViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.getZoom()), 
                                                (float)SCR_WIDTH / (float)SCR_HEIGHT, 
                                                0.1f, 
                                                100.0f);

        color_shader.setUniform("projection", projection);
        color_shader.setUniform("view", view);
    
        // render the triangle
        glBindVertexArray(cube_VAO);

        for (unsigned int i = 0; i < 10; i++) {
            // calculate the model matrix for each object and pass it to shader before drawing
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cube_positions[i]);
            float angle = 20.0f * (i + 1);
            model = glm::rotate(model, (float)glfwGetTime() * glm::radians(angle), 
                                glm::vec3(1.0f, 0.3f, 0.5f));
            color_shader.setUniform("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // lamp object
        light_cube_shader.use();
        light_cube_shader.setUniform("projection", projection);
        light_cube_shader.setUniform("view", view);

        glBindVertexArray(light_cube_VAO);
        for (unsigned int i = 0; i < 4; i++) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, point_light_positions[i]);
            model = glm::scale(model, glm::vec3(0.2f));
            light_cube_shader.setUniform("model", model);
            light_cube_shader.setUniform("diffuse_color", diffuse_color);            
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // gkfw
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glDeleteVertexArrays(1, &cube_VAO);
    glDeleteVertexArrays(1, &light_cube_VAO);
    glDeleteBuffers(1, &VBO);

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow *window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.processKeyboard(Hd2d::Camera_Movement::FORWARD, delta_time);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.processKeyboard(Hd2d::Camera_Movement::BACKWARD, delta_time);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.processKeyboard(Hd2d::Camera_Movement::LEFT, delta_time);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.processKeyboard(Hd2d::Camera_Movement::RIGHT, delta_time);
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouseCallback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (first_mouse)
    {
        lastX = xpos;
        lastY = ypos;
        first_mouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.processMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.processMouseScroll(static_cast<float>(yoffset));
}