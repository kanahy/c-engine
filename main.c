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
//
// Anime:
//   https://youtu.be/mtvdM-YMUJ4?t=5871
////////////////////////////////////////
#include "c-engine.h"


int main(int argc, char** argv) {
    GLFWwindow* window = window_create_opengl();
    camera_t camera = camera_initialize_2d();

    audio_device_t audio_device = audio_device_create();
    audio_buffer_t buffer = audio_buffer_create("data/resources/test.mp3");
    audio_source_t source = audio_source_create(&buffer);

    const char* textures[] = {
        "data/gui/diffuse.png",
        "data/resources/test.gif"
    };

    object_t object = object_create(
        "data/gui/shader.vs", NULL, "data/gui/shader.fs",
        NULL,
        textures, 2
    );

    {
        int window_width = 0;
        int window_height = 0;

        glfwGetWindowSize(window, &window_width, &window_height);

        // glm_scale(
        //     object.matrix,
        //     (vec3) {
        //         to_pixels(texture_get_width(&object.textures[0]), window_width),
        //         to_pixels(texture_get_height(&object.textures[0]), window_height),
        //         1.0f
        //     }
        // );
    }

    audio_source_play(&source);

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        gl_debug();

        camera_update(&camera, window);

        object_draw(&object, &camera);

        program_unuse();
        texture_unbind();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    object_destroy(&object);

    audio_source_destroy(&source);
    audio_buffer_destroy(&buffer);
    audio_device_destroy(&audio_device);

    glfwTerminate();

    return 0;
}
