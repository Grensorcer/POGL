#include "camera.hh"
#include <glm/geometric.hpp>
#include <iostream>

#define STEP_SIZE 0.3f

namespace mygl
{
    void Camera::init()
    {
        auto horizontal_target =
            glm::normalize(glm::vec3(target_.x, 0, target_.z));
        if (horizontal_target.z >= 0.0f)
        {
            if (horizontal_target.x >= 0.0f)
            {
                horizontal_angle_ =
                    360.0f - glm::degrees(asin(horizontal_target.z));
            }
            else
            {
                horizontal_angle_ =
                    180.0f + glm::degrees(asin(horizontal_target.z));
            }
        }
        else
        {
            if (horizontal_target.x >= 0.0f)
            {
                horizontal_angle_ = glm::degrees(asin(-horizontal_target.z));
            }
            else
            {
                horizontal_angle_ =
                    180.0f - glm::degrees(asin(-horizontal_target.z));
            }
        }

        vertical_angle_ = -glm::degrees(asin(target_.y));

        ulrd_[0] = false;
        ulrd_[1] = false;
        ulrd_[2] = false;
        ulrd_[3] = false;
        mouse_position_.x = width_ / 2;
        mouse_position_.y = width_ / 2;

        // glutWarpPointer(mouse_position_.x, mouse_position_.y);
    }
    bool Camera::on_keypress(int key)
    {
        bool res = true;
        switch (key)
        {
        case GLUT_KEY_UP: {
            position_ += target_ * STEP_SIZE;
            break;
        }
        case GLUT_KEY_LEFT: {
            glm::vec3 l = glm::normalize(glm::cross(up_, target_)) * STEP_SIZE;
            position_ += l;
            break;
        }
        case GLUT_KEY_RIGHT: {
            glm::vec3 r = glm::normalize(glm::cross(target_, up_)) * STEP_SIZE;
            position_ += r;
            break;
        }
        case GLUT_KEY_DOWN: {
            position_ -= target_ * STEP_SIZE;
            break;
        }
        default:
            res = false;
        }

        return res;
    }

    void Camera::on_mouse(int x, int y)
    {
        const int dx = x - mouse_position_.x;
        const int dy = y - mouse_position_.y;
        mouse_position_.x = x;
        mouse_position_.y = y;
        horizontal_angle_ -= dx / 10.f;
        vertical_angle_ += dy / 10.f;

        if (!dx)
        {
            if (x <= 10)
                ulrd_[1] = true;
            else if (x >= (width_ - 10))
                ulrd_[2] = true;
        }
        else
        {
            ulrd_[1] = false;
            ulrd_[2] = false;
        }

        if (!dy)
        {
            if (y <= 10)
                ulrd_[0] = true;
            else if (y >= (height_ - 10))
                ulrd_[3] = true;
        }
        else
        {
            ulrd_[0] = false;
            ulrd_[3] = false;
        }

        update();
    }

    void Camera::on_render()
    {
        bool update_status = false;
        if (ulrd_[1])
        {
            horizontal_angle_ += 0.5f;
            update_status = true;
        }
        else if (ulrd_[2])
        {
            horizontal_angle_ -= 0.5f;
            update_status = true;
        }

        if (ulrd_[0] && vertical_angle_ > -90.f)
        {
            update_status = true;
            vertical_angle_ -= 0.5f;
        }
        else if (ulrd_[3] && vertical_angle_ < 90.f)
        {
            update_status = true;
            vertical_angle_ += 0.5f;
        }

        if (update_status)
            update();
    }

    void Camera::update()
    {
        const glm::vec3 v{ 0.f, 1.f, 0.f };
        glm::mat4 horizontal_rot{ 1.f };
        glm::mat4 vertical_rot{ 1.f };

        horizontal_rot =
            glm::rotate(horizontal_rot, glm::radians(horizontal_angle_), v);

        auto view = glm::vec3(1.f, 0.f, 0.f);
        view = glm::normalize(glm::vec3(horizontal_rot * glm::vec4(view, 1.f)));

        const glm::vec h = normalize(glm::cross(v, view));
        vertical_rot =
            glm::rotate(vertical_rot, glm::radians(vertical_angle_), h);
        view = glm::normalize(glm::vec3(vertical_rot * glm::vec4(view, 1.f)));

        target_ = view;
        up_ = normalize(glm::cross(target_, h));
    }
} // namespace mygl