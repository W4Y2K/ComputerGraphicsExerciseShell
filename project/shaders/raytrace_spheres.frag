#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform vec3 camPos;
uniform mat4 invViewProj;
uniform vec3 lightDir;

const int MAX_SPHERES = 10;
uniform int sphereCount;

struct Sphere {
    vec3 center;
    float radius;
};
uniform Sphere spheres[MAX_SPHERES];

bool intersectSphere(vec3 ro, vec3 rd, vec3 center, float radius, out vec3 hit, out vec3 normal) {
    vec3 oc = ro - center;
    float b = dot(oc, rd);
    float c = dot(oc, oc) - radius * radius;
    float h = b * b - c;
    if (h < 0.0) return false;
    h = sqrt(h);
    float t = -b - h;
    hit = ro + t * rd;
    normal = normalize(hit - center);
    return true;
}

void main() {
    vec2 uv = TexCoords * 2.0 - 1.0;
    vec4 rayStartH = invViewProj * vec4(uv, -1.0, 1.0);
    vec4 rayEndH   = invViewProj * vec4(uv, 1.0, 1.0);
    vec3 ro = camPos;
    vec3 rd = normalize((rayEndH.xyz / rayEndH.w) - (rayStartH.xyz / rayStartH.w));

    vec3 hit, normal;
    float closest = 1e20;
    vec3 finalColor = vec3(0);

    for (int i = 0; i < sphereCount; ++i) {
        vec3 tmpHit, tmpNormal;
        if (intersectSphere(ro, rd, spheres[i].center, spheres[i].radius, tmpHit, tmpNormal)) {
            float dist = length(tmpHit - ro);
            if (dist < closest) {
                closest = dist;
                float diff = max(dot(-lightDir, tmpNormal), 0.0);
                finalColor = vec3(1.0, 0.5, 0.2) * diff;
            }
        }
    }

    if (closest < 1e19)
        FragColor = vec4(finalColor, 1.0);
    else
        discard;
}
