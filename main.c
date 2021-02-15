////////////////////////////////////////
// Project - mini engine.
//
//   Bla-bla-bla. Bla-bla and bla-bla.
//   Oh, yes, bla-bla bla bla-bla-bla bla
//   bla-bla.
//   Ну вы поняли короче...
//
//   Это мой проект, тут будет хлам и точка! (пока не доведу хоть до какого-то ума)
//
// Copyright (C) 2020-2021 Kana (kanahy.akama@bk.ru)
////////////////////////////////////////
// Contacts:
//   kanahy.akama@bk.ru
//   +380978337319 (only telegram)
//
// Links:
//   https://github.com/kanahy
//
// Skills:
//   C/C++
//   OpenAL
//   OpenGL
//   HTML/CSS/JS
////////////////////////////////////////
// Debug:
//   gcc main.c -std=c18 -Wall -Wconversion -lpthread -lglfw -lOpenGL -lopenal -ldl -lm -g -o main
//
// Release:
//   gcc main.c -std=c18 -Wall -Wconversion -lpthread -lglfw -lOpenGL -lopenal -ldl -lm -s -o main
//
// Launch:
//   ./main
//
// Compilation and Launch:
//   gcc main.c -std=c18 -Wall -Wconversion -lpthread -lglfw -lOpenGL -lopenal -ldl -lm -s -o main; ./main
//
// Other:
//   -Wextra -Werror
////////////////////////////////////////
// Мои родненькие живые обои, точнее код что бы они воспроизводились:
//   sleep 2s
//   xwinwrap -g 1920x1080 -ni -s -nf -b -un -ov -fdt -argb -- mpv --mute=yes --no-audio --no-osc --no-osd-bar --quiet --screen=0 --geometry=1920x1080+0+0 -wid WID --loop ~/.live-wallpaper/"the-wedding.mp4"
////////////////////////////////////////
// Полезные ссылки:
//   http://mackron.github.io/
//   https://github.com/nothings/single_file_libs
//   https://github.com/asc-community/MxEngine/blob/master/src/Platform/OpenAL/AudioBuffer.cpp#L89
//
// О, музончик подъехал:
//   https://www.youtube.com/watch?v=XFbbBm7K2LQ
//   https://www.youtube.com/watch?v=xr8ltGKiEmw
////////////////////////////////////////
#include "system.h"
#include "graphics_opengl.h"
#include "audio.h"


int main(int argc, char** argv) {
    GLFWwindow* window = window_create_opengl();
    camera_t camera = camera_initialize_2d();
    GLuint program = program_create_from_files("data/gui/shader.vs", NULL, "data/gui/shader.fs");
    GLuint diffuse = texture_create("data/gui/diffuse.png");
    GLsizei indices_count = 0;
    GLuint mesh = mesh_create(NULL, &indices_count);
    mat4 transform = GLM_MAT4_IDENTITY_INIT;

    glm_scale(
        transform,
        (vec3) {
            to_pixels(texture_get_width(&diffuse), 1920.0f),
            to_pixels(texture_get_height(&diffuse), 1080.0f),
            0.5f * to_pixels(texture_get_width(&diffuse), 1920.0f) / to_pixels(texture_get_height(&diffuse), 1080.0f)
        }
    );

    ALCcontext* audio_context = NULL;
    ALCdevice* audio_device = audio_device_create(&audio_context);
    ALuint buffer = audio_buffer_create("data/resources/test.mp3");
    ALuint source = audio_source_create(&buffer);

    audio_source_play(&source);

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        gl_debug();

        camera_update(&camera, window);

        texture_bind(&diffuse);
        program_use(&program, &camera, transform);

        mesh_draw(&mesh, &indices_count);

        program_unuse();
        texture_unbind();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    mesh_destroy(&mesh, &indices_count);
    texture_destroy(&diffuse);
    program_destroy(&program);

    audio_source_destroy(&source);
    audio_buffer_destroy(&buffer);
    audio_device_destroy(&audio_device, &audio_context);

    glfwTerminate();

    return 0;
}
