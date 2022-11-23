#ifndef _MESH_H__
#define _MESH_H__

#include <glm/glm.hpp>

#include <vector>
#include <memory>

#include "editor/include/shader.h"
#include "editor/include/texture2d.h"

#define MAX_BONE_INFLUENCE 4

namespace Hd2d {
    struct Vertex {
        // position
        glm::vec3 position_;
        // normal
        glm::vec3 normal_;
        // texCoords
        glm::vec2 texCoords_;
        // tangent
        glm::vec3 tangent_;
        // bitangent
        glm::vec3 bitangent_;
        //bone indexes which will influence this vertex
        int boneIDs_[MAX_BONE_INFLUENCE];
        //weights from each bone
        float weights_[MAX_BONE_INFLUENCE];
    };

    class Mesh {
    public:
        explicit Mesh(std::vector<Vertex>       vertices ,
                      std::vector<unsigned int> indices  ,
                      std::vector<Texture2D>    textures);

        ~Mesh() {

        }

        constexpr std::vector<Vertex>&       getVertices() {return vertices_;}
        constexpr std::vector<unsigned int>& getIndices () {return indices_ ;}
        constexpr std::vector<Texture2D>&    getTextures() {return textures_;}

        void draw(ShaderProgram& shader_program);

        void deleteBuffer();

    private:
        unsigned int VAO, VBO, EBO;

        // mesh data
        std::vector<Vertex>       vertices_;
        std::vector<unsigned int> indices_ ;
        std::vector<Texture2D>    textures_;

        void setupMesh();

    };    

} // namespace Hd2d

#endif // _MESH_H__