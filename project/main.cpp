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
#include <fstream>

// Globals
bool isWireframe = false;
bool showSkybox = true;
bool enableDirectionalLight = true;
glm::vec3 lightDirection(0.0f, -1.0f, -1.0f);
float cameraSpeed = 100.0f;
float sunRotationAngle = 90.0f;

bool animateCamera = false;
float cameraSplineTime = 0.0f;
float cameraSpeedFactor = 0.05f;  // Geschwindigkeit der Fahrt
SplinePath cameraPath;

// Spline UI
SplinePath userSplinePath;
std::unique_ptr<SplineRenderer> userSplineRenderer;
bool showUserSpline = true;

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

        if (ImGui::Button("Starte Kamerafahrt")) {
            animateCamera = true;
            cameraSplineTime = 0.0f; // Zurücksetzen
        }
        if (ImGui::Button("Stoppe Kamerafahrt")) {
            animateCamera = false;
        }

        ImGui::SliderFloat("Kamera-Geschwindigkeit", &cameraSpeed, 10.0f, 500.0f);
        if (ImGui::Button("Kamera zuruecksetzen")) camera.reset();

        float fov = camera.getFOV();
        if (ImGui::SliderFloat("Field of View", &fov, 10.0f, 120.0f))
            camera.setFOV(fov);  // 🆕 Änderung

        float nearClip = camera.getNearPlane();
        if (ImGui::SliderFloat("Near Plane", &nearClip, 0.01f, 10.0f))
            camera.setNearPlane(nearClip);  // 🆕 Änderung

        float farClip = camera.getFarPlane();
        if (ImGui::SliderFloat("Far Plane", &farClip, 100.0f, 10000.0f))
            camera.setFarPlane(farClip);  // 🆕 Änderung
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

   

    // Benutzerdefinierte Spline UI
    if (ImGui::CollapsingHeader("Benutzerdefinierter Spline", ImGuiTreeNodeFlags_DefaultOpen)) {
        static glm::vec3 newPoint = glm::vec3(0.0f);

        if (ImGui::Button("Punkt hinzufuegen")) {
            userSplinePath.addPoint(newPoint);
            userSplineRenderer->setSpline(userSplinePath);
            userSplineRenderer->upload();
        }
        ImGui::InputFloat3("Neuer Punkt", glm::value_ptr(newPoint));

        if (ImGui::Button("Spline loeschen")) {
            userSplinePath.clearPoints();
            userSplineRenderer->setSpline(userSplinePath);
            userSplineRenderer->upload();
        }

        auto& cps = const_cast<std::vector<glm::vec3>&>(userSplinePath.getControlPoints());
        for (size_t i = 0; i < cps.size(); ++i) {
            ImGui::PushID(static_cast<int>(i));
            ImGui::InputFloat3("Punkt", glm::value_ptr(cps[i]));
            if (ImGui::Button("Entfernen")) {
                cps.erase(cps.begin() + i);
                userSplineRenderer->setSpline(userSplinePath);
                userSplineRenderer->upload();
                ImGui::PopID();
                break;
            }
            ImGui::PopID();
        }

        if (ImGui::Button("Spline speichern")) {
            std::ofstream out("userspline.txt");
            for (const auto& p : cps) {
                out << p.x << " " << p.y << " " << p.z << "\n";
            }
        }

        if (ImGui::Button("Spline laden")) {
            std::ifstream in("userspline.txt");
            glm::vec3 p;
            cps.clear();
            while (in >> p.x >> p.y >> p.z) {
                cps.push_back(p);
            }
            userSplineRenderer->setSpline(userSplinePath);
            userSplineRenderer->upload();
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

    // Debug-Ausgabe vor dem Laden
    std::cout << "Loading texture from: " << path << std::endl;

    unsigned char* data = stbi_load(path, &w, &h, &c, 0);
    if (!data) {
        std::cerr << "Failed to load texture: " << path << std::endl;
        std::cerr << "stbi error: " << stbi_failure_reason() << std::endl;
        return 0;
    }

    // Debug-Ausgabe nach erfolgreichem Laden
    std::cout << "Texture loaded: " << w << "x" << h << " channels: " << c << std::endl;

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

    userSplineRenderer = std::make_unique<SplineRenderer>(); // Initialisierung Spline UI

    // 5) Kamera und Shader anlegen
    auto rootNode = std::make_shared<SceneNode>();
    auto cameraNode = std::make_shared<CameraNode>(window);
    cameraNode->transform =
        glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 100.0f, 300.0f));
    rootNode->addChild(cameraNode);



    float outerRadius = 400.0f;
    float height = 200.0f;
    int camCount = 12;
    for (int i = 0; i < camCount; ++i) {
        float angle = glm::radians(i * 360.0f / camCount);
        float x = cos(angle) * outerRadius;
        float z = sin(angle) * outerRadius;
        cameraPath.addPoint(glm::vec3(x, height, z));
    }
    cameraPath.addPoint(cameraPath.getControlPoints()[0]);
    cameraPath.addPoint(cameraPath.getControlPoints()[1]);



    Shader modelShader(
        "../../../../project/shaders/model.vert",
        "../../../../project/shaders/model.frag"
    );

        // Verbesserte Sonnen-Shader mit besseren Dateipfaden
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

    auto sunNode = std::make_shared<SelfRotatingNode>();
    sunNode->setModel(sunPlanet);
    sunNode->transform = glm::scale(glm::mat4(1.0f), glm::vec3(0.3f));
    sunNode->setRotationSpeed(10.0f);


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

    // Sonnen-Texturen laden
    unsigned int sunDiffuse = loadTexture("../../../../project/models/sun/textures/SunDiffuse.png");


    // Alternative Fallback wenn keine Textur gefunden wurde
    if (sunDiffuse == 0) {
        std::cerr << "[ERROR] Could not load sun texture, using default texture!" << std::endl;
        // Einen 1x1 weißen Pixel erstellen
        unsigned char white[] = { 255, 255, 255, 255 };

        glGenTextures(1, &sunDiffuse);
        glBindTexture(GL_TEXTURE_2D, sunDiffuse);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }


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
        // Planeten mit modelShader rendern
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

        // Nun die Sonne mit dem speziellen Sonnen-Shader zeichnen
       // Rotation aktualisieren
        sunRotationAngle += delta * 10.0f;
        if (sunRotationAngle > 360.0f)
            sunRotationAngle -= 360.0f;

        glm::mat4 sunModelMatrix = glm::mat4(1.0f);

        // Modell verschieben (Translation *vor* Rotation)
        sunModelMatrix = glm::translate(sunModelMatrix, glm::vec3(0.0f, 0.0f, 0.0f));

        // Dann Rotation (um die eigene Achse)
        sunModelMatrix = glm::rotate(sunModelMatrix,
            glm::radians(sunRotationAngle),
            glm::vec3(0.0f, 1.0f, 0.0f));

        // Dann Skalierung
        sunModelMatrix = glm::scale(sunModelMatrix, glm::vec3(0.9f));

        // Sonnen-Shader setzen
        sunShader.use();
        sunShader.setMat4("view", view);
        sunShader.setMat4("projection", proj);
        sunShader.setVec3("viewPos", cameraNode->getCamera().getPosition());
        sunShader.setVec3("lightPos", glm::vec3(0.0f, 0.0f, 0.0f));
        sunShader.setVec3("lightColor", glm::vec3(1.0f, 0.9f, 0.7f));
        sunShader.setMat4("model", sunModelMatrix);

        // Textur binden
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, sunDiffuse);
        sunShader.setInt("tex0", 0);

        // Additives Blending aktivieren
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);

        // Zeichnen
        sunPlanet->draw(sunShader.ID, sunModelMatrix);

        // Blending zurücksetzen
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_BLEND);



        // Orbit 1
        std::vector<glm::vec3> circlePoints;
        int count = 10;
        float r = 80.0f;

        for (int i = 0; i < count; ++i) {
            float a = glm::radians(i * 360.0f / count);
            circlePoints.push_back(glm::vec3(cos(a) * r, 0.0f, sin(a) * r));
        }

        // WICHTIG: Punkte so duplizieren, dass die Kurve richtig „herumlaufen“ kann
        SplinePath orbit;
        orbit.addPoint(circlePoints[count - 2]); // P0 (vor letzter)
        orbit.addPoint(circlePoints[count - 1]); // P1 (letzter)
        for (const auto& p : circlePoints)       // P2...Pn
            orbit.addPoint(p);
        orbit.addPoint(circlePoints[0]);         // Pn+1
        orbit.addPoint(circlePoints[1]);         // Pn+2

        SplineRenderer orbitRenderer;
        orbitRenderer.setSpline(orbit);
        orbitRenderer.upload();

		// Orbit 2
        std::vector<glm::vec3> circlePoints2;
        float r2 = 140.0f;
        int pointCount2 = 10;

        for (int i = 0; i < count; ++i) {
            float a = glm::radians(i * 360.0f / count);
            circlePoints2.push_back(glm::vec3(cos(a) * r2, 0.0f, sin(a) * r2));
        }

        // WICHTIG: Punkte so duplizieren, dass die Kurve richtig „herumlaufen“ kann
        SplinePath orbit2;
        orbit2.addPoint(circlePoints2[count - 2]); // P0 (vor letzter)
        orbit2.addPoint(circlePoints2[count - 1]); // P1 (letzter)
        for (const auto& p : circlePoints2)       // P2...Pn
            orbit2.addPoint(p);
        orbit2.addPoint(circlePoints2[0]);         // Pn+1
        orbit2.addPoint(circlePoints2[1]);         // Pn+2


        SplineRenderer renderer2;
        renderer2.setSpline(orbit2);
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

        // Spline UI
        if (showUserSpline) {
            splineShader.use();
            splineShader.setVec3("color", glm::vec3(0.9f, 0.2f, 0.2f));
            userSplineRenderer->draw(splineShader, view, proj);
        }

        // Kamerafahrt
        if (animateCamera) {
            const auto& points = cameraPath.getInterpolatedPoints(20);
            cameraSplineTime += delta * cameraSpeedFactor;
            if (cameraSplineTime > 1.0f) cameraSplineTime = 0.0f;

            int index = static_cast<int>(cameraSplineTime * points.size());
            if (index >= points.size()) index = points.size() - 1;

            glm::vec3 camPos = points[index];
            glm::vec3 lookAt = glm::vec3(0, 0, 0);
            cameraNode->getCamera().setFromExternalPosition(camPos, lookAt);
        }




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