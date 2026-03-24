#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include <cassert>
#include <cstring>
#include <fstream>
#define _USE_MATH_DEFINES
#include <cmath>
#include <climits>
#include <cfloat>
#include <algorithm>
#include <string>
#include <iostream>
#include <vector>
#include <memory>
#include <random>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Camera.h"
#include "GLSL.h"
#include "MatrixStack.h"
#include "Program.h"
#include "Shape.h"

using namespace std;

// --- Global State ---
GLFWwindow* window;
string RESOURCE_DIR = "./";
shared_ptr<Camera> camera;
shared_ptr<Program> progBP;
shared_ptr<Shape> bunnyMesh, teapotMesh, cubeMesh, sunMesh, frustumMesh;

// Task 6: Vertex Buffer Ground & Texture Globals
GLuint groundVAO;
GLuint groundVBO;
GLuint groundTextureID;

class GameObject {
public:
    shared_ptr<Shape> mesh;
    glm::vec3 pos;
    float rotY, scale;
    glm::vec3 color;
    GameObject(shared_ptr<Shape> m, glm::vec3 p, float r, float s, glm::vec3 c)
        : mesh(m), pos(p), rotY(r), scale(s), color(c) {
    }
};

class Light {
public:
    glm::vec3 position, color;
    Light(glm::vec3 p, glm::vec3 c) : position(p), color(c) {}
};

vector<shared_ptr<GameObject>> worldObjects;
shared_ptr<Light> sun;

bool keyW = false, keyA = false, keyS = false, keyD = false;
bool keyZ = false, keyT = false, keyShift = false;

// --- Task 6: Procedural Texture & Buffer Setup ---

void initGroundBuffer() {
    // Data layout: PosX, PosY, PosZ, NorX, NorY, NorZ, TexU, TexV
    // We set TexU/V to 20.0 to handle tiling automatically
    float groundData[] = {
        -1.0f, 0.0f, -1.0f,  0.0f, 1.0f, 0.0f,  0.0f,  0.0f,
        -1.0f, 0.0f,  1.0f,  0.0f, 1.0f, 0.0f,  0.0f,  20.0f,
         1.0f, 0.0f, -1.0f,  0.0f, 1.0f, 0.0f,  20.0f, 0.0f,
         1.0f, 0.0f,  1.0f,  0.0f, 1.0f, 0.0f,  20.0f, 20.0f,
    };

    glGenVertexArrays(1, &groundVAO);
    glBindVertexArray(groundVAO);

    glGenBuffers(1, &groundVBO);
    glBindBuffer(GL_ARRAY_BUFFER, groundVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(groundData), groundData, GL_STATIC_DRAW);

    // Attribute 0: Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    // Attribute 1: Normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    // Attribute 2: Texture Coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

    glBindVertexArray(0);
}

void initGroundTexture() {
    glGenTextures(1, &groundTextureID);
    glBindTexture(GL_TEXTURE_2D, groundTextureID);

    const int dim = 64;
    GLubyte texData[dim][dim][3];

    for (int i = 0; i < dim; i++) {
        for (int j = 0; j < dim; j++) {
            // Base green color
            int baseG = 120;
            // Add a random variation between -30 and +30
            int noise = (rand() % 60) - 30;

            texData[i][j][0] = 20 + (noise / 2);  // Slight red variation
            texData[i][j][1] = baseG + noise;     // Main green variation
            texData[i][j][2] = 20;                // Low blue
        }
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, dim, dim, 0, GL_RGB, GL_UNSIGNED_BYTE, texData);

    // Use GL_LINEAR for noise so the pixels blend smoothly
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

// --- Procedural Sphere Generator ---
void generateSphere(const string& filename, int rows, int cols) {
    ofstream out(filename);
    if (!out.is_open()) return;
    for (int r = 0; r <= rows; r++) {
        float phi = (float)M_PI * r / rows;
        for (int c = 0; c <= cols; c++) {
            float theta = 2.0f * (float)M_PI * c / cols;
            float x = sin(phi) * cos(theta);
            float y = cos(phi);
            float z = sin(phi) * sin(theta);
            out << "v " << x << " " << y << " " << z << endl;
            out << "vn " << x << " " << y << " " << z << endl;
        }
    }
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            int i0 = r * (cols + 1) + c + 1;
            int i1 = i0 + 1;
            int i2 = i0 + (cols + 1);
            int i3 = i2 + 1;
            out << "f " << i0 << "//" << i0 << " " << i2 << "//" << i2 << " " << i1 << "//" << i1 << endl;
            out << "f " << i1 << "//" << i1 << " " << i2 << "//" << i2 << " " << i3 << "//" << i3 << endl;
        }
    }
    out.close();
}

// --- Callbacks ---
static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    double xmouse, ymouse;
    glfwGetCursorPos(window, &xmouse, &ymouse);
    camera->mouseClicked((float)xmouse, (float)ymouse, (mods & GLFW_MOD_SHIFT), (mods & GLFW_MOD_CONTROL), (mods & GLFW_MOD_ALT));
}

static void cursor_position_callback(GLFWwindow* window, double xmouse, double ymouse) {
    camera->mouseMoved((float)xmouse, (float)ymouse);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) glfwSetWindowShouldClose(window, GL_TRUE);
    bool isPressed = (action != GLFW_RELEASE);
    if (key == GLFW_KEY_W) keyW = isPressed;
    if (key == GLFW_KEY_A) keyA = isPressed;
    if (key == GLFW_KEY_S) keyS = isPressed;
    if (key == GLFW_KEY_D) keyD = isPressed;
    if (key == GLFW_KEY_T && action == GLFW_PRESS) keyT = !keyT;
    if (key == GLFW_KEY_Z) {
        keyZ = isPressed;
        keyShift = (mods & GLFW_MOD_SHIFT) || (mods & GLFW_MOD_CAPS_LOCK);
    }
}

static void init() {
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);

    bunnyMesh = make_shared<Shape>();
    bunnyMesh->loadMesh(RESOURCE_DIR + "bunny.obj");
    bunnyMesh->init();

    teapotMesh = make_shared<Shape>();
    teapotMesh->loadMesh(RESOURCE_DIR + "teapot.obj");
    teapotMesh->init();

    cubeMesh = make_shared<Shape>();
    cubeMesh->loadMesh(RESOURCE_DIR + "cube.obj");
    cubeMesh->init();

    generateSphere(RESOURCE_DIR + "sphere_gen.obj", 24, 24);
    sunMesh = make_shared<Shape>();
    sunMesh->loadMesh(RESOURCE_DIR + "sphere_gen.obj");
    sunMesh->init();

    frustumMesh = make_shared<Shape>();
    frustumMesh->loadMesh(RESOURCE_DIR + "Frustum.obj");
    frustumMesh->init();

    // Task 6: Init custom ground plane and texture
    initGroundBuffer();
    initGroundTexture();

    progBP = make_shared<Program>();
    progBP->setShaderNames(RESOURCE_DIR + "bp_vert.glsl", RESOURCE_DIR + "bp_frag.glsl");
    progBP->init();
    progBP->addAttribute("aPos");
    progBP->addAttribute("aNor");
    progBP->addAttribute("aTex");
    progBP->addUniform("P");
    progBP->addUniform("MV");
    progBP->addUniform("MVit");
    progBP->addUniform("ka");
    progBP->addUniform("kd");
    progBP->addUniform("ks");
    progBP->addUniform("s");
    progBP->addUniform("lightPos");
    progBP->addUniform("lightCol");
    progBP->addUniform("texSampler");
    progBP->addUniform("useTexture");

    camera = make_shared<Camera>();
    sun = make_shared<Light>(glm::vec3(-30.0f, 40.0f, -30.0f), glm::vec3(1.0f, 1.0f, 1.0f));

    for (int i = 0; i < 210; i++) {
        float x = ((rand() % 1000) / 10.0f) - 50.0f;
        float z = ((rand() % 1000) / 10.0f) - 50.0f;
        auto mesh = (rand() % 2 == 0) ? bunnyMesh : teapotMesh;
        float s = 0.4f + (rand() % 100) / 150.0f;
        float r = (float)(rand() % 360);
        float y = -mesh->getMin().y * s;
        glm::vec3 color((rand() % 100) / 100.0f, (rand() % 100) / 100.0f, (rand() % 100) / 100.0f);
        worldObjects.push_back(make_shared<GameObject>(mesh, glm::vec3(x, y, z), r, s, color));
    }
}

void drawScene(shared_ptr<MatrixStack> P, shared_ptr<MatrixStack> MV, float t, bool drawSun) {
    glUniformMatrix4fv(progBP->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
    glm::vec3 lightPosEye = glm::vec3(MV->topMatrix() * glm::vec4(sun->position, 1.0f));
    glUniform3fv(progBP->getUniform("lightPos"), 1, glm::value_ptr(lightPosEye));
    glUniform3f(progBP->getUniform("lightCol"), 1.0f, 1.0f, 1.0f);

    if (drawSun) {
        MV->pushMatrix();
        MV->translate(sun->position);
        MV->scale(5.0f);
        glUniformMatrix4fv(progBP->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
        glUniformMatrix4fv(progBP->getUniform("MVit"), 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(MV->topMatrix()))));
        glUniform3f(progBP->getUniform("ka"), 1.0f, 1.0f, 0.0f);
        glUniform3f(progBP->getUniform("kd"), 0.0f, 0.0f, 0.0f);
        glUniform3f(progBP->getUniform("ks"), 0.0f, 0.0f, 0.0f);
        sunMesh->draw(progBP);
        MV->popMatrix();
    }

    // --- TASK 6: DRAW GROUND PLANE WITH TEXTURE ---
    MV->pushMatrix();
    MV->scale(glm::vec3(200.0f, 1.0f, 200.0f));
    glUniformMatrix4fv(progBP->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
    glUniformMatrix4fv(progBP->getUniform("MVit"), 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(MV->topMatrix()))));

    // Texture State
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, groundTextureID);
    glUniform1i(progBP->getUniform("texSampler"), 0);
    glUniform1i(progBP->getUniform("useTexture"), 1);

    glUniform3f(progBP->getUniform("ka"), 0.2f, 0.2f, 0.2f);
    glUniform3f(progBP->getUniform("kd"), 0.8f, 0.8f, 0.8f);
    glUniform3f(progBP->getUniform("ks"), 0.0f, 0.0f, 0.0f);
    glUniform1f(progBP->getUniform("s"), 1.0f);

    glBindVertexArray(groundVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);

    glUniform1i(progBP->getUniform("useTexture"), 0); // Turn off for objects
    MV->popMatrix();

    for (auto& obj : worldObjects) {
        MV->pushMatrix();
        float currentScale = obj->scale * (1.0f + 0.05f * (float)sin(t * 3.0f));
        float groundedY = -obj->mesh->getMin().y * currentScale;
        MV->translate(glm::vec3(obj->pos.x, groundedY, obj->pos.z));
        MV->rotate(glm::radians(obj->rotY), glm::vec3(0, 1, 0));
        MV->scale(currentScale);
        glUniformMatrix4fv(progBP->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
        glUniformMatrix4fv(progBP->getUniform("MVit"), 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(MV->topMatrix()))));
        glUniform3fv(progBP->getUniform("ka"), 1, glm::value_ptr(obj->color * 0.3f));
        glUniform3fv(progBP->getUniform("kd"), 1, glm::value_ptr(obj->color * 0.8f));
        glUniform3f(progBP->getUniform("ks"), 1.0f, 1.0f, 1.0f);
        glUniform1f(progBP->getUniform("s"), 100.0f);
        obj->mesh->draw(progBP);
        MV->popMatrix();
    }
}

static void render() {
    camera->updateWASD(keyW, keyA, keyS, keyD);
    camera->updateZoom(keyZ && !keyShift, keyZ && keyShift);

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    float aspect = (float)width / (float)height;
    camera->setAspect(aspect);

    auto P = make_shared<MatrixStack>();
    auto MV = make_shared<MatrixStack>();
    float t = (float)glfwGetTime();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    progBP->bind();

    // --- 1. MAIN VIEWPORT ---
    glViewport(0, 0, width, height);
    P->pushMatrix();
    camera->applyProjectionMatrix(P);
    MV->pushMatrix();
    camera->applyViewMatrix(MV);
    drawScene(P, MV, t, true);
    MV->popMatrix();
    P->popMatrix();

    // --- 2. TOP-DOWN MINIMAP (Task 5) ---
    if (keyT) {
        int vW = (int)(0.5 * width);
        int vH = (int)(0.5 * height);
        glViewport(0, 0, vW, vH);
        glClear(GL_DEPTH_BUFFER_BIT);
        P->pushMatrix();
        float range = 65.0f;
        float vAspect = (float)vW / (float)vH;
        if (vAspect >= 1.0f) P->multMatrix(glm::ortho(-range * vAspect, range * vAspect, -range, range, 0.1f, 500.0f));
        else P->multMatrix(glm::ortho(-range, range, -range / vAspect, range / vAspect, 0.1f, 500.0f));

        MV->pushMatrix();
        MV->translate(glm::vec3(0, 0, -100.0f));
        MV->rotate(glm::radians(90.0f), glm::vec3(1, 0, 0));
        drawScene(P, MV, t, false);

        // --- Frustum Visualization ---
        MV->pushMatrix();
        auto mainViewStack = make_shared<MatrixStack>();
        camera->applyViewMatrix(mainViewStack);
        glm::mat4 camMatrix = glm::inverse(mainViewStack->topMatrix());
        camMatrix[3][1] = 2.0f; // Keep it slightly above ground
        MV->multMatrix(camMatrix);

        float sY = tan(camera->getFOV() / 2.0f);
        float sX = aspect * sY;
        MV->scale(glm::vec3(sX, sY, 1.0f) * 8.0f);

        // --- BOLD BLACK SETTINGS ---
        glLineWidth(3.0f); // Set thickness (1.0 is default, 3.0 is bold)
        glUniformMatrix4fv(progBP->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));

        // Set all components to 0 for pure black
        glUniform3f(progBP->getUniform("ka"), 0.0f, 0.0f, 0.0f);
        glUniform3f(progBP->getUniform("kd"), 0.0f, 0.0f, 0.0f);
        glUniform3f(progBP->getUniform("ks"), 0.0f, 0.0f, 0.0f);

        frustumMesh->draw(progBP);

        glLineWidth(1.0f); // Reset to default so other wireframes aren't thick
        MV->popMatrix();
        P->popMatrix();
    }

    // --- 3. HUD PASS (Restored Bunny & Teapot) ---
    glViewport(0, 0, width, height);
    glClear(GL_DEPTH_BUFFER_BIT); // Clear depth so HUD is on top

    auto P_HUD = make_shared<MatrixStack>();
    P_HUD->pushMatrix();
    P_HUD->multMatrix(glm::ortho(-aspect, aspect, -1.0f, 1.0f, -10.0f, 10.0f));
    glUniformMatrix4fv(progBP->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P_HUD->topMatrix()));

    // Light for HUD (Static light so they look consistent)
    glUniform3f(progBP->getUniform("lightPos"), 1.0f, 1.0f, 5.0f);
    glUniform1i(progBP->getUniform("useTexture"), 0); // No texture on HUD items

    float hudX = aspect * 0.85f;
    float hudY = 0.70f;

    // --- HUD TEAPOT (Left Side) ---
    MV->pushMatrix();
    MV->loadIdentity();
    MV->translate(glm::vec3(-hudX, hudY, 0.0f));
    MV->rotate(t, glm::vec3(0, 1, 0)); // Spin over time
    MV->scale(0.15f);
    glUniformMatrix4fv(progBP->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
    glUniformMatrix4fv(progBP->getUniform("MVit"), 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(MV->topMatrix()))));
    glUniform3f(progBP->getUniform("ka"), 0.15f, 0.15f, 0.15f);
    glUniform3f(progBP->getUniform("kd"), 0.35f, 0.35f, 0.35f);
    glUniform3f(progBP->getUniform("ks"), 1.0f, 1.0f, 1.0f);
    glUniform1f(progBP->getUniform("s"), 15.0f);
    teapotMesh->draw(progBP);
    MV->popMatrix();

    // --- HUD BUNNY (Right Side) ---
    MV->pushMatrix();
    MV->loadIdentity();
    MV->translate(glm::vec3(hudX, hudY, 0.0f));
    MV->rotate(t, glm::vec3(0, 1, 0)); // Spin over time
    MV->scale(0.15f);
    glUniformMatrix4fv(progBP->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
    glUniformMatrix4fv(progBP->getUniform("MVit"), 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(MV->topMatrix()))));
    glUniform3f(progBP->getUniform("ka"), 0.15f, 0.15f, 0.15f);
    glUniform3f(progBP->getUniform("kd"), 0.35f, 0.35f, 0.35f);
    glUniform3f(progBP->getUniform("ks"), 1.0f, 1.0f, 1.0f);
    glUniform1f(progBP->getUniform("s"), 15.0f);
    bunnyMesh->draw(progBP);
    MV->popMatrix();

    P_HUD->popMatrix();
    progBP->unbind();
}

int main(int argc, char** argv) {
    if (argc >= 2) RESOURCE_DIR = argv[1] + string("/");
    if (!glfwInit()) return -1;
    window = glfwCreateWindow(1024, 768, "Blinn-Phong World", NULL, NULL);
    glfwMakeContextCurrent(window);
    glewExperimental = true;
    if (glewInit() != GLEW_OK) return -1;
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    init();
    while (!glfwWindowShouldClose(window)) {
        render();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}