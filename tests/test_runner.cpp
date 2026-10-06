#include "unit/bits_tests.cpp"
#include "unit/db_tests.cpp"
#include "unit/utils_tests.cpp"

int main()
{
    db_tests().run();
    bits_tests().run();
    utils_tests().run();
    return 0;
}