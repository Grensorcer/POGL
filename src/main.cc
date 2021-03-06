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
#include "wind.hh"

using namespace mygl;

std::vector<std::unique_ptr<Mesh>> scene;
std::map<std::string, std::shared_ptr<program>> programs;

DirectionalShadowMap shadow_map;
// CubeShadowMap shadow_map;

int camsize_x = 1920, camsize_y = 1080;

Camera camera{ camsize_x, camsize_y, glm::vec3(3, 1, 8), glm::vec3(0, 0, -1),
               glm::vec3(0, 1, 0) };

void mouse_function(int x, int y)
{
    camera.on_mouse(x, y);
}
void camera_keypress_function(int key, int, int)
{
    camera.on_keypress(key);
}

void setup_samplers(program &p)
{
    p.use();
    p.set_int("texture_sampler", 0);
    p.set_int("normal_sampler", 1);
    p.set_int("height_sampler", 2);
    p.set_int("shadowmap_sampler", 3);
}

void set_uniforms(std::shared_ptr<program> &program,
                  const glm::vec3 &light_position)
{
    program->use();
    glm::vec3 light_color{ 1, 1, 1 };
    static float ambient_light = 0.2f;

    program->set_vec3("light_position", light_position);
    program->set_vec3("light_color", light_color);
    program->set_float("ambient_light", ambient_light);
}

void compute_frame(const Wind &wind)
{
    auto &neighbour_program = programs["compute_neighbour"];
    for (auto &mesh : scene)
        mesh->compute(*neighbour_program);

    auto &collision_program = programs["compute_collision"];
    collision_program->use();
    for (auto &mesh : scene)
    {
        if (mesh->is_compute())
        {
            collision_program->set_mat4("world", mesh->get_world());
            for (auto &other : scene)
            {
                if (other != mesh)
                {
                    collision_program->set_mat4("collision_world",
                                                other->get_world());
                    for (const auto &entry : other->get_entries())
                    {
                        glBindVertexArray(entry.VAO);
                        collision_program->set_int("nb_collisions",
                                                   entry.num_vertices);
                        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7,
                                         entry.vertex_VBO);
                        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8,
                                         entry.normal_VBO);
                        mesh->compute(*collision_program, entry.num_vertices);
                        // utils::debug_buffer<glm::vec3>(entry.vertex_VBO,
                        //                               entry.num_vertices);
                    }
                }
            }
        }
    }
    glBindVertexArray(0);

    auto &cloth_program = programs["compute_cloth"];
    cloth_program->use();
    cloth_program->set_vec3("wind", wind.Force());
    for (auto &mesh : scene)
        mesh->compute(*cloth_program);

    auto &normal_program = programs["compute_normal"];
    for (auto &mesh : scene)
        mesh->compute(*normal_program);
}

void directional_shadow_frame(DirectionalShadowMap &shadow_map,
                              const glm::vec3 &view_position)
{
    glViewport(0, 0, 1024, 1024);
    shadow_map.write();
    glClear(GL_DEPTH_BUFFER_BIT);

    shadow_map.set_view(view_position);

    for (auto &mesh : scene)
    {
        setup_samplers(*mesh->get_shader());
        set_uniforms(mesh->get_shader(), view_position);

        const auto &world = mesh->get_world();
        const auto wvp =
            shadow_map.get_projection() * shadow_map.get_view() * world;
        auto &shader = mesh->get_shader();
        shader->set_mat4("world", world);
        shader->set_mat4("wvp", wvp);
        shader->set_mat4("light_wvp", wvp);
        shader->set_vec3("view_position", view_position);
        shader->set_vec3("light_position", view_position);

        mesh->render();
    }

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

    shadow_map.set_view(view_position);
    program->set_float("far_plane", 100.f);
    program->set_vec3("view_position", view_position);
    for (short i = 0; i < 6; ++i)
    {
        const auto wvp = shadow_map.get_projection() * shadow_map.get_view(i);
        auto cur = "wvp[" + std::to_string(i) + ']';
        program->set_mat4(cur, wvp);
    }

    for (auto &mesh : scene)
        mesh->render(program);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void complete_frame(const glm::vec3 &light_position)
{
    glViewport(0, 0, camsize_x, camsize_y);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shadow_map.read(GL_TEXTURE3);

    glm::mat4 projection = glm::perspective(
        glm::radians(45.f), float(camsize_x) / float(camsize_y), 0.1f, 1000.f);

    glm::mat4 view = glm::lookAt(
        camera.position(), camera.position() + camera.target(), camera.up());
    glm::mat4 light_view = shadow_map.get_view();

    const auto vp = projection * view;
    const auto lvp = projection * light_view;
    for (auto &mesh : scene)
    {
        setup_samplers(*mesh->get_shader());
        set_uniforms(mesh->get_shader(), light_position);

        const auto &world = mesh->get_world();
        auto &shader = mesh->get_shader();
        shader->set_mat4("world", world);
        shader->set_mat4("wvp", vp * world);
        shader->set_mat4("light_wvp", lvp * world);
        shader->set_vec3("view_position", camera.position());
        shader->set_vec3("light_position", light_position);
        shader->set_float("far_plane", 100.f);

        mesh->render();
    }
}

void display()
{
    static auto wind = Wind(0.5, 10.);
    static float rotation = 0.f;
    static float delta = 0.01f;

    rotation += delta;
    if (rotation >= 1)
        rotation = 0;
    wind.update();

    glm::vec3 light_position{ 10, 10, 10 };

    camera.on_render();
    compute_frame(wind);
    // cube_shadow_frame(shadow_map, light_position);
    directional_shadow_frame(shadow_map, light_position);
    complete_frame(light_position);
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

bool setup_scene()
{
    if (!shadow_map.init(1024, 1024))
        return false;

    scene.emplace_back(new QuadMesh("../data/model/cloth_more.obj"));
    scene[0]->set_shader(programs["render_quads"]);
    scene[0]->set_world(
        glm::translate(scene[0]->get_world(), glm::vec3(0, 1, 0)));

    /*
    scene.emplace_back(new TriangleMesh("../data/model/sphere.obj"));
    scene[1]->set_shader(programs["render"]);
    scene[1]->set_world(
        glm::translate(scene[1]->get_world(), glm::vec3(0, 0, 0)));
    */

    for (auto &mesh : scene)
    {
        if (!mesh->load())
            return false;
    }
    return true;
}

bool setup_shaders()
{
    auto render_vs_src =
        utils::read_file_content("../shaders/reliefMapping.vs");
    auto render_fs_src =
        utils::read_file_content("../shaders/reliefMapping.fs");

    auto renderq_tes_src =
        utils::read_file_content("../shaders/do_nothing_relief.glsl");
    auto renderq_fs_src =
        utils::read_file_content("../shaders/reliefMapping_quads.fs");

    auto svsrc = utils::read_file_content("../shaders/cubeShadowMap.vs");
    auto sgsrc = utils::read_file_content("../shaders/cubeShadowMap.gs");
    auto sfsrc = utils::read_file_content("../shaders/cubeShadowMap.fs");

    auto compute_cloth_src =
        utils::read_file_content("../shaders/compute_cloth.glsl");
    auto compute_neighbour_src =
        utils::read_file_content("../shaders/compute_neighbour.glsl");
    auto compute_collision_src =
        utils::read_file_content("../shaders/compute_collision.glsl");
    auto compute_normal_src =
        utils::read_file_content("../shaders/compute_normal.glsl");

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

    auto compute_cloth = program::make_compute(compute_cloth_src);
    programs["compute_cloth"] = compute_cloth;

    auto compute_neighbour = program::make_compute(compute_neighbour_src);
    programs["compute_neighbour"] = compute_neighbour;

    auto compute_normal = program::make_compute(compute_normal_src);
    programs["compute_normal"] = compute_normal;

    auto compute_collision = program::make_compute(compute_collision_src);
    programs["compute_collision"] = compute_collision;

    for (auto &program : programs)
    {
        if (!program.second->is_ready())
        {
            std::cerr << program.first << ": " << program.second->get_log();
            return false;
        }
    }

    render_quads->use();
    setup_samplers(*render_quads);
    cube_shadow->use();
    setup_samplers(*cube_shadow);
    render->use();
    setup_samplers(*render);

    return true;
}

int main(int argc, char **argv)
{
    initGlut(&argc, argv);
    if (!initGlew())
        return 1;
    initGl();

    if (!setup_shaders())
        return 1;

    std::cout << "Ran\n";
    if (!setup_scene())
    {
        std::cerr << "VAO setup failed\n";
        return 1;
    }

    glutMainLoop();
    return 0;
}
