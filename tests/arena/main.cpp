#include <veer_core/log.h>
#include <veer_core/db.h>

#include <veer_render/asset.h>
#include <veer_render/mesh.h>
#include <veer_render/graphics.h>
#include <veer_render/vk_shader.h>
#include <veer_render/window.h>

#include <glm/glm.hpp>

DERIVE_PROP(position3, PROPERTY_ROOT(glm::vec3));
DERIVE_PROP(speed, PROPERTY_ROOT(float));
DERIVE_PROP(range, PROPERTY_ROOT(float));
DERIVE_PROP(health, PROPERTY_ROOT(size_t));
DERIVE_PROP(damage, PROPERTY_ROOT(size_t));

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

using db = ve::database<
    defenders,
    attackers
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

void load_box_mesh()
{
    auto assets = *ve::assets::load("tests/assets/box.glb");
    for (const auto& asset : assets)
    {
        ve::mesh& mesh = ve::assets::mesh(asset.index);
        ve::trace("Loaded \"" + mesh.name() + "\"");
        auto _ = mesh.upload_to_gpu();
    }
}

void load_triangle_shader()
{
    ve::vk_shader_module shader{};
    shader.init("tests/assets/triangle.slang");
    shader.destroy();
}

void run_gfx()
{
    if (ve::gfx::init() == ve::error_code::success)
    {
        load_box_mesh();
        load_triangle_shader();
        {
            ve::window win; 
            auto _ = win.open("Arena");
            static size_t it = 0;
            while (true)
                if (++it >= 1000)
                    break;
            win.close();
        }
        ve::assets::unload_all();
        ve::gfx::destroy();
    }
}
}

int main()
{
    ve::log::init("arena");

    run_db();
    run_gfx();
    
    return 0;
}