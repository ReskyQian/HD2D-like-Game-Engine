// #ifndef _MODEL_H__
// #define _MODEL_H__

// #include <vector>
// #include <string>

// #include "editor/include/shader.h"
// #include "editor/include/texture2d.h"
// #include "editor/include/mesh.h"

// namespace Hd2d {
//     class Model {
//     public:
//         Model(std::string_view path);
//         ~Model();

//         void Draw(ShaderProgram& shader_program);
//     private:
//         // model data 
//         std::vector<Texture2D> textures_loaded;	// stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
//         std::vector<Mesh>    meshes;
//         std::string directory;
//         bool gammaCorrection;

//         void loadModel(std::string_view path);

//         void processNode(aiNode *node, const aiScene *scene);
//     };
// }

// #endif // _MODEL_H__