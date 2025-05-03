#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <stb_image.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <iostream>
#include <filesystem>
#include <memory>
#include "render/Model.h"
#include "render/Shader.h"
#include "core/SceneNode.h"
#include "core/CameraNode.h"
#include <vector>

// Struct für Raytracing Kugeln
struct Sphere {
    glm::vec3 center;
    float radius;
};

std::shared_ptr<SceneNode> sunNode;
std::shared_ptr<SceneNode> planet1;
std::shared_ptr<SceneNode> planet2;

// Globals
bool isWireframe = false; 
bool showSkybox = true; 
bool enableDirectionalLight = true;
glm::vec3 lightDirection(0.0f, -1.0f, -1.0f);
float cameraSpeed = 100.0f;

bool enableRaytrace = false; //  ImGui Toggle
GLuint quadVAO = 0, quadVBO;
std::vector<Sphere> tracedSpheres; // Kugelliste Raytracing

void framebuffer_size_callback(GLFWwindow* w, int width, int height) {
    glViewport(0, 0, width, height);
}



void processInput(GLFWwindow* window, Camera& camera, float deltaTime) {
    glm::vec3 forward = glm::normalize(camera.getTarget() - camera.getPosition());
    glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 movement(0.0f);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        movement += forward;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        movement -= forward;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        movement -= right;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        movement += right;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        movement += up;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        movement -= up;

    if (glm::length(movement) > 0.0f) {
        movement = glm::normalize(movement) * deltaTime * cameraSpeed;
        camera.moveTarget(movement);
    }
}

void setupImGui(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");
    
}

void renderImGui(Camera& camera) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    if (ImGui::Begin("Settings")) {

        // Kamera-Parameter zwischenspeichern
        static float fov = camera.getFOV();
        static float nearPlane = camera.getNearPlane();
        static float farPlane = camera.getFarPlane();

        // Kamera-Slider
        ImGui::SliderFloat("Field of View", &fov, 10.0f, 120.0f);
        ImGui::SliderFloat("Near Plane", &nearPlane, 0.01f, 10.0f);
        ImGui::SliderFloat("Far Plane", &farPlane, 100.0f, 10000.0f);

        // Kamera-Werte anwenden
        camera.setFOV(fov);
        camera.setNearPlane(nearPlane);
        camera.setFarPlane(farPlane);

        // Licht-Richtung (deine ursprüngliche Zeile)
        ImGui::SliderFloat3("Light Direction", glm::value_ptr(lightDirection), -1.0f, 1.0f);

        // Weitere Optionen
        ImGui::Checkbox("Wireframe Mode", &isWireframe);
        ImGui::Checkbox("Show Skybox", &showSkybox);
        ImGui::Checkbox("Directional Light", &enableDirectionalLight);

        // Raytracing
        ImGui::Checkbox("Raytracing Spheres", &enableRaytrace);

        // Kamera Reset
        if (ImGui::Button("Reset Camera")) {
            camera.reset();
            fov = camera.getFOV();
            nearPlane = camera.getNearPlane();
            farPlane = camera.getFarPlane();
        }

    }
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}


// Load Texture für skybox galaxy sphere
unsigned int loadTexture(const char* path) {
    unsigned int id;
    glGenTextures(1, &id);
    int w, h, c;
    unsigned char* data = stbi_load(path, &w, &h, &c, 0);
    GLenum format = (c == 4 ? GL_RGBA : GL_RGB);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(data);
    return id;
}

int main() {
    // Debugging start
    // 1) Ausgabe des aktuellen Arbeitsverzeichnisses
    std::cout << "[DEBUG] CWD: " << std::filesystem::current_path() << std::endl;

    // 2) GLFW initialisieren
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Solar System", nullptr, nullptr);
    if (!window) {
        std::cerr << "Window creation failed\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // 3) GLAD laden (Funktionspointer für OpenGL)
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "GLAD init failed\n";
        return -1;
    }

    // Änderung: Initialen Viewport setzen (erster Aufruf)
    int scrW, scrH;
    glfwGetFramebufferSize(window, &scrW, &scrH);
    glViewport(0, 0, scrW, scrH);

    // 4) **Depth-Test** aktivieren
    glEnable(GL_DEPTH_TEST);
    // Änderung: Back-Face Culling aktivieren
    glEnable(GL_CULL_FACE);               // Änderung: Back-Faces werden ausgefiltert
    glCullFace(GL_BACK);                  // Änderung: Welche Seite (Back)
    glFrontFace(GL_CCW);                  // Änderung: Vertex-Winding (Counter-Clockwise)

    setupImGui(window);

    // 5) Kamera und Shader anlegen
    auto rootNode = std::make_shared<SceneNode>();  
    auto cameraNode = std::make_shared<CameraNode>(window); 
    cameraNode->transform =
        glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 100.0f, 300.0f));
    rootNode->addChild(cameraNode);                    


    Shader modelShader(
        "../../../../project/shaders/model.vert",
        "../../../../project/shaders/model.frag"
    );
    Shader sunShader(
        "../../../../project/shaders/sun.vert",
        "../../../../project/shaders/sun.frag"
    );
    Shader rayShader(
        "../../../../project/shaders/raytrace_spheres.vert",
        "../../../../project/shaders/raytrace_spheres.frag"
    );

    // === Fullscreen Quad vorbereiten ===
    float quadVerts[] = {
        -1.0f, -1.0f,
         1.0f, -1.0f,
        -1.0f,  1.0f,
         1.0f,  1.0f
    };
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerts), quadVerts, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    // === Raytracing Kugeln definieren (Initiale Testwerte) ===
    tracedSpheres = {
        { glm::vec3(0, 0, 0), 5.0f },
        { glm::vec3(80, 0, 0), 6.0f },
        { glm::vec3(-130, 0, 50), 7.0f }
    };



    // 6) Modelle laden und SceneGraph aufbauen

    auto alienPlanet = std::make_shared<Model>("../../../../project/models/planets/alien_planet.glb");
    auto sunPlanet = std::make_shared<Model>("../../../../project/models/planets/sun.glb");
    auto shinyPlanet = std::make_shared<Model>("../../../../project/models/planets/shiny_planet.glb");

    auto pinkSpaceShip = std::make_shared<Model>("../../../../project/models/spaceships/spaceship_pink.glb");
    auto greySpaceShip = std::make_shared<Model>("../../../../project/models/spaceships/spaceship_gray.glb");
    auto ufo = std::make_shared<Model>("../../../../project/models/spaceships/ufo.glb");

    sunNode = std::make_shared<SceneNode>();
    sunNode->setModel(sunPlanet);
    sunNode->transform = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.05f));

    sunNode->setRotationSpeed(10.0f);
    rootNode->addChild(sunNode);

    // Alien Planet Node
    auto orbit1 = std::make_shared<SceneNode>();
    orbit1->setRotationSpeed(5.0f);
    planet1 = std::make_shared<SceneNode>();
    planet1->setModel(alienPlanet);
	planet1->setRotationSpeed(10.0f);
    planet1->transform = glm::translate(glm::mat4(1.0f), glm::vec3(80, 0, 0)) * glm::scale(glm::mat4(1.0f), glm::vec3(6));
    orbit1->addChild(planet1);
    rootNode->addChild(orbit1);

    // Purple Planet Node
    auto orbit2 = std::make_shared<SceneNode>();
    orbit2->setRotationSpeed(2.5f);
    planet2 = std::make_shared<SceneNode>();
    planet2->setModel(shinyPlanet);
	planet2->setRotationSpeed(10.0f);
    planet2->transform = glm::translate(glm::mat4(1.0f), glm::vec3(-130, 0, 50)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.1));
    orbit2->addChild(planet2);
    rootNode->addChild(orbit2);



    auto pinkShipNode = std::make_shared<OscillatingRotationNode>(
        glm::vec3(150, 40, 0), glm::vec3(0, 1, 0), 2.0f, 45.0f
    );
    pinkShipNode->setModel(pinkSpaceShip);
    rootNode->addChild(pinkShipNode);

    auto greyShipNode = std::make_shared<OscillatingTranslationNode>(
        glm::vec3(-180, 20, 60), glm::vec3(0, 1, 0), 1.5f, 25.0f
    );
    greyShipNode->setModel(greySpaceShip);
    rootNode->addChild(greyShipNode);



    float lastFrame = static_cast<float>(glfwGetTime());

    // Galaxy Skybox Setup
    Shader skyShader(
        "../../../../project/shaders/sky.vert",
        "../../../../project/shaders/sky.frag");

    Model skySphere("../../../../project/models/galaxy_skybox/inside_galaxy.glb");
    std::cout << "[DEBUG] SkySphere MeshCount: " << skySphere.getMeshCount() << std::endl; // Model laden Check
    unsigned int skyTex = loadTexture(
        "../../../../project/models/galaxy_skybox/inside_galaxy.png"
    );

    while (!glfwWindowShouldClose(window)) {
        // 1) Delta-Time berechnen
        float current = static_cast<float>(glfwGetTime());
        float delta = current - lastFrame;
        lastFrame = current;

        // 2) Events abfragen
        glfwPollEvents();

        // 3) Scene Graph updaten (inkl. CameraNode)
        rootNode->update(delta);

        // 4) Buffers löschen
        glPolygonMode(GL_FRONT_AND_BACK, isWireframe ? GL_LINE : GL_FILL);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 5) View & Projection aus CameraNode holen
        glm::mat4 view = cameraNode->getCamera().getViewMatrix();
        glm::mat4 proj = cameraNode->getCamera().getProjectionMatrix(1280.0f / 720.0f);

        if (enableRaytrace) {
            // === Dynamisch transformierte Sphären holen ===
            auto extractSphere = [](const SceneNode& node, float scale = 1.0f) -> Sphere {
                glm::mat4 global = node.getGlobalTransform();
                glm::vec3 center = glm::vec3(global[3]);
                float radius = glm::length(glm::vec3(global[0])) * scale;
                return { center, radius };
                };

            tracedSpheres.clear();
            tracedSpheres.push_back(extractSphere(*sunNode, 0.5f));
            tracedSpheres.push_back(extractSphere(*planet1, 1.0f));
            tracedSpheres.push_back(extractSphere(*planet2, 1.0f));

            rayShader.use();
            rayShader.setVec3("camPos", cameraNode->getCamera().getPosition());
            rayShader.setVec3("lightDir", lightDirection);
            glm::mat4 invVP = glm::inverse(proj * view);
            rayShader.setMat4("invViewProj", invVP);

            rayShader.setInt("sphereCount", static_cast<int>(tracedSpheres.size()));
            for (int i = 0; i < tracedSpheres.size(); ++i) {
                rayShader.setVec3("spheres[" + std::to_string(i) + "].center", tracedSpheres[i].center);
                rayShader.setFloat("spheres[" + std::to_string(i) + "].radius", tracedSpheres[i].radius);
            }

            glBindVertexArray(quadVAO);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        }
        else {
            // === Originales Rendering (Skybox, Modelle etc.) ===
            if (showSkybox) {
                GLboolean wasCull = glIsEnabled(GL_CULL_FACE);
                glDisable(GL_CULL_FACE);
                glDepthMask(GL_FALSE);
                skyShader.use();
                skyShader.setMat4("view", glm::mat4(glm::mat3(view)));
                skyShader.setMat4("projection", proj);
                glm::mat4 skyModel = glm::scale(glm::mat4(1.0f), glm::vec3(2000.0f));
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, skyTex);
                skyShader.setInt("equirectangularMap", 0);
                skySphere.draw(skyShader.ID, skyModel);
                glDepthMask(GL_TRUE);
                if (wasCull) glEnable(GL_CULL_FACE);
            }

            modelShader.use();
            modelShader.setInt("texture_diffuse", 0);
            modelShader.setVec3("lightDir", enableDirectionalLight ? lightDirection : glm::vec3(0.0f));
            modelShader.setVec3("lightColor", glm::vec3(1.0f));
            modelShader.setVec3("viewPos", cameraNode->getCamera().getPosition());
            modelShader.setMat4("view", view);
            modelShader.setMat4("projection", proj);
            rootNode->draw(glm::mat4(1.0f), modelShader.ID);
        }

        renderImGui(cameraNode->getCamera());
        glfwSwapBuffers(window);
        processInput(window, cameraNode->getCamera(), delta);
    }


    // 10) Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
