#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include <vector>

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

const unsigned int SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;

// global variables used for rendering
// -----------------------------------
Shader* shader = nullptr;
Shader* shadowMap_shader;
Shader* skybox_shader;
Shader* deferred_shader;
Shader* lighting_shader;

// post-fx shaders
Shader* copy_shader;
Shader* compose_shader;
Shader* blur_shader;
Shader* bloom_shader;
Shader* celshading_shader;
Shader* outline_shader;

// models
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
glm::mat4 view;
glm::mat4 projection;
glm::mat4 viewProjection;


unsigned int skyboxVAO; // skybox handle
unsigned int cubemapTexture; // skybox texture handle

unsigned int shadowMap, shadowMapFBO;
glm::mat4 lightSpaceMatrix;

GLuint gBuffer, accumBuffer;
GLuint gAlbedo, gNormal, gOthers, gAccum, gDepth;


GLuint tempBuffers[2] = { 0, 0 };
GLuint tempTextures[2] = { 0, 0 };

// global variables used for control
// ---------------------------------
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
float deltaTime;
bool isPaused = false; // stop camera movement when GUI is open

float lightRotationSpeed = 1.0f;

// structure to hold lighting info
// -------------------------------
struct Light
{
    Light(glm::vec3 position, glm::vec3 color, float intensity, float radius, bool shadow = false)
        : position(position), color(color), intensity(intensity), radius(radius), shadow(shadow)
    {
    }

    glm::vec3 position;
    glm::vec3 color;
    float intensity;
    float radius;
    bool shadow;
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
        lights.emplace_back(glm::vec3(-3.0f, 1.0f, -0.75f), glm::vec3(1.0f, 1.0f, 1.0f), 2.0f, 0.0f, true);

        // light 2
        lights.emplace_back(glm::vec3( 1.0f, 1.5f, 0.0f), glm::vec3(0.7f, 0.2f, 1.0f), 1.0f, 10.0f);
    }

    std::vector<Light> lights;

    // material
    glm::vec3 reflectionColor = glm::vec3(0.9f, 0.9f, 0.2f);
    float roughness = 0.25f;
    float metalness = 0.0f;


    //TODO 9.1 9.2 9.4 9.5 and 9.6 : Add configuration values





} config;


enum class PostFXMode
{
    None,
    Realistic,
    CelShading,
} postFXMode;


// function declarations
// ---------------------
void initFrameBuffers(GLFWwindow* window);
void setLightUniforms(Light &light, Camera* viewSpace);
void updateCameraMatrices();

void drawCube();
void drawQuad();
void drawSkybox();
void drawShadowMap();
void drawObjects();
void drawGui();
void drawDeferredLight(Light& light);
void drawFullscreenPass(const char* sourceTextureName, GLuint sourceTexture);

unsigned int initSkyboxBuffers();
unsigned int loadCubemap(vector<std::string> faces);
void createShadowMap();

void prepareGeometryPass();
void restoreGeometryPass();
void prepareDeferredPass();
void restoreDeferredPass();


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
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Exercise 9", NULL, NULL);
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

    // Default to realistic PostFX
    // TODO 9.5 : Change to cel-shading
    postFXMode = PostFXMode::Realistic;

    // load the shaders
    // ----------------------------------
    skybox_shader = new Shader("shaders/skybox.vert", "shaders/skybox.frag");
    shadowMap_shader = new Shader("shaders/shadowmap.vert", "shaders/shadowmap.frag");
    deferred_shader = new Shader("shaders/deferred_shading.vert", "shaders/deferred_shading.frag");
    lighting_shader = new Shader("shaders/lighting.vert", "shaders/lighting.frag");

    copy_shader = new Shader("shaders/fullscreen.vert", "shaders/copy.frag");
    compose_shader = new Shader("shaders/fullscreen.vert", "shaders/compose.frag");
    blur_shader = new Shader("shaders/fullscreen.vert", "shaders/blur.frag");
    bloom_shader = new Shader("shaders/fullscreen.vert", "shaders/bloom.frag");
    celshading_shader = new Shader("shaders/fullscreen.vert", "shaders/celshading.frag");
    outline_shader = new Shader("shaders/fullscreen.vert", "shaders/outline.frag");


    // load the 3D models
    // ----------------------------------
    carBodyModel = new Model("car/Body_LOD0.obj");
    carPaintModel = new Model("car/Paint_LOD0.obj");
    carInteriorModel = new Model("car/Interior_LOD0.obj");
    carLightModel = new Model("car/Light_LOD0.obj");
    carWindowsModel = new Model("car/Windows_LOD0.obj");
    carWheelModel = new Model("car/Wheel_LOD0.obj");
    floorModel = new Model("floor/floor.obj");

    // init skybox
    vector<std::string> faces
    {
        "skybox/right.tga",
        "skybox/left.tga",
        "skybox/top.tga",
        "skybox/bottom.tga",
        "skybox/front.tga",
        "skybox/back.tga"
    };
    cubemapTexture = loadCubemap(faces);
    skyboxVAO = initSkyboxBuffers();

    createShadowMap();

    // set up the z-buffer
    // -------------------
    glDepthRange(-1,1); // make the NDC a right handed coordinate system, with the camera pointing towards -z
    glEnable(GL_DEPTH_TEST); // turn on z-buffer depth test
    glDepthFunc(GL_LESS); // draws fragments that are closer to the screen in NDC

    //set up gbuffers
    initFrameBuffers(window);

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

        processInput(window);

        // Rotate light 2
        if (lightRotationSpeed > 0.0f)
        {
            glm::vec4 rotatedLight = glm::rotate(glm::mat4(1.0f), lightRotationSpeed * deltaTime, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(config.lights[1].position, 1.0f);
            config.lights[1].position = glm::vec3(rotatedLight.x, rotatedLight.y, rotatedLight.z);
        }

        updateCameraMatrices();

        drawShadowMap();

        // Enable SRGB framebuffer
        glEnable(GL_FRAMEBUFFER_SRGB);

        glBindFramebuffer(GL_FRAMEBUFFER, accumBuffer);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // 1. geometry pass: render scene's geometry/color data into gbuffer
        {
            shader = deferred_shader;
            shader->use();

            glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

            prepareGeometryPass();

            drawObjects();

            restoreGeometryPass();

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }


        // 2. lighting pass: calculate lighting using the gbuffer's content
        {
            shader = lighting_shader;
            shader->use();

            glBindFramebuffer(GL_FRAMEBUFFER, accumBuffer);

            prepareDeferredPass();

            // render lights
            for (int i = 0; i < config.lights.size(); ++i)
            {
                Light& light = config.lights[i];

                drawDeferredLight(light);
            }

            restoreDeferredPass();

            // NEW! Draw skybox at the end, so we only process those fragments that are in the background
            drawSkybox();

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        // Final pass, render accumulation buffer
        if (postFXMode == PostFXMode::Realistic)
        {
            //TODO 9.4 : Bloom pass
            if (tempBuffers[0] != 0)
            {
                shader = copy_shader;
                shader->use();

                glBindFramebuffer(GL_FRAMEBUFFER, tempBuffers[0]);

                drawFullscreenPass("SourceTexture", gAccum);

                glBindFramebuffer(GL_FRAMEBUFFER, 0);
            }

            //TODO 9.3 : Blur passes
            for (int i = 0; i < 0; ++i)
            {
                shader = blur_shader;
                shader->use();

                int width, height;
                glfwGetFramebufferSize(window, &width, &height);

                // Horizontal blur pass
                glBindFramebuffer(GL_FRAMEBUFFER, tempBuffers[1]);
                shader->setVec2("blurScale", glm::vec2(1.0f / width, 0.0f));
                drawFullscreenPass("SourceTexture", tempTextures[0]);

                // Vertical blur pass
                glBindFramebuffer(GL_FRAMEBUFFER, tempBuffers[0]);
                shader->setVec2("blurScale", glm::vec2(0, 1.0f / height));
                drawFullscreenPass("SourceTexture", tempTextures[1]);

                glBindFramebuffer(GL_FRAMEBUFFER, 0);
            }

            //TODO 9.1 : Composition pass
            {
                //TODO 9.1 : Change the shader used for this pass to compose instead of copy
                shader = copy_shader;
                shader->use();

                //TODO 9.4 : Add tempTextures[0] as GL_TEXTURE1 and pass it as "BloomTexture"


                //TODO 9.1 : Add the exposure uniform


                //TODO 9.2 : Add the color grading uniforms




                drawFullscreenPass("SourceTexture", gAccum);
            }
        }
        else if (postFXMode == PostFXMode::CelShading)
        {
            // TODO 9.5 : Cel-shading pass
            {
                shader = copy_shader;
                shader->use();

                drawFullscreenPass("SourceTexture", gAccum);
            }
            // TODO 9.6 : Add the outline pass
            {
            }
        }
        else
        {
            shader = copy_shader;
            shader->use();

            drawFullscreenPass("SourceTexture", gAccum);
        }


        // Disable SRGB framebuffer
        glDisable(GL_FRAMEBUFFER_SRGB);

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

    delete deferred_shader;
    delete lighting_shader;
    delete skybox_shader;
    delete shadowMap_shader;

    delete copy_shader;
    delete compose_shader;
    delete bloom_shader;
    delete blur_shader;
    delete celshading_shader;
    delete outline_shader;

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

        ImGui::Text("Light 1: ");
        ImGui::DragFloat3("light 1 direction", (float*)&config.lights[0].position, .1f, -20, 20);
        ImGui::ColorEdit3("light 1 color", (float*)&config.lights[0].color);
        ImGui::SliderFloat("light 1 intensity", &config.lights[0].intensity, 0.0f, 5.0f);
        ImGui::Separator();

        ImGui::Text("Light 2: ");
        ImGui::DragFloat3("light 2 position", (float*)&config.lights[1].position, .1f, -20, 20);
        ImGui::ColorEdit3("light 2 color", (float*)&config.lights[1].color);
        ImGui::SliderFloat("light 2 intensity", &config.lights[1].intensity, 0.0f, 5.0f);
        ImGui::SliderFloat("light 2 radius", &config.lights[1].radius, 0.01f, 50.0f);
        ImGui::SliderFloat("light 2 speed", &lightRotationSpeed, 0.0f, 2.0f);
        ImGui::Separator();

        ImGui::Text("Car paint material: ");
        ImGui::ColorEdit3("color", (float*)&config.reflectionColor);
        ImGui::SliderFloat("roughness", &config.roughness, 0.01f, 1.0f);
        ImGui::SliderFloat("metalness", &config.metalness, 0.0f, 1.0f);
        ImGui::Separator();

        ImGui::Text("Post-processing: ");
        //TODO 9.1 9.2 9.4 9.5 and 9.6 : Add UI for configuration values






        ImGui::Text("Shading model: ");
        {
            if (ImGui::RadioButton("Realistic PostFX", postFXMode == PostFXMode::Realistic)) { postFXMode = PostFXMode::Realistic; }
            if (ImGui::RadioButton("Cel-Shading PostFX", postFXMode == PostFXMode::CelShading)) { postFXMode = PostFXMode::CelShading; }
            if (ImGui::RadioButton("No PostFX", postFXMode == PostFXMode::None)) { postFXMode = PostFXMode::None; }
        }

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}


void prepareGeometryPass()
{
    glClear(GL_DEPTH_BUFFER_BIT);

    // set up skybox texture
    shader->setInt("skybox", 5);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);

    shader->setMat4("view", view);
    shader->setMat4("viewProjection", viewProjection);
    shader->setVec3("cameraPosition", camera.Position);
}

void restoreGeometryPass()
{
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void prepareDeferredPass()
{
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

    // Set view projection for all lights
    shader->setMat4("viewProjection", viewProjection);
    // set inverse projection to reconstruct position from depth
    shader->setMat4("invProjection", glm::inverse(projection));

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
}

void restoreDeferredPass()
{
    // Restore values
    glDisable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ZERO);
    glDisable(GL_DEPTH_CLAMP);
    glCullFace(GL_BACK);
    glDisable(GL_CULL_FACE);
    glDepthMask(true);
    glEnable(GL_DEPTH_TEST);
}

void drawDeferredLight(Light& light)
{
    setLightUniforms(light, &camera);
    // Select geometry to render
    if (light.radius == 0)
    {
        // The quad is already in clip space
        glm::mat4 identity(1.0f);
        shader->setMat4("model", identity);
        shader->setMat4("viewProjection", identity);

        // Directional lights render a quad
        drawQuad();
    }
    else
    {
        // The model is positioned at the center of the light, with a size equal to the radius
        glm::mat4 model = glm::translate(glm::mat4(1.0f), light.position) * glm::scale(glm::mat4(1.0f), glm::vec3(light.radius));
        shader->setMat4("model", model);
        shader->setMat4("viewProjection", viewProjection);

        // Positional lights render a cube
        drawCube();
    }
}

void drawFullscreenPass(const char* sourceTextureName, GLuint sourceTexture)
{
    glDisable(GL_DEPTH_TEST);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sourceTexture);
    shader->setInt(sourceTextureName, 0);

    drawQuad();

    glEnable(GL_DEPTH_TEST);
}

void initFrameBuffers(GLFWwindow* window)
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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // normal color buffer
    glGenTextures(1, &gNormal);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, width, height, 0, GL_RG, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // others color buffer
    glGenTextures(1, &gOthers);
    glBindTexture(GL_TEXTURE_2D, gOthers);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // accumulation buffer
    // TODO 9.1 : Change the format of the accumulation buffer to 16bit floating point (4 components)
    glGenTextures(1, &gAccum);
    glBindTexture(GL_TEXTURE_2D, gAccum);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // depth texture buffer
    glGenTextures(1, &gDepth);
    glBindTexture(GL_TEXTURE_2D, gDepth);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // attach textures to framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gAlbedo, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gOthers, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, gAccum, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, gDepth, 0);

    // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering
    unsigned int attachments[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
    glDrawBuffers(4, attachments);

    // finally check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;


    // configure accumulation buffer framebuffer
    // ------------------------------
    glGenFramebuffers(1, &accumBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, accumBuffer);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gAccum, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, gDepth, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    // TODO 9.3 : Generate 2 frame buffers (variable tempBuffers) and 2 textures (variable tempTextures)



    for (int i = 0; i < 2; ++i)
    {
        // TODO 9.3 : Bind and configure temp textures with the same format as the accumulation buffer



        // TODO 9.3 : Bind temp framebuffers and attach the corresponding temp texture as color attachment 0



    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void setLightUniforms(Light& light, Camera* viewSpace)
{
    glm::vec3 position = light.position;
    glm::mat4 shadowMatrix = lightSpaceMatrix;

    // if a camera is provided, the light position will be relative to it, in view space
    if (viewSpace)
    {
        glm::vec4 viewSpacePosition = viewSpace->GetViewMatrix() * glm::vec4(light.position, light.radius > 0.0f ? 1.0f : 0.0f);
        position = glm::vec3(viewSpacePosition.x, viewSpacePosition.y, viewSpacePosition.z);

        if (light.shadow)
        {
            shadowMatrix = lightSpaceMatrix * glm::inverse(viewSpace->GetViewMatrix());
        }
    }

    // light uniforms
    shader->setVec3("lightPosition", position);
    shader->setVec3("lightColor", light.color * light.intensity * glm::pi<float>());
    shader->setFloat("lightRadius", light.radius);

    // shadow uniforms
    if (light.shadow)
    {
        shader->setMat4("lightSpaceMatrix", shadowMatrix);
        shader->setInt("ShadowMap", 5);
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, shadowMap);
        //shader->setFloat("shadowBias", config.shadowBias * 0.01f);
    }
}

void updateCameraMatrices()
{
    view = camera.GetViewMatrix();
    projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

    viewProjection = projection * view;
}


// init the VAO of the skybox
// --------------------------
unsigned int initSkyboxBuffers() {
    // triangles forming the six faces of a cube
    // note that the camera is placed inside of the cube, so the winding order
    // is selected to make the triangles visible from the inside
    float skyboxVertices[108]{
        // positions
        -1.0f, 1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,

        -1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,

        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,

        -1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, 1.0f
    };

    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);

    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    return skyboxVAO;
}

// loads a cubemap texture from 6 individual texture faces
// order:
// +X (right)
// -X (left)
// +Y (top)
// -Y (bottom)
// +Z (front)
// -Z (back)
// -------------------------------------------------------
unsigned int loadCubemap(vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrComponents;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrComponents, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_SRGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_MIRRORED_REPEAT);

    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    return textureID;
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
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
            1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
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


void drawSkybox()
{
    // render skybox
    glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
    skybox_shader->use();
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    skybox_shader->setMat4("projection", projection);
    skybox_shader->setMat4("view", view);
    skybox_shader->setInt("skybox", 0);

    // skybox cube
    glBindVertexArray(skyboxVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS); // set depth function back to default
}


void createShadowMap()
{
    // create depth texture
    glGenTextures(1, &shadowMap);
    glBindTexture(GL_TEXTURE_2D, shadowMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // if you replace GL_LINEAR with GL_NEAREST you will see pixelation in the borders of the shadow
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // if you replace GL_LINEAR with GL_NEAREST you will see pixelation in the borders of the shadow
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    // attach depth texture as FBO's depth buffer
    glGenFramebuffers(1, &shadowMapFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void drawShadowMap()
{
    Shader* currShader = shader;
    shader = shadowMap_shader;

    // setup depth shader
    shader->use();

    // We use an ortographic projection since it is a directional light.
    // left, right, bottom, top, near and far values define the 3D volume relative to
    // the light position and direction that will be rendered to produce the depth texture.
    // Geometry outside of this range will not be considered when computing shadows.
    float near_plane = 1.0f;
    float shadowMapSize = 6.0f;
    float shadowMapDepthRange = 10.0f;
    float half = shadowMapSize / 2.0f;
    glm::mat4 lightProjection = glm::ortho(-half, half, -half, half, near_plane, near_plane + shadowMapDepthRange);
    glm::mat4 lightView = glm::lookAt(glm::normalize(config.lights[0].position) * shadowMapDepthRange * 0.5f, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
    lightSpaceMatrix = lightProjection * lightView;
    shader->setMat4("lightSpaceMatrix", lightSpaceMatrix);

    // setup framebuffer size
    int viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);

    // bind our depth texture to the frame buffer
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);

    // clear the depth texture/depth buffer
    glClear(GL_DEPTH_BUFFER_BIT);

    // draw scene from the light's perspective into the depth texture
    drawObjects();

    // unbind the depth texture from the frame buffer, now we can render to the screen (frame buffer) again
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

    shader = currShader;
}

void drawObjects()
{
    // the typical transformation uniforms are already set for you, these are:
    // model (for each model part we draw)

    // material uniforms for car paint
    shader->setVec3("reflectionColor", config.reflectionColor);
    shader->setFloat("roughness", config.roughness);
    shader->setFloat("metalness", config.metalness);
    shader->setVec4("texCoordTransform", glm::vec4(1,1,0,0));    

    glm::mat4 model = glm::mat4(1.0f);
    shader->setMat4("model", model);
    carPaintModel->Draw(*shader);

    // material uniforms for other car parts (hardcoded)
    shader->setVec3("reflectionColor", 1.0f, 1.0f, 1.0f);
    shader->setFloat("roughness", 0.35f);
    shader->setFloat("metalness", 0.0f);

    carBodyModel->Draw(*shader);

    // draw car
    shader->setMat4("model", model);
    carLightModel->Draw(*shader);
    carInteriorModel->Draw(*shader);

    // draw wheel
    model = glm::translate(glm::mat4(1.0f), glm::vec3(-.7432f, .328f, 1.39f));
    shader->setMat4("model", model);
    carWheelModel->Draw(*shader);

    // draw wheel
    model = glm::translate(glm::mat4(1.0f), glm::vec3(-.7432f, .328f, -1.28f));
    shader->setMat4("model", model);
    carWheelModel->Draw(*shader);

    // draw wheel
    model = glm::rotate(glm::mat4(1.0f), glm::pi<float>(), glm::vec3(0.0, 1.0, 0.0));
    model = glm::translate(model, glm::vec3(-.7432f, .328f, 1.28f));
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
    shader->setFloat("roughness", 0.9f);
    shader->setVec4("texCoordTransform", glm::vec4(4.0f, 4.0f, 0, 0));
    floorModel->Draw(*shader);
    shader->setVec4("texCoordTransform", glm::vec4(1, 1, 0, 0));

    shader->setFloat("roughness", 0.05f);
    model = glm::mat4(1.0f);
    shader->setMat4("model", model);
    //carWindowsModel->Draw(*shader);
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

