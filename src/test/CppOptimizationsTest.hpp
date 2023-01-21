#pragma once

int nb = 0;

int NbIteration() {
    nb++;
    return 3;
}


TEST(CppOptimization, NbCallsInUpperBoudariesInForLoop)
{
    for(int i = 0; i < NbIteration(); ++i)
    {
        // Does something
    }
    EXPECT_EQ(nb, 4);
}
