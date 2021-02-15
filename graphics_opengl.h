#ifndef graphics_h
#define graphics_h


#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


#include <cglm/cglm.h>     // Math

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h> // Image loader

#include <cgltf/cgltf.h>   // 2D/3D models, materials, scenes, etc...
#include <glad/gl.h>       // OpenGL loader

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>    // Windows


// Set position in pixels:  1 - offset in pixels;  2 - object size in pixels
// Set scale    in pixels:  1 - scale  in pixels;  2 - window size in pixels
#define to_pixels(image_size, window_size) \
    (float)((1.0f / (float)window_size) * (float)image_size)

#define to_percent(image_size, window_size) \
    (float)((float)image_size / (float)window_size)


typedef struct animated_texture_t {
    GLuint* frames;
    GLuint* delays;
    GLuint frames_count;
    GLuint current_frame;
    double current_time;
} animated_texture_t;

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


GLuint texture_create(const char* file_name) {
    GLuint result = 0;
    int width = 0;
    int height = 0;
    void* pixels = NULL;

    pixels = stbi_load(file_name, &width, &height, NULL, STBI_rgb_alpha);

    if (pixels) {
        glCreateTextures(GL_TEXTURE_2D, 1, &result);
        gl_debug();

        if (result) {
            glTextureParameteri(result, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            gl_debug();
            glTextureParameteri(result, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            gl_debug();
            glTextureParameteri(result, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
            gl_debug();
            glTextureParameteri(result, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            gl_debug();
            glTextureStorage2D(result, 1, GL_RGBA8, (GLsizei)width, (GLsizei)height);
            gl_debug();
            glTextureSubImage2D(result, 0, 0, 0, (GLsizei)width, (GLsizei)height, GL_RGBA, GL_UNSIGNED_BYTE, (const void*)pixels);
            gl_debug();
            glGenerateTextureMipmap(result);
            gl_debug();
        }

        stbi_image_free(pixels);
    }

    return result;
}

void texture_destroy(GLuint* texture) {
    glDeleteTextures(1, texture);
    gl_debug();

    *texture = 0;
}

void texture_bind(const GLuint* texture) {
    glBindTexture(GL_TEXTURE_2D, *texture);
    gl_debug();
}

void texture_unbind() {
    glBindTexture(GL_TEXTURE_2D, 0);
    gl_debug();
}

GLint texture_get_width(const GLuint* texture) {
    GLint result = 0;
    glGetTextureLevelParameteriv(*texture, 0, GL_TEXTURE_WIDTH, &result);
    gl_debug();
    return result;
}

GLint texture_get_height(const GLuint* texture) {
    GLint result = 0;
    glGetTextureLevelParameteriv(*texture, 0, GL_TEXTURE_HEIGHT, &result);
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

    size_t file_size = 0;
    void* file_data = file_load(file_name, true, &file_size);

    if (file_data) {
        pixels = stbi_load_gif_from_memory((const stbi_uc*)file_data, (int)file_size, &delays, &width, &height, &frames_count, NULL, STBI_rgb_alpha);

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

        file_free(&file_data);
    }

    return result;
}

void animated_texture_destroy(animated_texture_t* animated_texture) {
    if (animated_texture->frames) {
        for (GLuint i = 0; i < animated_texture->frames_count; ++i) {
            glDeleteTextures(1, &animated_texture->frames[i]);
            gl_debug();
        }

        free(animated_texture->frames);
        animated_texture->frames = NULL;
    }

    if (animated_texture->delays) {
        free(animated_texture->delays);
        animated_texture->delays = NULL;
    }

    animated_texture->frames_count = 0;
    animated_texture->current_frame = 0;
    animated_texture->current_time = 0.0;
}

void animated_texture_update(animated_texture_t* animated_texture, double time) {
    if ((time - animated_texture->current_time) * 1000.0 >= animated_texture->delays[animated_texture->current_frame]) {
        animated_texture->current_frame = (animated_texture->current_frame + 1) % animated_texture->frames_count;
        animated_texture->current_time = time;
    }
}


GLuint shader_create(const char* file_name, GLenum type) {
    GLuint result = 0;
    size_t file_size = 0;
    void* file_data = file_load(file_name, true, &file_size);

    if (file_data) {
        result = glCreateShader(type);
        gl_debug();

        if (result) {
            glShaderSource(result, 1, (const GLchar* const*)&file_data, NULL);
            gl_debug();
            glCompileShader(result);
            gl_debug();
        }
        else {
            puts("glCreateShader error");
        }

        file_free(&file_data);
    }

    return result;
}

void shader_destroy(GLuint* shader) {
    glDeleteShader(*shader);
    gl_debug();

    *shader = 0;
}

bool shader_check(const GLuint* shader, GLenum type) {
    GLint success = 0;
    GLchar info_log[1024];

    glGetShaderiv(*shader, GL_COMPILE_STATUS, &success);
    gl_debug();

    if (!success) {
        glGetShaderInfoLog(*shader, sizeof(info_log), NULL, info_log);
        gl_debug();

        printf(
            "[SHADER_COMPILATION_ERROR]: %s\n",
            type == GL_VERTEX_SHADER ? "vertex shader" :
            type == GL_GEOMETRY_SHADER ? "geometry shader" :
            type == GL_FRAGMENT_SHADER ? "fragment shader" :
            "Unknown shader"
        );
        printf("Info log: %s\n", info_log);

        return false;
    }

    return true;
}


GLuint program_create(const GLuint* vertex_shader, const GLuint* geometry_shader, const GLuint* fragment_shader) {
    GLuint result = 0;

    result = glCreateProgram();
    gl_debug();

    if (result) {
        if (vertex_shader) {
            glAttachShader(result, *vertex_shader);
            gl_debug();
        }

        if (geometry_shader) {
            glAttachShader(result, *geometry_shader);
            gl_debug();
        }

        if (fragment_shader) {
            glAttachShader(result, *fragment_shader);
            gl_debug();
        }

        glLinkProgram(result);
        gl_debug();
    }

    return result;
}

GLuint program_create_from_files(const char* vertex_shader_file_name, const char* geometry_shader_file_name, const char* fragment_shader_file_name) {
    GLuint result = 0;
    GLuint vertex_shader = 0;
    GLuint geometry_shader = 0;
    GLuint fragment_shader = 0;

    if (vertex_shader_file_name) {
        vertex_shader = shader_create(vertex_shader_file_name, GL_VERTEX_SHADER);
    }
    if (geometry_shader_file_name) {
        geometry_shader = shader_create(geometry_shader_file_name, GL_GEOMETRY_SHADER);
    }
    if (fragment_shader_file_name) {
        fragment_shader = shader_create(fragment_shader_file_name, GL_FRAGMENT_SHADER);
    }

    result = program_create(
        vertex_shader ? &vertex_shader : NULL,
        geometry_shader ? &geometry_shader : NULL,
        fragment_shader ? &fragment_shader : NULL
    );

    if (fragment_shader) {
        shader_destroy(&fragment_shader);
    }
    if (geometry_shader) {
        shader_destroy(&geometry_shader);
    }
    if (vertex_shader) {
        shader_destroy(&vertex_shader);
    }

    return result;
}

void program_destroy(GLuint* program) {
    glDeleteProgram(*program);
    gl_debug();

    *program = 0;
}

bool program_check(const GLuint* program) {
    GLint success = 0;
    GLchar info_log[1024];

    glGetProgramiv(*program, GL_LINK_STATUS, &success);
    gl_debug();

    if (!success) {
        glGetProgramInfoLog(*program, sizeof(info_log), NULL, info_log);
        gl_debug();

        printf("Error GL_LINK_STATUS: program shader %u\n", *program);
        printf("Info log: %s\n", info_log);

        return false;
    }

    return true;
}

void program_use(const GLuint* program, const camera_t* camera, mat4 transform) {
    glUseProgram(*program);
    gl_debug();
    glUniformMatrix4fv(glGetUniformLocation(*program, "projection"), 1, GL_FALSE, &camera->projection[0][0]);
    gl_debug();
    glUniformMatrix4fv(glGetUniformLocation(*program, "view"), 1, GL_FALSE, &camera->view[0][0]);
    gl_debug();
    glUniformMatrix4fv(glGetUniformLocation(*program, "model"), 1, GL_FALSE, &transform[0][0]);
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

GLuint mesh_create(const char* file_name, GLsizei* indices_count) {
    GLuint result = 0;

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

    GLuint vertices_count = 0;

    vertices_count = array_size(positions) / 3;

    *indices_count = array_size(indices);
    result = _mesh_create_(vertices_count, positions, NULL, texture_coords, colors, NULL, NULL, *indices_count, indices);

    return result;
}

void mesh_destroy(GLuint* vao, GLsizei* indices_count) {
    glDeleteVertexArrays(1, vao);
    gl_debug();

    *vao = 0;
    *indices_count = 0;
}

void mesh_draw(const GLuint* vao, const GLsizei* indices_count) {
    glBindVertexArray(*vao);
    gl_debug();
    glDrawElements(GL_TRIANGLES, *indices_count, GL_UNSIGNED_INT, NULL);
    gl_debug();
    glBindVertexArray(0);
    gl_debug();
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

void camera_switch_2d(camera_t* camera) {
    glm_ortho(-1.0f, 1.0f, -1.0f, 1.0f, FLT_MIN, FLT_MAX, camera->projection);
}

void camera_switch_3d(camera_t* camera) {
    glm_perspective(45.0f, 1.0f, FLT_MIN, FLT_MAX, camera->projection);
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

void camera_update(camera_t* camera, GLFWwindow* window) {
    static vec3 up = { 0.0f, 1.0f, 0.0f };
    float speed = 0.01f;

    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT)) {
        speed *= 50.0f;
    }


    if (glfwGetKey(window, GLFW_KEY_LEFT)) {
        camera->rotation[0] -= 1.0f * speed;

        if (camera->rotation[0] < 0.0f) {
            camera->rotation[0] += 360.0f;
        }

        camera->direction[0] = sinf(glm_rad(camera->rotation[0]));
        camera->direction[2] = -cosf(glm_rad(camera->rotation[0]));
    }

    if (glfwGetKey(window, GLFW_KEY_RIGHT)) {
        camera->rotation[0] += 1.0f * speed;

        if (camera->rotation[0] >= 360.0f) {
            camera->rotation[0] -= 360.0f;
        }

        camera->direction[0] = sinf(glm_rad(camera->rotation[0]));
        camera->direction[2] = -cosf(glm_rad(camera->rotation[0]));
    }

    if (glfwGetKey(window, GLFW_KEY_UP)) {
        camera->rotation[1] += 1.0f * speed;

        if (camera->rotation[1] >= 89.0f) {
            camera->rotation[1] = 89.0f;
        }

        camera->direction[1] = tanf(glm_rad(camera->rotation[1]));
    }

    if (glfwGetKey(window, GLFW_KEY_DOWN)) {
        camera->rotation[1] -= 1.0f * speed;

        if (camera->rotation[1] < -89.0f) {
            camera->rotation[1] = -89.0f;
        }

        camera->direction[1] = tanf(glm_rad(camera->rotation[1]));
    }


    if (glfwGetKey(window, GLFW_KEY_A)) {
        vec3 buffer = { 0.0f, 0.0f, 0.0f };

        glm_vec3_cross(camera->direction, up, buffer);

        camera->position[0] -= glm_rad(buffer[0]) * speed;
        camera->position[1] -= glm_rad(buffer[1]) * speed;
        camera->position[2] -= glm_rad(buffer[2]) * speed;
    }

    if (glfwGetKey(window, GLFW_KEY_D)) {
        vec3 buffer = { 0.0f, 0.0f, 0.0f };

        glm_vec3_cross(camera->direction, up, buffer);

        camera->position[0] += glm_rad(buffer[0]) * speed;
        camera->position[1] += glm_rad(buffer[1]) * speed;
        camera->position[2] += glm_rad(buffer[2]) * speed;
    }

    if (glfwGetKey(window, GLFW_KEY_Q)) {
        camera->position[1] -= glm_rad(1.0f) * speed;
    }

    if (glfwGetKey(window, GLFW_KEY_E)) {
        camera->position[1] += glm_rad(1.0f) * speed;
    }

    if (glfwGetKey(window, GLFW_KEY_W)) {
        camera->position[0] += glm_rad(camera->direction[0]) * speed;
        camera->position[2] += glm_rad(camera->direction[2]) * speed;
    }

    if (glfwGetKey(window, GLFW_KEY_S)) {
        camera->position[0] -= glm_rad(camera->direction[0]) * speed;
        camera->position[2] -= glm_rad(camera->direction[2]) * speed;
    }


    if (glfwGetKey(window, GLFW_KEY_F)) {
        glm_vec3_zero(camera->position);
        glm_vec3_zero(camera->rotation);
        glm_vec3_zero(camera->direction);
        glm_vec3_zero(camera->center);

        camera->direction[2] = -1.0f;
    }


    if (glfwGetKey(window, GLFW_KEY_2)) {
        camera_switch_2d(camera);
    }

    if (glfwGetKey(window, GLFW_KEY_3)) {
        camera_switch_3d(camera);
    }

    // camera->direction[0] = cosf(glm_rad(camera->rotation[0])) * cosf(glm_rad(camera->rotation[1]));
    // camera->direction[1] = sinf(glm_rad(camera->rotation[1]));
    // camera->direction[2] = sinf(glm_rad(camera->rotation[0])) * cosf(glm_rad(camera->rotation[1]));

    camera->center[0] = camera->position[0] + camera->direction[0];
    camera->center[1] = camera->position[1] + camera->direction[1];
    camera->center[2] = camera->position[2] + camera->direction[2];

    glm_lookat(camera->position, camera->center, up, camera->view);
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
    // if (size < 256) {
    //     text[size++] = (unsigned char)codepoint;
    // }
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
        glfwSetErrorCallback(glfw_error_callback);
        // glfwWindowHint(GLFW_SAMPLES, 16);
        glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GL_TRUE);
        glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
        glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

        result = glfwCreateWindow(1920 / 2, 1080 / 2, "Project", NULL, NULL);

        if (result) {
            glfwMakeContextCurrent(result);

            glfwSetWindowPos(result, 1920 / 4, 1080 / 4);

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


#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus


#endif // graphics_h
