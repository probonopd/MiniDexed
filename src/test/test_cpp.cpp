#include <gtest/gtest.h>

#include "../fx_components.h"

int nb = 0;

int NbIteration() {
    nb++;
    return 3;
}

TEST(Cpp, NbCallsInUpperBoudariesInForLoop)
{
    for(int i = 0; i < NbIteration(); ++i)
    {
        // Does something
    }
    EXPECT_EQ(nb, 4);
}

#define CLASS_INIT(clazz) clazz::StaticInit()
class StaticCtorTest
{
private:
    static int n_;

public:
    int i_;

    static int StaticInit()
    {
        static int i = 0;
        i++;

        StaticCtorTest::n_ = 2;

        return i;
    }

    StaticCtorTest() : i_(0)
    {
        static int init = CLASS_INIT(StaticCtorTest);
        static int NB = 0;
        EXPECT_EQ(init, 1);

        this->i_ = ++NB;

        EXPECT_EQ(StaticCtorTest::n_, 2);
    }

    ~StaticCtorTest()
    {
    }
};

int StaticCtorTest::n_ = 0;

TEST(Cpp, StaticCtorTest)
{
    StaticCtorTest obj1;
    StaticCtorTest obj2;
    StaticCtorTest obj3;

    EXPECT_EQ(obj1.i_, 1);
    EXPECT_EQ(obj2.i_, 2);
    EXPECT_EQ(obj3.i_, 3);
}
