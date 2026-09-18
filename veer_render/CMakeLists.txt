cmake_minimum_required(VERSION 3.21..4.4)

set(VEER_RENDER_SOURCES
    src/buffer.cpp
    src/asset.cpp
    src/graphics.cpp
    src/mesh.cpp
    src/shader.cpp
    src/texture.cpp
    src/vk_allocator.cpp
    src/vk_command.cpp
    src/vk_context.cpp
    src/vk_instance.cpp
    src/vk_device.cpp
    src/vk_pipeline.cpp
    src/vk_queue.cpp
    src/vk_surface.cpp
    src/vk_swapchain.cpp
    src/window.cpp

    src/asset_manager.cpp
    src/gltf.cpp
    src/ktx2.cpp
)

set(VEER_RENDER_HEADERS
    include/veer_render/asset.h
    include/veer_render/buffer.h
    include/veer_render/graphics.h
    include/veer_render/mesh.h
    include/veer_render/shader.h
    include/veer_render/texture.h
    include/veer_render/vk_allocator.h
    include/veer_render/vk_command.h
    include/veer_render/vk_context.h
    include/veer_render/vk_device.h
    include/veer_render/vk_instance.h
    include/veer_render/vk_pipeline.h
    include/veer_render/vk_queue.h
    include/veer_render/vk_surface.h
    include/veer_render/vk_swapchain.h
    include/veer_render/window.h

    include/internal/asset_manager.h
    include/internal/gltf.h
    include/internal/ktx2.h
)

add_library(veer_render STATIC
    ../extern/tinygltf/tiny_gltf_v3.h
    $<TARGET_OBJECTS:SDL3-static>
    $<TARGET_OBJECTS:ktx>
    $<TARGET_OBJECTS:basisu_encoder>
    $<TARGET_OBJECTS:dfdutils>
    ${VEER_RENDER_SOURCES}
    ${VEER_RENDER_HEADERS}
)

veer_target_common(veer_render)

target_include_directories(veer_render
    PRIVATE
        ../extern
)

target_link_libraries(
    veer_render
    PUBLIC
        veer_core
        Vulkan::Vulkan
        slang::slang
    PRIVATE
        $<BUILD_LOCAL_INTERFACE:VulkanMemoryAllocator>
        $<BUILD_LOCAL_INTERFACE:glm-header-only>
        $<BUILD_LOCAL_INTERFACE:SDL3-static>
        $<BUILD_LOCAL_INTERFACE:ktx>
)
