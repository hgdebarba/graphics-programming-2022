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
Shader* pbr_shading;
Model* carPaintModel;
Model* floorModel;
Camera camera(glm::vec3(0.0f, 1.6f, 5.0f), glm::vec3(0.0f, 1.0f, 0.0f), (float)SCR_WIDTH / SCR_HEIGHT);
Camera cullingCamera;

bool updateCulling = true;
int cullingShader = -1;




GLuint sourceInstanceBuffer;
GLuint visibleInstanceBuffer;
GLuint indirectDrawBuffer;

Shader* skyboxShader;
unsigned int skyboxVAO; // skybox handle
unsigned int cubemapTexture; // skybox texture handle

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
        lights.emplace_back(glm::vec3(-1.0f, 1.0f, -0.5f), glm::vec3(1.0f, 1.0f, 1.0f), 1.0f, 0.0f);
    }

    // material
    glm::vec3 reflectionColor = {0.9f, 0.9f, 0.2f};
    float roughness = 0.5f;
    float metalness = 0.0f;

    std::vector<Light> lights;

    bool enableCulling = true;

    // TODO 12.2 : Change the default value to true
    bool enableInstancing = true;
} config;

// structure to hold car instances
struct Car
{
    glm::mat4 modelMatrix;
    glm::vec4 color;
};
std::vector<Car> cars;

// function declarations
// ---------------------
void setAmbientUniforms(glm::vec3 ambientLightColor);
void setLightUniforms(Light &light);
void setupForwardAdditionalPass();
void resetForwardAdditionalPass();
void drawSkybox();
void drawObjects();
void drawGui();
unsigned int initSkyboxBuffers();
unsigned int loadCubemap(vector<std::string> faces);

bool isFrustumVisibleSphere(const Camera &camera, const glm::mat4& matrix, float radius);
void createCarInstances();
void createCullingCompute();
void runCullingCompute();

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Exercise 12", NULL, NULL);
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

    // NEW! Disable V sync to have better visibility on the frame rate
    glfwSwapInterval(0);

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
    pbr_shading = new Shader("shaders/common_shading.vert", "shaders/pbr_shading.frag");
    shader = pbr_shading;

    carPaintModel = new Model("car/Paint_LOD0.obj");

    floorModel = new Model("floor/floor.obj");

    // create all cars
    createCarInstances();

    // create compute shader for frustum culling on GPU
    createCullingCompute();

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
    skyboxShader = new Shader("shaders/skybox.vert", "shaders/skybox.frag");

    // set up the z-buffer
    // -------------------
    glDepthRange(-1,1); // make the NDC a right handed coordinate system, with the camera pointing towards -z
    glEnable(GL_DEPTH_TEST); // turn on z-buffer depth test
    glDepthFunc(GL_LESS); // draws fragments that are closer to the screen in NDC


    // Enable SRGB framebuffer
    glEnable(GL_FRAMEBUFFER_SRGB);

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

        glm::mat4 projection = camera.GetProjectionMatrix();
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 viewProjection = projection * view;

        processInput(window);

        glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        drawSkybox();

        shader->use();


        // First light + ambient
        setAmbientUniforms(glm::vec3(1.0f));
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

        drawGui();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    // -------
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    delete carPaintModel;
    delete floorModel;
    delete pbr_shading;

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

    if (isPaused)
    {
        ImGui::Begin("Settings");

        ImGui::Text("Light 1: ");
        ImGui::DragFloat3("light 1 direction", (float*)&config.lights[0].position, .1f, -20, 20);
        ImGui::ColorEdit3("light 1 color", (float*)&config.lights[0].color);
        ImGui::SliderFloat("light 1 intensity", &config.lights[0].intensity, 0.0f, 2.0f);
        ImGui::Separator();

        ImGui::Text("Car paint material: ");
        ImGui::SliderFloat("roughness", &config.roughness, 0.01f, 1.0f);
        ImGui::SliderFloat("metalness", &config.metalness, 0.0f, 1.0f);
        ImGui::Separator();

        ImGui::Checkbox("Frustum Culling", &config.enableCulling);
        ImGui::Checkbox("Instancing",  &config.enableInstancing);

        ImGui::End();
    }

    ImGui::Begin("FPS", nullptr, ImGuiWindowFlags_NoDecoration);
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::End();

    glDisable(GL_FRAMEBUFFER_SRGB);
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glEnable(GL_FRAMEBUFFER_SRGB);
}


void setAmbientUniforms(glm::vec3 ambientLightColor)
{
    // ambient uniforms
    shader->setVec4("ambientLightColor", glm::vec4(ambientLightColor, glm::length(ambientLightColor) > 0.0f ? 1.0f : 0.0f));
}

void setLightUniforms(Light& light)
{
    glm::vec3 lightEnergy = light.color * light.intensity;

    lightEnergy *= glm::pi<float>();

    // light uniforms
    shader->setVec3("lightPosition", light.position);
    shader->setVec3("lightColor", lightEnergy);
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
    setAmbientUniforms(glm::vec3(1.0f));

    //Disable blend and restore default blend function
    glDisable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ZERO);

    // Restore default depth test
    glDepthFunc(GL_LESS);
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
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    return textureID;
}



void drawSkybox()
{
    // render skybox
    glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
    skyboxShader->use();
    glm::mat4 projection = camera.GetProjectionMatrix();
    glm::mat4 view = camera.GetViewMatrix();
    skyboxShader->setMat4("projection", projection);
    skyboxShader->setMat4("view", view);
    skyboxShader->setInt("skybox", 0);

    // skybox cube
    glBindVertexArray(skyboxVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS); // set depth function back to default
}

void drawObjects()
{
    // the typical transformation uniforms are already set for you, these are:
    // projection (perspective projection matrix)
    // view (to map world space coordinates to the camera space, so the camera position becomes the origin)
    // model (for each model part we draw)

    // camera parameters
    glm::mat4 projection = camera.GetProjectionMatrix();
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 viewProjection = projection * view;

    // camera position
    shader->setVec3("camPosition", camera.Position);
    // set viewProjection matrix uniform
    shader->setMat4("viewProjection", viewProjection);

    // set up skybox texture
    shader->setInt("skybox", 5);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);

    // material uniforms for car paint
    shader->setFloat("roughness", config.roughness);
    shader->setFloat("metalness", config.metalness);

    // Copy current camera to culling camera, if culling update is enabled
    // Normally you would use the camera directly, we do it in this way so you can pause culling, move the camera, and observe the culling results easily
    if (updateCulling)
        cullingCamera = camera;

    // Draw all cars
    if (!config.enableInstancing)
    {
        for (const Car& car : cars)
        {
            // TODO 12.1 : Only execute this block if culling is not enabled or if the bounding sphere is visible in cullingCamera
            if (!config.enableCulling || isFrustumVisibleSphere(cullingCamera, car.modelMatrix, 2.5f))
            {
                shader->setMat4("model", car.modelMatrix);
                shader->setVec4("reflectionColor", car.color);
                carPaintModel->Draw(*shader);
            }
        }
    }
    else
    {
        // TODO 12.3 : if culling is enabled, run culling compute
        if (config.enableCulling)
        {
            runCullingCompute();
        }


        // TODO 12.3 : Bind the visible instance buffer, if culling is enabled, or source instance buffer, if it is not
        // TODO 12.2 : Bind the source instance buffer as GL_SHADER_STORAGE_BUFFER, with index 0
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, config.enableCulling ? visibleInstanceBuffer : sourceInstanceBuffer);

        // TODO 12.3 : Add an extra parameter with the indirect buffer, if culling is enabled, or 0, if it is not
        // TODO 12.2 : Draw the carPaintModel, using the same shader, but with an extra parameter for the number of cars
        carPaintModel->Draw(*shader, (int)cars.size(), config.enableCulling ? indirectDrawBuffer : 0);

        // TODO 12.2 : Unbind the GL_SHADER_STORAGE_BUFFER
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
    }

    // draw floor
    {
        glm::mat4 model = glm::scale(glm::mat4(1.0), glm::vec3(5.f, 5.f, 5.f));
        shader->setMat4("model", model);
        shader->setVec4("reflectionColor", 1.0f, 1.0f, 1.0f, 1.0f);
        shader->setFloat("metalness", 0.0f);
        shader->setFloat("roughness", 0.95f);
        floorModel->Draw(*shader);
    }
}

// Checks if a bounding sphere (determined by its transform matrix and radius) is visible inside the frustum of a camera
bool isFrustumVisibleSphere(const Camera& camera, const glm::mat4& matrix, float radius)
{
    bool visible = true;

    glm::vec3 center = matrix[3];
    glm::vec3 planePoint, planeNormal;
    for (int plane = (int)Camera_Planes::FIRST_PLANE; plane < (int)Camera_Planes::PLANE_COUNT; ++plane)
    {
        camera.GetFrustumPlane((Camera_Planes)plane, planePoint, planeNormal);

        // TODO 12.1 : Get the distance of the center to the plane and compare it to the radius. If it is not visible, return false.
        visible = glm::dot(center - planePoint, planeNormal) > -radius;

        if (!visible)
            break;
    }
    return visible;
}

void createCarInstances()
{
    const glm::ivec2 side(40, 15); // Create a grid of 81 x 31 cars ~ 2500 cars
    const glm::vec2 separation(2.0f, 5.0f);
    int carCount = (side.x * 2 + 1) * (side.y * 2 + 1);
    cars.reserve(carCount);
    for (int j = -side.y; j <= side.y; ++j)
    {
        for (int i = -side.x; i <= side.x; ++i)
        {
            Car car;
            // Model transformation matrix. No rotation or scale, just translation
            car.modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(i * separation.x, 0.0f, j * separation.y));
            // Random color
            car.color = glm::vec4(rand() / double(RAND_MAX), rand() / double(RAND_MAX), rand() / double(RAND_MAX), 1.0f);
            cars.push_back(car);
        }
    }

    // create a buffer that contains all the instance data. It is STATIC because we won't modify it
    glGenBuffers(1, &sourceInstanceBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, sourceInstanceBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, cars.size() * sizeof(Car), cars.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // create a buffer that can contain all the instance data. It is DYNAMIC because we will copy only the visible instances every frame
    glGenBuffers(1, &visibleInstanceBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, visibleInstanceBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, cars.size() * sizeof(Car), cars.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // create the indirect draw buffer
    glGenBuffers(1, &indirectDrawBuffer);
}

void createCullingCompute()
{
    cullingShader = glCreateProgram();

    int computeShader = glCreateShader(GL_COMPUTE_SHADER);

    std::ifstream shaderStream("shaders/culling.glsl");
    std::ostringstream stringStream;
    stringStream << shaderStream.rdbuf();
    string shaderCodeStr = stringStream.str();
    const char* shaderCode = shaderCodeStr.c_str();
    glShaderSource(computeShader, 1, &shaderCode, nullptr);
    glCompileShader(computeShader);
    Shader::checkCompileErrors(computeShader, "COMPUTE");
    glAttachShader(cullingShader, computeShader);

    glLinkProgram(cullingShader);
    Shader::checkCompileErrors(cullingShader, "PROGRAM");

    glDeleteShader(computeShader);
}

void runCullingCompute()
{
    // Fill the indirect buffer with the initial data
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirectDrawBuffer);
    int indirectData[5] =
    {
        (int)carPaintModel->meshes[0].indices.size(),
        0, // instance count
        0, // first index
        0, // base vertex
        0  // base instance
    };
    glBufferData(GL_DRAW_INDIRECT_BUFFER, 5 * sizeof(int), indirectData, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);

    // Set the compute shader as the active shader
    glUseProgram(cullingShader);

    // Gather the 6 frustum planes
    glm::vec3 planes[6 * 2];
    for (int plane = (int)Camera_Planes::FIRST_PLANE; plane < (int)Camera_Planes::PLANE_COUNT; ++plane)
    {
        cullingCamera.GetFrustumPlane((Camera_Planes)plane, planes[plane * 2], planes[plane * 2 + 1]);
    }
    // Pass the uniforms
    glUniform1f(glGetUniformLocation(cullingShader, "cullingRadius"), 2.5f);
    glUniform3fv(glGetUniformLocation(cullingShader, "frustumPlanes"), 6 * 2, (const float*)planes);

    // Bind the buffers:
    // - sourceInstanceBuffer: the instance data of all the cars
    // - visibleInstanceBuffer: the destination buffer, to store only the visible cars
    // - indirectDrawBuffer: the indirect buffer, to modify the count of visible instances
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, sourceInstanceBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, visibleInstanceBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, indirectDrawBuffer);
    // Dispatch the cars, in groups of 64
    glDispatchCompute(((int)cars.size() + 63) / 64, 1, 1);

    // Make sure that the visibleInstanceBuffer and indirectDrawBuffer are finished being written to
    glMemoryBarrier(GL_ATOMIC_COUNTER_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);

    // restore pbr shader
    shader->use();
}


void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (isPaused)
        return;

    // movement commands
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera_Movement::FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera_Movement::BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera_Movement::LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera_Movement::RIGHT, deltaTime);
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
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        isPaused = !isPaused;
        glfwSetInputMode(window, GLFW_CURSOR, isPaused ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
    }

    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
    {
        updateCulling = !updateCulling;
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