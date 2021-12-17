#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <vector>
#include <map>
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
#include "shadow_map.hh"
#include "camera.hh"

using namespace mygl;

GLint gWorldMatrixLocation;
GLint gProjectionMatrixLocation;
GLint gLightProjectionMatrixLocation;
GLint gViewPositionLocation;
GLint gAmbientLight;
GLint gLightPositionLocation;
GLint gLightColorLocation;
GLint gTextureLocation;
GLint gNormalsLocation;
GLint gHeightLocation;
GLint gShadowMapLocation;

std::vector<std::unique_ptr<Mesh>> scene;
std::map<std::string, std::shared_ptr<program>> shaders;

DirectionalShadowMap shadow_map;

Camera camera{ 1920, 1080, glm::vec3(3, 4, 8), glm::vec3(0, 0, -1),
               glm::vec3(0, 1, 0) };

void mouse_function(int x, int y)
{
    camera.on_mouse(x, y);
}
void camera_keypress_function(int key, int, int)
{
    camera.on_keypress(key);
}

void set_uniforms(const glm::vec3 &light_position)
{
    glm::vec3 light_color{ 1, 1, 1 };
    static float ambient_light = 0.1f;

    glUniform3fv(gLightPositionLocation, 1, &light_position[0]);
    glUniform3fv(gLightColorLocation, 1, &light_color[0]);
    glUniform1f(gAmbientLight, ambient_light);

    glUniform1i(gTextureLocation, 0);
    glUniform1i(gNormalsLocation, 1);
    glUniform1i(gHeightLocation, 2);
    glUniform1i(gShadowMapLocation, 3);
}

void directional_shadow_frame(DirectionalShadowMap &shadow_map,
                              const glm::mat4 &world,
                              const glm::vec3 &view_position)
{
    glViewport(0, 0, 1024, 1024);
    shadow_map.write();
    glClear(GL_DEPTH_BUFFER_BIT);

    // glm::mat4 projection = glm::ortho(-10.f, 10.f, -10.f, 10.f, 0.1f,
    // 1000.f);

    shadow_map.set_view(view_position);
    const auto wvp =
        shadow_map.get_projection() * shadow_map.get_view() * world;

    glUniformMatrix4fv(gProjectionMatrixLocation, 1, false, &wvp[0][0]);
    glUniformMatrix4fv(gLightProjectionMatrixLocation, 1, false, &wvp[0][0]);
    glUniform3fv(gViewPositionLocation, 1, &view_position[0]);

    for (auto &mesh : scene)
        mesh->render(*shaders["render"]);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void cube_shadow_frame(CubeShadowMap &shadow_map, const glm::mat4 &world,
                       const glm::vec3 &view_position)
{
    glViewport(0, 0, 1024, 1024);
    shadow_map.write();
    glClear(GL_DEPTH_BUFFER_BIT);

    // glm::mat4 projection = glm::ortho(-10.f, 10.f, -10.f, 10.f, 0.1f,
    // 1000.f);

    shadow_map.set_view(view_position);
    const GLfloat *wvps[6];
    for (short i = 0; i < 6; ++i)
    {
        const auto wvp =
            shadow_map.get_projection() * shadow_map.get_view(i) * world;
        wvps[i] = &wvp[0][0];
    }
    glUniformMatrix4fv(gProjectionMatrixLocation, 6, false, *wvps);
    glUniformMatrix4fv(gLightProjectionMatrixLocation, 6, false, *wvps);
    glUniform3fv(gViewPositionLocation, 1, &view_position[0]);

    for (auto &mesh : scene)
        mesh->render(*shaders["render"]);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void complete_frame(const glm::mat4 &world, const glm::vec3 &light_position)
{
    glViewport(0, 0, 1920, 1080);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shadow_map.read(GL_TEXTURE3);

    glm::mat4 projection =
        glm::perspective(glm::radians(45.f), 1920.f / 1080.f, 0.1f, 1000.f);

    glm::mat4 view = glm::lookAt(
        camera.position(), camera.position() + camera.target(), camera.up());
    glm::mat4 light_view =
        glm::lookAt(light_position, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    const auto wvp = projection * view * world;
    const auto lwvp = projection * light_view * world;

    glUniformMatrix4fv(gProjectionMatrixLocation, 1, false, &wvp[0][0]);
    glUniformMatrix4fv(gLightProjectionMatrixLocation, 1, false, &lwvp[0][0]);
    glUniform3fv(gViewPositionLocation, 1, &(camera.position()[0]));

    for (auto &mesh : scene)
        mesh->render(*shaders["render"]);
}

void display()
{
    static float rotation = 0.f;
    static float delta = 0.001f;
    rotation += delta;
    if (rotation >= 1)
        rotation = 0;

    glm::vec3 light_position{ 15, 15, 15 };
    set_uniforms(light_position);

    glm::mat4 world = glm::mat4(1.0f);
    world =
        glm::rotate(world, glm::radians(rotation * 360), glm::vec3(0, 1, 0));
    // world = glm::translate(world, glm::vec3(rotation, 0, 0));
    glUniformMatrix4fv(gWorldMatrixLocation, 1, false, &world[0][0]);

    camera.on_render();
    directional_shadow_frame(shadow_map, world, light_position);
    complete_frame(world, light_position);
    // glutPostRedisplay();
    glutSwapBuffers();
}

bool initGlut(int *argc, char **argv)
{
    glutInit(argc, argv);
    glutInitContextVersion(4, 5);
    glutInitContextProfile(GLUT_CORE_PROFILE);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(1920, 1080);
    glutInitWindowPosition(10, 10);
    glutCreateWindow("My first render");
    glutDisplayFunc(display);
    glutIdleFunc(display);
    glutPassiveMotionFunc(mouse_function);
    glutSpecialFunc(camera_keypress_function);
    glutWarpPointer(camera.mouse_x(), camera.mouse_y());
    // glutGameModeString("1920x1080@32");
    // glutEnterGameMode();
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
    // glEnable(GL_CULL_FACE);
    glClearColor(149.f / 255.f, 213.f / 255.f, 230.f / 255.f, 0);
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(utils::messageCallback, 0);
    return true;
}

bool setup_vao(GLuint program_id)
{
    gProjectionMatrixLocation = glGetUniformLocation(program_id, "wvp");
    gLightProjectionMatrixLocation =
        glGetUniformLocation(program_id, "light_wvp");
    gWorldMatrixLocation = glGetUniformLocation(program_id, "world");
    gViewPositionLocation = glGetUniformLocation(program_id, "view_position");
    gLightPositionLocation = glGetUniformLocation(program_id, "light_position");
    gLightColorLocation = glGetUniformLocation(program_id, "light_color");
    gAmbientLight = glGetUniformLocation(program_id, "ambient_light");
    gTextureLocation = glGetUniformLocation(program_id, "texture_sampler");
    gNormalsLocation = glGetUniformLocation(program_id, "normal_sampler");
    gHeightLocation = glGetUniformLocation(program_id, "height_sampler");
    gShadowMapLocation = glGetUniformLocation(program_id, "shadowmap_sampler");

    if (!shadow_map.init(1024, 1024))
        return false;

    scene.emplace_back(new Mesh("../data/model/elephant_plane.obj"));

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

    auto vsrc = utils::read_file_content("../shaders/reliefMapping.vs");
    auto fsrc = utils::read_file_content("../shaders/reliefMapping.fs");
    const std::map<GLuint, std::string> shader_map{
        { GL_VERTEX_SHADER, vsrc }, { GL_FRAGMENT_SHADER, fsrc }
    };

    auto render = program::make_program(shader_map);
    shaders["render"] = render;

    if (!render->is_ready())
    {
        std::cerr << render->get_log();
        return 1;
    }

    render->use();

    if (!setup_vao(render->id()))
    {
        std::cerr << "VAO setup failed\n";
        return 1;
    }

    glutMainLoop();

    return 0;
}