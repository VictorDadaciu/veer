cmake_minimum_required(VERSION 3.21..4.4)

set(VEER_RENDER_SOURCES
    src/buffer.cpp
    src/asset.cpp
    src/graphics.cpp
    src/mesh.cpp
    src/vk_allocator.cpp
    src/vk_context.cpp
    src/vk_instance.cpp
    src/vk_device.cpp
    src/vk_queue.cpp
    src/vk_surface.cpp
    src/vk_swapchain.cpp
    src/window.cpp
)

set(VEER_RENDER_HEADERS
    include/veer_render/asset.h
    include/veer_render/buffer.h
    include/veer_render/graphics.h
    include/veer_render/mesh.h
    include/veer_render/vk_allocator.h
    include/veer_render/vk_context.h
    include/veer_render/vk_device.h
    include/veer_render/vk_instance.h
    include/veer_render/vk_queue.h
    include/veer_render/vk_surface.h
    include/veer_render/vk_swapchain.h
    include/veer_render/window.h
)

add_library(render STATIC
    ../extern/tinygltf/tiny_gltf_v3.h
    $<TARGET_OBJECTS:SDL3-static>
    ${VEER_RENDER_SOURCES}
    ${VEER_RENDER_HEADERS}
)

veer_target_common(render)

target_include_directories(render
    PRIVATE
        ../extern/SDL3/include
        ../extern/vma/include
        ../extern/glm
        ../extern
)

target_link_libraries(render PUBLIC
    core
    Vulkan::Vulkan
)

