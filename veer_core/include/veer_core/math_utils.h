#pragma once

#include <glm/glm.hpp>

// TODO: make this nicer
namespace ve::math
{
constexpr glm::mat2 ident2 = glm::mat2(1);
constexpr glm::mat3 ident3 = glm::mat3(1);
constexpr glm::mat4 ident4 = glm::mat4(1);

constexpr glm::vec2 origin2 = glm::vec2();
constexpr glm::vec2 up2 = glm::vec2(0.f, 1.f);
constexpr glm::vec2 right2 = glm::vec2(1.f, 0.f);
constexpr glm::vec2 left2 = glm::vec2(-1.f, 0.f);
constexpr glm::vec2 down2 = glm::vec2(0.f, -1.f);

constexpr glm::vec3 origin3 = glm::vec3();
constexpr glm::vec3 up3 = glm::vec3(0.f, 1.f, 0.f);
constexpr glm::vec3 right3 = glm::vec3(1.f, 0.f, 0.f);
constexpr glm::vec3 left3 = glm::vec3(-1.f, 0.f, 0.f);
constexpr glm::vec3 down3 = glm::vec3(0.f, -1.f, 0.f);
constexpr glm::vec3 forward = glm::vec3(0.f, 0.f, 1.f);
constexpr glm::vec3 backward = glm::vec3(0.f, 0.f, -1.f);
}