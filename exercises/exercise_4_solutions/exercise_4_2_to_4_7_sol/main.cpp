#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <shader_s.h>

#include <iostream>
#include <vector>
#include <chrono>

void bindAttributes();
void createVertexBufferObject();
void emitParticle(float x, float y, float velocityX, float velocityY, float currentTime);
// glfw functions
void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

// const settings
const unsigned int SCR_WIDTH = 600;
const unsigned int SCR_HEIGHT = 600;

// application global variables
float lastX, lastY;                             // used to compute delta movement of the mouse
float currentTime;
unsigned int VAO, VBO;                          // vertex array and buffer objects
const unsigned int vertexBufferSize = 65536;    // # of particles

// TODO 4.2 update the number of attributes in a particle
const unsigned int particleSize = 5;            // particle attributes

const unsigned int sizeOfFloat = 4;             // bytes in a float
unsigned int particleId = 0;                    // keep track of last particle to be updated
Shader *shaderProgram;                          // our shader program

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
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);


    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // build and compile our shader program
    // ------------------------------------
    shaderProgram = new Shader("shaders/shader.vert", "shaders/shader.frag");

    // NEW!
    // enable built in variable gl_PointSize in the vertex shader
    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

    // TODO 4.4 enable alpha blending (for transparency)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_DST_ALPHA);

    createVertexBufferObject();

    // render every loopInterval seconds
    float loopInterval = 0.02f;
    auto begin = std::chrono::high_resolution_clock::now();

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // update current time
        auto frameStart = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> appTime = frameStart - begin;
        currentTime = appTime.count();

        // glfw input
        processInput(window);

        // set background color and replace frame buffer colors with the clear color
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // set shader program and the uniform value "currentTime"
        shaderProgram->use();

        // TODO 4.3 set uniform variable related to current time
        shaderProgram->setFloat("currentTime", currentTime);

        // render particles
        glBindVertexArray(VAO);
        glDrawArrays(GL_POINTS, 0, vertexBufferSize);

        // show the frame buffer
        glfwSwapBuffers(window);
        glfwPollEvents();

        // control render loop frequency
        std::chrono::duration<float> elapsed = std::chrono::high_resolution_clock::now()-frameStart;
        while (loopInterval > elapsed.count()) {
            // busy waiting
            elapsed = std::chrono::high_resolution_clock::now() - frameStart;
        }
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

void bindAttributes(){
    int posSize = 2; // each position has x,y
    GLuint vertexLocation = glGetAttribLocation(shaderProgram->ID, "pos");
    glEnableVertexAttribArray(vertexLocation);
    glVertexAttribPointer(vertexLocation, posSize, GL_FLOAT, GL_FALSE, particleSize * sizeOfFloat, 0);

    // TODO 4.2 set velocity and timeOfBirth shader attributes
        int velSize = 2;
    GLuint vertexVelocity = glGetAttribLocation(shaderProgram->ID, "velocity");
    glEnableVertexAttribArray(vertexVelocity);
    glVertexAttribPointer(vertexVelocity, velSize, GL_FLOAT, GL_FALSE,
                          particleSize * sizeOfFloat, (void*) (posSize * sizeOfFloat));

    int timeBirthSize = 1;
    GLuint timeBirthLocation = glGetAttribLocation(shaderProgram->ID, "timeOfBirth");
    glEnableVertexAttribArray(timeBirthLocation);
    glVertexAttribPointer(timeBirthLocation, timeBirthSize, GL_FLOAT, GL_FALSE,
                          particleSize * sizeOfFloat,  (void*) ( (posSize + velSize) * sizeOfFloat));
}

void createVertexBufferObject(){
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // initialize particle buffer, set all values to 0
    std::vector<float> data(vertexBufferSize * particleSize);
    for(unsigned int i = 0; i < data.size(); i++)
        data[i] = 0.0f;

    // allocate at openGL controlled memory
    glBufferData(GL_ARRAY_BUFFER, vertexBufferSize * particleSize * sizeOfFloat, &data[0], GL_DYNAMIC_DRAW);
    bindAttributes();
}

void emitParticle(float x, float y, float velocityX, float velocityY, float timeOfBirth){
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    float data[particleSize];
    data[0] = x;
    data[1] = y;

    // TODO 4.2 , add velocity and timeOfBirth to the particle data
    data[2] = velocityX;
    data[3] = velocityY;
    data[4] = timeOfBirth;

    // upload only parts of the buffer
    glBufferSubData(GL_ARRAY_BUFFER, particleId * particleSize * sizeOfFloat, particleSize * sizeOfFloat, data);
    particleId = (particleId + 1) % vertexBufferSize;
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // get screen size and click coordinates
    double xPos, yPos;
    int xScreen, yScreen;
    glfwGetCursorPos(window, &xPos, &yPos);
    glfwGetWindowSize(window, &xScreen, &yScreen);

    // convert from screen space to normalized display coordinates
    float xNdc = (float) xPos/(float) xScreen * 2.0f -1.0f;
    float yNdc = (float) yPos/(float) yScreen * 2.0f -1.0f;
    yNdc = -yNdc;

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        // compute velocity based on two consecutive updates
        float velocityX = xNdc - lastX;
        float velocityY = yNdc - lastY;
        float max_rand = (float) (RAND_MAX);
        // create 5 to 10 particles per frame
        int i = (int) ((float) (rand()) / max_rand) * 5;
        for (; i < 10; i++) {
            // add some randomness to the movement parameters
            float offsetX = ((float) (rand()) / max_rand - .5f) * .1f;
            float offsetY = ((float) (rand()) / max_rand - .5f) * .1f;
            float offsetVelX = ((float) (rand()) / max_rand - .5f) * .1f;
            float offsetVelY = ((float) (rand()) / max_rand - .5f) * .1f;
            // create the particle
            emitParticle(xNdc + offsetX, yNdc + offsetY, velocityX + offsetVelX, velocityY + offsetVelY, currentTime);
        }
    }
    lastX = xNdc;
    lastY = yNdc;

}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}