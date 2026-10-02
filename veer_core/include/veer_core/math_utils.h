#pragma once

#include <glm/glm.hpp>

#include <limits>

namespace ve
{
template<typename math_type_t>
struct constants;

template<>
struct constants<glm::vec2>
{
    static constexpr glm::vec2 origin = glm::vec2();
    static constexpr glm::vec2 up = glm::vec2(0.f, 1.f);
    static constexpr glm::vec2 down = glm::vec2(0.f, -1.f);
    static constexpr glm::vec2 right = glm::vec2(1.f, 0.f);
    static constexpr glm::vec2 left = glm::vec2(-1.f, 0.f);
    static constexpr glm::vec2 min = glm::vec2(std::numeric_limits<float>::min());
    static constexpr glm::vec2 max = glm::vec2(std::numeric_limits<float>::max());
};

template<>
struct constants<glm::vec3>
{
    static constexpr glm::vec3 origin = glm::vec3();
    static constexpr glm::vec3 up = glm::vec3(0.f, 1.f, 0.f);
    static constexpr glm::vec3 down = glm::vec3(0.f, -1.f, 0.f);
    static constexpr glm::vec3 right = glm::vec3(1.f, 0.f, 0.f);
    static constexpr glm::vec3 left = glm::vec3(-1.f, 0.f, 0.f);
    static constexpr glm::vec3 forward = glm::vec3(0.f, 0.f, 1.f);
    static constexpr glm::vec3 backward = glm::vec3(0.f, 0.f, -1.f);
    static constexpr glm::vec3 min = glm::vec3(std::numeric_limits<float>::min());
    static constexpr glm::vec3 max = glm::vec3(std::numeric_limits<float>::max());
};

template<>
struct constants<glm::mat2>
{
    static constexpr glm::mat2 zero = glm::mat2();
    static constexpr glm::mat2 identity = glm::mat2(1);
};

template<>
struct constants<glm::mat3>
{
    static constexpr glm::mat3 zero = glm::mat3();
    static constexpr glm::mat3 identity = glm::mat3(1);
};

template<>
struct constants<glm::mat4>
{
    static constexpr glm::mat4 zero = glm::mat4();
    static constexpr glm::mat4 identity = glm::mat4(1);
};

namespace math
{
    glm::vec2 swapped(const glm::vec2&) noexcept;
}
}