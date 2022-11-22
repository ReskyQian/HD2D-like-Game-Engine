#ifndef _TEXTURE2D_H__
#define _TEXTURE2D_H__

#include <string>
#include <glad/glad.h>
#include <memory>

namespace Hd2d {
    struct CptFileHead
    {
        char type_[3];
        int mipmap_level_;
        int width_;
        int height_;
        int gl_texture_format_;
        int compress_size_;

        // diff if the file head is cpt
        static bool isCptFile(char* file_head) {
            return strlen(file_head) ==  3  &&
                        file_head[0] == 'c' && 
                        file_head[1] == 'p' &&
                        file_head[2] == 't' ;
        }
    };

    class Texture2D
    {
    public:
        explicit Texture2D() = default;

        constexpr GLuint getTextureId() {return gl_texture_id_;}
        constexpr std::string_view getTextureType() { return texture_type_;}
        constexpr void setTextureType(std::string_view type) { texture_type_ = type;}
        constexpr std::string_view getPath() { return path_;}
        constexpr void setPath(std::string_view path) { path_ = path;}

        static bool isCptFileExist(std::string_view image_file_path);
        static std::shared_ptr<Texture2D> loadFromFile(std::string_view image_file_path);
        static std::shared_ptr<Texture2D> loadFromCptFile(std::string_view image_file_path);
        static void compressImageFile(std::string_view image_file_path, std::string_view save_image_file_path);
        static std::shared_ptr<Texture2D> loadTexture(std::string_view png_path, std::string_view cpt_path);
        static void configTexture();
        static void generateMipmap();

    private:
        int mipmap_level_;
        int width_;
        int height_;

        GLenum gl_texture_format_;
        GLenum image_data_format_;
        GLuint gl_texture_id_;

        std::string_view texture_type_;
        std::string_view path_;
    };
}

#endif // TEXTURE2D_H__