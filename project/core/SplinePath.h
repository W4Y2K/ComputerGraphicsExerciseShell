#pragma once
#include <glm/glm.hpp>
#include <vector>

class SplinePath {
public:
    SplinePath();

    void addPoint(const glm::vec3& p);
    void clearPoints();
	const std::vector<glm::vec3>& getControlPoints() const;
    const std::vector<glm::vec3>& getInterpolatedPoints(int segmentsPerCurve = 20);

private:
    std::vector<glm::vec3> controlPoints;
    std::vector<glm::vec3> interpolated;
    glm::vec3 catmullRom(const glm::vec3& p0, const glm::vec3& p1,
        const glm::vec3& p2, const glm::vec3& p3, float t);
};
