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
#include <vector>
#include "render/Model.h"
#include "render/Shader.h"
#include "core/SceneNode.h"
#include "core/CameraNode.h"
#include <SplinePath.h>
#include <SplineRenderer.h>

// Globals
bool isWireframe = false;
bool showSkybox = true;
bool enableDirectionalLight = true;
glm::vec3 lightDirection(0.0f, -1.0f, -1.0f);
float cameraSpeed = 100.0f;

struct PointLight {
    glm::vec3 position;

    float constant;
    float linear;
    float quadratic;

    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};

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
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        movement += up;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
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

void renderImGui(Camera& camera, std::vector<PointLight>& pointLights) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Settings");

    // Kamera-Einstellungen
    if (ImGui::CollapsingHeader("Kamera", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::SliderFloat("Kamera-Geschwindigkeit", &cameraSpeed, 10.0f, 500.0f);
        if (ImGui::Button("Kamera zurücksetzen")) camera.reset();
    }

    // Allgemeine Beleuchtungseinstellungen
    if (ImGui::CollapsingHeader("Allgemeine Beleuchtung", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Checkbox("Wireframe-Modus", &isWireframe);
        ImGui::Checkbox("Skybox anzeigen", &showSkybox);
        ImGui::Checkbox("Direktionales Licht", &enableDirectionalLight);

        if (enableDirectionalLight) {
            ImGui::SliderFloat3("Lichtrichtung", glm::value_ptr(lightDirection), -1.0f, 1.0f);
        }
    }

    // Point-Light-Einstellungen
    if (ImGui::CollapsingHeader("Punktlichter", ImGuiTreeNodeFlags_DefaultOpen)) {
        // Steuerelemente für die Anzahl aktiver Punktlichter
        int numLights = pointLights.size();
        static int activePointLights = std::min(numLights, 4); // MAX_POINT_LIGHTS ist 4
        ImGui::SliderInt("Aktive Punktlichter", &activePointLights, 0, std::min(numLights, 4));

        // Steuerelemente für jedes aktive Punktlicht anzeigen
        for (int i = 0; i < activePointLights && i < pointLights.size(); i++) {
            if (ImGui::TreeNode(("Punktlicht " + std::to_string(i + 1)).c_str())) {
                ImGui::SliderFloat3(("Position##" + std::to_string(i)).c_str(),
                    glm::value_ptr(pointLights[i].position), -200.0f, 200.0f);

                // Lichtfarben-Steuerelemente
                float ambient[3] = { pointLights[i].ambient.r, pointLights[i].ambient.g, pointLights[i].ambient.b };
                float diffuse[3] = { pointLights[i].diffuse.r, pointLights[i].diffuse.g, pointLights[i].diffuse.b };
                float specular[3] = { pointLights[i].specular.r, pointLights[i].specular.g, pointLights[i].specular.b };

                if (ImGui::ColorEdit3(("Ambient##" + std::to_string(i)).c_str(), ambient)) {
                    pointLights[i].ambient = glm::vec3(ambient[0], ambient[1], ambient[2]);
                }

                if (ImGui::ColorEdit3(("Diffuse##" + std::to_string(i)).c_str(), diffuse)) {
                    pointLights[i].diffuse = glm::vec3(diffuse[0], diffuse[1], diffuse[2]);
                }

                if (ImGui::ColorEdit3(("Specular##" + std::to_string(i)).c_str(), specular)) {
                    pointLights[i].specular = glm::vec3(specular[0], specular[1], specular[2]);
                }

                // Dämpfungssteuerelemente
                ImGui::SliderFloat(("Konstant##" + std::to_string(i)).c_str(), &pointLights[i].constant, 0.1f, 2.0f);
                ImGui::SliderFloat(("Linear##" + std::to_string(i)).c_str(), &pointLights[i].linear, 0.0001f, 0.1f, "%.4f");
                ImGui::SliderFloat(("Quadratisch##" + std::to_string(i)).c_str(), &pointLights[i].quadratic, 0.000001f, 0.01f, "%.6f");

                ImGui::TreePop();
            }
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
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

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

    // 6) Modelle laden und SceneGraph aufbauen
    auto alienPlanet = std::make_shared<Model>("../../../../project/models/planets/alien_planet.glb");
    auto sunPlanet = std::make_shared<Model>("../../../../project/models/planets/sun.glb");
    auto shinyPlanet = std::make_shared<Model>("../../../../project/models/planets/shiny_planet.glb");

    auto pinkSpaceShip = std::make_shared<Model>("../../../../project/models/spaceships/spaceship_pink.glb");
    auto greySpaceShip = std::make_shared<Model>("../../../../project/models/spaceships/spaceship_gray.glb");
    auto ufo = std::make_shared<Model>("../../../../project/models/spaceships/ufo.glb");

    auto sunNode = std::make_shared<SceneNode>();
    sunNode->setModel(sunPlanet);
    sunNode->transform = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0)) *
        glm::scale(glm::mat4(1.0f), glm::vec3(0.3f));
    sunNode->setRotationSpeed(0.05f);
    // WICHTIG: Fügen Sie die Sonne nicht zum rootNode hinzu, um sie separat zu rendern

    // Erstelle die Point Lights
    std::vector<PointLight> pointLights;

    // Sonnenlicht (zentrales, helles Licht)
    PointLight sunLight;
    sunLight.position = glm::vec3(0.0f, 0.0f, 0.0f); // Sonnenzentrum
    sunLight.constant = 1.0f;
    sunLight.linear = 0.0014f;
    sunLight.quadratic = 0.000007f;
    sunLight.ambient = glm::vec3(0.1f, 0.1f, 0.05f);
    sunLight.diffuse = glm::vec3(1.0f, 0.9f, 0.7f);
    sunLight.specular = glm::vec3(1.0f, 1.0f, 0.8f);
    pointLights.push_back(sunLight);


    // Alien Planet Node
    auto orbit1 = std::make_shared<SceneNode>();
    orbit1->setRotationSpeed(5.0f);
    auto planet1 = std::make_shared<SceneNode>();
    planet1->setModel(alienPlanet);
    planet1->setRotationSpeed(10.0f);
    planet1->transform = glm::translate(glm::mat4(1.0f), glm::vec3(80, 0, 0)) * glm::scale(glm::mat4(1.0f), glm::vec3(6));
    orbit1->addChild(planet1);
    rootNode->addChild(orbit1);

    // Purple Planet Node
    auto orbit2 = std::make_shared<SceneNode>();
    orbit2->setRotationSpeed(2.5f);
    auto planet2 = std::make_shared<SceneNode>();
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
    std::cout << "[DEBUG] SkySphere MeshCount: " << skySphere.getMeshCount() << std::endl;
    unsigned int skyTex = loadTexture(
        "../../../../project/models/galaxy_skybox/inside_galaxy.png"
    );

    // Sonnen-Textur separat laden für direkten Zugriff
    unsigned int sunTexture = loadTexture(
        "../../../../project/models/planets/sun_diffuse.png"  // Anpassen an den tatsächlichen Pfad
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

        // 6) Skybox rendern
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

        // 7) Modelle rendern
        // Planeten mit modelShader rendern (Sonne ist nie im Szenengraph, also müssen wir sie nicht entfernen)
        modelShader.use();
        modelShader.setInt("texture_diffuse", 0);
        modelShader.setVec3("lightDir", lightDirection);
        modelShader.setVec3("lightColor", glm::vec3(1.0f, 0.9f, 0.7f));
        modelShader.setVec3("viewPos", cameraNode->getCamera().getPosition());
        modelShader.setMat4("view", view);
        modelShader.setMat4("projection", proj);
        modelShader.setBool("enableDirectionalLight", enableDirectionalLight);

        // Punktlichter an den Shader übergeben
        modelShader.setInt("numPointLights", pointLights.size());
        for (unsigned int i = 0; i < pointLights.size() && i < 4; i++) {
            std::string number = std::to_string(i);
            modelShader.setVec3("pointLights[" + number + "].position", pointLights[i].position);
            modelShader.setVec3("pointLights[" + number + "].ambient", pointLights[i].ambient);
            modelShader.setVec3("pointLights[" + number + "].diffuse", pointLights[i].diffuse);
            modelShader.setVec3("pointLights[" + number + "].specular", pointLights[i].specular);
            modelShader.setFloat("pointLights[" + number + "].constant", pointLights[i].constant);
            modelShader.setFloat("pointLights[" + number + "].linear", pointLights[i].linear);
            modelShader.setFloat("pointLights[" + number + "].quadratic", pointLights[i].quadratic);
        }

        // Planeten zeichnen (ohne Sonne)
        rootNode->draw(glm::mat4(1.0f), modelShader.ID);

        // KORRIGIERT: Sonne wieder zum SceneGraph hinzufügen
        rootNode->addChild(sunNode);

        // DEBUG-TEST: Verwenden Sie einen festen Farbwert für die Sonne, um zu testen
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0); // Unbind alle Texturen
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, 0); // Unbind alle Texturen

        // Verwenden Sie den modelShader für die Sonne als Test
        modelShader.use();
        modelShader.setMat4("view", view);
        modelShader.setMat4("projection", proj);
        modelShader.setVec3("viewPos", cameraNode->getCamera().getPosition());
        // Hier eine auffällige Farbe verwenden, damit Sie sehen können, ob es die Sonne ist
        modelShader.setVec3("lightColor", glm::vec3(1.0f, 0.0f, 0.0f)); // Reines Rot

        sunNode->draw(glm::mat4(1.0f), modelShader.ID);

        // Füge die Sonne wieder zum Scene Graph hinzu für Update-Operationen im nächsten Frame
        rootNode->addChild(sunNode);




        SplinePath orbitPath;
        for (int i = 0; i < 9; ++i) {
            float angle = glm::radians(i * 40.0f); // 0° – 320°
            float radius = 80.0f;
            orbitPath.addPoint(glm::vec3(cos(angle) * radius, 0.0f, sin(angle) * radius));
        }

        // ❗ Wiederhole Punkte für geschlossene Kurve
        orbitPath.addPoint(orbitPath.getControlPoints()[1]);
        orbitPath.addPoint(orbitPath.getControlPoints()[2]);

        SplineRenderer orbitRenderer;
        orbitRenderer.setSpline(orbitPath);
        orbitRenderer.upload();


        SplinePath orbitPath2;
        float radius2 = 130.0f;
        for (int i = 0; i < 9; ++i) {
            float angle = glm::radians(i * 40.0f);
            orbitPath2.addPoint(glm::vec3(cos(angle) * radius2, 0.0f, sin(angle) * radius2));
        }
        // Spline schließen
        orbitPath2.addPoint(orbitPath2.getControlPoints()[1]);
        orbitPath2.addPoint(orbitPath2.getControlPoints()[2]);

        SplineRenderer renderer2;
        renderer2.setSpline(orbitPath2);
        renderer2.upload();



        // Add the following line to define and initialize the splineShader object  
        Shader splineShader(  
           "../../../../project/shaders/spline.vert",  
           "../../../../project/shaders/spline.frag"  
        );

        splineShader.use();
        splineShader.setVec3("color", glm::vec3(1.0, 0.5, 0.2));  // für Orbit 1
        splineShader.setVec3("color", glm::vec3(0.2, 0.8, 1.0));  // für Orbit 2       


        // Orbit 1
        orbitRenderer.draw(splineShader, view, proj);

        // Orbit 2
        renderer2.draw(splineShader, view, proj);


        // 8) ImGui zeichnen (mit Camera aus dem Graph)
        renderImGui(cameraNode->getCamera(), pointLights);

        // 9) Buffer swap
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