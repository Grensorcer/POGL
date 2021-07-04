#include <iostream>
#include <fstream>
#include <string>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "program.hh"
#include "utils.hh"

using namespace mygl;

unsigned int VAO;

void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
    glutSwapBuffers();
}

bool initGlut(int *argc, char **argv)
{
    glutInit(argc, argv);
    glutInitContextVersion(4, 5);
    glutInitContextProfile(GLUT_CORE_PROFILE);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(1960, 1080);
    glutInitWindowPosition(10, 10);
    glutCreateWindow("My first render");
    glutDisplayFunc(display);
    return true;
}

bool initGlew()
{
    glewExperimental = true;
    GLenum glew_status = glewInit();
    if (glew_status != GLEW_OK)
    {
        std::cerr << "Error: glewInit: " << glewGetErrorString(glew_status)
                  << std::endl;
        return false;
    }

    return true;
}

bool initGl()
{
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    // glDepthRange(0., 1.);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_CULL_FACE);
    glClearColor(0, 0, 0, 0);
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(utils::messageCallback, 0);
    return true;
}

void setup_vao(GLuint program_id)
{
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glm::mat4 projection =
        glm::perspective(glm::radians(45.f), 1960.f / 1080.f, 0.1f, 100.f);
    glm::mat4 view =
        glm::lookAt(glm::vec3(5, 5, 5), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    projection *= view;

    glm::vec3 light_position{ 1, 2, 2 };
    glm::vec3 light_color{ 1, 1, 1 };

    unsigned int vertices_VBO;
    unsigned int colors_VBO;
    unsigned int texture_VBO;
    unsigned int texture;

    // Setup vertices
    glGenBuffers(1, &vertices_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, vertices_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(gl_static::vertices),
                 gl_static::vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
    glEnableVertexAttribArray(0);

    // Setup colors
    glGenBuffers(1, &colors_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, colors_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(gl_static::colors), gl_static::colors,
                 GL_STATIC_DRAW);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
    glEnableVertexAttribArray(1);

    // Setup texture coordinates
    glGenBuffers(1, &texture_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, texture_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(gl_static::texture_coords),
                 gl_static::texture_coords, GL_STATIC_DRAW);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
    glEnableVertexAttribArray(2);

    // Setup texture
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, n;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load("../data/Seamless_Pebbles_Texture.jpg",
                                    &width, &height, &n, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);

    // Setup projection matrix
    GLint proj_location = glGetUniformLocation(program_id, "projection_matrix");
    glUniformMatrix4fv(proj_location, 1, false, &projection[0][0]);

    // Setup light
    GLint light_position_location =
        glGetUniformLocation(program_id, "light_position");
    glUniform3fv(light_position_location, 1, &light_position[0]);
    GLint light_color_location =
        glGetUniformLocation(program_id, "light_color");
    glUniform3fv(light_color_location, 1, &light_color[0]);

    glBindVertexArray(0);
}

int main(int argc, char **argv)
{
    initGlut(&argc, argv);
    if (!initGlew())
        return 1;
    initGl();

    auto vsrc = utils::read_file_content("../shaders/first.vs");
    auto fsrc = utils::read_file_content("../shaders/first.fs");

    program test = program::make_program(vsrc, fsrc);
    if (!test.is_ready())
    {
        std::cerr << test.get_log();
        return 1;
    }

    test.use();
    setup_vao(test.id());

    glutMainLoop();

    return 0;
}