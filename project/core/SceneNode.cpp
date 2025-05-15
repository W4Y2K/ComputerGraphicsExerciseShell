#include "SceneNode.h"
#include "Model.h"

SceneNode::SceneNode() {
    transform = glm::mat4(1.0f);
}

void SceneNode::setModel(std::shared_ptr<Model> model) {
    this->model = model;
}

void SceneNode::addChild(std::shared_ptr<SceneNode> child) {
    children.push_back(child);
}

void SceneNode::removeChild(std::shared_ptr<SceneNode> child) {
    auto it = std::find(children.begin(), children.end(), child);
    if (it != children.end()) {
        children.erase(it);
    }
}

void SceneNode::update(float deltaTime) {
    if (rotationSpeed != 0.0f) {
        currentRotation += rotationSpeed * deltaTime;
        if (currentRotation > 360.0f) currentRotation -= 360.0f;
    }

    for (auto& child : children) {
        child->update(deltaTime);
    }
}

void SceneNode::draw(const glm::mat4& parentTransform, unsigned int shaderID) {
    glm::mat4 rotation = glm::rotate(glm::mat4(1.0f),
        glm::radians(currentRotation),
        glm::vec3(0.0f, 1.0f, 0.0f));

    glm::mat4 localTransform = transform * rotation;

    glm::mat4 globalTransform = parentTransform * localTransform;

    if (model) {
        model->draw(shaderID, globalTransform);
    }

    for (auto& child : children) {
        child->draw(globalTransform, shaderID);
    }
}

void SceneNode::setRotationSpeed(float speed) {
    rotationSpeed = speed;
}

float SceneNode::getRotationSpeed() const {
    return rotationSpeed;
}


void SelfRotatingNode::draw(const glm::mat4& parentTransform, unsigned int shaderID) {
    glm::mat4 rotation = glm::rotate(glm::mat4(1.0f),
        glm::radians(currentRotation),
        glm::vec3(0.0f, 1.0f, 0.0f));

    glm::mat4 localTransform = rotation * transform;

    glm::mat4 globalTransform = parentTransform * localTransform;

    if (model) {
        model->draw(shaderID, globalTransform);
    }

    for (auto& child : children) {
        child->draw(globalTransform, shaderID);
    }
}

OscillatingRotationNode::OscillatingRotationNode(glm::vec3 basePosition, glm::vec3 axis, float speed, float maxAngle)
    : basePosition(basePosition), axis(glm::normalize(axis)), speed(speed), maxAngle(maxAngle) {
}

void OscillatingRotationNode::update(float deltaTime) {
    time += deltaTime;
    float angle = sin(time * speed) * maxAngle;
    transform = glm::translate(glm::mat4(1.0f), basePosition) *
        glm::rotate(glm::mat4(1.0f), glm::radians(angle), axis);
    SceneNode::update(deltaTime);
}

// -------- OscillatingTranslationNode --------
OscillatingTranslationNode::OscillatingTranslationNode(glm::vec3 basePosition, glm::vec3 direction, float speed, float amplitude)
    : basePosition(basePosition), dir(glm::normalize(direction)), speed(speed), amplitude(amplitude) {
}

void OscillatingTranslationNode::update(float deltaTime) {
    time += deltaTime;
    float offset = sin(time * speed) * amplitude;
    glm::vec3 movement = dir * offset;
    transform = glm::translate(glm::mat4(1.0f), basePosition + movement);
    SceneNode::update(deltaTime);
}
