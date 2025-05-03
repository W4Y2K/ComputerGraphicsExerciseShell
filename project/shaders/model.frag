#version 330 core
out vec4 FragColor;
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

struct PointLight {
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

#define MAX_POINT_LIGHTS 4

uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 viewPos;
uniform sampler2D texture_diffuse;
uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform int numPointLights;
uniform bool enableDirectionalLight;

// Funktions-Prototypen
vec3 CalcDirLight(vec3 direction, vec3 ambient, vec3 diffuse, vec3 specular, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{
    // Eigenschaften
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    
    // Default-Material-Eigenschaften
    float shininess = 32.0;
    
    // Ergebnis initialisieren
    vec3 result = vec3(0.0);
    
    // Direktionales Licht hinzufügen, wenn aktiviert
    if(enableDirectionalLight) {
        vec3 ambient = vec3(0.1);
        vec3 diffuse = lightColor;
        vec3 specular = lightColor;
        result += CalcDirLight(lightDir, ambient, diffuse, specular, norm, viewDir);
    }
    
    // Alle Punktlicht-Beiträge hinzufügen
    for(int i = 0; i < numPointLights && i < MAX_POINT_LIGHTS; i++) {
        result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);
    }
    
    // Wenn keine Lichter aktiviert sind, Standard-Umgebungslicht verwenden
    if (!enableDirectionalLight && numPointLights == 0) {
        result = vec3(texture(texture_diffuse, TexCoords)) * 0.2;
    }
    
    // Ausgabefarbe setzen
    FragColor = vec4(result, 1.0);
}

// Berechnet den Farbbeitrag eines direktionalen Lichtquelle
vec3 CalcDirLight(vec3 direction, vec3 ambient, vec3 diffuse, vec3 specular, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-direction);
    
    // Diffuse Schattierung
    float diff = max(dot(normal, lightDir), 0.0);
    
    // Spekulare Schattierung
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    
    // Ergebnisse kombinieren
    vec3 ambientResult = ambient * vec3(texture(texture_diffuse, TexCoords));
    vec3 diffuseResult = diffuse * diff * vec3(texture(texture_diffuse, TexCoords));
    vec3 specularResult = specular * spec * vec3(texture(texture_diffuse, TexCoords));
    
    return (ambientResult + diffuseResult + specularResult);
}

// Berechnet den Farbbeitrag einer Punktlichtquelle
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    
    // Diffuse Schattierung
    float diff = max(dot(normal, lightDir), 0.0);
    
    // Spekulare Schattierung
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    
    // Dämpfung
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
                 light.quadratic * (distance * distance));    
    
    // Ergebnisse kombinieren
    vec3 ambient = light.ambient * vec3(texture(texture_diffuse, TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(texture_diffuse, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(texture_diffuse, TexCoords));
    
    // Dämpfung anwenden
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    
    return (ambient + diffuse + specular);
}