#include <cppunit.h>

#include <veer_core/bits.h>

#include <cstddef>

struct bits_tests : public Cppunit
{
    void test_get()
    {
        {
            uint8_t flags = 0;
            CHECKT(!ve::bits::get(flags, 0));
        }
        {
            uint16_t flags = 1 << 6;
            CHECKT(!ve::bits::get(flags, 4));
            CHECKT(ve::bits::get(flags, 6));
        }
        {
            uint32_t flags = 0xff00ff00;
            CHECKT(!ve::bits::get(flags, 7));
            CHECKT(ve::bits::get(flags, 15));
            CHECKT(!ve::bits::get(flags, 23));
            CHECKT(ve::bits::get(flags, 31));
        }
    }

    void test_set()
    {
        {
            CHECK(ve::bits::set(0u, 0), 1);
            CHECK(ve::bits::set(1u, 4), 17);
        }
        {
            uint16_t flags = ve::bits::set(0u, 6);
            CHECKT(ve::bits::get(flags, 6));
            CHECKT(!ve::bits::get(flags, 0));
        }
    }

    void test_unset()
    {
        {
            CHECK(ve::bits::unset(0xffu, 0), 0xfe);
            CHECK(ve::bits::unset(0xfeu, 0), 0xfe);
            CHECK(ve::bits::unset(0xfeu, 4), 0xee);
        }
        {
            uint16_t flags = ve::bits::unset(0xffffu, 6);
            CHECKT(!ve::bits::get(flags, 6));
            CHECKT(ve::bits::get(flags, 0));
        }
    }

    void test_mask()
    {
        CHECK(ve::bits::mask(16), 0x000000000000ffffzu);
        CHECK(ve::bits::mask(16, 1), 0x000000000001fffezu);
        CHECK(ve::bits::mask<uint8_t>(2, 3), 0x18u);
        CHECK(ve::bits::mask<uint16_t>(16), 0xffffu);
    }

    void test_list() override
    {
        test_get();
        test_set();
        test_unset();
        test_mask();
    }
};
