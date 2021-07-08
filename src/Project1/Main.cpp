//
// Sandbox program for Computer Graphics For Games (G4G)
// created May 2021 by Eric Ameres for experimenting
// with OpenGL and various graphics algorithms
//

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <string>
#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <filesystem>

#include "shader_s.h"

#pragma warning( disable : 26451 )
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);

// settings
const unsigned int SCR_WIDTH = 1240;
const unsigned int SCR_HEIGHT = 910;

unsigned int VBO, VAO[3], EBO, indexCount[3], VCO, VTO, VNO;
unsigned int texture[2];
unsigned int cubeTexture;
unsigned int vaoCount = 0;

unsigned int cubeVAO, cubeVBO;

glm::mat4 pMat;
glm::mat4 vMat;

glm::mat4* iModelMatrices;

// image buffer used by raster drawing basics.cpp
extern unsigned char imageBuff[256][256][4];

char vtext[1024 * 8]; // fragment shader source code
char ftext[1024 * 8]; // vertex shader source code

Shader ourShader;

int nTexture = 0;

using namespace std;

float myMatrix[4][4] = { { 1.0f, 0.0f, 0.0f, 0.5f },{ 0.0f, 1.0f, 0.0f, 0.5f },{ 0.0f, 0.0f, 1.0f, 0.0f },{ 0.0f, 0.0f, 0.0f, 1.0f } };

string readFile(const char* filePath) {
    std::string content;
    std::ifstream myFile;
    // ensure ifstream objects can throw exceptions:
    myFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try{
        // open files
        myFile.open(filePath);
        std::stringstream myStream;
        // read file's buffer contents into streams
        myStream << myFile.rdbuf();
        // close file handler
        myFile.close();
        // convert stream into string
        content = myStream.str();
    }
    catch (std::ifstream::failure& e)
    {
        std::cout << "ERROR::FILE_NOT_SUCCESFULLY_READ" << std::endl;
    }
    return content;
}
void writeMatrix(const char* filePath) {
    std::ofstream myfile;

    myfile.open(filePath);
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            myfile << myMatrix[i][j];
            myfile << " ";
        }
        myfile << "\n";
    }
    myfile.close();
}

void readMatrix(const char* filePath) {
    std::ofstream myfile;

    ifstream source;
    
    source.open(filePath, ios::in);

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            source >> myMatrix[i][j];
        }
    }
    myfile.close();
}


void saveShaders() {
    std::ofstream myfile;

    myfile.open("data/texture.vs");
    myfile << vtext;
    myfile.close();

    myfile.open("data/texture.fs");
    myfile << ftext;
    myfile.close();

    writeMatrix("data/myMatrix.txt");
}

int myTexture();


void setupTextures()
{
    // create textures 
        // -------------------------
    glGenTextures(2, texture);

    // first texture is loaded from a file
    glBindTexture(GL_TEXTURE_2D, texture[0]); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object

    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // load image, create texture and generate mipmaps
    int width = 0, height = 0, nrChannels = 0;

    stbi_set_flip_vertically_on_load(true);

    unsigned char* data = stbi_load("data/rpi.png", &width, &height, &nrChannels, 0);

    if (data)
    {
        if (nrChannels == 4)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        else
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

        glGenerateMipmap(GL_TEXTURE_2D);
    }

    stbi_image_free(data);

    // second texture is a buffer we will be generating for pixel experiments
    glBindTexture(GL_TEXTURE_2D, texture[1]); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object

    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // load image, create texture and generate mipmaps
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, (const void*)imageBuff);
    glGenerateMipmap(GL_TEXTURE_2D);
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
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}
void setupSkybox()
{
    float cubeVertexPositions[108] =
    { -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f, 1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f,
        1.0f, -1.0f, -1.0f, 1.0f, -1.0f,  1.0f, 1.0f,  1.0f, -1.0f,
        1.0f, -1.0f,  1.0f, 1.0f,  1.0f,  1.0f, 1.0f,  1.0f, -1.0f,
        1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f, 1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f, 1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f, 1.0f,  1.0f, -1.0f, 1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f
    };

    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);

    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertexPositions), cubeVertexPositions, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    // load textures
    // -------------
    vector<std::string> faces
    {
        "data/cubemap/xp.jpg",
        "data/cubemap/xn.jpg",
        "data/cubemap/yp.jpg",
        "data/cubemap/yn.jpg",
        "data/cubemap/zp.jpg",
        "data/cubemap/zn.jpg",
    };
    cubeTexture = loadCubemap(faces);
}


void setupIMatrices() {
    unsigned int amount = 100000;
    iModelMatrices = new glm::mat4[amount];
    for (unsigned int i = 0; i < amount; i++)
    {
        glm::mat4 model = glm::mat4(1.0f);

        model = glm::translate(model, glm::vec3((rand()%100)-50, (rand() % 100) - 50, (rand() % 100) - 50));

        iModelMatrices[i] = model;
    }

    // configure instanced array
    // -------------------------
    unsigned int buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, amount * sizeof(glm::mat4), &iModelMatrices[0], GL_STATIC_DRAW);

    // set transformation matrices as an instance vertex attribute (with divisor 1)
    // -----------------------------------------------------------------------------------------------------------------------------------

    // set attribute pointers for matrix (4 times vec4)
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
    glEnableVertexAttribArray(7);
    glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));

    glVertexAttribDivisor(4, 1);
    glVertexAttribDivisor(5, 1);
    glVertexAttribDivisor(6, 1);
    glVertexAttribDivisor(7, 1);
}

void setupCube()
{

    float vertexPositions[108] = {
        -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f,
        1.0f, -1.0f, -1.0f, 1.0f, -1.0f,  1.0f, 1.0f,  1.0f, -1.0f, 1.0f, -1.0f,  1.0f, 1.0f,  1.0f,  1.0f, 1.0f,  1.0f, -1.0f,
        1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f, 1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f, 1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f, 1.0f,  1.0f, -1.0f, 1.0f,  1.0f,  1.0f,    1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f
    };

    float vertexNormals[108] = {
         0.0f,  0.0f, -1.0f,   0.0f,  0.0f, -1.0f,   0.0f,  0.0f, -1.0f,   0.0f,  0.0f, -1.0f,   0.0f,  0.0f, -1.0f,   0.0f,  0.0f, -1.0f,
         1.0f,  0.0f,  0.0f,   1.0f,  0.0f,  0.0f,   1.0f,  0.0f,  0.0f,   1.0f,  0.0f,  0.0f,   1.0f,  0.0f,  0.0f,   1.0f,  0.0f,  0.0f,
         0.0f,  0.0f,  1.0f,   0.0f,  0.0f,  1.0f,   0.0f,  0.0f,  1.0f,   0.0f,  0.0f,  1.0f,   0.0f,  0.0f,  1.0f,   0.0f,  0.0f,  1.0f,
        -1.0f,  0.0f,  0.0f,  -1.0f,  0.0f,  0.0f,  -1.0f,  0.0f,  0.0f,  -1.0f,  0.0f,  0.0f,  -1.0f,  0.0f,  0.0f,  -1.0f,  0.0f,  0.0f,
         0.0f, -1.0f,  0.0f,   0.0f, -1.0f,  0.0f,   0.0f, -1.0f,  0.0f,   0.0f, -1.0f,  0.0f,   0.0f, -1.0f,  0.0f,   0.0f, -1.0f,  0.0f,
         0.0f,  1.0f,  0.0f,   0.0f,  1.0f,  0.0f,   0.0f,  1.0f,  0.0f,   0.0f,  1.0f,  0.0f,   0.0f,  1.0f,  0.0f,   0.0f,  1.0f,  0.0f,
    };
    float texCoords[72] = {
        1.0f, 1.0f,  1.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 1.0f,  1.0f, 1.0f,
        1.0f, 0.0f,  0.0f, 0.0f,  1.0f, 1.0f,  0.0f, 0.0f,  0.0f, 1.0f,  1.0f, 1.0f,
        1.0f, 0.0f,  0.0f, 0.0f,  1.0f, 1.0f,  0.0f, 0.0f,  0.0f, 1.0f,  1.0f, 1.0f,
        1.0f, 0.0f,  0.0f, 0.0f,  1.0f, 1.0f,  0.0f, 0.0f,  0.0f, 1.0f,  1.0f, 1.0f,
        1.0f, 1.0f,  1.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 1.0f,  1.0f, 1.0f,
        1.0f, 1.0f,  1.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 1.0f,  1.0f, 1.0f
    };

    glBindVertexArray(VAO[vaoCount++]);
    indexCount[vaoCount - 1] = 0;

    // position attribute
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexPositions), vertexPositions, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);

    // we're not using a color attribute for this model
    // glGenBuffers(1, &VCO);
    
    // texture attribute
    glGenBuffers(1, &VTO);
    glBindBuffer(GL_ARRAY_BUFFER, VTO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(texCoords), texCoords, GL_STATIC_DRAW);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(2);

    // normal attribute
    glGenBuffers(1, &VNO);
    glBindBuffer(GL_ARRAY_BUFFER, VNO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexNormals), vertexNormals, GL_STATIC_DRAW);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(3);

    setupIMatrices();
}

void drawIMGUI() {
    // Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
    {
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        static float f = 0.0f;

        ImGui::Begin("Graphics For Games");  // Create a window and append into it.

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);


        static ImGuiInputTextFlags flags = ImGuiInputTextFlags_AllowTabInput;
        ImGui::InputTextMultiline("Vertex Shader", vtext, IM_ARRAYSIZE(vtext), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16), flags);
        ImGui::InputTextMultiline("Fragment Shader", ftext, IM_ARRAYSIZE(ftext), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16), flags);

        if (ImGui::Button("Use Shaders", ImVec2(100, 20)))
            ourShader.reload(vtext, ftext);



        ImGui::SameLine(115);

        if (ImGui::Button("Save Shaders", ImVec2(120, 20)))
        {
            saveShaders();
        }



        ImGui::SameLine(240);

        if (ImGui::Button("Swap Texture", ImVec2(100, 20)))
            nTexture ^= 1;


        ImGui::InputFloat4("one", myMatrix[0]);
        ImGui::InputFloat4("two", myMatrix[1]);
        ImGui::InputFloat4("three", myMatrix[2]);
        ImGui::InputFloat4("four", myMatrix[3]);

        ImGui::End();

        // IMGUI Rendering
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
}
int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Graphics4Games", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    Shader cubeShader("data/cube.vs", "data/cube.fs");

    strcpy_s(ftext, readFile("data/texture.fs").c_str());
    strcpy_s(vtext, readFile("data/texture.vs").c_str());

    readMatrix("data/myMatrix.txt");

    // build and compile our shader program
    // ------------------------------------
    ourShader.reload(vtext, ftext);

    myTexture();

    glGenVertexArrays(3, VAO);

    setupSkybox();
    setupCube();

    setupTextures();

    // render loop
    // -----------

    pMat = glm::perspective(1.0472f, ((float)SCR_WIDTH / (float)SCR_HEIGHT), .1f, 1000.0f);	//  1.0472 radians = 60 degrees
    vMat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f  , 0.0f, -4.0f));

    while (!glfwWindowShouldClose(window))
    {
        // input
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // glfw: poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwPollEvents();  

        glClear(GL_DEPTH_BUFFER_BIT);
        glClear(GL_COLOR_BUFFER_BIT);

        // draw/display the cube map "skybox"

        cubeShader.use();

        glUniformMatrix4fv(glGetUniformLocation(cubeShader.ID, "v_matrix"), 1, 
            GL_FALSE, glm::value_ptr(vMat));

        glUniformMatrix4fv(glGetUniformLocation(cubeShader.ID, "proj_matrix"), 1, 
            GL_FALSE, glm::value_ptr(pMat));

        glBindVertexArray(cubeVAO);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubeTexture);

        glEnable(GL_CULL_FACE);
        glFrontFace(GL_CCW);	// cube is CW, but we are viewing the inside
        glDisable(GL_DEPTH_TEST);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glEnable(GL_DEPTH_TEST);

        // these following lines are really where our geometry gets rendered!

        // first activate up the textures for and activate our "normal" shader

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture[nTexture]);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubeTexture);
        ourShader.use();
                
        // set the uniform for the current program (the way the "old" text and lecture do it)
        glUniformMatrix4fv(glGetUniformLocation(ourShader.ID, "pMat"), 1, GL_FALSE, glm::value_ptr(pMat));
        glUniformMatrix4fv(glGetUniformLocation(ourShader.ID, "vMat"), 1, GL_FALSE, glm::value_ptr(vMat));

        // create transformations
        glm::mat4 transform = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
        glm::mat4 transform2 = glm::mat4(
            myMatrix[0][0], myMatrix[1][0], myMatrix[2][0], myMatrix[3][0],
            myMatrix[0][1], myMatrix[1][1], myMatrix[2][1], myMatrix[3][1],
            myMatrix[0][2], myMatrix[1][2], myMatrix[2][2], myMatrix[3][2],
            myMatrix[0][3], myMatrix[1][3], myMatrix[2][3], myMatrix[3][3]
        );

        glEnable(GL_CULL_FACE);
        glFrontFace(GL_CW);
        glCullFace(GL_BACK);

        // display/draw the models in the scene

        for (int i = 0; i < vaoCount; i++) {
            transform = glm::translate(transform, glm::vec3(0.0f, -0.0f+(float)i*2.0f, 0.0f));
            transform = glm::rotate(transform, (float)glfwGetTime(), glm::vec3(0.5f, 1.0f, 0.5f));

            glProgramUniformMatrix4fv(ourShader.ID, glGetUniformLocation(ourShader.ID, "mMat"), 1, GL_FALSE, glm::value_ptr(transform2));
            glProgramUniformMatrix4fv(ourShader.ID, glGetUniformLocation(ourShader.ID, "mMat2"), 1, GL_FALSE, glm::value_ptr(transform));

            glBindVertexArray(VAO[i]);
            if (indexCount[i] > 0) {
                glDrawElementsInstanced(GL_TRIANGLES, indexCount[i], GL_UNSIGNED_INT, 0, 1);
                //glDrawArrays(GL_TRIANGLE_STRIP, 0, 12);
            }
            else
                glDrawArraysInstanced(GL_TRIANGLES, 0, 36, 1000);
        }

        drawIMGUI();
        glfwSwapBuffers(window);
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions
    glViewport(0, 0, width, height);
    pMat = glm::perspective(1.0472f, (float)width/(float)height, 0.1f, 1000.0f);	//  1.0472 radians = 60 degrees
}