#pragma once

#include <gtest/gtest.h>

#include <iomanip>
#include <iostream>
#include <locale>
#include <random>
#include <string>

#include "../fx.h"

#define STR(x) #x

void setupOuputStreamFocCSV(std::ostream& out);

class FxComponentFixture : public testing::Test
{
public:
    FxComponentFixture();

    virtual void SetUp() override;
    virtual void TearDown() override;

    std::string getResultFile(const string& filename);

    float32_t getRandomValue();

    random_device rd_;
    mt19937 gen_;
    uniform_real_distribution<float32_t> dist_;
};
