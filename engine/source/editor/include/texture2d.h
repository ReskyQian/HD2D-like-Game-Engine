#ifndef _TEXTURE2D_H__
#define _TEXTURE2D_H__

#include <string>
#include <glad/glad.h>

class Texture2D
{
public:
    Texture2D(const Texture2D&) = delete;
    Texture2D& operator=(const Texture2D&) = delete;
    Texture2D(Texture2D&& other) = delete;
    Texture2D& operator=(Texture2D&& other) = delete;

    static Texture2D* LoadFromFile(std::string_view image_file_path);
    static Texture2D* LoadFromCptFile(std::string_view image_file_path);
    static void CompressImageFile(std::string_view image_file_path, std::string_view save_image_file_path);

public:
    int mipmap_level_;
    int width_;
    int height_;

    GLenum gl_texture_format_;
    GLuint gl_texture_id_;
   
    struct CptFileHead
    {
        char type_[3];
        int mipmap_level_;
        int width_;
        int height_;
        int gl_texture_format_;
        int compress_size_;
    };

private:
    explicit Texture2D() : mipmap_level_(0) {};
    ~Texture2D();
};


#endif // TEXTURE2D_H__