#pragma once
#include <glm/vec3.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <GL/glut.h>

namespace mygl
{
    class Camera
    {
    public:
        Camera(int width, int height)
            : width_{ width }
            , height_{ height }
            , position_{ 0, 0, 0 }
            , target_{ 0, 0, 1 }
            , up_{ 0, 1, 0 }
            , horizontal_angle_{ 0 }
            , vertical_angle_{ 0 }
        {
            init();
        }

        Camera(int width, int height, const glm::vec3 &position,
               const glm::vec3 &target, const glm::vec3 &up)
            : width_{ width }
            , height_{ height }
            , position_{ position }
            , target_{ glm::normalize(target) }
            , up_{ glm::normalize(up) }
            , horizontal_angle_{ 0 }
            , vertical_angle_{ 0 }
        {
            init();
        }

        void init();

        const glm::vec3 &position()
        {
            return position_;
        }

        const glm::vec3 &target()
        {
            return target_;
        }

        const glm::vec3 &up()
        {
            return up_;
        }

        float mouse_x()
        {
            return mouse_position_.x;
        }

        float mouse_y()
        {
            return mouse_position_.y;
        }

        bool on_keypress(int key);
        void on_mouse(int x, int y);
        void on_render();
        void update();

    private:
        int width_;
        int height_;
        glm::vec3 position_;
        glm::vec3 target_;
        glm::vec3 up_;
        glm::vec2 mouse_position_;
        float horizontal_angle_;
        float vertical_angle_;
        bool ulrd_[4];
    };
} // namespace mygl