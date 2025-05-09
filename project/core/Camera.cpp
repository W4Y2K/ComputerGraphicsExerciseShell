// Camera.cpp
#include "Camera.h"
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

Camera::Camera(GLFWwindow* win) : window(win) {
    reset();
}

void Camera::reset() {
    distance = 5.0f;
    yaw = -90.0f;
    pitch = 0.0f;
}

void Camera::update() {
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
        leftMousePressed = true;
    }
    else {
        leftMousePressed = false;
        firstMouse = true;
    }

    processMouseInput();
    processScrollInput();
}

glm::mat4 Camera::getViewMatrix() const {
    float radYaw = glm::radians(yaw);
    float radPitch = glm::radians(pitch);

    glm::vec3 dir;
    dir.x = cos(radYaw) * cos(radPitch);
    dir.y = sin(radPitch);
    dir.z = sin(radYaw) * cos(radPitch);

    glm::vec3 camPos = target - dir * distance;
    return glm::lookAt(camPos, target, up);
}


glm::mat4 Camera::getProjectionMatrix(float aspectRatio) const {
    return glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
}

void Camera::processMouseInput() {
    if (!leftMousePressed) return;

    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    if (firstMouse) {
        lastX = static_cast<float>(xpos);
        lastY = static_cast<float>(ypos);
        firstMouse = false;
    }

    float xoffset = static_cast<float>(xpos - lastX);
    float yoffset = static_cast<float>(lastY - ypos); // umgekehrt
    lastX = static_cast<float>(xpos);
    lastY = static_cast<float>(ypos);

    float sensitivity = 0.1f;
    yaw += xoffset * sensitivity;
    pitch += yoffset * sensitivity;

    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;
}

void Camera::processScrollInput() {
    // Implementierbar via Scroll-Callback – kann optional ergänzt werden
}

// Neuer Getter f r die berechnete Kamera-Position
glm::vec3 Camera::getPosition() const {
    float radYaw = glm::radians(yaw);
    float radPitch = glm::radians(pitch);

    glm::vec3 dir;
    dir.x = cos(radYaw) * cos(radPitch);
    dir.y = sin(radPitch);
    dir.z = sin(radYaw) * cos(radPitch);

    return target - dir * distance;
}

void Camera::setFromExternalPosition(const glm::vec3& position, const glm::vec3& targetPos) {
    target = targetPos;
    glm::vec3 dir = glm::normalize(target - position);
    distance = glm::length(target - position);
    pitch = glm::degrees(asin(dir.y));
    yaw = glm::degrees(atan2(dir.z, dir.x));
}