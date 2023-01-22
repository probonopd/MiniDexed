#include "test_fixture.h"

#include <iomanip>
#include <iostream>
#include <sstream>
#include <locale>
#include <random>
#include <string>

void setupOuputStreamFocCSV(std::ostream& out)
{
    struct comma_separator : numpunct<char>
    {
        virtual char do_decimal_point() const override { return ','; }
    };

    out.imbue(locale(out.getloc(), new comma_separator));
    out << fixed << showpoint;
}

FxComponentFixture::FxComponentFixture() :
    testing::Test(),
    gen_(rd_()),
    dist_(-1.0f, 1.0f)
{
}

void FxComponentFixture::SetUp()
{
}

void FxComponentFixture::TearDown()
{
}

string FxComponentFixture::getResultFile(const std::string& filename)
{
    return std::string(STR(OUTPUT_FOLDER)) + "/" + filename;
}

float32_t FxComponentFixture::getRandomValue()
{
    return this->dist_(this->gen_);
}
