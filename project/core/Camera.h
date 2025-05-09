// Camera.h
#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>

class Camera {
public:
    Camera(GLFWwindow* window);

    // Getter für GUI
    float getFOV() const { return fov; }
    float getNearPlane() const { return nearPlane; }
    float getFarPlane() const { return farPlane; }

    // Setter für GUI
    void setFOV(float value) { fov = value; }
    void setNearPlane(float value) { nearPlane = value; }
    void setFarPlane(float value) { farPlane = value; }

    void update();
    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix(float aspectRatio) const;

    void reset();
    glm::vec3 getPosition() const; // aktuelle Welt-position

    glm::vec3& getTarget() {
        return target;
    }
    void moveTarget(const glm::vec3& offset) {
        target += offset;
    }
    void move(const glm::vec3& offset) {
        target += offset;
    }

    void setFromExternalPosition(const glm::vec3& position, const glm::vec3& targetPos);

private:
    GLFWwindow* window;

    float distance = 5.0f;
    float yaw = -90.0f;
    float pitch = 0.0f;
    float lastX = 400.0f, lastY = 300.0f;
    bool firstMouse = true;
    bool leftMousePressed = false;

    float fov = 45.0f;
    float nearPlane = 0.1f;
    float farPlane = 5000.0f;

    glm::vec3 target = glm::vec3(0.0f);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

    void processMouseInput();
    void processScrollInput();
};
