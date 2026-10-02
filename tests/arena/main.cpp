#include <veer_core/db.h>
#include <veer_core/log.h>
#include <veer_core/math_utils.h>

#include <veer_render/assets.h>
#include <veer_render/camera.h>
#include <veer_render/inputs.h>
#include <veer_render/mesh.h>
#include <veer_render/graphics.h>
#include <veer_render/shader.h>
#include <veer_render/texture.h>
#include <veer_render/timing.h>
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
    auto res = ve::shader::load("tests/assets/simple_model.slang");
    if (!res)
        return std::numeric_limits<uint8_t>::max();
    return static_cast<uint8_t>(*res);
}

uint8_t load_asset(const std::string& path)
{
    auto res = ve::assets::load(path);
    if (!res)
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
        row.cell<position3>() = glm::vec3(4.5f * i, 0, 0);
        row.cell<tex_i>() = tex_index;
        row.cell<mesh_i>() = mesh_index;
        row.cell<shader_i>() = shader_index;
        db::push_back<objects>(row);
    }
}

static void handle_cam(ve::simple_fps_camera& cam)
{
    const auto& mouse_rel = ve::inputs::mouse_rel();
    glm::vec2 swapped(mouse_rel.y, -mouse_rel.x);
    cam.rotate_by(glm::radians(0.1f * swapped));

    int8_t x = ve::inputs::is_pressed(ve::keycode::d) - ve::inputs::is_pressed(ve::keycode::a);
    int8_t y = ve::inputs::is_pressed(ve::keycode::space) - ve::inputs::is_pressed(ve::keycode::c);
    int8_t z = ve::inputs::is_pressed(ve::keycode::w) - ve::inputs::is_pressed(ve::keycode::s);
    if (!x && !y && !z) return;
    cam.translate_by(
        ve::time::dt() *
        (7.5f + 10.f * ve::inputs::is_pressed(ve::keycode::left_shift)) *
        glm::normalize(glm::mat3(cam.right(), cam.up(), cam.forward()) * glm::vec3(x, y, z))
    );
}

void run_gfx()
{
    if (!ve::failed(ve::gfx::init()))
    {
        ve::window win;
        auto _ = win.open("Arena");
        initialize_db();
        auto cam = ve::simple_fps_camera(win.aspect_ratio()).translate_to(10.f * ve::math::forward);

        // TODO: actually handle correctly
        auto row = db::row<objects>(0);
        auto& pipeline = ve::shader::get(row.cell<shader_i>());
        auto& tex = ve::assets::texture(row.cell<tex_i>());

        size_t frames{};
        auto start_time = ve::time::now();
        while (true) // TODO: handle loop better
        {
            ve::inputs::process();
            if (ve::inputs::quit() || ve::inputs::just_pressed(ve::keycode::escape))
                break;

            if (ve::inputs::just_double_pressed(ve::mouse_button::left))
                ve::trace("Just double-clicked mouse button left");
            if (ve::inputs::is_holding(ve::mouse_button::right))
                ve::trace("Holding right mouse button");
            
            handle_cam(cam);

            auto& frame_data = pipeline.current_frame();
            frame_data.globals.resize(1);
            frame_data.globals[0].view = cam.view();
            frame_data.globals[0].proj = cam.proj();
            frame_data.globals.commit();

            frame_data.model.resize(db::count<objects>());
            db::iterate<SELECT(position3, tex_i), FROM(objects)>(
                [&frame_data](const auto& e, auto& pos, auto& ti)
                {
                    size_t i = static_cast<size_t>(e);
                    pos.y = std::sin(ve::time::seconds(ve::time::now()) + i);
                    auto& model = frame_data.model[i];
                    model.transform = glm::rotate(glm::translate(glm::mat4(1), pos), glm::radians(90.f), ve::math::forward);
                    model.tex_index = ti;
                }
            );
            frame_data.model.commit();

            auto cb = ve::gfx::begin_draw(win);
            if (!cb)
            {
                ve::error("Failed to begin draw");
                break;
            }
            cb->bind_pipeline(pipeline);
            cb->bind_texture(tex, pipeline.layout);
            cb->push_constants(pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT, &frame_data.globals.address);
            db::iterate<SELECT(position3, mesh_i), FROM(objects)>(
                [&cb, &frame_data, &pipeline](const auto& e, auto& pos, auto& mi)
                {
                    size_t i = static_cast<size_t>(e);
                    auto address = frame_data.model.address + i * sizeof(ve::model_data);
                    cb->push_constants(pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT, &address, sizeof(VkDeviceAddress));
                    cb->draw_mesh(ve::assets::mesh(mi));
                }
            );
            if (ve::failed(ve::gfx::end_draw(win, *cb)))
            {
                ve::error("Failed to end draw");
                break;
            }

            ve::gfx::advance_frame();
            ++frames;
        }
        ve::trace("# frames: {}", frames);
        ve::trace("Avg. frame time: {}", ve::time::duration(start_time, ve::time::now()) / frames);
        ve::trace("Avg. fps: {}", frames / ve::time::duration(start_time, ve::time::now()));
        win.close();
        ve::gfx::destroy();
    }
}
}

int main()
{
    ve::log::init("arena", ve::log::level::debug);  

    if (false) run_db();
    run_gfx();
    
    return 0;
}