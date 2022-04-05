#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <iostream>

#include "RayMarcher.h"
#include "SDFCamera.h"
#include "SDFObject.h"
#include "SDFGeometry.h"
#include "SDFShader.h"
#include "SDFMaterial.h"

// glfw and input functions
// ------------------------
GLFWwindow* initOpenGL(unsigned int width, unsigned int height, const char* title);
void shutdownOpenGL();
void key_input_callback(GLFWwindow* window, int button, int other, int action, int mods);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

// imgui functions
void initImGui(GLFWwindow* window);
void drawGui();
void shutdownImGui();
bool showGui = false;

// global object functions
void createGlobalObjects();
void deleteGlobalObjects();

// screen settings
// ---------------
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;


// Globals
RayMarcher* s_RayMarcher = nullptr;
SDFCamera* s_Camera = nullptr;
SDFGeometry* s_QuadGeometry = nullptr;
SDFGeometry* s_CubeGeometry = nullptr;


// TODO 10.1 : global variables for parameters in UI for exercise 10.1
glm::vec3 sphereColor(0.0f, 0.0f, 1.0f);
glm::vec3 sphereCenter(-2.0f, 0.0f, 0.0f);
GLfloat sphereRadius(1.25f);

glm::vec3 boxColor(1.0f, 0.0f, 0.0f);
glm::vec3 boxCenter(2.0f, 0.0f, 0.0f);
glm::vec3 boxRotation(0.0f, 0.0f, 0.0f);
glm::vec3 boxSize(1.0f, 1.0f, 1.0f);

// TODO 10.2 : Add global variables for parameters in UI



int main()
{
    GLFWwindow* window = initOpenGL(SCR_WIDTH, SCR_HEIGHT, "Exercise 10");
    if (!window)
        return -1;

    initImGui(window);

    // Create RayMarcher, default geometries, and camera
    createGlobalObjects();

    // Add new full scren quad object
    SDFShader defaultShader(SHADER_FOLDER "raymarcher.vert", SHADER_FOLDER "exercise10_1.frag");
    SDFMaterial defaultMaterial(&defaultShader);
    SDFObject defaultObject(s_QuadGeometry, &defaultMaterial);
    s_RayMarcher->AddObject(&defaultObject);

    // EXTRA: Object using a cube instead of a full screen pass
    //SDFShader sphereShader(SHADER_FOLDER "raymarcher.vert", SHADER_FOLDER "sphere_lit.frag");
    //SDFMaterial sphereMaterial(&sphereShader);
    //SDFObject sphereObject(s_CubeGeometry, &sphereMaterial);
    //s_RayMarcher->AddObject(&sphereObject);
    //sphereObject.SetModelMatrix(glm::scale(glm::translate(glm::mat4(1), glm::vec3(sphereCenter)), glm::vec3(sphereRadius)));
    //sphereMaterial.SetPropertyValue<glm::vec3>("sphereColor", sphereColor);

    glEnable(GL_FRAMEBUFFER_SRGB);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(true);

    glEnable(GL_DEPTH_CLAMP);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    while (!glfwWindowShouldClose(window))
    {
        static float lastFrame = 0.0f;
        float currentFrame = (float)glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // TODO 10.1 : Set sphere uniforms here, as material properties




        // TODO 10.1 : Material properties for the box, uncomment
        //glm::mat4 boxMatrix(1.0f);
        //boxMatrix = glm::translate(boxMatrix, boxCenter);
        //boxMatrix = glm::rotate(boxMatrix, boxRotation.z, glm::vec3(0, 0, 1));
        //boxMatrix = glm::rotate(boxMatrix, boxRotation.x, glm::vec3(1, 0, 0));
        //boxMatrix = glm::rotate(boxMatrix, boxRotation.y, glm::vec3(0, 1, 0));
        //defaultMaterial.SetPropertyValue<glm::mat4>("boxMatrix", s_Camera->GetViewMatrix() * boxMatrix);
        //defaultMaterial.SetPropertyValue<glm::vec3>("boxColor", boxColor);
        //defaultMaterial.SetPropertyValue<glm::vec3>("boxSize", boxSize);


        // TODO 10.2 : Set uniforms here, as material properties







        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        s_RayMarcher->Render();

        if (showGui)
        {
            drawGui();
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    deleteGlobalObjects();

    shutdownImGui();
    shutdownOpenGL();
    return 0;
}


void createGlobalObjects()
{
    float verticesQuad[] =
    {
        -1.0f,  1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f,
        1.0f,  1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
    };
    s_QuadGeometry = new SDFGeometry(false, GL_TRIANGLE_STRIP, 4, verticesQuad, 4);

    float verticesCube[] =
    {
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
    };
    GLuint indicesCube[] =
    {
        1, 0, 2,   1, 2, 3, // Back
        0, 4, 6,   0, 6, 2, // Left
        4, 5, 7,   4, 7, 6, // Front
        5, 1, 3,   5, 3, 7, // Right
        7, 3, 2,   7, 2, 6, // Top
        1, 5, 4,   1, 4, 0, // Bottom
    };
    s_CubeGeometry = new SDFGeometry(true, GL_TRIANGLES, 8, verticesCube, 2 * 3 * 6, indicesCube);

    s_Camera = new SDFCamera();
    s_Camera->SetAspect((float)SCR_WIDTH / SCR_HEIGHT);

    s_RayMarcher = new RayMarcher();
    s_RayMarcher->SetCamera(s_Camera);
}

void deleteGlobalObjects()
{
    delete s_RayMarcher;
    s_RayMarcher = nullptr;

    delete s_Camera;
    s_Camera = nullptr;

    delete s_QuadGeometry;
    s_QuadGeometry = nullptr;

    delete s_CubeGeometry;
    s_CubeGeometry = nullptr;
}


GLFWwindow* initOpenGL(unsigned int width, unsigned int height, const char* title)
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return nullptr;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_input_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return nullptr;
    }

    return window;
}

void shutdownOpenGL()
{
    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
}

void initImGui(GLFWwindow* window)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");
}

void drawGui()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    {
        ImGui::Begin("Properties");

        //TODO 10.1 : Properties for exercise 10.1, you can comment these out for 10.2
        {
            ImGui::ColorEdit3("Sphere Color", (float*)&sphereColor);
            ImGui::DragFloat3("Sphere Center", (float*)&sphereCenter, 0.0025f, -100, 100);
            ImGui::SliderFloat("Sphere Radius", &sphereRadius, 0.0f, 5.0f);
            ImGui::Separator();

            //TODO 10.1 : We are missing some properties!



            ImGui::SliderAngle("Box Rotation X", (float*)&boxRotation.x, -180.0f, 180.0f);
            ImGui::SliderAngle("Box Rotation Y", (float*)&boxRotation.y, -180.0f, 180.0f);
            ImGui::SliderAngle("Box Rotation Z", (float*)&boxRotation.z, -180.0f, 180.0f);
            ImGui::Separator();
        }

        //TODO 10.2 : Add properties here





        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void shutdownImGui()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void key_input_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS)
    {
        switch (key)
        {
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, true);
            break;
        case GLFW_KEY_SPACE:
            showGui = !showGui;
            break;
        case GLFW_KEY_R:
            if (s_RayMarcher)
                s_RayMarcher->ReloadShaders();
            break;
        }
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // glfw: whenever the window size changed (by OS or user resize) this callback function executes

    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);

    if (s_Camera)
        s_Camera->SetAspect((float)width / height);
}
