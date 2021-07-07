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
GLint gProjectionMatrixLocation;
GLint gAmbientLight;
GLint gLightPositionLocation;
GLint gLightColorLocation;
GLuint gTextureLocation;

void load_texture(const char *texture_path)
{
    glGenTextures(1, &gTextureLocation);
    glBindTexture(GL_TEXTURE_2D, gTextureLocation);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, n;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load(texture_path, &width, &height, &n, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);
}

void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    static float scale = 0.f;
    static float delta = 0.005f;
    static float ambient_light = 0.3f;

    glm::vec3 light_position{ 1, 2, 2 };
    glm::vec3 light_color{ 1, 1, 1 };

    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 projection =
        glm::perspective(glm::radians(45.f), 1960.f / 1080.f, 0.1f, 100.f);
    glm::mat4 view =
        glm::lookAt(glm::vec3(3, 3, 5), glm::vec3(0, 0, 0), glm::vec3(0, 0, 1));
    projection *= view;

    scale += delta;
    if (scale >= 1 || scale <= -1)
        delta *= -1;

    // model = glm::scale(model, glm::vec3(scale));
    model = glm::rotate(model, scale * 3, glm::vec3(0, 0, 1));
    // model = glm::translate(model, glm::vec3(0, 0, 0));
    projection *= model;

    glUniformMatrix4fv(gProjectionMatrixLocation, 1, false, &projection[0][0]);
    glUniform3fv(gLightPositionLocation, 1, &light_position[0]);
    glUniform3fv(gLightColorLocation, 1, &light_color[0]);
    glUniform1f(gAmbientLight, ambient_light);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 3 * 4, GL_UNSIGNED_SHORT, (void *)0);
    glBindVertexArray(0);
    // glutPostRedisplay();
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
    glutIdleFunc(display);
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

    unsigned int vertices_VBO;
    unsigned int colors_VBO;
    unsigned int texture_VBO;
    unsigned int IBO;

    // Setup vertices
    glGenBuffers(1, &vertices_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, vertices_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(gl_static::pyramid_vertices),
                 gl_static::pyramid_vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

    // Setup colors
    glGenBuffers(1, &colors_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, colors_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(gl_static::pyramid_colors),
                 gl_static::pyramid_colors, GL_STATIC_DRAW);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

    // Setup texture coordinates
    glGenBuffers(1, &texture_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, texture_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(gl_static::pyramid_texture_coords),
                 gl_static::pyramid_texture_coords, GL_STATIC_DRAW);

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);

    // Setup index buffer
    glGenBuffers(1, &IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(gl_static::pyramid_indices),
                 gl_static::pyramid_indices, GL_STATIC_DRAW);

    gProjectionMatrixLocation =
        glGetUniformLocation(program_id, "projection_matrix");
    gLightPositionLocation = glGetUniformLocation(program_id, "light_position");
    gLightColorLocation = glGetUniformLocation(program_id, "light_color");
    gAmbientLight = glGetUniformLocation(program_id, "ambient_light");

    load_texture("../data/Seamless_Pebbles_Texture.jpg");

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