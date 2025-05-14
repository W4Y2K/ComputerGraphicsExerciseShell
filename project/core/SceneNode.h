#pragma once
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Model;

class SceneNode {
public:
    SceneNode();

    virtual ~SceneNode() = default;

    void setModel(std::shared_ptr<Model> model);
    void addChild(std::shared_ptr<SceneNode> child);
    void removeChild(std::shared_ptr<SceneNode> child);
    // Setter und Getter für Rotationsgeschwindigkeit
    void setRotationSpeed(float speed);
    float getRotationSpeed() const;

    virtual void update(float deltaTime);
    virtual void draw(const glm::mat4& parentTransform, unsigned int shaderID);

    glm::mat4 transform;  // Lokale Transformation (Translation, Rotation etc.)

protected:
    std::shared_ptr<Model> model;
    std::vector<std::shared_ptr<SceneNode>> children;

    // Beispiel für Rotation um eigene Achse
    float rotationSpeed = 0.0f; // Grad/Sekunde
    float currentRotation = 0.0f;
};

class SelfRotatingNode : public SceneNode {
public:
    void draw(const glm::mat4& parentTransform, unsigned int shaderID) override;
};


class OscillatingRotationNode : public SceneNode {
public:
    OscillatingRotationNode(glm::vec3 basePosition, glm::vec3 axis, float speed, float maxAngle);
    void update(float deltaTime) override;

protected:
    glm::vec3 basePosition;
    glm::vec3 axis;
    float speed;
    float maxAngle;
    float time = 0.0f;
};

class OscillatingTranslationNode : public SceneNode {
public:
    OscillatingTranslationNode(glm::vec3 basePosition, glm::vec3 direction, float speed, float amplitude);
    void update(float deltaTime) override;

protected:
    glm::vec3 basePosition;
    glm::vec3 dir;
    float speed;
    float amplitude;
    float time = 0.0f;
};


