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

// function declarations
// ---------------------
void setLightUniforms();
void drawObjects();
void drawGui();

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
Shader* gouraud_shading;
Shader* phong_shading;
Model* carModel;
Model* carWheel;
Model* floorModel;
Camera camera(glm::vec3(0.0f, 1.6f, 5.0f));

// global variables used for control
// ---------------------------------
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
float deltaTime;
bool isPaused = false; // stop camera movement when GUI is open


// structure to hold lighting info
// -------------------------------
struct Config {

    // ambient light
    glm::vec3 ambientLightColor = {1.0f, 1.0f, 1.0f};
    float ambientLightIntensity = 0.2f;

    // light 1
    glm::vec3 light1Position = {-0.8f, 2.4f, 0.0f};
    glm::vec3 light1Color = {1.0f, 1.0f, 1.0f};
    float light1Intensity = 1.0f;

    // light 2
    glm::vec3 light2Position = {1.8f, .7f, 2.2f};
    glm::vec3 light2Color = {0.5f, 0.0f, 1.0f};
    float light2Intensity = 1.0f;

    // material
    glm::vec3 reflectionColor = {1.0f, 1.0f, 0.0f};
    float ambientReflectance = 0.5f;
    float diffuseReflectance = 0.5f;
    float specularReflectance = 0.7f;
    float specularExponent = 20.0f;

} config;



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
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Exercise 5", NULL, NULL);
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
    gouraud_shading = new Shader("shaders/gouraud_shading.vert", "shaders/gouraud_shading.frag");
    phong_shading = new Shader("shaders/phong_shading.vert", "shaders/phong_shading.frag");
    shader = gouraud_shading;
    carModel = new Model(std::vector<string>{"car/Body_LOD0.obj", "car/Interior_LOD0.obj", "car/Paint_LOD0.obj", "car/Light_LOD0.obj", "car/Windows_LOD0.obj"});
    carWheel = new Model("car/Wheel_LOD0.obj");
    floorModel = new Model("floor/floor.obj");

    // set up the z-buffer
    // -------------------
    glDepthRange(-1,1); // make the NDC a right handed coordinate system, with the camera pointing towards -z
    glEnable(GL_DEPTH_TEST); // turn on z-buffer depth test
    glDepthFunc(GL_LESS); // draws fragments that are closer to the screen in NDC


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

        glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader->use();
        setLightUniforms();
        drawObjects();

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

    delete carModel;
    delete floorModel;
    delete carWheel;
    delete gouraud_shading;
    delete phong_shading;

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}


void drawGui(){
    // NEW! we are using the header library Dear ImGui to set the variables in a graphical interface
    //  here is where we define the buttons and how they map to the variables in our program

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
        ImGui::DragFloat3("light 1 position", (float*)&config.light1Position, .1f, -20, 20);
        ImGui::ColorEdit3("light 1 color", (float*)&config.light1Color);
        ImGui::SliderFloat("light 1 intensity", &config.light1Intensity, 0.0f, 1.0f);
        ImGui::Separator();

        ImGui::Text("Light 2: ");
        ImGui::DragFloat3("light 2 position", (float*)&config.light2Position, .1f, -20, 20);
        ImGui::ColorEdit3("light 2 color", (float*)&config.light2Color);
        ImGui::SliderFloat("light 2 intensity", &config.light2Intensity, 0.0f, 1.0f);
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
            if (ImGui::RadioButton("Gouraud Shading", shader == gouraud_shading)) { shader = gouraud_shading; }
            if (ImGui::RadioButton("Phong Shading", shader == phong_shading)) { shader = phong_shading; }
        }
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}


void setLightUniforms()
{
    // TODO exercise 5 - set the missing uniform variables here
    // light uniforms
    shader->setVec3("ambientLightColor", config.ambientLightColor * config.ambientLightIntensity);
    shader->setVec3("light1Position", config.light1Position);
	shader->setVec3("light1Color", config.light1Color * config.light1Intensity);
	shader->setVec3("light2Position", config.light2Position);
	shader->setVec3("light2Color", config.light2Color * config.light2Intensity);
}


void drawObjects(){

    // camera position
    shader->setVec3("camPosition", camera.Position);

    // TODO exercise 5 - set the missing uniform variables here
    // material uniforms
    shader->setVec3("reflectionColor", config.reflectionColor);
    shader->setFloat("ambientReflectance", config.ambientReflectance);
    shader->setFloat("diffuseReflectance", config.diffuseReflectance);
    shader->setFloat("specularReflectance", config.specularReflectance);
    shader->setFloat("specularExponent", config.specularExponent);


    // the typical transformation uniforms are already set for you, these are:
    // projection (perspective projection matrix)
    // view (to map world space coordinates to the camera space, so the camera position becomes the origin)
    // model (for each model part we draw)

    // camera parameters
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 viewProjection = projection * view;

    // set viewProjection matrix uniform
    shader->setMat4("viewProjection", viewProjection);

    // NEW! we use the Model class to load the geometry and dispatch the render commands to OpenGL
    // draw car
    glm::mat4 model = glm::mat4(1.0f);
    shader->setMat4("model", model);
    carModel->Draw();

    // draw wheel
    model = glm::translate(glm::mat4(1.0f), glm::vec3(-.7432f, .328f, 1.39f));
    shader->setMat4("model", model);
    carWheel->Draw();

    // draw wheel
    model = glm::translate(glm::mat4(1.0f), glm::vec3(-.7432f, .328f, -1.39f));
    shader->setMat4("model", model);
    carWheel->Draw();

    // draw wheel
    model = glm::rotate(glm::mat4(1.0f), glm::pi<float>(), glm::vec3(0.0, 1.0, 0.0));
    model = glm::translate(model, glm::vec3(-.7432f, .328f, 1.39f));
    shader->setMat4("model", model);
    carWheel->Draw();

    // draw wheel
    model = glm::rotate(glm::mat4(1.0f), glm::pi<float>(), glm::vec3(0.0, 1.0, 0.0));
    model = glm::translate(model, glm::vec3(-.7432f, .328f, -1.39f));
    shader->setMat4("model", model);
    carWheel->Draw();

    // draw floor,
    // notice that we overwrite the value of one of the uniform variables to set a different floor color
    shader->setVec3("reflectionColor", .2f, .5f, .2f);
    model = glm::scale(glm::mat4(1.0), glm::vec3(5.f, 5.f, 5.f));
    shader->setMat4("model", model);
    floorModel->Draw();
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