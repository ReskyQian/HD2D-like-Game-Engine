#include "editor/include/texture2d.h"

#include <string>
#include <string_view>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

using std::ifstream;
using std::ofstream;
using std::ios;

namespace Hd2d {
    /// @brief load texture from image file to GPU
    /// @param image_file_path  texture file path
    /// @return image info
    std::shared_ptr<Texture2D> Texture2D::loadFromFile(std::string_view image_file_path) {
        std::shared_ptr<Texture2D> texture2d = std::make_shared<Texture2D>();

        // don't flip the image for alignment in OpenGL
        stbi_set_flip_vertically_on_load(false);
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
                    image_data_format = GL_ALPHA;
                    break;
                case 3:
                    image_data_format = GL_RGB;
                    texture2d->gl_texture_format_=GL_COMPRESSED_RGB;
                    break;
                case 4:
                    image_data_format = GL_RGBA;
                    texture2d->gl_texture_format_=GL_COMPRESSED_RGBA;
                    break;
            }
        }

        glGenTextures(1, &(texture2d->gl_texture_id_));
        glBindTexture(GL_TEXTURE_2D, texture2d->gl_texture_id_);

        // upload normal texture
        glTexImage2D(GL_TEXTURE_2D, texture2d->mipmap_level_, 
                    texture2d->gl_texture_format_, texture2d->width_, texture2d->height_, 0, 
                    image_data_format, GL_UNSIGNED_BYTE, data);

        configTexture();

        stbi_image_free(data);

        return texture2d;
    }

    /// @brief load texture from compressed image file to GPU
    /// @param image_file_path compressed texture file path
    /// @return image info
    std::shared_ptr<Texture2D> Texture2D::loadFromCptFile(std::string_view image_file_path) {
        std::shared_ptr<Texture2D> texture2d = std::make_shared<Texture2D>();

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

            configTexture();

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
    void Texture2D::compressImageFile(std::string_view image_file_path, std::string_view save_image_file_path) {
        std::shared_ptr<Texture2D> texture2d = loadFromFile(image_file_path);

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

        free(img);
    }

    /// @brief differ if file exists and if it is cpt format
    /// @param image_file_path cpt file path
    /// @return if true, cpt has generated and usable
    bool Texture2D::isCptFileExist(std::string_view image_file_path) {
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

    std::shared_ptr<Texture2D> Texture2D::loadCubemap(std::vector<std::string>& faces) {
        std::shared_ptr<Texture2D> texture2d = std::make_shared<Texture2D>();
        glGenTextures(1, &(texture2d->gl_texture_id_));
        glBindTexture(GL_TEXTURE_CUBE_MAP, texture2d->gl_texture_id_);

        // skybox normally has rgb without a
        int channels_in_file;
        for (unsigned int i = 0; i < faces.size(); i++)
        {
            unsigned char *data = stbi_load(faces[i].c_str(), 
            &texture2d->width_, &texture2d->height_, &channels_in_file, 0);
            if (data)
            {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, texture2d->width_, texture2d->height_, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
                stbi_image_free(data);
            }
            else
            {
                std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
                stbi_image_free(data);
            }
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        return texture2d;
    }

    /// @brief Load texture from image file path
    /// @param cpt_path default load path
    /// @param png_path if cpt path is invalid, load from png path
    /// @return texture description of image
    std::shared_ptr<Texture2D> Texture2D::loadTexture(std::string_view png_path, std::string_view cpt_path) {
        if (!isCptFileExist(cpt_path)) {
            compressImageFile(png_path, cpt_path);
        }
        return loadFromCptFile(cpt_path);    
    }

    /// @brief general config texture
    void Texture2D::configTexture() {
        // wrapper
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); 
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // filters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    void Texture2D::generateMipmap() {
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    }

    void Texture2D::configClampWrapper() {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat 
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
}