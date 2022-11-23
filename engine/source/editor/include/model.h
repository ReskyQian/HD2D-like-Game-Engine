#ifndef _MODEL_H__
#define _MODEL_H__

#include <vector>
#include <string>
#include <memory>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "editor/include/shader.h"
#include "editor/include/texture2d.h"
#include "editor/include/mesh.h"

namespace Hd2d {
    class Model {
    public:
        Model(std::string_view path);

        void draw(ShaderProgram& shader_program);

        void deleteBuffer();

    private:
        // model data
        std::vector<Texture2D> textures_loaded_;	// stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
        std::vector<Mesh>      meshes_;
        std::string_view       directory_;
        bool                   gammaCorrection_;

        void loadModel(std::string_view path);

        void processNode(aiNode *node, const aiScene *scene);
        std::shared_ptr<Mesh> processMesh(aiMesh *mesh, const aiScene *scene);
        std::shared_ptr<std::vector<Texture2D>> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName);
    };
}

#endif // _MODEL_H__