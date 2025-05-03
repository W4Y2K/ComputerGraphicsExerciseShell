#version 330 core

in vec3 Normal;
in vec3 crntPos;
in vec2 texCoord;

uniform vec3 lightPos;
uniform vec3 camPos;
uniform vec3 lightColor;

uniform sampler2D tex0; // diffuse texture
uniform sampler2D tex1; // specular texture map

out vec4 FragColor;

void main()
{
    vec3 lightVec = lightPos - crntPos;
    float dist = length(lightVec);
    float a = 0.5;
    float b = 0.2;
    float inten = 1.0 / (a * dist * dist + b * dist + 1.0);

    float ambient = 0.4;

    vec3 normal = normalize(Normal);
    vec3 lightDir = normalize(lightVec);
    float diff = max(dot(normal, lightDir), 0.0);

    vec3 viewDir = normalize(camPos - crntPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    float specularStrength = 1.0;

    vec3 texColor = texture(tex0, texCoord).rgb;
    float specMap = texture(tex1, texCoord).r;

    vec3 lighting = lightColor * inten;
    vec3 emission = texColor * 1.0; // Sonnenemission

    vec3 result = texColor * lighting * (ambient + diff) +
                  vec3(specularStrength * spec * specMap) * lighting +
                  emission;

    // Bloom-Vorbereitung: Helligkeit künstlich erhöhen
    result = min(result * 1.2, vec3(1.0));

    FragColor = vec4(result, 1.0);
}
