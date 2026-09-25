#include <veer_core/log.h>
#include <veer_core/db.h>

#include <veer_render/assets.h>
#include <veer_render/inputs.h>
#include <veer_render/mesh.h>
#include <veer_render/graphics.h>
#include <veer_render/shader.h>
#include <veer_render/vk_context.h>
#include <veer_render/window.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

DERIVE_PROP(position3, PROPERTY_ROOT(glm::vec3));
DERIVE_PROP(speed, PROPERTY_ROOT(float));
DERIVE_PROP(range, PROPERTY_ROOT(float));
DERIVE_PROP(health, PROPERTY_ROOT(size_t));
DERIVE_PROP(damage, PROPERTY_ROOT(size_t));
DERIVE_PROP(tex_i, PROPERTY_ROOT(size_t));
DERIVE_PROP(mesh_i, PROPERTY_ROOT(size_t));
DERIVE_PROP(shader_i, PROPERTY_ROOT(size_t));

using defenders = TABLE(
    COLUMN_IMPL(position3),
    COLUMN_IMPL(range),
    COLUMN_IMPL_AS(health, uint16_t),
    COLUMN_IMPL_AS(damage, uint8_t)
);

using attackers = TABLE(
    COLUMN_IMPL(position3),
    COLUMN_IMPL(speed),
    COLUMN_IMPL_AS(health, uint8_t),
    COLUMN_IMPL_AS(damage, uint16_t)
);

using objects = TABLE(
    COLUMN_IMPL(position3),
    COLUMN_IMPL_AS(tex_i, uint8_t),
    COLUMN_IMPL_AS(mesh_i, uint8_t),
    COLUMN_IMPL_AS(shader_i, uint8_t)
);

using db = ve::database<
    defenders,
    attackers,
    objects
>;

namespace
{
void create_defenders(size_t n)
{
    for (size_t i = 0; i < n; ++i)
    {
        defenders::row row;
        row.cell<position3>() = glm::vec3(i, i, 0);
        row.cell<range>() = 10 + i;
        row.cell<health>() = 1000;
        row.cell<damage>() = 50;
        db::push_back(row);
    }
}

void create_attackers(size_t n)
{
    for (size_t i = 0; i < n; ++i)
    {
        attackers::row row;
        row.cell<position3>() = glm::vec3(n - i, n - i, 0);
        row.cell<speed>() =  3;
        row.cell<health>() = 150;
        row.cell<damage>() = 1000;
        db::push_back(row);
    }
}

void run_db()
{
    create_defenders(3);
    create_attackers(15);
    ve::info("Starting battle!");

    ve::info("Shooting defenders' arrows!");
    db::iterate<SELECT(range, damage), FROM()>([](const auto& i, auto& rng, auto& dmg)
    {
        dmg += rng;
        ve::info("defender: {}, {}", dmg, rng);
    });
    
    ve::info("Calculating attackers' position!");
    db::iterate<SELECT(position3, speed), FROM()>([](const auto& i, auto& pos, auto& spd)
    {
        pos += glm::vec3(10, 10, 10);
        spd += static_cast<float>(i);
        ve::info("attacker: ({}, {}, {}), {}", pos.x, pos.y, pos.z, spd);
    });

    ve::info("Calculating health!");
    db::iterate<SELECT(health), FROM()>([](const auto& i, auto& hp)
    {
        hp -= 5 + static_cast<PARAM_TYPE(hp)>(i);
        IF_DECLTYPE_IS(i, ve::row_index<defenders>)
        {
            ve::info("defender: {}", hp);
        }
        ELSE_IF_DECLTYPE_IS(i, ve::row_index<attackers>)
        {
            ve::info("attacker: {}", hp);
        }
    });

    ve::info("Finished battle!");
}

uint8_t load_shader()
{
    auto res = ve::shader::load("tests/assets/triangle.slang");
    if (!res.has_value())
        return std::numeric_limits<uint8_t>::max();
    return static_cast<uint8_t>(*res);
}

uint8_t load_asset(const std::string& path)
{
    auto res = ve::assets::load(path);
    if (!res.has_value())
        return std::numeric_limits<uint8_t>::max();
    return static_cast<uint8_t>((*res)[0].index);
}

void initialize_db()
{
    // /home/victordadaciu/workspace/veer/
    auto shader_index = load_shader();
    auto tex_index = load_asset("tests/assets/dog.ktx2");
    auto mesh_index = load_asset("tests/assets/box.glb");
    for (size_t i = 0; i < 3; ++i)
    {
        objects::row row{};
        row.cell<position3>() = glm::vec3(i, 0, 0);
        row.cell<tex_i>() = tex_index;
        row.cell<mesh_i>() = mesh_index;
        row.cell<shader_i>() = shader_index;
        db::push_back<objects>(row);
    }
}

void run_gfx()
{
    if (ve::gfx::init() == ve::error_code::success)
    {
        ve::window win;
        auto _ = win.open("Arena");
        auto& ctx = ve::vk_context::get();
        initialize_db();
        for (auto& frame : ctx.frames)
        {
            frame.data.data.proj = glm::perspective(glm::radians(60.0f), win.aspect_ratio(), 0.1f, 32.0f);
            frame.data.data.view = glm::translate(glm::mat4(1.0f), glm::vec3(-1.f, 0.f, 0.f));
        }
        // TODO: actually handle correctly
        auto row = db::row<objects>(0);
        auto& pipeline = ve::shader::get(row.cell<shader_i>());
        auto& mesh = ve::assets::mesh(row.cell<mesh_i>());
        auto& tex = ve::assets::texture(row.cell<tex_i>());

        while (true) // TODO: handle loop better
        {
            ve::inputs::process();
            if (ve::inputs::quit_requested())
                break;
            
            auto& frame = ctx.current_frame();
            db::iterate<SELECT(position3, tex_i, mesh_i), FROM(objects)>
            (
                [&frame](const auto& e, auto& pos, auto& tex, auto& mesh)
                {
                    size_t i = static_cast<size_t>(e);
                    frame.data.data.model[i] = glm::translate(glm::mat4(1.f), pos);
                }
            );
            frame.commit();

            ve::vk_command_buffer cb{};
            {
                auto res = ve::gfx::begin_draw(win);
                if (!res.has_value())
                {
                    ve::error("Failed to begin draw");
                    break;
                }
                cb = std::move(*res);
            }
            cb.bind_pipeline(pipeline);
            cb.bind_texture(tex, pipeline.layout);
            // TODO: make nicer
            vkCmdPushConstants(cb, pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(VkDeviceAddress), &frame.data.device_address);
            cb.draw_mesh(mesh);

            ctx.advance_frame();
        }
        win.close();
        ve::gfx::destroy();
    }
}
}

int main()
{
    ve::log::init("arena", ve::log::level::trace);

    run_db();
    run_gfx();
    
    return 0;
}