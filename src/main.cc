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

std::vector<std::unique_ptr<Mesh>> scene;
std::map<std::string, std::shared_ptr<program>> programs;

DirectionalShadowMap shadow_map;

int camsize_x = 1820, camsize_y = 980;

Camera camera{ camsize_x, camsize_y, glm::vec3(3, 4, 8), glm::vec3(0, 0, -1),
               glm::vec3(0, 1, 0) };

void mouse_function(int x, int y)
{
    camera.on_mouse(x, y);
}
void camera_keypress_function(int key, int, int)
{
    camera.on_keypress(key);
}

void set_uniforms(program &program, const glm::vec3 &light_position)
{
    program.use();
    glm::vec3 light_color{ 1, 1, 1 };
    static float ambient_light = 0.1f;

    program.set_vec3("light_position", light_position);
    program.set_vec3("light_color", light_color);
    program.set_float("ambient_light", ambient_light);
}

void compute_frame()
{
    auto &program = programs["compute"];
    for (auto &mesh : scene)
        mesh->compute(*program);
}

void directional_shadow_frame(DirectionalShadowMap &shadow_map,
                              const glm::mat4 &world,
                              const glm::vec3 &view_position)
{
    auto &program = programs["render_quads"];
    program->use();

    glViewport(0, 0, 1024, 1024);
    shadow_map.write();
    glClear(GL_DEPTH_BUFFER_BIT);

    // glm::mat4 projection = glm::ortho(-10.f, 10.f, -10.f, 10.f, 0.1f,
    // 1000.f);

    shadow_map.set_view(view_position);
    const auto wvp =
        shadow_map.get_projection() * shadow_map.get_view() * world;

    program->set_mat4("world", world);
    program->set_mat4("wvp", wvp);
    program->set_mat4("light_wvp", wvp);
    program->set_vec3("view_position", view_position);

    for (auto &mesh : scene)
        mesh->render(*program);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void cube_shadow_frame(CubeShadowMap &shadow_map,
                       const glm::vec3 &view_position)
{
    auto &program = programs["cube_shadow"];
    program->use();
    glViewport(0, 0, camsize_x, camsize_y);
    shadow_map.write();
    glClear(GL_DEPTH_BUFFER_BIT);

    // glm::mat4 projection = glm::ortho(-10.f, 10.f, -10.f, 10.f, 0.1f,
    // 1000.f);

    shadow_map.set_view(view_position);
    program->set_float("far_plane", 25.f);
    program->set_vec3("view_position", view_position);
    for (short i = 0; i < 6; ++i)
    {
        const auto wvp = shadow_map.get_projection() * shadow_map.get_view(i);
        auto cur = "wvp[" + std::to_string(i) + ']';
        program->set_mat4(cur, wvp);
    }

    for (auto &mesh : scene)
        mesh->render(*program);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void complete_frame(const glm::mat4 &world, const glm::vec3 &light_position)
{
    auto &program = programs["render_quads"];
    program->use();
    glViewport(0, 0, camsize_x, camsize_y);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shadow_map.read(GL_TEXTURE3);

    glm::mat4 projection =
        glm::perspective(glm::radians(45.f), float(camsize_x) / float(camsize_y), 0.1f, 1000.f);

    glm::mat4 view = glm::lookAt(
        camera.position(), camera.position() + camera.target(), camera.up());
    glm::mat4 light_view =
        glm::lookAt(light_position, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    const auto wvp = projection * view * world;
    const auto lwvp = projection * light_view * world;

    program->set_mat4("world", world);
    program->set_mat4("wvp", wvp);
    program->set_mat4("light_wvp", lwvp);
    program->set_vec3("view_position", camera.position());

    for (auto &mesh : scene)
        mesh->render(*program);
}

void display()
{
    static float rotation = 0.f;
    static float delta = 0.001f;
    rotation += delta;
    if (rotation >= 1)
        rotation = 0;

    glm::vec3 light_position{ 15, 15, 15 };
    set_uniforms(*programs["render"], light_position);
    set_uniforms(*programs["render_quads"], light_position);

    glm::mat4 world = glm::mat4(1.0f);
    world =
        glm::rotate(world, glm::radians(rotation * 360), glm::vec3(0, 1, 0));
    // world = glm::translate(world, glm::vec3(rotation, 0, 0));

    camera.on_render();
    compute_frame();
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
    glutInitWindowSize(camsize_x, camsize_y);
    glutInitWindowPosition(10, 10);
    glutCreateWindow("My first render");
    glutDisplayFunc(display);
    glutIdleFunc(display);
    glutPassiveMotionFunc(mouse_function);
    glutSpecialFunc(camera_keypress_function);
    glutWarpPointer(camera.mouse_x(), camera.mouse_y());
    // glutGameModeString("camsize_xxcamsize_y@32");
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

void setup_uniforms(program &p)
{
    /*
    gProjectionMatrixLocation = glGetUniformLocation(program_id, "wvp");
    gLightProjectionMatrixLocation =
        glGetUniformLocation(program_id, "light_wvp");
    gWorldMatrixLocation = glGetUniformLocation(program_id, "world");
    gViewPositionLocation = glGetUniformLocation(program_id, "view_position");
    gLightPositionLocation = glGetUniformLocation(program_id, "light_position");
    gLightColorLocation = glGetUniformLocation(program_id, "light_color");
    gFarPlaneLocation = glGetUniformLocation(program_id, "far_plane");
    gAmbientLight = glGetUniformLocation(program_id, "ambient_light");
    gTextureLocation = glGetUniformLocation(program_id, "texture_sampler");
    gNormalsLocation = glGetUniformLocation(program_id, "normal_sampler");
    gHeightLocation = glGetUniformLocation(program_id, "height_sampler");
    gShadowMapLocation = glGetUniformLocation(program_id, "shadowmap_sampler");
    */

    p.set_int("texture_sampler", 0);
    p.set_int("normal_sampler", 1);
    p.set_int("height_sampler", 2);
    p.set_int("shadowmap_sampler", 3);
}

bool setup_scene()
{
    if (!shadow_map.init(1024, 1024))
        return false;

    scene.emplace_back(new QuadMesh("../data/model/cloth.obj"));

    for (auto &mesh : scene)
    {
        if (!mesh->load())
            return false;
    }
    return true;
}

bool setup_shaders()
{
    auto render_vs_src = utils::read_file_content("../shaders/normals.vs");
    auto render_fs_src = utils::read_file_content("../shaders/normals.fs");

    auto renderq_tes_src =
        utils::read_file_content("../shaders/do_nothing.glsl");
    auto renderq_fs_src =
        utils::read_file_content("../shaders/normals_quads.fs");

    auto svsrc = utils::read_file_content("../shaders/cubeShadowMap.vs");
    auto sgsrc = utils::read_file_content("../shaders/cubeShadowMap.gs");
    auto sfsrc = utils::read_file_content("../shaders/cubeShadowMap.fs");

    auto compute_src =
        utils::read_file_content("../shaders/compute_gravity.glsl");

    std::map<GLuint, std::string> shader_map{
        { GL_VERTEX_SHADER, render_vs_src },
        { GL_FRAGMENT_SHADER, render_fs_src }
    };

    auto render = program::make_program(shader_map);
    programs["render"] = render;

    shader_map[GL_TESS_EVALUATION_SHADER] = renderq_tes_src;
    shader_map[GL_FRAGMENT_SHADER] = renderq_fs_src;
    auto render_quads = program::make_program(shader_map);
    programs["render_quads"] = render_quads;

    shader_map.erase(GL_TESS_EVALUATION_SHADER);
    shader_map[GL_VERTEX_SHADER] = svsrc;
    shader_map[GL_GEOMETRY_SHADER] = sgsrc;
    shader_map[GL_FRAGMENT_SHADER] = sfsrc;
    auto cube_shadow = program::make_program(shader_map);
    programs["cube_shadow"] = cube_shadow;

    auto compute = program::make_compute(compute_src);
    programs["compute"] = compute;

    if (!render->is_ready())
    {
        std::cerr << render->get_log();
        return false;
    }
    if (!render_quads->is_ready())
    {
        std::cerr << cube_shadow->get_log();
        return false;
    }
    if (!cube_shadow->is_ready())
    {
        std::cerr << cube_shadow->get_log();
        return false;
    }
    if (!compute->is_ready())
    {
        std::cerr << cube_shadow->get_log();
        return false;
    }

    render_quads->use();
    setup_uniforms(*render_quads);
    cube_shadow->use();
    setup_uniforms(*cube_shadow);
    render->use();
    setup_uniforms(*render);

    return true;
}

int main(int argc, char **argv)
{
    std::cout << 1 << std::endl;
    initGlut(&argc, argv);
    std::cout << 2 << std::endl;
    if (!initGlew())
        return 1;
    initGl();
    std::cout << 3 << std::endl;

    if (!setup_shaders())
        return 1;
    std::cout << 4 << std::endl;

    if (!setup_scene())
    {
        std::cerr << "VAO setup failed\n";
        return 1;
    }
    std::cout << 5 << std::endl;

    glutMainLoop();
    std::cout << 6 << std::endl;
    return 0;
}
