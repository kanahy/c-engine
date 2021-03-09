////////////////////////////////////////
// C-Engine - Easy to use engine.
//
// Copyright (C) 2020-2021 Kana
//
// Mail:   kanahy.akama@bk.ru
// GitHub: https://github.com/kanahy
////////////////////////////////////////
#ifndef c_engine_h
#define c_engine_h

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <dirent.h>
#include <pthread.h>


#define array_size(array) ((unsigned int)(sizeof(array) / sizeof(array[0])))


typedef enum directory_content_type {
    DIRECTORY_CONTENT_TYPE_UNKNOWN             = 0,
    DIRECTORY_CONTENT_TYPE_FIFO                = 1,
    DIRECTORY_CONTENT_TYPE_DEVICE              = 2,
    DIRECTORY_CONTENT_TYPE_DIRECTORY           = 4,
    DIRECTORY_CONTENT_TYPE_BLOCK_DEVICE        = 6,
    DIRECTORY_CONTENT_TYPE_REGULAR_FILE        = 8,
    DIRECTORY_CONTENT_TYPE_SYMBOLIC_LINK       = 10,
    DIRECTORY_CONTENT_TYPE_LOCAL_DOMAIN_SOCKET = 12,
    DIRECTORY_CONTENT_TYPE_WHITEOUT            = 14
} directory_content_type;

typedef enum file_type {
    FILE_TYPE_BINARY,
    FILE_TYPE_TEXT
} file_type;


typedef struct directory_content_t {
    char* name;
    directory_content_type type;
} directory_content_t;

typedef struct directory_t {
    char* path;
    directory_content_t* content;
    size_t content_count;
} directory_t;

typedef struct file_t {
    void* data;
    size_t size;
} file_t;


void file_free(file_t* self) {
    if (self->data) {
        free(self->data);
        self->data = NULL;
        self->size = 0;
    }
}

file_t file_load(const char* file_name, file_type type) {
    file_t result = {
        .data = NULL,
        .size = 0
    };
    FILE* stream = NULL;
    long stream_size = 0;

    stream = fopen(file_name, type == FILE_TYPE_TEXT ? "rt" : "rb");

    if (stream) {
        if (!fseek(stream, 0, SEEK_END)) {
            stream_size = ftell(stream);

            if (stream_size) {
                result.size = (size_t)stream_size;

                if (!fseek(stream, 0, SEEK_SET)) {
                    result.data = calloc(result.size, sizeof(char));

                    if (result.data) {
                        if (!fread(result.data, result.size, sizeof(char), stream)) {
                            file_free(&result);
                        }
                    }
                }
            }
        }

        if (fclose(stream)) {
            printf("%s is not closed\n", file_name);
        }
    }

    return result;
}

const char* file_get_extension(const char* file_name) {
    const char* dot = strrchr(file_name, '.');

    if (!dot || strcmp(file_name, dot) <= 0) {
        return NULL;
    }

    return dot + 1;
}

bool file_check_extension(const char* file_name, const char* extension) {
    return strcmp(file_get_extension(file_name), extension) == 0;
}


directory_content_t directory_content_new(struct dirent* dirent) {
    directory_content_t result = {
        .name = NULL,
        .type = DIRECTORY_CONTENT_TYPE_UNKNOWN
    };

    size_t path_size = 0;

    path_size = strlen(dirent->d_name);

    if (path_size) {
        result.name = (char*)calloc(path_size + 1, sizeof(char));

        if (result.name) {
            result.name = strncpy(result.name, dirent->d_name, path_size);
            result.name[path_size] = '\0';

            switch (dirent->d_type) {
                case 1: result.type = DIRECTORY_CONTENT_TYPE_FIFO; break;
                case 2: result.type = DIRECTORY_CONTENT_TYPE_DEVICE; break;
                case 4: result.type = DIRECTORY_CONTENT_TYPE_DIRECTORY; break;
                case 6: result.type = DIRECTORY_CONTENT_TYPE_BLOCK_DEVICE; break;
                case 8: result.type = DIRECTORY_CONTENT_TYPE_REGULAR_FILE; break;
                case 10: result.type = DIRECTORY_CONTENT_TYPE_SYMBOLIC_LINK; break;
                case 12: result.type = DIRECTORY_CONTENT_TYPE_LOCAL_DOMAIN_SOCKET; break;
                case 14: result.type = DIRECTORY_CONTENT_TYPE_WHITEOUT; break;
                default: result.type = DIRECTORY_CONTENT_TYPE_UNKNOWN; break;
            }
        }
    }

    return result;
}

void directory_content_delete(directory_content_t* self) {
    if (self->name) {
        free(self->name);
        self->name = NULL;
    }

    self->type = DIRECTORY_CONTENT_TYPE_UNKNOWN;
}


directory_t directory_load(const char* path) {
    directory_t result = {
        .path = NULL,
        .content = NULL,
        .content_count = 0
    };

    DIR* dir = NULL;
    struct dirent* dirent = NULL;
    size_t count = 0;
    size_t current = 0;
    size_t path_size = 0;

    dir = opendir(path);

    if (dir) {
        path_size = strlen(path);
        result.path = (char*)calloc(path_size + 1, sizeof(char));
        result.path = strncpy(result.path, path, path_size);
        result.path[path_size] = '\0';
        path_size = 0;

        while ((dirent = readdir(dir))) {
            ++count;
        }

        rewinddir(dir);

        result.content = (directory_content_t*)calloc(count, sizeof(directory_content_t));

        if (result.content) {
            while ((dirent = readdir(dir))) {
                result.content[current] = directory_content_new(dirent);
                ++current;
            }

            result.content_count = count;
        }

        closedir(dir);
    }

    return result;
}

void directory_free(directory_t* self) {
    if (self->path) {
        free(self->path);
        self->path = NULL;
    }

    if (self->content) {
        for (size_t i = 0; i < self->content_count; ++i) {
            directory_content_delete(&self->content[i]);
        }

        free(self->content);
        self->content = NULL;
    }

    self->content_count = 0;
}


#include <cglm/cglm.h>     // Math

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h> // Image loader

#define CGLTF_IMPLEMENTATION
#include <cgltf/cgltf.h>   // 2D/3D models, materials, scenes, etc...

#include <glad/gl.h>       // OpenGL loader

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>    // Windows


// Set position in pixels:  1 - offset in pixels;  2 - object size in pixels
// Set scale    in pixels:  1 - scale  in pixels;  2 - window size in pixels
#define to_pixels(image_size, window_size) ((1.0f / (float)window_size) * (float)image_size)
#define to_percent(image_size, window_size) ((float)image_size / (float)window_size)


typedef enum shader_type {
    SHADER_TYPE_VERTEX,
    SHADER_TYPE_GEOMETRY,
    SHADER_TYPE_FRAGMENT
} shader_type;


typedef struct texture_t {
    GLuint id;
} texture_t;

typedef struct animated_texture_t {
    GLuint* frames;
    GLuint* delays;
    GLuint frames_count;
    GLuint current_frame;
    double current_time;
} animated_texture_t;

typedef struct shader_t {
    GLuint id;
} shader_t;

typedef struct program_t {
    GLuint id;
} program_t;

typedef struct mesh_t {
    GLuint id;
    GLsizei indices_count;
} mesh_t;

typedef struct object_t {
    vec3 position;
    vec3 rotation;
    vec3 scale;
    mat4 matrix;
    program_t program;
    mesh_t mesh;
    texture_t* textures;
    int textures_count;
} object_t;

typedef struct camera_t {
    mat4 projection;
    mat4 view;
    vec3 position;
    vec3 rotation;
    vec3 direction;
    vec3 center;
} camera_t;


GLenum gl_debug() {
    GLenum result = glGetError();

    if (result != GL_NO_ERROR) {
        switch (result) {
            case GL_INVALID_ENUM: puts("GL error: GL_INVALID_ENUM"); break;
            case GL_INVALID_VALUE: puts("GL error: GL_INVALID_VALUE"); break;
            case GL_INVALID_OPERATION: puts("GL error: GL_INVALID_OPERATION"); break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: puts("GL error: GL_INVALID_FRAMEBUFFER_OPERATION"); break;
            case GL_OUT_OF_MEMORY: puts("GL error: GL_OUT_OF_MEMORY"); break;
            case GL_STACK_UNDERFLOW: puts("GL error: GL_STACK_UNDERFLOW"); break;
            case GL_STACK_OVERFLOW: puts("GL error: GL_STACK_OVERFLOW"); break;
            default: puts("GL error: UNKNOWN"); break;
        }
    }

    return result;
}


texture_t texture_create(const char* file_name) {
    texture_t result = {
        .id = 0
    };
    int width = 0;
    int height = 0;
    void* pixels = NULL;

    pixels = stbi_load(file_name, &width, &height, NULL, STBI_rgb_alpha);

    if (pixels) {
        glCreateTextures(GL_TEXTURE_2D, 1, &result.id);
        gl_debug();

        if (result.id) {
            glTextureParameteri(result.id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            gl_debug();
            glTextureParameteri(result.id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            gl_debug();
            glTextureParameteri(result.id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
            gl_debug();
            glTextureParameteri(result.id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            gl_debug();
            glTextureStorage2D(result.id, 1, GL_RGBA8, (GLsizei)width, (GLsizei)height);
            gl_debug();
            glTextureSubImage2D(result.id, 0, 0, 0, (GLsizei)width, (GLsizei)height, GL_RGBA, GL_UNSIGNED_BYTE, (const void*)pixels);
            gl_debug();
            glGenerateTextureMipmap(result.id);
            gl_debug();
        }

        stbi_image_free(pixels);
    }

    return result;
}

void texture_destroy(texture_t* self) {
    glDeleteTextures(1, &self->id);
    gl_debug();

    self->id = 0;
}

void texture_bind(const texture_t* self) {
    glBindTexture(GL_TEXTURE_2D, self->id);
    gl_debug();
}

void texture_unbind() {
    glBindTexture(GL_TEXTURE_2D, 0);
    gl_debug();
}

GLint texture_get_width(const texture_t* self) {
    GLint result = 0;
    glGetTextureLevelParameteriv(self->id, 0, GL_TEXTURE_WIDTH, &result);
    gl_debug();
    return result;
}

GLint texture_get_height(const texture_t* self) {
    GLint result = 0;
    glGetTextureLevelParameteriv(self->id, 0, GL_TEXTURE_HEIGHT, &result);
    gl_debug();
    return result;
}


animated_texture_t animated_texture_create(const char* file_name) {
    animated_texture_t result = {
        .frames = NULL,
        .delays = NULL,
        .frames_count = 0,
        .current_frame = 0,
        .current_time = 0.0
    };

    unsigned char* pixels = NULL;
    int* delays = NULL;
    int width = 0;
    int height = 0;
    int frames_count = 0;

    file_t file = file_load(file_name, FILE_TYPE_BINARY);

    if (file.data) {
        pixels = stbi_load_gif_from_memory((const stbi_uc*)file.data, (int)file.size, &delays, &width, &height, &frames_count, NULL, STBI_rgb_alpha);

        if (pixels) {
            result.frames = (GLuint*)calloc((size_t)frames_count, sizeof(GLuint));

            if (result.frames) {
                result.delays = (GLuint*)delays;
                result.frames_count = (GLuint)frames_count;

                for (int i = 0; i < frames_count; ++i) {
                    glCreateTextures(GL_TEXTURE_2D, 1, &result.frames[i]);
                    gl_debug();

                    if (result.frames[i]) {
                        glTextureParameteri(result.frames[i], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                        gl_debug();
                        glTextureParameteri(result.frames[i], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                        gl_debug();
                        glTextureParameteri(result.frames[i], GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
                        gl_debug();
                        glTextureParameteri(result.frames[i], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                        gl_debug();
                        glTextureStorage2D(result.frames[i], 1, GL_RGBA8, width, height);
                        gl_debug();
                        glTextureSubImage2D(result.frames[i], 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, &pixels[width * height * i * 4]);
                        gl_debug();
                        glGenerateTextureMipmap(result.frames[i]);
                        gl_debug();
                    }
                    else {
                        for (int j = i; j > 0; --j) {
                            glDeleteTextures(1, &result.frames[j - 1]);
                            gl_debug();
                        }

                        free(result.frames);
                        free(result.delays);

                        result.frames = NULL;
                        result.delays = NULL;
                        result.frames_count = 0;

                        break;
                    }
                }
            }

            stbi_image_free(pixels);
        }

        file_free(&file);
    }

    return result;
}

void animated_texture_destroy(animated_texture_t* self) {
    if (self->frames) {
        for (GLuint i = 0; i < self->frames_count; ++i) {
            glDeleteTextures(1, &self->frames[i]);
            gl_debug();
        }

        free(self->frames);
        self->frames = NULL;
    }

    if (self->delays) {
        free(self->delays);
        self->delays = NULL;
    }

    self->frames_count = 0;
    self->current_frame = 0;
    self->current_time = 0.0;
}

void animated_texture_update(animated_texture_t* self, double time) {
    if ((time - self->current_time) * 1000.0 >= self->delays[self->current_frame]) {
        self->current_frame = (self->current_frame + 1) % self->frames_count;
        self->current_time = time;
    }
}


shader_t shader_create(const char* file_name, shader_type type) {
    shader_t result = {
        .id = 0
    };
    file_t file = file_load(file_name, FILE_TYPE_TEXT);

    if (file.data) {
        result.id = glCreateShader(
            type == SHADER_TYPE_VERTEX ? GL_VERTEX_SHADER :
            type == SHADER_TYPE_GEOMETRY ? GL_GEOMETRY_SHADER :
            GL_FRAGMENT_SHADER
        );
        gl_debug();

        if (result.id) {
            glShaderSource(result.id, 1, (const GLchar* const*)&file.data, NULL);
            gl_debug();
            glCompileShader(result.id);
            gl_debug();
        }
        else {
            puts("glCreateShader error");
        }

        file_free(&file);
    }

    return result;
}

void shader_destroy(shader_t* self) {
    glDeleteShader(self->id);
    gl_debug();

    self->id = 0;
}

bool shader_check(const shader_t* self, shader_type type) {
    GLint success = 0;
    GLchar info_log[1024];

    glGetShaderiv(self->id, GL_COMPILE_STATUS, &success);
    gl_debug();

    if (!success) {
        glGetShaderInfoLog(self->id, sizeof(info_log), NULL, info_log);
        gl_debug();

        printf(
            "[SHADER_COMPILATION_ERROR]: %s\n",
            type == SHADER_TYPE_VERTEX ? "vertex shader" :
            type == SHADER_TYPE_GEOMETRY ? "geometry shader" :
            type == SHADER_TYPE_FRAGMENT ? "fragment shader" :
            "Unknown shader"
        );
        printf("Info log: %s\n", info_log);

        return false;
    }

    return true;
}


program_t program_create(const shader_t* vertex_shader, const shader_t* geometry_shader, const shader_t* fragment_shader) {
    program_t result = {
        .id = 0
    };

    result.id = glCreateProgram();
    gl_debug();

    if (result.id) {
        if (vertex_shader) {
            glAttachShader(result.id, vertex_shader->id);
            gl_debug();
        }

        if (geometry_shader) {
            glAttachShader(result.id, geometry_shader->id);
            gl_debug();
        }

        if (fragment_shader) {
            glAttachShader(result.id, fragment_shader->id);
            gl_debug();
        }

        glLinkProgram(result.id);
        gl_debug();
    }

    return result;
}

void program_destroy(program_t* self) {
    glDeleteProgram(self->id);
    gl_debug();

    self->id = 0;
}

bool program_check(const program_t* self) {
    GLint success = 0;
    GLchar info_log[1024];

    glGetProgramiv(self->id, GL_LINK_STATUS, &success);
    gl_debug();

    if (!success) {
        glGetProgramInfoLog(self->id, sizeof(info_log), NULL, info_log);
        gl_debug();

        printf("Error GL_LINK_STATUS: program shader %u\n", self->id);
        printf("Info log: %s\n", info_log);

        return false;
    }

    return true;
}

void program_use(const program_t* self, const camera_t* camera, mat4 matrix) {
    glUseProgram(self->id);
    gl_debug();
    glUniformMatrix4fv(glGetUniformLocation(self->id, "projection"), 1, GL_FALSE, &camera->projection[0][0]);
    gl_debug();
    glUniformMatrix4fv(glGetUniformLocation(self->id, "view"), 1, GL_FALSE, &camera->view[0][0]);
    gl_debug();
    glUniformMatrix4fv(glGetUniformLocation(self->id, "model"), 1, GL_FALSE, &matrix[0][0]);
    gl_debug();
}

void program_unuse() {
    glUseProgram(0);
    gl_debug();
}


GLuint _mesh_create_(GLuint vertices_count, const GLfloat* positions, const GLfloat* normals, const GLfloat* texture_coords, const GLfloat* colors, const GLfloat* tangents, const GLfloat* bitangents, GLsizei indices_count, const GLuint* indices) {
    GLuint result = 0;
    GLuint vbo_positions = 0;
    GLuint vbo_normals = 0;
    GLuint vbo_tex_coords = 0;
    GLuint vbo_colors = 0;
    GLuint vbo_tangents = 0;
    GLuint vbo_bitangents = 0;
    GLuint ebo = 0;

    glCreateVertexArrays(1, &result);
    gl_debug();
    glBindVertexArray(result);
    gl_debug();

    if (positions) {
        glGenBuffers(1, &vbo_positions);
        gl_debug();
        glBindBuffer(GL_ARRAY_BUFFER, vbo_positions);
        gl_debug();
        glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)((int)sizeof(GLfloat) * vertices_count * 3), positions, GL_STATIC_DRAW);
        gl_debug();
        glEnableVertexAttribArray(0);
        gl_debug();
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        gl_debug();
    }
    if (normals) {
        glGenBuffers(1, &vbo_normals);
        gl_debug();
        glBindBuffer(GL_ARRAY_BUFFER, vbo_normals);
        gl_debug();
        glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)((int)sizeof(GLfloat) * vertices_count * 3), normals, GL_STATIC_DRAW);
        gl_debug();
        glEnableVertexAttribArray(1);
        gl_debug();
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        gl_debug();
    }
    if (texture_coords) {
        glGenBuffers(1, &vbo_tex_coords);
        gl_debug();
        glBindBuffer(GL_ARRAY_BUFFER, vbo_tex_coords);
        gl_debug();
        glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)((int)sizeof(GLfloat) * vertices_count * 2), texture_coords, GL_STATIC_DRAW);
        gl_debug();
        glEnableVertexAttribArray(2);
        gl_debug();
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, NULL);
        gl_debug();
    }
    if (colors) {
        glGenBuffers(1, &vbo_colors);
        gl_debug();
        glBindBuffer(GL_ARRAY_BUFFER, vbo_colors);
        gl_debug();
        glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)((int)sizeof(GLfloat) * vertices_count * 4), colors, GL_STATIC_DRAW);
        gl_debug();
        glEnableVertexAttribArray(3);
        gl_debug();
        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 0, NULL);
        gl_debug();
    }
    if (tangents) {
        glGenBuffers(1, &vbo_tangents);
        gl_debug();
        glBindBuffer(GL_ARRAY_BUFFER, vbo_tangents);
        gl_debug();
        glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)((int)sizeof(GLfloat) * vertices_count * 3), tangents, GL_STATIC_DRAW);
        gl_debug();
        glEnableVertexAttribArray(4);
        gl_debug();
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        gl_debug();
    }
    if (bitangents) {
        glGenBuffers(1, &vbo_bitangents);
        gl_debug();
        glBindBuffer(GL_ARRAY_BUFFER, vbo_bitangents);
        gl_debug();
        glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)((int)sizeof(GLfloat) * vertices_count * 3), bitangents, GL_STATIC_DRAW);
        gl_debug();
        glEnableVertexAttribArray(5);
        gl_debug();
        glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        gl_debug();
    }
    if (indices) {
        glCreateBuffers(1, &ebo);
        gl_debug();
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        gl_debug();
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)((int)sizeof(int) * indices_count), indices, GL_STATIC_DRAW);
        gl_debug();
    }

    glBindVertexArray(0);
    gl_debug();

    if (indices) {
        glDeleteBuffers(1, &ebo);
        gl_debug();
    }
    if (bitangents) {
        glDeleteBuffers(1, &vbo_bitangents);
        gl_debug();
    }
    if (tangents) {
        glDeleteBuffers(1, &vbo_tangents);
        gl_debug();
    }
    if (colors) {
        glDeleteBuffers(1, &vbo_colors);
        gl_debug();
    }
    if (texture_coords) {
        glDeleteBuffers(1, &vbo_tex_coords);
        gl_debug();
    }
    if (normals) {
        glDeleteBuffers(1, &vbo_normals);
        gl_debug();
    }
    if (positions) {
        glDeleteBuffers(1, &vbo_positions);
        gl_debug();
    }

    return result;
}

void mesh_destroy(mesh_t* self) {
    glDeleteVertexArrays(1, &self->id);
    gl_debug();

    self->id = 0;
    self->indices_count = 0;
}

mesh_t mesh_create(const char* file_name) {
    #define load_accessor(type, nbcomp, acc, dst) { \
        cgltf_size n = 0; \
        type* buf = (type*)acc->buffer_view->buffer->data + acc->buffer_view->offset / sizeof(type) + acc->offset / sizeof(type); \
        for (cgltf_size k = 0; k < acc->count; ++k) { \
            for (size_t l = 0; l < nbcomp; ++l) { \
                dst[nbcomp * k + l] = buf[n + l]; \
            } \
            n += (cgltf_size)(acc->stride / sizeof(type)); \
        } \
    }

    mesh_t result = {
        .id = 0,
        .indices_count = 0
    };
    cgltf_options options = {
        .type = cgltf_file_type_invalid,
        .json_token_count = 0,
        .memory = {
            .alloc = NULL,
            .free = NULL,
            .user_data = NULL
        },
        .file = {
            .read = NULL,
            .release = NULL,
            .user_data = NULL
        }
    };
    cgltf_data* data = NULL;

    if (file_name) {
        if (cgltf_parse_file(&options, file_name, &data) == cgltf_result_success) {
            if (cgltf_load_buffers(&options, data, file_name) == cgltf_result_success) {


                cgltf_size primitives_count = 0;

                for (cgltf_size i = 0; i < data->meshes_count; ++i) {
                    for (cgltf_size j = 0; j < data->meshes[i].primitives_count; ++j) {
                        ++primitives_count;
                    }
                }

                float* positions = NULL;
                float* normals = NULL;
                float* tangents = NULL;
                float* texcoords = NULL;
                float* colors = NULL;
                float* joints = NULL;
                float* weights = NULL;
                uint32_t* indices = NULL;
                cgltf_size vertices_count = 0;
                cgltf_size indices_count = 0;

                for (cgltf_size i = 0; i < data->meshes_count; ++i) {
                    for (cgltf_size p = 0; p < data->meshes[i].primitives_count; ++p) {
                        for (cgltf_size j = 0; j < data->meshes[i].primitives[p].attributes_count; ++j) {
                            switch (data->meshes[i].primitives[p].attributes[j].type) {
                                case cgltf_attribute_type_invalid: {
                                } break;
                                case cgltf_attribute_type_position: {
                                    cgltf_accessor* acc = data->meshes[i].primitives[p].attributes[j].data;

                                    positions = (float*)calloc(acc->count * 3, sizeof(float));
                                    vertices_count = acc->count;

                                    if (positions) {
                                        load_accessor(float, 3, acc, positions)
                                    }
                                } break;
                                case cgltf_attribute_type_normal: {
                                } break;
                                case cgltf_attribute_type_tangent: {
                                } break;
                                case cgltf_attribute_type_texcoord: {
                                    cgltf_accessor* acc = data->meshes[i].primitives[p].attributes[j].data;

                                    switch (acc->component_type) {
                                        case cgltf_component_type_invalid: {
                                        } break;
                                        case cgltf_component_type_r_8: {
                                        } break;
                                        case cgltf_component_type_r_8u: {
                                        } break;
                                        case cgltf_component_type_r_16: {
                                        } break;
                                        case cgltf_component_type_r_16u: {
                                        } break;
                                        case cgltf_component_type_r_32u: {
                                        } break;
                                        case cgltf_component_type_r_32f: {
                                            texcoords = (float*)calloc(acc->count * 2, sizeof(float));

                                            if (texcoords) {
                                                load_accessor(float, 2, acc, texcoords)
                                            }
                                        } break;
                                        default: {
                                        } break;
                                    }
                                } break;
                                case cgltf_attribute_type_color: {
                                    cgltf_accessor* acc = data->meshes[i].primitives[p].attributes[j].data;

                                    switch (acc->component_type) {
                                        case cgltf_component_type_invalid: {
                                        } break;
                                        case cgltf_component_type_r_8: {
                                        } break;
                                        case cgltf_component_type_r_8u: {
                                        } break;
                                        case cgltf_component_type_r_16: {
                                        } break;
                                        case cgltf_component_type_r_16u: {
                                            colors = (float*)calloc(acc->count * 4, sizeof(float));

                                            if (colors) {
                                                uint16_t _colors_[acc->count * 4 * sizeof(uint16_t)];
                                                uint32_t _col_count = array_size(_colors_) / sizeof(uint16_t);

                                                load_accessor(uint16_t, 4, acc, _colors_)

                                                for (int x = 0; x < _col_count; ++x) {
                                                    colors[x] = _colors_[x] == 0 ? 0.0f : 256.0f / (_colors_[x] / 256.0f);
                                                }
                                            }
                                        } break;
                                        case cgltf_component_type_r_32u: {
                                        } break;
                                        case cgltf_component_type_r_32f: {
                                        } break;
                                        default: {
                                        } break;
                                    }
                                } break;
                                case cgltf_attribute_type_joints: {
                                } break;
                                case cgltf_attribute_type_weights: {
                                } break;
                                default: {
                                } break;
                            }
                        }
                        if (data->meshes[i].primitives[p].indices->count) {
                            cgltf_accessor* acc = data->meshes[i].primitives[p].indices;

                            indices_count = data->meshes[i].primitives[p].indices->count;

                            switch (acc->component_type) {
                                case cgltf_component_type_invalid: {
                                } break;
                                case cgltf_component_type_r_8: {
                                } break;
                                case cgltf_component_type_r_8u: {
                                } break;
                                case cgltf_component_type_r_16: {
                                } break;
                                case cgltf_component_type_r_16u: {
                                    indices = (uint32_t*)calloc(vertices_count, sizeof(uint32_t));

                                    if (indices) {
                                        load_accessor(uint16_t, 1, acc, indices)
                                    }
                                } break;
                                case cgltf_component_type_r_32u: {
                                } break;
                                case cgltf_component_type_r_32f: {
                                } break;
                                default: {
                                } break;
                            }
                        }
                    }

                    if (result.id) {
                        mesh_destroy(&result);
                    }

                    result.id = _mesh_create_((GLuint)vertices_count, positions, normals, texcoords, colors, tangents, NULL, (GLsizei)indices_count, (const GLuint*)indices);
                    result.indices_count = (GLsizei)indices_count;

                    if (positions) {
                        free(positions);
                        positions = NULL;
                    }
                    if (normals) {
                        free(normals);
                        normals = NULL;
                    }
                    if (tangents) {
                        free(tangents);
                        tangents = NULL;
                    }
                    if (texcoords) {
                        free(texcoords);
                        texcoords = NULL;
                    }
                    if (colors) {
                        free(colors);
                        colors = NULL;
                    }
                    if (joints) {
                        free(joints);
                        joints = NULL;
                    }
                    if (weights) {
                        free(weights);
                        weights = NULL;
                    }

                    vertices_count = 0;
                    indices_count = 0;
                }


            }
            else {
                puts("Failed to cgltf_load_buffers()");
            }

            cgltf_free(data);
        }
        else {
            puts("Failed to cgltf_parse_file()");
        }
    }
    else {
        printf("Failed to %s\n", file_name);
    }

    return result;
}

void mesh_draw(const mesh_t* self) {
    glBindVertexArray(self->id);
    gl_debug();
    glDrawElements(GL_TRIANGLES, self->indices_count, GL_UNSIGNED_INT, NULL);
    gl_debug();
    glBindVertexArray(0);
    gl_debug();
}


object_t object_default() {
    object_t result = {
        .position = GLM_VEC3_ZERO_INIT,
        .rotation = GLM_VEC3_ZERO_INIT,
        .scale = GLM_VEC3_ONE_INIT,
        .matrix = GLM_MAT4_IDENTITY_INIT,
        .program.id = 0,
        .mesh = {
            .id = 0,
            .indices_count = 0
        },
        .textures = NULL,
        .textures_count = 0
    };

    return result;
}

object_t object_create(
    const char* vertex_shader_file_name,
    const char* geometry_shader_file_name,
    const char* fragment_shader_file_name,
    const char* mesh_file_name,
    const char** texture_file_names,
    int textures_count
) {
    object_t result = object_default();
    texture_t textures[textures_count];

    shader_t vertex_shader = { .id = 0 };
    shader_t geometry_shader = { .id = 0 };
    shader_t fragment_shader = { .id = 0 };

    if (vertex_shader_file_name) {
        vertex_shader = shader_create(vertex_shader_file_name, SHADER_TYPE_VERTEX);

        if (!shader_check(&vertex_shader, SHADER_TYPE_VERTEX)) {
            return object_default();
        }
    }
    if (geometry_shader_file_name) {
        geometry_shader = shader_create(geometry_shader_file_name, SHADER_TYPE_GEOMETRY);

        if (!shader_check(&geometry_shader, SHADER_TYPE_GEOMETRY)) {
            if (vertex_shader_file_name) {
                shader_destroy(&vertex_shader);
            }

            return object_default();
        }
    }
    if (fragment_shader_file_name) {
        fragment_shader = shader_create(fragment_shader_file_name, SHADER_TYPE_FRAGMENT);

        if (!shader_check(&fragment_shader, SHADER_TYPE_FRAGMENT)) {
            if (vertex_shader_file_name) {
                shader_destroy(&vertex_shader);
            }

            if (geometry_shader_file_name) {
                shader_destroy(&geometry_shader);
            }

            return object_default();
        }
    }

    result.program = program_create(
        vertex_shader.id && shader_check(&vertex_shader, SHADER_TYPE_VERTEX) ? &vertex_shader : NULL,
        geometry_shader.id && shader_check(&vertex_shader, SHADER_TYPE_GEOMETRY) ? &geometry_shader : NULL,
        fragment_shader.id && shader_check(&vertex_shader, SHADER_TYPE_FRAGMENT) ? &fragment_shader : NULL
    );

    if (fragment_shader.id) {
        shader_destroy(&fragment_shader);
    }
    if (geometry_shader.id) {
        shader_destroy(&geometry_shader);
    }
    if (vertex_shader.id) {
        shader_destroy(&vertex_shader);
    }

    if (!program_check(&result.program)) {
        return object_default();
    }

    for (int i = 0; i < textures_count; ++i) {
        textures[i] = texture_create(texture_file_names[i]);

        if (!textures[i].id) {
            for (int j = 0; j < i; ++j) {
                texture_destroy(&textures[j]);
            }

            printf("Error load:\n    %s\n", texture_file_names[i]);

            return object_default();
        }
    }

    if (!mesh_file_name) {
        GLfloat positions[] = {
            -1.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 0.0f,
            1.0f, -1.0f, 0.0f,
            -1.0f, -1.0f, 0.0f

            // -1.0f, 1.0f, -1.0f,
            // 1.0f, 1.0f, -1.0f,
            // 1.0f, 1.0f, 0.0f,
            // -1.0f, 1.0f, 0.0f
        };

        GLfloat texture_coords[] = {
            0.0f, 0.0f,
            1.0f, 0.0f,
            1.0f, 1.0f,
            0.0f, 1.0f

            // 0.0f, 0.0f,
            // 1.0f, 0.0f,
            // 1.0f, 1.0f,
            // 0.0f, 1.0f
        };

        GLfloat colors[] = {
            1.0f, 1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f, 1.0f

            // 1.0f, 1.0f, 1.0f, 1.0f,
            // 1.0f, 1.0f, 1.0f, 1.0f,
            // 1.0f, 1.0f, 1.0f, 1.0f,
            // 1.0f, 1.0f, 1.0f, 1.0f
        };

        GLuint indices[] = {
            0, 1, 2,
            2, 3, 0
            // 4, 5, 6,
            // 6, 7, 4
        };

        GLuint vertices_count = array_size(positions) / 3;

        result.mesh.indices_count = array_size(indices);
        result.mesh.id = _mesh_create_(vertices_count, positions, NULL, texture_coords, colors, NULL, NULL, result.mesh.indices_count, indices);

        if (!result.mesh.id) {
            for (int i = 0; i < textures_count; ++i) {
                texture_destroy(&textures[i]);
            }

            mesh_destroy(&result.mesh);
            program_destroy(&result.program);

            return object_default();
        }
    }

    result.textures = (texture_t*)calloc((size_t)textures_count, sizeof(texture_t));

    if (result.textures) {
        for (int i = 0; i < textures_count; ++i) {
            result.textures[i] = textures[i];
        }
    }

    result.textures_count = textures_count;

    return result;
}

void object_destroy(object_t* self) {
    for (int i = 0; i < self->textures_count; ++i) {
        texture_destroy(&self->textures[i]);
    }

    self->textures_count = 0;

    if (self->mesh.id) {
        mesh_destroy(&self->mesh);
    }

    if (self->program.id) {
        program_destroy(&self->program);
    }

    *self = object_default();
}

void object_draw(object_t* self, camera_t* camera) {
    for (int i = 0; i < self->textures_count; ++i) {
        glActiveTexture(GL_TEXTURE0 + (GLenum)i);
        gl_debug();
        texture_bind(&self->textures[i]);
    }

    program_use(&self->program, camera, self->matrix);

    glUniform1i(glGetUniformLocation(self->program.id, "texture_diffuse1"), 0);
    gl_debug();
    glUniform1i(glGetUniformLocation(self->program.id, "texture_diffuse2"), 1);
    gl_debug();

    mesh_draw(&self->mesh);
}


camera_t camera_default() {
    camera_t result;

    glm_mat4_identity(result.projection);
    glm_mat4_identity(result.view);
    glm_vec3_zero(result.position);
    glm_vec3_zero(result.rotation);
    glm_vec3_zero(result.direction);
    glm_vec3_zero(result.center);

    return result;
}

void camera_switch_2d(camera_t* self) {
    glm_ortho(-1.0f, 1.0f, -1.0f, 1.0f, FLT_MIN, FLT_MAX, self->projection);
}

void camera_switch_3d(camera_t* self) {
    glm_perspective(45.0f, 1.0f, FLT_MIN, FLT_MAX, self->projection);
}

camera_t camera_initialize_2d() {
    camera_t result = camera_default();
    camera_switch_2d(&result);
    result.direction[2] = -1.0f;
    return result;
}

camera_t camera_initialize_3d() {
    camera_t result = camera_default();
    camera_switch_3d(&result);
    result.direction[2] = -1.0f;
    return result;
}

void camera_update(camera_t* self, GLFWwindow* window) {
    static vec3 up = { 0.0f, 1.0f, 0.0f };
    float speed = 0.01f;

    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT)) {
        speed *= 50.0f;
    }


    if (glfwGetKey(window, GLFW_KEY_LEFT)) {
        self->rotation[0] -= 1.0f * speed;

        if (self->rotation[0] < 0.0f) {
            self->rotation[0] += 360.0f;
        }

        self->direction[0] = sinf(glm_rad(self->rotation[0]));
        self->direction[2] = -cosf(glm_rad(self->rotation[0]));
    }

    if (glfwGetKey(window, GLFW_KEY_RIGHT)) {
        self->rotation[0] += 1.0f * speed;

        if (self->rotation[0] >= 360.0f) {
            self->rotation[0] -= 360.0f;
        }

        self->direction[0] = sinf(glm_rad(self->rotation[0]));
        self->direction[2] = -cosf(glm_rad(self->rotation[0]));
    }

    if (glfwGetKey(window, GLFW_KEY_UP)) {
        self->rotation[1] += 1.0f * speed;

        if (self->rotation[1] >= 89.0f) {
            self->rotation[1] = 89.0f;
        }

        self->direction[1] = tanf(glm_rad(self->rotation[1]));
    }

    if (glfwGetKey(window, GLFW_KEY_DOWN)) {
        self->rotation[1] -= 1.0f * speed;

        if (self->rotation[1] < -89.0f) {
            self->rotation[1] = -89.0f;
        }

        self->direction[1] = tanf(glm_rad(self->rotation[1]));
    }


    if (glfwGetKey(window, GLFW_KEY_A)) {
        vec3 buffer = { 0.0f, 0.0f, 0.0f };

        glm_vec3_cross(self->direction, up, buffer);

        self->position[0] -= glm_rad(buffer[0]) * speed;
        self->position[1] -= glm_rad(buffer[1]) * speed;
        self->position[2] -= glm_rad(buffer[2]) * speed;
    }

    if (glfwGetKey(window, GLFW_KEY_D)) {
        vec3 buffer = { 0.0f, 0.0f, 0.0f };

        glm_vec3_cross(self->direction, up, buffer);

        self->position[0] += glm_rad(buffer[0]) * speed;
        self->position[1] += glm_rad(buffer[1]) * speed;
        self->position[2] += glm_rad(buffer[2]) * speed;
    }

    if (glfwGetKey(window, GLFW_KEY_Q)) {
        self->position[1] -= glm_rad(1.0f) * speed;
    }

    if (glfwGetKey(window, GLFW_KEY_E)) {
        self->position[1] += glm_rad(1.0f) * speed;
    }

    if (glfwGetKey(window, GLFW_KEY_W)) {
        self->position[0] += glm_rad(self->direction[0]) * speed;
        self->position[2] += glm_rad(self->direction[2]) * speed;
    }

    if (glfwGetKey(window, GLFW_KEY_S)) {
        self->position[0] -= glm_rad(self->direction[0]) * speed;
        self->position[2] -= glm_rad(self->direction[2]) * speed;
    }


    if (glfwGetKey(window, GLFW_KEY_F)) {
        glm_vec3_zero(self->position);
        glm_vec3_zero(self->rotation);
        glm_vec3_zero(self->direction);
        glm_vec3_zero(self->center);

        self->direction[2] = -1.0f;
    }


    if (glfwGetKey(window, GLFW_KEY_2)) {
        camera_switch_2d(self);
    }

    if (glfwGetKey(window, GLFW_KEY_3)) {
        camera_switch_3d(self);
    }

    self->center[0] = self->position[0] + self->direction[0];
    self->center[1] = self->position[1] + self->direction[1];
    self->center[2] = self->position[2] + self->direction[2];

    glm_lookat(self->position, self->center, up, self->view);
}


void glfw_error_callback(int error, const char* description) {
    puts("GLFW Error:");
    printf("    Error: %i\n", error);
    printf("    Description: %s\n", description);
}

void glfw_monitor_callback(GLFWmonitor* monitor, int event) {
}

void glfw_window_pos_callback(GLFWwindow* window, int xpos, int ypos) {
}

void glfw_window_size_callback(GLFWwindow* window, int width, int height) {
}

void glfw_window_close_callback(GLFWwindow* window) {
}

void glfw_window_refresh_callback(GLFWwindow* window) {
}

void glfw_window_focus_callback(GLFWwindow* window, int focused) {
}

void glfw_window_iconify_callback(GLFWwindow* window, int iconified) {
}

void glfw_window_maximize_callback(GLFWwindow* window, int maximized) {
}

void glfw_framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    gl_debug();
}

void glfw_window_content_scale_callback(GLFWwindow* window, float xscale, float yscale) {
}

void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    // printf("key: %i\n", key);
    // printf("scancode: %i\n", scancode);
    // printf("action: %i\n", action);
    // printf("mods: %i\n", mods);

    // if (key == 259 && (action == 1 || action == 2))
    //     if (size > 0)
    //         text[--size] = '\0';

    // if (key == 257 && action == 1 && size) {
    //     directory_free(&directory);
    //     directory_get(&directory, (const char*)text);
    // }
}

void glfw_char_callback(GLFWwindow* window, unsigned int codepoint) {
}

void glfw_char_mods_callback(GLFWwindow* window, unsigned int codepoint, int mods) {
}

void glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
}

void glfw_cursor_pos_callback(GLFWwindow* window, double xpos, double ypos) {
    // int width = 0;
    // int height = 0;

    // glfwGetWindowSize(window, &width, &height);

    // if ((int)xpos != width / 2) {
    //     CAMERA.rotation[0] += (float)(xpos > width / 2 ? xpos * 0.01 : -xpos * 0.01);

    //     if (CAMERA.rotation[0] < 0.0f)
    //         CAMERA.rotation[0] += 360.0f;

    //     if (CAMERA.rotation[0] >= 360.0f)
    //         CAMERA.rotation[0] -= 360.0f;

    //     CAMERA.direction[0] = sinf(glm_rad(CAMERA.rotation[0]));
    //     CAMERA.direction[2] = -cosf(glm_rad(CAMERA.rotation[0]));
    // }

    // if ((int)ypos != height / 2) {
    //     CAMERA.rotation[1] -= (float)(ypos > height / 2 ? ypos * 0.01 : -ypos * 0.01);

    //     if (CAMERA.rotation[1] >= 89.0f)
    //         CAMERA.rotation[1] = 89.0f;

    //     if (CAMERA.rotation[1] < -89.0f)
    //         CAMERA.rotation[1] = -89.0f;

    //     CAMERA.direction[1] = tanf(glm_rad(CAMERA.rotation[1]));
    // }

    // glfwSetCursorPos(window, (double)width / 2.0, (double)height / 2.0);
}

void glfw_cursor_enter_callback(GLFWwindow* window, int entered) {
}

void glfw_scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
}

void glfw_drop_callback(GLFWwindow* window, int count, const char** names) {
}

void glfw_joystick_callback(int jid, int event) {
}


void GLAPIENTRY gl_debug_message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* user_param) {
    // if (id == 131169 || id == 131185 || id == 131218 || id == 131204) {
    //     return;
    // }

    puts("OpenGL Debug Message:");
    printf("    Source[%i]: ", source);

    switch (source) {
        case GL_DEBUG_SOURCE_API: puts("API"); break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM: puts("Window System"); break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: puts("GLuint Compiler"); break;
        case GL_DEBUG_SOURCE_THIRD_PARTY: puts("Third Party"); break;
        case GL_DEBUG_SOURCE_APPLICATION: puts("Application"); break;
        case GL_DEBUG_SOURCE_OTHER: puts("Other"); break;
        default: puts("Unknown"); break;
    }

    printf("    Type[%i]: ", type);

    switch (type) {
        case GL_DEBUG_TYPE_ERROR: puts("Error"); break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: puts("Deprecated Behaviour"); break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: puts("Undefined Behaviour"); break;
        case GL_DEBUG_TYPE_PORTABILITY: puts("Portability"); break;
        case GL_DEBUG_TYPE_PERFORMANCE: puts("Performance"); break;
        case GL_DEBUG_TYPE_MARKER: puts("Marker"); break;
        case GL_DEBUG_TYPE_PUSH_GROUP: puts("Push Group"); break;
        case GL_DEBUG_TYPE_POP_GROUP: puts("Pop Group"); break;
        case GL_DEBUG_TYPE_OTHER: puts("Other"); break;
        default: puts("Unknown"); break;
    }

    printf("    ID: %u\n", id);
    printf("    Severity[%i]: ", severity);

    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH: puts("High"); break;
        case GL_DEBUG_SEVERITY_MEDIUM: puts("Medium"); break;
        case GL_DEBUG_SEVERITY_LOW: puts("Low"); break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: puts("Notification"); break;
        default: puts("Unknown"); break;
    }

    printf("    Length: %i\n", length);
    printf("    Message: %s\n", message);
}

bool gl_load() {
    if (gladLoadGL(glfwGetProcAddress)) {
        glEnable(GL_DEBUG_OUTPUT);
        gl_debug();
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        gl_debug();
        glDebugMessageCallback(gl_debug_message_callback, NULL);
        gl_debug();
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
        gl_debug();

        glEnable(GL_DEPTH_TEST);
        gl_debug();
        glEnable(GL_BLEND);
        gl_debug();
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        gl_debug();

        return true;
    }

    return false;
}


GLFWwindow* window_create_opengl() {
    GLFWwindow* result = NULL;

    if (glfwInit()) {
        const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

        glfwSetErrorCallback(glfw_error_callback);
        // glfwWindowHint(GLFW_SAMPLES, 16);
        // glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
        // glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
        glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);

        result = glfwCreateWindow(mode->width / 2, mode->height / 2, "Project", NULL, NULL);

        if (result) {
            glfwMakeContextCurrent(result);

            glfwSetWindowPos(result, mode->width / 4, mode->height / 4);

            glfwSetMonitorCallback(glfw_monitor_callback);
            glfwSetWindowPosCallback(result, glfw_window_pos_callback);
            glfwSetWindowSizeCallback(result, glfw_window_size_callback);
            glfwSetWindowCloseCallback(result, glfw_window_close_callback);
            glfwSetWindowRefreshCallback(result, glfw_window_refresh_callback);
            glfwSetWindowFocusCallback(result, glfw_window_focus_callback);
            glfwSetWindowIconifyCallback(result, glfw_window_iconify_callback);
            glfwSetWindowMaximizeCallback(result, glfw_window_maximize_callback);
            glfwSetFramebufferSizeCallback(result, glfw_framebuffer_size_callback);
            glfwSetWindowContentScaleCallback(result, glfw_window_content_scale_callback);
            glfwSetKeyCallback(result, glfw_key_callback);
            glfwSetCharCallback(result, glfw_char_callback);
            glfwSetCharModsCallback(result, glfw_char_mods_callback);
            glfwSetMouseButtonCallback(result, glfw_mouse_button_callback);
            glfwSetCursorPosCallback(result, glfw_cursor_pos_callback);
            glfwSetCursorEnterCallback(result, glfw_cursor_enter_callback);
            glfwSetScrollCallback(result, glfw_scroll_callback);
            glfwSetDropCallback(result, glfw_drop_callback);
            glfwSetJoystickCallback(glfw_joystick_callback);

            // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

            if (gl_load()) {
                return result;
            }

            glfwDestroyWindow(result);
            result = NULL;
        }

        glfwTerminate();
    }

    return result;
}


#include <AL/al.h>
#include <AL/alc.h>

#include <stb/stb_vorbis.c>

#define DR_FLAC_IMPLEMENTATION
#include <dr/dr_flac.h>

#define DR_WAV_IMPLEMENTATION
#include <dr/dr_wav.h>

#define DR_MP3_IMPLEMENTATION
#include <dr/dr_mp3.h>


typedef struct audio_device_t {
    ALCdevice* device;
    ALCcontext* context;
} audio_device_t;

typedef struct audio_buffer_t {
    ALuint id;
} audio_buffer_t;

typedef struct audio_source_t {
    ALuint id;
} audio_source_t;


ALCenum alc_debug(ALCdevice* device) {
    ALCenum result = alcGetError(device);

    if (result != ALC_NO_ERROR) {
        printf("ALC error: %s\n", alcGetString(device, result));
    }

    return result;
}

ALenum al_debug() {
    ALenum result = alGetError();

    if (result != AL_NO_ERROR) {
        printf("AL error: %s\n", alGetString(result));
    }

    return result;
}


audio_device_t audio_device_create() {
    audio_device_t result = {
        .device = NULL,
        .context = NULL
    };

    result.device = alcOpenDevice(NULL);

    if (result.device) {
        result.context = alcCreateContext(result.device, NULL);

        if (result.context) {
            if (alcMakeContextCurrent(result.context)) {
                return result;
            }
            else {
                puts("Failed to alcMakeContextCurrent()");
            }

            alcDestroyContext(result.context);

            if (alc_debug(result.device) == ALC_NO_ERROR) {
                result.context = NULL;
            }
            else {
                puts("Failed to alcDestroyContext()");
            }
        }
        else {
            puts("Failed to alcCreateContext()");
        }

        if (alcCloseDevice(result.device)) {
            result.device = NULL;
        }
        else {
            puts("Failed to alcCloseDevice()");
        }
    }
    else {
        puts("Failed to alcOpenDevice()");
    }

    return result;
}

void audio_device_destroy(audio_device_t* self) {
    if (alcMakeContextCurrent(NULL) == ALC_TRUE) {
        if (self->context) {
            alcDestroyContext(self->context);

            if (alc_debug(self->device) == ALC_NO_ERROR) {
                self->context = NULL;
            }
            else {
                puts("Failed to alcDestroyContext()");
            }
        }

        if (self->device) {
            if (alcCloseDevice(self->device) == ALC_TRUE) {
                self->device = NULL;
            }
            else {
                if (alc_debug(self->device) != ALC_NO_ERROR) {
                    // Error...
                }

                puts("Failed to alcCloseDevice()");
            }
        }
    }
    else {
        puts("Failed to alcMakeContextCurrent()");
    }
}


ALenum audio_get_format_from_channel_count(unsigned int channel_count) {
    ALenum format = 0;

    switch (channel_count) {
        case 1: format = AL_FORMAT_MONO16; break;
        case 2: format = AL_FORMAT_STEREO16; break;
        case 4: format = alGetEnumValue("AL_FORMAT_QUAD16"); break;
        case 6: format = alGetEnumValue("AL_FORMAT_51CHN16"); break;
        case 7: format = alGetEnumValue("AL_FORMAT_61CHN16"); break;
        case 8: format = alGetEnumValue("AL_FORMAT_71CHN16"); break;
        default: format = 0; break;
    }

    // Fixes a bug on OS X
    if (format == -1) {
        format = 0;
    }

    return format;
}


void audio_buffer_destroy(audio_buffer_t* self) {
    if (self && self->id) {
        alDeleteBuffers(1, &self->id);

        if (al_debug() == AL_NO_ERROR) {
            self->id = 0;
        }
    }
}

audio_buffer_t audio_buffer_create(const char* file_name) {
    audio_buffer_t result = {
        .id = 0
    };

    if (file_check_extension(file_name, "ogg")) {
        int size = 0;
        int channels = 0;
        int sample_rate = 0;
        short* data = NULL;

        size = stb_vorbis_decode_filename(file_name, &channels, &sample_rate, &data);

        if (data) {
            alGenBuffers(1, &result.id);

            if (al_debug() == AL_NO_ERROR) {
                alBufferData(result.id, audio_get_format_from_channel_count((unsigned int)channels), data, (ALsizei)((unsigned int)size * (unsigned int)channels * sizeof(int16_t)), (ALsizei)sample_rate);

                if (al_debug() != AL_NO_ERROR) {
                    audio_buffer_destroy(&result);
                }
            }

            free(data);
            data = NULL;
        }
    }
    else if (file_check_extension(file_name, "flac")) {
        drflac* flac_data = drflac_open_file(file_name, NULL);

        if (flac_data) {
            int16_t* data = (int16_t*)calloc((size_t)flac_data->totalPCMFrameCount * flac_data->channels, sizeof(int16_t));

            if (data) {
                drflac_read_pcm_frames_s16(flac_data, flac_data->totalPCMFrameCount, data);

                alGenBuffers(1, &result.id);

                if (al_debug() == AL_NO_ERROR) {
                    alBufferData(result.id, audio_get_format_from_channel_count(flac_data->channels), data, (ALsizei)(flac_data->totalPCMFrameCount * flac_data->channels * sizeof(int16_t)), (ALsizei)flac_data->sampleRate);

                    if (al_debug() != AL_NO_ERROR) {
                        audio_buffer_destroy(&result);
                    }
                }

                free(data);
            }

            drflac_close(flac_data);
        }
    }
    else if (file_check_extension(file_name, "wav")) {
        drwav wav_data;

        if (drwav_init_file(&wav_data, file_name, NULL)) {
            int16_t* data = (int16_t*)calloc((size_t)wav_data.totalPCMFrameCount * wav_data.channels, sizeof(int16_t));
            drwav_read_pcm_frames_s16(&wav_data, wav_data.totalPCMFrameCount, data);

            if (data) {
                alGenBuffers(1, &result.id);

                if (al_debug() == AL_NO_ERROR) {
                    alBufferData(result.id, audio_get_format_from_channel_count(wav_data.channels), data, (ALsizei)(wav_data.totalPCMFrameCount * wav_data.channels * sizeof(int16_t)), (ALsizei)wav_data.sampleRate);

                    if (al_debug() != AL_NO_ERROR) {
                        audio_buffer_destroy(&result);
                    }
                }
            }

            drwav_uninit(&wav_data);
        }
    }
    else if (file_check_extension(file_name, "mp3")) {
        drmp3_config config = {
            .channels = 0,
            .sampleRate = 0
        };

        drmp3_uint64 total_pcm_frame_count = 0;
        drmp3_int16* data = NULL;

        data = drmp3_open_file_and_read_pcm_frames_s16(file_name, &config, &total_pcm_frame_count, NULL);

        if (data) {
            alGenBuffers(1, &result.id);

            if (al_debug() == AL_NO_ERROR) {
                alBufferData(result.id, audio_get_format_from_channel_count(config.channels), data, (ALsizei)(total_pcm_frame_count * config.channels * sizeof(int16_t)), (ALsizei)config.sampleRate);

                if (al_debug() != AL_NO_ERROR) {
                    audio_buffer_destroy(&result);
                }
            }

            free(data);
            data = NULL;
        }
    }

    return result;
}

ALint audio_buffer_get_frequency(const audio_buffer_t* self) {
    if (self && self->id) {
        ALint result = 0;
        alGetBufferi(self->id, AL_FREQUENCY, &result);
        return al_debug() == AL_NO_ERROR ? result : 0;
    }

    return 0;
}

ALint audio_buffer_get_bits(const audio_buffer_t* self) {
    if (self && self->id) {
        ALint result = 0;
        alGetBufferi(self->id, AL_BITS, &result);
        return al_debug() == AL_NO_ERROR ? result : 0;
    }

    return 0;
}

ALint audio_buffer_get_channel_count(const audio_buffer_t* self) {
    if (self && self->id) {
        ALint result = 0;
        alGetBufferi(self->id, AL_CHANNELS, &result);
        return al_debug() == AL_NO_ERROR ? result : 0;
    }

    return 0;
}

ALint audio_buffer_get_size(const audio_buffer_t* self) {
    if (self && self->id) {
        ALint result = 0;
        alGetBufferi(self->id, AL_SIZE, &result);
        return al_debug() == AL_NO_ERROR ? result : 0;
    }

    return 0;
}

ALfloat audio_buffer_get_time(const audio_buffer_t* self) {
    if (self && self->id) {
        ALint frequency = audio_buffer_get_frequency(self);
        ALint bits = audio_buffer_get_bits(self);
        ALint channel_count = audio_buffer_get_channel_count(self);
        ALint size = audio_buffer_get_size(self);

        if (!frequency || !bits || !channel_count || !size) {
            return FLT_MIN;
        }

        return (1.0f * (float)size) / (float)(frequency * channel_count * (bits / 8));
    }

    return FLT_MIN;
}


void audio_source_destroy(audio_source_t* self) {
    if (self && self->id) {
        alSourceStop(self->id);

        if (al_debug() == AL_NO_ERROR) {
            alSourcei(self->id, AL_BUFFER, 0);

            if (al_debug() == AL_NO_ERROR) {
                alDeleteSources(1, &self->id);

                if (al_debug() == AL_NO_ERROR) {
                    self->id = 0;
                }
            }
        }
    }
}

audio_source_t audio_source_create(const audio_buffer_t* buffer) {
    audio_source_t result = {
        .id = 0
    };

    if (buffer && buffer->id) {
        alGenSources(1, &result.id);

        if (al_debug() == AL_NO_ERROR) {
            alSourcei(result.id, AL_BUFFER, (ALint)buffer->id);

            if (al_debug() != AL_NO_ERROR) {
                audio_source_destroy(&result);
            }
        }
    }

    return result;
}

void audio_source_set_position(const audio_source_t* self, ALfloat x, ALfloat y, ALfloat z) {
    if (self && self->id) {
        alSource3f(self->id, AL_POSITION, x, y, z);
        al_debug();
    }
}

void audio_source_set_time(const audio_source_t* self, ALfloat time) {
    if (self && self->id) {
        alSourcef(self->id, AL_SEC_OFFSET, time < FLT_MIN ? FLT_MIN : time > 100.0f ? 100.0f : time);
        al_debug();
    }
}

void audio_source_play(const audio_source_t* self) {
    if (self && self->id) {
        alSourcePlay(self->id);
        al_debug();
    }
}

void audio_source_stop(const audio_source_t* self) {
    if (self && self->id) {
        alSourceStop(self->id);
        al_debug();
    }
}

void audio_source_rewind(const audio_source_t* self) {
    if (self && self->id) {
        alSourceRewind(self->id);
        al_debug();
    }
}

void audio_source_pause(const audio_source_t* self) {
    if (self && self->id) {
        alSourcePause(self->id);
        al_debug();
    }
}


#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif // c_engine_h
