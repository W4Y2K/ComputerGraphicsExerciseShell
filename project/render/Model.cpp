#include "Model.h"
#include "Mesh.h"
#include "stb_image.h"
#include <tiny_gltf.h>
#include <iostream>

// Die Model-Klasse lädt ein glTF- oder glb-Modell, verarbeitet seine Meshes und Texturen
// und bietet eine draw()-Funktion zum Rendern mit einem OpenGL-Shaderprogramm.

Model::Model(const std::string& path) {
    loadModel(path);
}

void Model::draw(unsigned int shaderProgram, const glm::mat4& modelMatrix) {
    glUseProgram(shaderProgram);
    GLint locModel = glGetUniformLocation(shaderProgram, "model");
    if (locModel >= 0)
        glUniformMatrix4fv(locModel, 1, GL_FALSE, &modelMatrix[0][0]);



    unsigned int diffuseNr = 1;
    for (const auto& mesh : meshes) {
        for (unsigned int i = 0; i < mesh.textures.size(); ++i) {
            glActiveTexture(GL_TEXTURE0 + i);
            const std::string& name = mesh.textures[i].type;
            std::string number = std::to_string(diffuseNr++);
            glUniform1i(glGetUniformLocation(shaderProgram, (name + number).c_str()), i);
            glBindTexture(GL_TEXTURE_2D, mesh.textures[i].id);
        }

        glBindVertexArray(mesh.VAO);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mesh.indexCount), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        glActiveTexture(GL_TEXTURE0);
    }
}

void Model::loadModel(const std::string& path) {
    tinygltf::Model gltfModel;
    tinygltf::TinyGLTF loader;
    std::string err, warn;
 
    loader.SetImageLoader([](tinygltf::Image* image, const int imageIndex, std::string* err,  
                            std::string* warn, int req_width, int req_height,  
                            const unsigned char* bytes, int size, void* user_data) -> bool {  

       int width, height, channels;  
       unsigned char* data = stbi_load_from_memory(bytes, size, &width, &height, &channels, 4);  
       if (!data) {  
           if (err) *err = "Failed to load image using stb_image.";  
           return false;  
       }  

       image->width = width;  
       image->height = height;  
       image->component = 4;  
       image->image.resize(width * height * 4);  
       std::memcpy(image->image.data(), data, width * height * 4);  
       stbi_image_free(data);  
       return true;  
    }, nullptr);  

    bool ret = loader.LoadBinaryFromFile(&gltfModel, &err, &warn, path);


    if (!warn.empty()) std::cout << "Warn: " << warn << std::endl;
    if (!err.empty()) std::cerr << "Err: " << err << std::endl;
    if (!ret) {
        std::cerr << "Failed to load glTF model: " << path << std::endl;
        return;
    }

    for (const auto& mesh : gltfModel.meshes) {
        for (const auto& primitive : mesh.primitives) {
            std::vector<Vertex> vertices;
            std::vector<unsigned int> indices;

            const auto& posAccessor = gltfModel.accessors[primitive.attributes.at("POSITION")];
            const auto& posView = gltfModel.bufferViews[posAccessor.bufferView];
            const auto& posBuffer = gltfModel.buffers[posView.buffer];

            const unsigned char* posBase = posBuffer.data.data() + posView.byteOffset + posAccessor.byteOffset;

            int posStride = posAccessor.ByteStride(posView);
            if(posStride == 0) posStride = 12;

            vertices.resize(posAccessor.count);

            for (size_t i = 0; i < posAccessor.count; ++i) {
                const float* p = reinterpret_cast<const float*> (posBase + i * posStride);
                vertices[i].Position = {p[0], p[1], p[2]};
            }

            if (primitive.attributes.count("NORMAL")) {
                const auto& normAccessor = gltfModel.accessors[primitive.attributes.at("NORMAL")];
                const auto& normView = gltfModel.bufferViews[normAccessor.bufferView];
                const auto& normBuffer = gltfModel.buffers[normView.buffer];

                int nStride = normAccessor.ByteStride(normView);

                if(nStride==0) nStride=12;

                const unsigned char* nBase= normBuffer.data.data() + normView.byteOffset + normAccessor.byteOffset;

                for(size_t i = 0 ; i < normAccessor.count; ++i){
                    const float* n = reinterpret_cast<const float*> (nBase + i * nStride);
                    vertices[i].Normal = {n[0], n[1], n[2]};
                }
            } else { 
                for (auto& v : vertices) v.Normal = {0,1,0};
            }
)
            if (primitive.attributes.count("TEXCOORD_0")) {
                const auto& uvAccessor = gltfModel.accessors[primitive.attributes.at("TEXCOORD_0")];
                const auto& uvView = gltfModel.bufferViews[uvAccessor.bufferView];
                const auto& uvBuffer = gltfModel.buffers[uvView.buffer];
                
                int uvStride = uvAccessor.ByteStride(uvView);

                if(uvStride == 0) uvStride = 8;

                const unsigned char* uvBase= uvBuffer.data.data() + uvView.byteOffset + uvAccessor.byteOffset;

                for(size_t i = 0; i < uvAccessor.count; ++i){
                    const float* t = reinterpret_cast<const float*>(uvBase + i * uvStride);
                    vertices[i].TexCoords={t[0],t[1]};
                }
            }

            const auto& idxAccessor = gltfModel.accessors[primitive.indices];
            const auto& idxView = gltfModel.bufferViews[idxAccessor.bufferView];
            const auto& idxBuffer = gltfModel.buffers[idxView.buffer];

            const unsigned char* iBase = idxBuffer.data.data() + idxView.byteOffset + idxAccessor.byteOffset;

            switch (idxAccessor.componentType) {
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
                    for (size_t i = 0; i < idxAccessor.count; ++i) 
                        indices.push_back(reinterpret_cast<const uint8_t*> (iBase)[i]);
                    break;
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
                    for (size_t i = 0; i < idxAccessor.count; ++i)
                        indices.push_back(reinterpret_cast<const uint16_t*> (iBase)[i]);
                    break;
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
                    for (size_t i = 0; i < idxAccessor.count; ++i)
                        indices.push_back(reinterpret_cast<const uint32_t*> (iBase)[i]);
                    break;
            }

            std::vector<Texture> textures;

            if (primitive.material >= 0) {
                const auto& mat = gltfModel.materials[primitive.material];
                int texIdx = mat.pbrMetallicRoughness.baseColorTexture.index;

                if (texIdx >= 0) {
                    int imgIdx = gltfModel.textures[texIdx].source;
                    const auto& img = gltfModel.images[imgIdx];

                    GLuint id; 
                    glGenTextures(1, &id); 
                    glBindTexture(GL_TEXTURE_2D, id);

                    GLenum fmt = (img.component == 4) ? GL_RGBA : GL_RGB;

                    glTexImage2D(GL_TEXTURE_2D,0,fmt,
                                 img.width,img.height,0,fmt,GL_UNSIGNED_BYTE,
                                 img.image.data());
                    glGenerateMipmap(GL_TEXTURE_2D);
                    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
                    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
                    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

                    textures.push_back({id,"texture_diffuse",""});
                }
            }

            meshes.emplace_back(vertices, indices, textures);
        }
    }
}

Model::Mesh::Mesh(const std::vector<Vertex>& vertices,
    const std::vector<unsigned int>& indices,
    const std::vector<Texture>& textures)
    : textures(textures), indexCount(static_cast<unsigned int>(indices.size())) {

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

    glBindVertexArray(0);
}
