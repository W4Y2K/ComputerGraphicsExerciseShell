#include "SplineRenderer.h"

SplineRenderer::SplineRenderer() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
}

SplineRenderer::~SplineRenderer() {
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
}

void SplineRenderer::setSpline(SplinePath& path) {
    points = path.getInterpolatedPoints();  // z.B. 100–200 Punkte
}

void SplineRenderer::upload() {
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(glm::vec3), points.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

void SplineRenderer::draw(const Shader& shader, const glm::mat4& view, const glm::mat4& proj) {
    shader.use();
    shader.setMat4("view", view);
    shader.setMat4("projection", proj);
    shader.setMat4("model", glm::mat4(1.0f));
    glBindVertexArray(VAO);
    glDrawArrays(GL_LINE_STRIP, 0, static_cast<GLsizei>(points.size()));
    glBindVertexArray(0);
}
