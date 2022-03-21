#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include <vector>

// NEW! as our scene gets more complex, we start using more helper classes
//  I recommend that you read through the camera.h and model.h files to see if you can map the the previous
//  lessons to this implementation
#include "shader.h"
#include "camera.h"
#include "model.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// glfw and input functions
// ------------------------
void processInput(GLFWwindow* window);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void key_input_callback(GLFWwindow* window, int button, int other, int action, int mods);
void cursor_input_callback(GLFWwindow* window, double posX, double posY);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

// screen settings
// ---------------
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// global variables used for rendering
// -----------------------------------
Shader* shader;
Shader* forward_shading;
Shader* deferred_shading;
Shader* lighting_shader;
Model* carBodyModel;
Model* carPaintModel;
Model* carInteriorModel;
Model* carLightModel;
Model* carWindowsModel;
Model* carWheelModel;
Model* floorModel;
GLuint carBodyTexture;
GLuint carPaintTexture;
GLuint carLightTexture;
GLuint carWindowsTexture;
GLuint carWheelTexture;
GLuint floorTexture;
Camera camera(glm::vec3(0.0f, 1.6f, 5.0f));

GLuint gBuffer;
GLuint gAlbedo, gNormal, gOthers, gDepth;

// global variables used for control
// ---------------------------------
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
float deltaTime;
bool isPaused = false; // stop camera movement when GUI is open


// structure to hold lighting info
// -------------------------------
struct Light
{
    Light(glm::vec3 position, glm::vec3 color, float intensity, float radius)
        : position(position), color(color), intensity(intensity), radius(radius)
    {
    }

    glm::vec3 position;
    glm::vec3 color;
    float intensity;
    float radius;
};

// structure to hold config info
// -------------------------------
struct Config
{
    Config() : lights()
    {
        // Adding lights
        //lights.emplace_back(position, color, intensity, radius);

        // light 1
        lights.emplace_back(glm::vec3({ -0.8f, 2.4f, 0.0f }), glm::vec3({ 1.0f, 1.0f, 1.0f }), 1.0f, 10.0f);

        // light 2
        lights.emplace_back(glm::vec3({ 1.8f, .7f, 2.2f }), glm::vec3({ 0.5f, 0.0f, 1.0f }), 1.0f, 3.0f);

        // TODO 7.2 : Add a third light
        lights.emplace_back(glm::vec3({ 0.0f, 2.0f, -2.0f }), glm::vec3({ 0.25f, 1.0f, 0.25f }), 1.0f, 5.0f);

        // Extra lights
        /*
        srand(13);
        float maxDist = 10.f, maxHeight = 1.0f;
        for (unsigned int i = 0; i < 1000; i++)
        {
            bool valid = false;
            glm::vec3 pos, col;
            while (!valid) { // so the lights are in a circular arrangement (instead of squared)
                pos.x = ((rand() % 100) / 100.f) * maxDist * 2 - maxDist;
                pos.z = ((rand() % 100) / 100.f) * maxDist * 2 - maxDist;
                pos.y = ((rand() % 100) / 100.f) * maxHeight;
                if (glm::dot(pos, pos) < maxDist * maxDist + maxHeight * maxHeight)
                    valid = true;
            }

            // also calculate random color
            col.r = ((rand() % 100) / 200.f) + 0.5f; // between 0.5 and 1.0
            col.g = ((rand() % 100) / 200.f) + 0.5f; // between 0.5 and 1.0
            col.b = ((rand() % 100) / 200.f) + 0.5f; // between 0.5 and 1.0

            lights.emplace_back(pos, col, 0.25f, 1.0f);
        }
        //*/
    }

    // ambient light
    glm::vec3 ambientLightColor = {1.0f, 1.0f, 1.0f};
    float ambientLightIntensity = 0.25f;

    // material
    glm::vec3 reflectionColor = {1.0f, 1.0f, 0.0f};
    float ambientReflectance = 0.75f;
    float diffuseReflectance = 0.75f;
    float specularReflectance = 0.75f;
    float specularExponent = 10.0f;

    std::vector<Light> lights;

} config;



// function declarations
// ---------------------
void initGBuffers(GLFWwindow* window);
void setAmbientUniforms(glm::vec3 ambientLightColor);
void setLightUniforms(Light &light, Camera* viewSpace = nullptr);
void setupForwardAdditionalPass();
void resetForwardAdditionalPass();
void drawCube();
void drawQuad();
void drawObjects();
void drawGui();


int main()
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
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Exercise 7", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, cursor_input_callback);
    glfwSetKeyCallback(window, key_input_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // load the shaders and the 3D models
    // ----------------------------------
    forward_shading = new Shader("shaders/forward_shading.vert", "shaders/forward_shading.frag");
    deferred_shading = new Shader("shaders/deferred_shading.vert", "shaders/deferred_shading.frag");
    lighting_shader = new Shader("shaders/lighting.vert", "shaders/lighting.frag");
    shader = forward_shading;

    carBodyModel = new Model("car/Body_LOD0.obj");
    carPaintModel = new Model("car/Paint_LOD0.obj");
    carInteriorModel = new Model("car/Interior_LOD0.obj");
    carLightModel = new Model("car/Light_LOD0.obj");
    carWindowsModel = new Model("car/Windows_LOD0.obj");
    carWheelModel = new Model("car/Wheel_LOD0.obj");
    floorModel = new Model("floor/floor.obj");

    // set up the z-buffer
    // -------------------
    glDepthRange(-1,1); // make the NDC a right handed coordinate system, with the camera pointing towards -z
    glEnable(GL_DEPTH_TEST); // turn on z-buffer depth test
    glDepthFunc(GL_LESS); // draws fragments that are closer to the screen in NDC


    //set up gbuffers
    initGBuffers(window);


    // Dear IMGUI init
    // ---------------
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");


    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        static float lastFrame = 0.0f;
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 viewProjection = projection * view;

        processInput(window);

        glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (shader == deferred_shading)
        {
            // DEFERRED PATH

            // 1. geometry pass: render scene's geometry/color data into gbuffer
            {
                shader->use();
                glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                shader->setMat4("view", view);
                drawObjects();
                glBindFramebuffer(GL_FRAMEBUFFER, 0);

            }

            // 2. lighting pass: calculate lighting using the gbuffer's content
            {
                shader = lighting_shader;
                shader->use();

                // Bind g-buffers as textures
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, gAlbedo);
                shader->setInt("AlbedoGBuffer", 0);
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, gNormal);
                shader->setInt("NormalGBuffer", 1);
                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, gOthers);
                shader->setInt("OthersGBuffer", 2);
                glActiveTexture(GL_TEXTURE3);
                glBindTexture(GL_TEXTURE_2D, gDepth);
                shader->setInt("DepthBuffer", 3);

                // set inverse projection to reconstruct position from depth
                shader->setMat4("invProjection", glm::inverse(projection));

                // 2.1 Draw a fullscreen quad for first light + ambient
                // No transformation, quad coordinates already in clip space
                glm::mat4 identity = glm::mat4(1.0f);
                shader->setMat4("model", identity);
                shader->setMat4("viewProjection", identity);
                setAmbientUniforms(config.ambientLightColor * config.ambientLightIntensity);
                setLightUniforms(config.lights[0], &camera);
                drawQuad();


                // 2.2 draw a cube for each additional light.
                // The cube shape is not ideal, it has been implemented like this for simplicity. Still better than a quad for smaller lights

                // Set view projection for all light boxes
                shader->setMat4("viewProjection", viewProjection);

                // No ambient for other lights
                setAmbientUniforms(glm::vec3(0.0f));

                // Render additional lights in additive
                glEnable(GL_BLEND);
                glBlendFunc(GL_ONE, GL_ONE);

                // Depth clamp ignores clipping with near and far planes
                glEnable(GL_DEPTH_CLAMP);

                // Render only the back faces of the box
                glEnable(GL_CULL_FACE);
                glCullFace(GL_FRONT);

                // Disable depth write
                glDepthMask(false);

                // Disable depth test
                glDisable(GL_DEPTH_TEST);

                // render additional lights
                for (int i = 1; i < config.lights.size(); ++i)
                {
                    // The cube is positioned at the center of the light, with a size equal to the radius
                    glm::mat4 model = glm::translate(glm::mat4(1.0f), config.lights[i].position) * glm::scale(glm::mat4(1.0f), glm::vec3(config.lights[i].radius));
                    shader->setMat4("model", model);
                    setLightUniforms(config.lights[i], &camera);
                    drawCube();
                }


                // Restore values
                glDisable(GL_BLEND);
                glBlendFunc(GL_ONE, GL_ZERO);
                glDisable(GL_DEPTH_CLAMP);
                glCullFace(GL_BACK);
                glDisable(GL_CULL_FACE);
                glDepthMask(true);
                glEnable(GL_DEPTH_TEST);

                shader = deferred_shading;
            }
        }
        else
        {
            // FORWARD PATH

            shader->use();

            // First light + ambient
            setAmbientUniforms(config.ambientLightColor * config.ambientLightIntensity);
            setLightUniforms(config.lights[0]);
            drawObjects();

            // Additional additive lights
            setupForwardAdditionalPass();
            for (int i = 1; i < config.lights.size(); ++i)
            {
                setLightUniforms(config.lights[i]);
                drawObjects();
            }
            resetForwardAdditionalPass();
        }

        if (isPaused) {
            drawGui();
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    // -------
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    delete carBodyModel;
    delete carPaintModel;
    delete carInteriorModel;
    delete carLightModel;
    delete carWindowsModel;
    delete carWheelModel;
    delete floorModel;
    delete forward_shading;
    delete deferred_shading;
    delete lighting_shader;

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}


void drawGui(){
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    {
        ImGui::Begin("Settings");

        ImGui::Text("Ambient light: ");
        ImGui::ColorEdit3("ambient light color", (float*)&config.ambientLightColor);
        ImGui::SliderFloat("ambient light intensity", &config.ambientLightIntensity, 0.0f, 1.0f);
        ImGui::Separator();

        ImGui::Text("Light 1: ");
        ImGui::DragFloat3("light 1 position", (float*)&config.lights[0].position, .1f, -20, 20);
        ImGui::ColorEdit3("light 1 color", (float*)&config.lights[0].color);
        ImGui::SliderFloat("light 1 intensity", &config.lights[0].intensity, 0.0f, 2.0f);
        ImGui::SliderFloat("light 1 radius", &config.lights[0].radius, 0.01f, 50.0f);
        ImGui::Separator();

        ImGui::Text("Light 2: ");
        ImGui::DragFloat3("light 2 position", (float*)&config.lights[1].position, .1f, -20, 20);
        ImGui::ColorEdit3("light 2 color", (float*)&config.lights[1].color);
        ImGui::SliderFloat("light 2 intensity", &config.lights[1].intensity, 0.0f, 2.0f);
        ImGui::SliderFloat("light 2 radius", &config.lights[1].radius, 0.01f, 50.0f);
        ImGui::Separator();

        // TODO 7.2 : Add the UI controllers for the third light
        ImGui::Text("Light 3: ");
        ImGui::DragFloat3("light 3 position", (float*)&config.lights[2].position, .1f, -20, 20);
        ImGui::ColorEdit3("light 3 color", (float*)&config.lights[2].color);
        ImGui::SliderFloat("light 3 intensity", &config.lights[2].intensity, 0.0f, 2.0f);
        ImGui::SliderFloat("light 3 radius", &config.lights[2].radius, 0.01f, 50.0f);
        ImGui::Separator();


        ImGui::Text("Material: ");
        ImGui::ColorEdit3("reflection color", (float*)&config.reflectionColor);
        ImGui::SliderFloat("ambient reflectance", &config.ambientReflectance, 0.0f, 1.0f);
        ImGui::SliderFloat("diffuse reflectance", &config.diffuseReflectance, 0.0f, 1.0f);
        ImGui::SliderFloat("specular reflectance", &config.specularReflectance, 0.0f, 1.0f);
        ImGui::SliderFloat("specular exponent", &config.specularExponent, 0.0f, 100.0f);
        ImGui::Separator();

        ImGui::Text("Shading model: ");
        {
            if (ImGui::RadioButton("Forward Shading", shader == forward_shading)) { shader = forward_shading; }
            if (ImGui::RadioButton("Deferred Shading", shader == deferred_shading)) { shader = deferred_shading; }
        }
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}


void initGBuffers(GLFWwindow* window)
{
    // configure g-buffer framebuffer
    // ------------------------------
    glGenFramebuffers(1, &gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    // albedo color buffer
    glGenTextures(1, &gAlbedo);
    glBindTexture(GL_TEXTURE_2D, gAlbedo);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gAlbedo, 0);

    // normal color buffer
    glGenTextures(1, &gNormal);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, width, height, 0, GL_RG, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

    // others color buffer
    glGenTextures(1, &gOthers);
    glBindTexture(GL_TEXTURE_2D, gOthers);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gOthers, 0);

    // depth texture buffer
    glGenTextures(1, &gDepth);
    glBindTexture(GL_TEXTURE_2D, gDepth);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, gDepth, 0);

    // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering
    unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, attachments);

    // finally check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void setAmbientUniforms(glm::vec3 ambientLightColor)
{
    // ambient uniforms
    shader->setVec3("ambientLightColor", ambientLightColor);
}

void setLightUniforms(Light& light, Camera* viewSpace)
{
    glm::vec3 position = light.position;

    // if a camera is provided, the light position will be relative to it, in view space
    if (viewSpace)
    {
        glm::vec4 viewSpacePosition = viewSpace->GetViewMatrix() * glm::vec4(light.position, 1.0f);
        position = glm::vec3(viewSpacePosition.x, viewSpacePosition.y, viewSpacePosition.z);
    }

    // light uniforms
    shader->setVec3("lightPosition", position);
    shader->setVec3("lightColor", light.color * light.intensity);
    shader->setFloat("lightRadius", light.radius);
}

void setupForwardAdditionalPass()
{
    // Remove ambient from additional passes
    setAmbientUniforms(glm::vec3(0.0f));

    // Enable additive blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    // Set depth test to GL_EQUAL (only the fragments that match the depth buffer are rendered)
    glDepthFunc(GL_EQUAL);
}

void resetForwardAdditionalPass()
{
    // Restore ambient
    setAmbientUniforms(config.ambientLightColor * config.ambientLightIntensity);

    //Disable blend and restore default blend function
    glDisable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ZERO);

    // Restore default depth test
    glDepthFunc(GL_LESS);
}








// drawQuad() renders a 2x2 XY quad in NDC
// ---------------------------------------
void drawQuad()
{
    static unsigned int quadVAO = 0, quadVBO;
    if (quadVAO == 0)
    {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
            1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
            1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

// drawCube() renders a 3D cube.
// -----------------------------
void drawCube()
{
    static unsigned int cubeVAO = 0, cubeVBO = 0;
    // initialize (if necessary)
    if (cubeVAO == 0)
    {
        float vertices[] = {
            // back face
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
            1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right
            1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
            // front face
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
            1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
            1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
            1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
            -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
            // left face
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            // right face
            1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
            1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
            1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right
            1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
            1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
            1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left
            // bottom face
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
            1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
            1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
            1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
            -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
            // top face
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
            1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
            1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right
            1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
            -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left
        };
        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        // fill buffer
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        // link vertex attributes
        glBindVertexArray(cubeVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    // render Cube
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

void drawObjects()
{
    // the typical transformation uniforms are already set for you, these are:
    // projection (perspective projection matrix)
    // view (to map world space coordinates to the camera space, so the camera position becomes the origin)
    // model (for each model part we draw)

    // camera parameters
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 viewProjection = projection * view;

    // camera position
    shader->setVec3("camPosition", camera.Position);
    // set viewProjection matrix uniform
    shader->setMat4("viewProjection", viewProjection);

    // material uniforms for car paint
    shader->setVec3("reflectionColor", config.reflectionColor);
    shader->setFloat("ambientReflectance", config.ambientReflectance);
    shader->setFloat("diffuseReflectance", config.diffuseReflectance);
    shader->setFloat("specularReflectance", config.specularReflectance);
    shader->setFloat("specularExponent", config.specularExponent);

    glm::mat4 model = glm::mat4(1.0f);
    shader->setMat4("model", model);
    carBodyModel->Draw(*shader);
    carPaintModel->Draw(*shader);

    // material uniforms for other car parts (hardcoded)
    shader->setVec3("reflectionColor", 1.0f, 1.0f, 1.0f);
    shader->setFloat("ambientReflectance", 0.75f);
    shader->setFloat("diffuseReflectance", 0.75f);
    shader->setFloat("specularReflectance", 0.75f);
    shader->setFloat("specularExponent", 10.0f);
    shader->setInt("textureAlbedo", 0);

    // draw car
    shader->setMat4("model", model);
    carLightModel->Draw(*shader);
    carInteriorModel->Draw(*shader);

    // draw wheel
    model = glm::translate(glm::mat4(1.0f), glm::vec3(-.7432f, .328f, 1.39f));
    shader->setMat4("model", model);
    carWheelModel->Draw(*shader);

    // draw wheel
    model = glm::translate(glm::mat4(1.0f), glm::vec3(-.7432f, .328f, -1.39f));
    shader->setMat4("model", model);
    carWheelModel->Draw(*shader);

    // draw wheel
    model = glm::rotate(glm::mat4(1.0f), glm::pi<float>(), glm::vec3(0.0, 1.0, 0.0));
    model = glm::translate(model, glm::vec3(-.7432f, .328f, 1.39f));
    shader->setMat4("model", model);
    carWheelModel->Draw(*shader);

    // draw wheel
    model = glm::rotate(glm::mat4(1.0f), glm::pi<float>(), glm::vec3(0.0, 1.0, 0.0));
    model = glm::translate(model, glm::vec3(-.7432f, .328f, -1.39f));
    shader->setMat4("model", model);
    carWheelModel->Draw(*shader);

    // draw floor
    model = glm::scale(glm::mat4(1.0), glm::vec3(5.f, 5.f, 5.f));
    shader->setMat4("model", model);
    floorModel->Draw(*shader);

    shader->setFloat("specularReflectance", 1.0f);
    shader->setFloat("specularExponent", 20.0f);
    model = glm::mat4(1.0f); 
    shader->setMat4("model", model);

    carWindowsModel->Draw(*shader);
}


void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (isPaused)
        return;

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


void cursor_input_callback(GLFWwindow* window, double posX, double posY){

    // camera rotation
    static bool firstMouse = true;
    if (firstMouse)
    {
        lastX = (float)posX;
        lastY = (float)posY;
        firstMouse = false;
    }

    float xoffset = (float)posX - lastX;
    float yoffset = lastY - (float)posY; // reversed since y-coordinates go from bottom to top

    lastX = (float)posX;
    lastY = (float)posY;

    if (isPaused)
        return;

    // we use the handy camera class from LearnOpenGL to handle our camera
    camera.ProcessMouseMovement(xoffset, yoffset);
}


void key_input_callback(GLFWwindow* window, int button, int other, int action, int mods){
    // controls pause mode
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS){
        isPaused = !isPaused;
        glfwSetInputMode(window, GLFW_CURSOR, isPaused ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
    }

}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll((float)yoffset);
}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}