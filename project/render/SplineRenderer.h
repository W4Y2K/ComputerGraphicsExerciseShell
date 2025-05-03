#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include "SplinePath.h"
#include "Shader.h"

class SplineRenderer {
public:
    SplineRenderer();
    ~SplineRenderer();

    void setSpline(SplinePath& path);
    void upload();  // Sendet Daten an die GPU
    void draw(const Shader& shader, const glm::mat4& view, const glm::mat4& proj);

private:
    GLuint VAO, VBO;
    std::vector<glm::vec3> points;
};
