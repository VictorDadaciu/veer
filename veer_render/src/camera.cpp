#include "camera.h"

#include <veer_core/log.h>

#include <glm/gtc/matrix_transform.hpp>

namespace ve
{
constexpr float threshold = glm::radians(82.f);

const glm::mat4& simple_fps_camera::view() const
{
    return view_mat;
}

const glm::mat4& simple_fps_camera::recalculate_view()
{
    rot_mat = glm::rotate(math::ident4, rot.y, math::up3) * glm::rotate(math::ident4, rot.x, math::right3);
    return view_mat = glm::inverse(glm::translate(math::ident4, pos) * rot_mat);
}

const glm::mat4& simple_fps_camera::proj() const
{
    return proj_mat;
}

const glm::mat4& simple_fps_camera::recalculate_proj(float aspect_ratio)
{
    return proj_mat = glm::perspective(fov, aspect_ratio, near_plane, far_plane);
}

simple_fps_camera& simple_fps_camera::translate_by(const glm::vec3& offset)
{
    pos += offset;
    recalculate_view();
    return *this;
}

simple_fps_camera& simple_fps_camera::translate_to(const glm::vec3& position)
{
    pos = position;
    recalculate_view();
    return *this;
}

simple_fps_camera& simple_fps_camera::rotate_by(const glm::vec2& offset)
{
    rot += offset;
    rot.x = glm::clamp(rot.x, -threshold, threshold);
    recalculate_view();
    return *this;
}

simple_fps_camera& simple_fps_camera::rotate_to(const glm::vec2& rotation)
{
    rot = rotation;
    rot.x = glm::clamp(rot.x, threshold, threshold);
    recalculate_view();
    return *this;
}

glm::vec3 simple_fps_camera::forward() const
{
    return -backward();
}

glm::vec3 simple_fps_camera::backward() const
{
    return rot_mat[2];
}

glm::vec3 simple_fps_camera::right() const
{
    return rot_mat[0];
}

glm::vec3 simple_fps_camera::left() const
{
    return -right();
}

glm::vec3 simple_fps_camera::up() const
{
    return -down();
}

glm::vec3 simple_fps_camera::down() const
{
    return rot_mat[1];
}
}