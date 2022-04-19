#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include <vector>
#include <chrono>
#include <string>
#include <glm/gtx/transform.hpp>
#include "rt_renderer.h"
#include "primitives.h"

#include "camera.h"

// glfw callbacks
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void button_input_callback(GLFWwindow* window, int button, int action, int mods);
void cursor_input_callback(GLFWwindow* window, double posX, double posY);
void processInput(GLFWwindow* window);

// rasterization grid resolution
const int max_W = 64, max_H = 64;

// window resolution
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 800;

Camera camera(glm::vec3(0.9f, 0.0f, 1.5f));
rt::Renderer renderer;

float deltaTime = 0;
unsigned int rtDepth = 2;

int main()
{
    using namespace std;

    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Exercise 9", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetMouseButtonCallback(window, button_input_callback);
    glfwSetCursorPosCallback(window, cursor_input_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }


    // load the 3D models
    // -----------------
    std::vector<glm::vec3> points;
    std::vector<glm::vec4> colors;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> uvs;
    Primitives::makeCube(2.f, points, normals, uvs, colors);


    vector<rt::vertex> vts;
    glm::mat4 scale = glm::scale(glm::vec3(.25f,.25f,.25f));
    for (unsigned int i = 0; i < points.size(); i++){
        rt::vertex v{scale * glm::vec4(points[i], 1.0f),
                    glm::vec4(normals[i], 0),
                    colors[i],
                    uvs[i]
        };
        vts.push_back(v);
    }

    glm::mat4 outsideout = glm::scale(glm::vec3(-2.f,-2.f,-2.f));
    for (unsigned int i = 0; i < points.size(); i++){
        rt::vertex v{outsideout * glm::vec4(points[i], 1.0f),
                     glm::vec4(normals[i], 0),
                     rt::grey,
                     uvs[i]
        };
        vts.push_back(v);
    }



    // initialize our custom frame buffer
    // ----------------------------------
    // every frame we will: draw to it, upload it to a texture, and copy the texture to the window frame buffer.
    FrameBuffer<uint32_t> customBuffer(max_W, max_H);


    // initialize texture we will use to upload our buffer to GPU
    // ----------------------------------------------------------
    unsigned int bufferTexture;

    glGenTextures(1, &bufferTexture);
    glBindTexture(GL_TEXTURE_2D, bufferTexture);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // initialize openGL frame buffer object
    // ------------------------------------
    // this helps handling writing the texture content into the window frame buffer
    GLuint oglFrameBuffer = 0;
    glGenFramebuffers(1, &oglFrameBuffer);

    // render loop
    // -----------
    // render every loopInterval seconds
    float loopInterval = 1.f/60.f;
    auto begin = chrono::high_resolution_clock::now();

    std::cout << "Key mapping:" << std::endl;
    std::cout << "1 - one intersection (aka ray-casting rendering)" << std::endl;
    std::cout << "2 - one reflection" << std::endl;
    std::cout << "3 - two reflections" << std::endl;
    std::cout << "4 - three reflections" << std::endl;
    std::cout << "5 - four reflections" << std::endl;

    while (!glfwWindowShouldClose(window))
    {
        // update current time
        auto frameStart = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> appTime = frameStart - begin;

        processInput(window);

        // render to our custom frame buffer
        // ---------------------------------
        customBuffer.clearBuffer(rt::Colors::toRGBA32(rt::Colors::black));

        glm::mat4 scale = glm::scale(glm::vec3(.5f,.5f,.5f));

        renderer.render(vts, glm::mat4(1), camera.GetViewMatrix(), 70.0f, rtDepth, customBuffer);

        // show our rendered image
        // -----------------------
        // upload the custom color buffer to the GPU using the texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, bufferTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, max_W, max_H, 0, GL_RGBA, GL_UNSIGNED_BYTE, customBuffer.buffer);

        // set opengl frame buffer object to read from our texture, we will copy from it
        glBindFramebuffer(GL_READ_FRAMEBUFFER, oglFrameBuffer);
        glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferTexture, 0);

        // bind the window frame buffer, where we want to copy the contents of the texture to
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

        // copy from the frame buffer object (access the texture) to the window frame buffer
        int size_W, size_H;
        glfwGetFramebufferSize(window, &size_W, &size_H);
        glBlitFramebuffer(0,0, max_W, max_H, 0, 0, size_W, size_H, GL_COLOR_BUFFER_BIT, GL_NEAREST);

        // display frame buffer
        glfwSwapBuffers(window);
        glfwPollEvents();

        // control render loop frequency (busy wait)
        std::chrono::duration<float> elapsed = std::chrono::high_resolution_clock::now()-frameStart;
        while (loopInterval > elapsed.count()) {
            elapsed = std::chrono::high_resolution_clock::now() - frameStart;
        }
        deltaTime = elapsed.count();
        glfwSetWindowTitle(window, ("Exercise 9 - FPS: " + std::to_string(int(1.0f/deltaTime + .5f))).c_str());
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}


void cursor_input_callback(GLFWwindow* window, double posX, double posY){
    // camera rotation
    static float lastX = (float)SCR_WIDTH / 2.0;
    static float lastY = (float)SCR_HEIGHT / 2.0;
    static bool firstMouse = true;

    if (firstMouse)
    {
        lastX = posX;
        lastY = posY;
        firstMouse = false;
    }

    float xoffset = posX - lastX;
    float yoffset = lastY - posY; // reversed since y-coordinates go from bottom to top

    lastX = posX;
    lastY = posY;

    // we use the handy camera class from LearnOpenGL to handle our camera
    camera.ProcessMouseMovement(xoffset, yoffset);
}

void button_input_callback(GLFWwindow* window, int button, int action, int mods){
    static bool toggleClicked = true;
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        toggleClicked = !toggleClicked;
        glfwSetInputMode(window, GLFW_CURSOR, toggleClicked ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
    }
}

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) rtDepth = 1;
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) rtDepth = 2;
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) rtDepth = 3;
    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) rtDepth = 4;
    if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) rtDepth = 5;

    // movement commands
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);

}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}