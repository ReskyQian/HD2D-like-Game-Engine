#include "editor/include/texture2d.h"

#include <string>
#include <string_view>
#include <fstream>
#include <sstream>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

using std::ifstream;
using std::ofstream;
using std::ios;

/// @brief load texture from image file to GPU
/// @param image_file_path  texture file path
/// @return image info
Texture2D* Texture2D::LoadFromFile(std::string_view image_file_path)
{
    Texture2D* texture2d = new Texture2D();

    // flip the image for alignment in OpenGL
    stbi_set_flip_vertically_on_load(true);
    int channels_in_file;

    unsigned char* data = stbi_load(
        std::string{image_file_path}.c_str(), 
        &(texture2d->width_), 
        &(texture2d->height_), 
        &channels_in_file, 
        0
    );

    int image_data_format = GL_RGB;
    if (data!= nullptr)
    {
        //decide color type according to nums of channel
        switch (channels_in_file) {
            case 1:
            {
                image_data_format = GL_ALPHA;
                break;
            }
            case 3:
            {
                image_data_format = GL_RGB;
                texture2d->gl_texture_format_=GL_COMPRESSED_RGB;
                break;
            }
            case 4:
            {
                image_data_format = GL_RGBA;
                texture2d->gl_texture_format_=GL_COMPRESSED_RGBA;
                break;
            }
        }
    }

    glGenTextures(1, &(texture2d->gl_texture_id_));
    glBindTexture(GL_TEXTURE_2D, texture2d->gl_texture_id_);

    // upload normal texture
    glTexImage2D(GL_TEXTURE_2D, texture2d->mipmap_level_, 
                texture2d->gl_texture_format_, texture2d->width_, texture2d->height_, 0, 
                image_data_format, GL_UNSIGNED_BYTE, data);

    glGenerateMipmap(GL_TEXTURE_2D);

    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // filters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    stbi_image_free(data);

    return texture2d;
}

/// @brief load texture from compressed image file to GPU
/// @param image_file_path compressed texture file path
/// @return image info
Texture2D* Texture2D::LoadFromCptFile(std::string_view image_file_path)
{
    Texture2D* texture2d = new Texture2D();

    std::ifstream fs{};
    fs.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try {
        fs.open(std::string{image_file_path}, ios::in | ios::binary);
        std::stringstream ss{};
        ss << fs.rdbuf();
        fs.close();
        CptFileHead cpt_file_head;
        ss.read((char*)&cpt_file_head, sizeof(CptFileHead));
        unsigned char* data =(unsigned char*)malloc(cpt_file_head.compress_size_);
        ss.read((char*)data, cpt_file_head.compress_size_);

        texture2d->gl_texture_format_ = cpt_file_head.gl_texture_format_;
        texture2d->width_ = cpt_file_head.width_;
        texture2d->height_ = cpt_file_head.height_;

        glGenTextures(1, &(texture2d->gl_texture_id_));
        glBindTexture(GL_TEXTURE_2D, texture2d->gl_texture_id_);

        // Upload compressoed texture
        glCompressedTexImage2D(GL_TEXTURE_2D, 0, texture2d->gl_texture_format_, 
                                texture2d->width_, texture2d->height_, 0, 
                                cpt_file_head.compress_size_, data);

        glGenerateMipmap(GL_TEXTURE_2D);

        // set the texture wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        // filters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        delete (data);
    }
    catch (std::ifstream::failure e) {
        std::cout << "Error::Texture::IMAGE_File_Not_Successfully_Read" << std::endl;
    }

    return texture2d;
}

/// @brief to compress a texture and save it, 
///        info comes from image_file,
///        data comes from GPU, which ensures to be uploaded by LoadFromFile() 
/// @param image_file_path normal texture file path
/// @param save_image_file_path compressed texture saved file path
void Texture2D::CompressImageFile(std::string_view image_file_path, std::string_view save_image_file_path)
{
    Texture2D* texture2d = Texture2D::LoadFromFile(image_file_path);

    // get success or not
    GLint compress_success=0;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_COMPRESSED, &compress_success);

    // get compress size
    GLint compress_size=0;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_COMPRESSED_IMAGE_SIZE, &compress_size);

    // get compress format
    GLint compress_format=0;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &compress_format);

    //4. download compressed texture
    void* img = malloc(compress_size);
    glGetCompressedTexImage(GL_TEXTURE_2D, 0, img);

    //5. save compressed texture
    ofstream output_file_stream(std::string{save_image_file_path}, ios::out | ios::binary);

    CptFileHead cpt_file_head;
    cpt_file_head.type_[0]           = 'c';
    cpt_file_head.type_[1]           = 'p';
    cpt_file_head.type_[2]           = 't';
    cpt_file_head.mipmap_level_      = texture2d->mipmap_level_;
    cpt_file_head.width_             = texture2d->width_;
    cpt_file_head.height_            = texture2d->height_;
    cpt_file_head.gl_texture_format_ = compress_format;
    cpt_file_head.compress_size_     = compress_size;

    output_file_stream.write((char*)&cpt_file_head, sizeof(CptFileHead));
    output_file_stream.write((char*)img, compress_size);
    output_file_stream.close();
}

/// @brief differ if file exists and if it is cpt format
/// @param image_file_path cpt file path
/// @return if true, cpt has generated and usable
bool Texture2D::IsCptFileExist(std::string_view image_file_path)
{
    bool isExist = false;
    bool isCpt = false;
    std::ifstream fs{};
    fs.open(std::string{image_file_path}, ios::in | ios::binary);
    isExist = fs.good();
    if (isExist) {
        std::stringstream ss{};
        ss << fs.rdbuf();
        char file_head[3];
        ss.read(file_head, sizeof(file_head));
        isCpt = CptFileHead::isCptFile(file_head);
    }
    fs.close();
    return isExist && isCpt;
}