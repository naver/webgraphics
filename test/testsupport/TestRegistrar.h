#pragma once

#include "TestRunner.h"

class TestRegistrar
{
public:
    ~TestRegistrar() = default;

    static void RegisterTests(TestRunner&);

private:
    TestRegistrar() = default;
};
