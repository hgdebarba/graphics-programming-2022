#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <shader.h>

#include <iostream>
#include <vector>
#include <math.h>

// structure to hold the info necessary to render an object
struct SceneObject {
    unsigned int VAO;           // vertex array object handle
    unsigned int vertexCount;   // number of vertices in the object
    float r, g, b;              // for object color
    float x, y;                 // for position offset
};

// declaration of the function you will implement in exercise 2.1
SceneObject instantiateCone(float r, float g, float b, float offsetX, float offsetY);
// mouse, keyboard and screen reshape glfw callbacks
void button_input_callback(GLFWwindow* window, int button, int action, int mods);
void key_input_callback(GLFWwindow* window, int button, int other,int action, int mods);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

// settings
const unsigned int SCR_WIDTH = 600;
const unsigned int SCR_HEIGHT = 600;

// global variables we will use to store our objects, shaders, and active shader
std::vector<SceneObject> sceneObjects;
std::vector<Shader> shaderPrograms;
Shader* activeShader;


int main()
{
    // glfw: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

    // glfw window creation
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Exercise 2 - Voronoi Diagram", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    // setup frame buffer size callback
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    // setup input callbacks
    glfwSetMouseButtonCallback(window, button_input_callback); // NEW!
    glfwSetKeyCallback(window, key_input_callback); // NEW!

    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // NEW!
    // build and compile the shader programs
    shaderPrograms.push_back(Shader("shaders/shader.vert", "shaders/color.frag"));
    shaderPrograms.push_back(Shader("shaders/shader.vert", "shaders/distance.frag"));
    shaderPrograms.push_back(Shader("shaders/shader.vert", "shaders/distance_color.frag"));
    activeShader = &shaderPrograms[0];

    // NEW!
    // set up the z-buffer
    glDepthRange(1,-1); // make the NDC a right handed coordinate system, with the camera pointing towards -z
    glEnable(GL_DEPTH_TEST); // turn on z-buffer depth test
    glDepthFunc(GL_LESS); // draws fragments that are closer to the screen in NDC

    // TODO exercise 2.6
    /* Comment this line to enable 2.6
    // enable blending
	glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    // choose the right blending factors to produce additive blending
    glBlendFunc(GL_ONE, GL_ONE); // Uncomment for additive blending
	// glBlendFunc(GL_DST_COLOR, GL_ZERO); // Uncomment for multiplicative blending. Don't forget to change clear color to white!
    //*/

    // render loop
    while (!glfwWindowShouldClose(window)) {
        // background color
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        // notice that now we are clearing two buffers, the color and the z-buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // render the cones
        glUseProgram(activeShader->ID);

        // TODO exercise 2.3
        // Iterate through the scene objects, for each object:
        // - bind the VAO; set the uniform variables; and draw.
        for (auto it = sceneObjects.begin(); it != sceneObjects.end(); it++){
            glBindVertexArray(it->VAO);
            activeShader->setVec3("uColor", it->r, it->g, it->b);
            activeShader->setVec2("uPosition", it->x, it->y);
            glDrawElements(GL_TRIANGLES, it->vertexCount, GL_UNSIGNED_INT, 0);
        }


        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    glfwTerminate();
    return 0;
}


// creates a cone triangle mesh, uploads it to openGL and returns the VAO associated to the mesh
SceneObject instantiateCone(float r, float g, float b, float offsetX, float offsetY){

    // Create an instance of a SceneObject,
    SceneObject sceneObject{};

    // you will need to store offsetX, offsetY, r, g and b in the object.
	sceneObject.x = offsetX;
	sceneObject.y = offsetY;
    sceneObject.r = r;
	sceneObject.g = g;
	sceneObject.b = b;

    // Build the geometry into an std::vector<float> or float array.

	std::vector<float> vertexData;
	std::vector<GLint> vertexIndices;

	// vertex center
	vertexData.push_back(0.0f);
	vertexData.push_back(0.0f);
	vertexData.push_back(1.0f);

	int triangleCount = 16;
	float PI = 3.14159265f;
	float angleInterval = (2 * PI) / (float)triangleCount;
    float radius = 3.0f;

	for (int i = 0; i <= triangleCount; i++) {
		float angle = i * angleInterval;
		// vertex circle at angle i*angleInterval
		vertexData.push_back(cos(angle) * radius);
		vertexData.push_back(sin(angle) * radius);
		vertexData.push_back(0.0f);
	}

	for (int i = 0; i < triangleCount; i++) {
		vertexIndices.push_back(0);
		vertexIndices.push_back(i + 1);
		vertexIndices.push_back(i + 2);
	}

    // Store the number of vertices in the mesh in the scene object.
    sceneObject.vertexCount = triangleCount * 3;

    // Declare and generate a VAO and VBO (and an EBO if you decide the work with indices).
	unsigned int VAO, VBO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

    // Bind and set the VAO and VBO (and optionally a EBO) in the correct order.
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(GLfloat), vertexData.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, vertexIndices.size() * sizeof(GLint), vertexIndices.data(), GL_STATIC_DRAW);

    // Set the position attribute pointers in the shader.
    int posAttributeLocation = glGetAttribLocation(activeShader->ID, "pos");
	glEnableVertexAttribArray(posAttributeLocation);
	glVertexAttribPointer(posAttributeLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);

    // Store the VAO handle in the scene object.
    sceneObject.VAO = VAO;

    // Unbind VAO, VBO (and optionally a EBO)
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // 'return' the scene object for the cone instance you just created.
    return sceneObject;
}

// glfw: called whenever a mouse button is pressed
void button_input_callback(GLFWwindow* window, int button, int action, int mods){
    // TODO exercise 2.2
    // (exercises 1.9 and 2.2 can help you with implementing this function)

    // Test button press, see documentation at:
    //     https://www.glfw.org/docs/latest/input_guide.html#input_mouse_button
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        // If a left mouse button press was detected, call instantiateCone:
        // - Push the return value to the back of the global 'vector<SceneObject> sceneObjects'.
        // - The click position should be transformed from screen coordinates to normalized device coordinates,
        //   to obtain the offset values that describe the position of the object in the screen plane.
        // - A random value in the range [0, 1] should be used for the r, g and b variables.
        double xPos, yPos;
        glfwGetCursorPos(window, &xPos, &yPos);
        float offsetX = static_cast<float>((xPos / SCR_WIDTH) * 2 - 1);
		float offsetY = static_cast<float>((1 - yPos / SCR_HEIGHT) * 2 - 1);
		SceneObject sceneObject = instantiateCone(rand()/(RAND_MAX + 1.0f), rand() / (RAND_MAX + 1.0f), rand() / (RAND_MAX + 1.0f), offsetX, offsetY);
        sceneObjects.push_back(sceneObject);
	}
}

// glfw: called whenever a keyboard key is pressed
void key_input_callback(GLFWwindow* window, int key, int other,int action, int mods){
    // TODO exercise 2.4

    // Set the activeShader variable by detecting when the keys 1, 2 and 3 were pressed;
    // see documentation at https://www.glfw.org/docs/latest/input_guide.html#input_keyboard
    // Key 1 sets the activeShader to &shaderPrograms[0];
    //   and so on.
    if (action == GLFW_PRESS)
    {
        switch (key)
        {
        case GLFW_KEY_1:
            activeShader = &shaderPrograms[0];
            break;
		case GLFW_KEY_2:
			activeShader = &shaderPrograms[1];
			break;
		case GLFW_KEY_3:
			activeShader = &shaderPrograms[2];
			break;
        }
    }
}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}