#include <iostream>
#include <fstream>
#include <string>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

#include "program.hh"
#include "utils.hh"

using namespace mygl;

unsigned int VAO;

void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 12 * 3);
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

    unsigned int vertices_VBO;
    unsigned int colors_VBO;

    // Setup vertices
    glGenBuffers(1, &vertices_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, vertices_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(gl_static::cube_vertices),
                 gl_static::cube_vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
    glEnableVertexAttribArray(0);

    // Setup colors
    glGenBuffers(1, &colors_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, colors_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(gl_static::cube_colors),
                 gl_static::cube_colors, GL_STATIC_DRAW);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
    glEnableVertexAttribArray(1);

    // Setup projection matrix
    GLint proj_location = glGetUniformLocation(program_id, "projection_matrix");
    glUniformMatrix4fv(proj_location, 1, false, &projection[0][0]);

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