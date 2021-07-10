#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <vector>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

#include "program.hh"
#include "utils.hh"
#include "texture.hh"
#include "mesh.hh"

using namespace mygl;

GLint gWorldMatrixLocation;
GLint gProjectionMatrixLocation;
GLint gViewPositionLocation;
GLint gAmbientLight;
GLint gLightPositionLocation;
GLint gLightColorLocation;
GLint gTextureLocation;
GLint gNormalsLocation;
GLint gHeightLocation;

std::vector<std::unique_ptr<Mesh>> scene;

void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    static float scale = 0.f;
    static float delta = 0.005f;
    static float ambient_light = 0.3f;

    glm::vec3 light_position{ 0, 3, 0 };
    glm::vec3 light_color{ 1, 1, 1 };

    glm::mat4 world = glm::mat4(1.0f);
    glm::mat4 projection =
        glm::perspective(glm::radians(45.f), 1960.f / 1080.f, 0.1f, 100.f);

    glm::vec3 view_position = glm::vec3(0, 2, 5);
    glm::mat4 view =
        glm::lookAt(view_position, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    projection *= view;

    scale += delta;
    if (scale >= 1 || scale <= -1)
        delta *= -1;

    // model = glm::scale(model, glm::vec3(scale));
    world = glm::rotate(world, scale * 3, glm::vec3(1, 0, 0));
    // world = glm::translate(world, glm::vec3(scale, 0, 0));
    projection *= world;

    glUniformMatrix4fv(gWorldMatrixLocation, 1, false, &world[0][0]);
    glUniformMatrix4fv(gProjectionMatrixLocation, 1, false, &projection[0][0]);
    glUniform3fv(gViewPositionLocation, 1, &view_position[0]);
    glUniform3fv(gLightPositionLocation, 1, &light_position[0]);
    glUniform3fv(gLightColorLocation, 1, &light_color[0]);
    glUniform1f(gAmbientLight, ambient_light);

    glUniform1i(gTextureLocation, 0);
    glUniform1i(gNormalsLocation, 1);
    glUniform1i(gHeightLocation, 2);

    for (auto &mesh : scene)
        mesh->render();
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

bool setup_vao(GLuint program_id)
{
    gProjectionMatrixLocation = glGetUniformLocation(program_id, "wvp");
    gWorldMatrixLocation = glGetUniformLocation(program_id, "world");
    gViewPositionLocation = glGetUniformLocation(program_id, "view_position");
    gLightPositionLocation = glGetUniformLocation(program_id, "light_position");
    gLightColorLocation = glGetUniformLocation(program_id, "light_color");
    gAmbientLight = glGetUniformLocation(program_id, "ambient_light");
    gTextureLocation = glGetUniformLocation(program_id, "texture_sampler");
    gNormalsLocation = glGetUniformLocation(program_id, "normal_sampler");
    gHeightLocation = glGetUniformLocation(program_id, "height_sampler");

    scene.emplace_back(new Mesh("../data/model/monkey_pyramid.obj"));

    for (auto &mesh : scene)
    {
        if (!mesh->load())
            return false;
    }
    return true;
}

int main(int argc, char **argv)
{
    initGlut(&argc, argv);
    if (!initGlew())
        return 1;
    initGl();

    auto vsrc = utils::read_file_content("../shaders/normalmap.vs");
    auto fsrc = utils::read_file_content("../shaders/normalmap.fs");

    program test = program::make_program(vsrc, fsrc);
    if (!test.is_ready())
    {
        std::cerr << test.get_log();
        return 1;
    }

    test.use();

    if (!setup_vao(test.id()))
    {
        std::cerr << "VAO setup failed\n";
        return 1;
    }

    glutMainLoop();

    return 0;
}