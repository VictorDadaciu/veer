#pragma once

#include <veer_core/math_utils.h>

#include <glm/glm.hpp>

namespace ve
{
struct simple_fps_camera
{
    simple_fps_camera() = delete;
    simple_fps_camera(float aspect_ratio)
    {
        view_mat = recalculate_view();
        proj_mat = recalculate_proj(aspect_ratio);
    }
    simple_fps_camera(const simple_fps_camera&) = default;
    simple_fps_camera(simple_fps_camera&&) = default;
    simple_fps_camera& operator=(const simple_fps_camera&) = default;
    simple_fps_camera& operator=(simple_fps_camera&&) = default;
    
    [[nodiscard]] const glm::mat4& view() const;
    const glm::mat4& recalculate_view();

    [[nodiscard]] const glm::mat4& proj() const;
    const glm::mat4& recalculate_proj(float);

    simple_fps_camera& translate_by(const glm::vec3&);
    simple_fps_camera& translate_to(const glm::vec3&);

    simple_fps_camera& rotate_by(const glm::vec2&);
    simple_fps_camera& rotate_to(const glm::vec2&);

    [[nodiscard]] glm::vec3 forward() const;
    [[nodiscard]] glm::vec3 backward() const;
    [[nodiscard]] glm::vec3 right() const;
    [[nodiscard]] glm::vec3 left() const;
    [[nodiscard]] glm::vec3 up() const;
    [[nodiscard]] glm::vec3 down() const;

    glm::vec3 pos{};
    glm::vec2 rot{};
    float near_plane = 0.1f;
    float far_plane = 256.f;
    float fov = glm::radians(60.f);

protected:
    glm::mat4 rot_mat;
    glm::mat4 view_mat;
    glm::mat4 proj_mat;
};
}