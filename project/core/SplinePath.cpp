#include "SplinePath.h"

SplinePath::SplinePath() {}

void SplinePath::addPoint(const glm::vec3& p) {
    controlPoints.push_back(p);
}

void SplinePath::clearPoints() {
    controlPoints.clear();
}
const std::vector<glm::vec3>& SplinePath::getControlPoints() const {  
   return controlPoints;  
}


const std::vector<glm::vec3>& SplinePath::getInterpolatedPoints(int segmentsPerCurve) {
    interpolated.clear();
    if (controlPoints.size() < 4) return interpolated;

    for (size_t i = 1; i < controlPoints.size() - 2; ++i) {
        for (int j = 0; j < segmentsPerCurve; ++j) {
            float t = (float)j / segmentsPerCurve;
            interpolated.push_back(catmullRom(
                controlPoints[i - 1], controlPoints[i], controlPoints[i + 1], controlPoints[i + 2], t
            ));
        }
    }
    return interpolated;
}

glm::vec3 SplinePath::catmullRom(const glm::vec3& p0, const glm::vec3& p1,
    const glm::vec3& p2, const glm::vec3& p3, float t) {
    float t2 = t * t;
    float t3 = t2 * t;
    return 0.5f * ((2.0f * p1) +
        (-p0 + p2) * t +
        (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t2 +
        (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t3);
}
