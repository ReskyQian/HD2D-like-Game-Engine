#include "editor/include/model.h"

#include <iostream>

namespace Hd2d {
    Model::Model(std::string_view path) {
        loadModel(path);
    }

    void Model::loadModel(std::string_view path) {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(std::string{path}, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
        // check for errors
        if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
        {
            std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
            return;
        }
        // retrieve the directory path of the filepath
        directory_ = path.substr(0, path.find_last_of('/'));

        // process ASSIMP's root node recursively
        processNode(scene->mRootNode, scene);
    }

    void Model::processNode(aiNode *node, const aiScene *scene) {
        // process each mesh located at the current node
        for(unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            // the node object only contains indices to index the actual objects in the scene. 
            // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            std::shared_ptr<Mesh> our_mesh = processMesh(mesh, scene);
            meshes_.push_back(*our_mesh);
        }
        // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
        for(unsigned int i = 0; i < node->mNumChildren; i++)
        {
            processNode(node->mChildren[i], scene);
        }
    }

    std::shared_ptr<Mesh> Model::processMesh(aiMesh *mesh, const aiScene *scene) {
        // data to fill
        std::shared_ptr<std::vector<Vertex>>       vertices     = std::make_shared<std::vector<Vertex>>();
        std::shared_ptr<std::vector<unsigned int>> indices      = std::make_shared<std::vector<unsigned int>>();
        std::shared_ptr<std::vector<Texture2D>>    all_textures = std::make_shared<std::vector<Texture2D>>();;

        // walk through each of the mesh's vertices
        for(unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;
            glm::vec3 vector; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
            // positions
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.position_ = vector;
            // normals
            if (mesh->HasNormals())
            {
                vector.x = mesh->mNormals[i].x;
                vector.y = mesh->mNormals[i].y;
                vector.z = mesh->mNormals[i].z;
                vertex.normal_ = vector;
            }
            // texture coordinates
            if(mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
            {
                glm::vec2 vec;
                // a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
                // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
                vec.x = mesh->mTextureCoords[0][i].x; 
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.texCoords_ = vec;
                // tangent
                vector.x = mesh->mTangents[i].x;
                vector.y = mesh->mTangents[i].y;
                vector.z = mesh->mTangents[i].z;
                vertex.tangent_ = vector;
                // bitangent
                vector.x = mesh->mBitangents[i].x;
                vector.y = mesh->mBitangents[i].y;
                vector.z = mesh->mBitangents[i].z;
                vertex.bitangent_ = vector;
            }
            else
                vertex.texCoords_ = glm::vec2(0.0f, 0.0f);

            vertices->push_back(vertex);
        }
        // now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
        for(unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            // retrieve all indices of the face and store them in the indices vector
            for(unsigned int j = 0; j < face.mNumIndices; j++)
                indices->push_back(face.mIndices[j]);        
        }
        // process materials
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];    
        // we assume a convention for sampler names in the shaders. Each diffuse texture should be named
        // as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
        // Same applies to other texture as the following list summarizes:
        // diffuse: texture_diffuseN
        // specular: texture_specularN
        // normal: texture_normalN

        // 1. diffuse maps
        std::shared_ptr<std::vector<Texture2D>> diffuseMaps = 
        loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        all_textures->insert(all_textures->end(), diffuseMaps->begin(), diffuseMaps->end());
        // 2. specular maps
        std::shared_ptr<std::vector<Texture2D>> specularMaps =
        loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        all_textures->insert(all_textures->end(), specularMaps->begin(), specularMaps->end());
        // 3. normal maps
        std::shared_ptr<std::vector<Texture2D>> normalMaps =
        loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
        all_textures->insert(all_textures->end(), normalMaps->begin(), normalMaps->end());
        // 4. height maps
        std::shared_ptr<std::vector<Texture2D>> heightMaps =
        loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
        all_textures->insert(all_textures->end(), heightMaps->begin(), heightMaps->end());  
        // return a mesh object created from the extracted mesh data
        return std::make_shared<Mesh>(*vertices, *indices, *all_textures);
    }

    std::shared_ptr<std::vector<Texture2D>> Model::loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName) {
        std::shared_ptr<std::vector<Texture2D>> textures = std::make_shared<std::vector<Texture2D>>();
        for(unsigned int i = 0; i < mat->GetTextureCount(type); i++)
        {
            aiString str;
            mat->GetTexture(type, i, &str);
            // check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
            bool skip = false;
            for(unsigned int j = 0; j < textures_loaded_.size(); j++)
            {
                if(std::strcmp(textures_loaded_[j].getPath().data(), str.C_Str()) == 0)
                {
                    textures->push_back(textures_loaded_[j]);
                    skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
                    break;
                }
            }
            if(!skip)
            {   // if texture hasn't been loaded already, load it
                std::string texture_path = std::string{directory_} + "/" + str.C_Str();
                std::shared_ptr<Texture2D> texture = Texture2D::loadFromFile(texture_path);
                texture->setTextureType(typeName);
                texture->setPath(str.C_Str());
                textures->push_back(*texture);
                textures_loaded_.push_back(*texture);  // store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
            }
        }
        return textures; 
    }

    void Model::draw(ShaderProgram& shader_program) {
        for(unsigned int i = 0; i < meshes_.size(); i++)
            meshes_[i].draw(shader_program);
    }
}