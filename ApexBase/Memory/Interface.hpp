#pragma once
#include "driver.hpp"


namespace I
{
    template <typename T>
    T Read(uintptr_t address)
    {
        return kernel->read_t<T>(address);
    }

    template <typename T>
    bool Write(uintptr_t address, T value)
    {
        return kernel->write_t<T>(address, value);
    }
}
