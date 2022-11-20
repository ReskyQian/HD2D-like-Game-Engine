// #include <Importer.hpp>
// #include <scene.h>
// #include <postprocess.h>
// #include <iostream>

// #include "editor/include/model.h"

// namespace Hd2d {
//     Model::Model(std::string_view path) {
//         loadModel(path);
//     }

//     void Model::loadModel(std::string_view path) {
//         Assimp::Importer importer;
//         const aiScene* scene = importer.ReadFile(std::string{path}, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
//         // check for errors
//         if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
//         {
//             std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
//             return;
//         }
//         // retrieve the directory path of the filepath
//         directory = path.substr(0, path.find_last_of('/'));

//         // process ASSIMP's root node recursively
//         processNode(scene->mRootNode, scene);
//     }

//     void Model::processNode(aiNode *node, const aiScene *scene) {

//     }

//     void Model::Draw(ShaderProgram& shader_program) {

//     }
// }