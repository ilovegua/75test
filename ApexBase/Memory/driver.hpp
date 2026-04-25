#pragma once
#include <Windows.h>
#include <iostream>
#include <string>
#include <vector>

#ifndef DRIVER_HPP
#define DRIVER_HPP

namespace driver
{
    class communicate_t
    {
    private:
        std::int32_t m_pid = 0;

    public:
        std::uintptr_t BaseAddress = 0;
        std::uintptr_t dtb = 0;

        bool initialize_handle()
        {
            return true;
        }

        bool attach(int a_pid)
        {
            m_pid = a_pid;
            return true;
        }

        const std::uint32_t get_process_pid(const std::wstring& proc_name)
        {
            return 1234; // your pid nigga
        }

        const std::uintptr_t get_dtb(std::uint32_t pid)
        {
            return 0x1000; // your dtb niggas
        }

        const std::uintptr_t get_image_base(const char* module_name)
        {
            return 0x400000; // the placeholder base u fuckin cuck
        }

        bool read(uintptr_t address, void* buffer, size_t size)
        {
            return false;
        }

        bool write(uintptr_t address, void* buffer, size_t size)
        {
            return false;
        }

        template <typename T>
        T read_t(const uintptr_t address)
        {
            T result{};
            return result;
        }

        template <typename T>
        bool write_t(const uintptr_t address, T value)
        {
            return false;
        }

        bool is_valid(uint64_t address) const {
            return address > 0x400000 && address < 0x7FFFFFFFFFFFFFFF;
        }
    };
}

inline driver::communicate_t* kernel = new driver::communicate_t();

inline bool is_valid(const uint64_t address) {
    return address > 0x400000 && address < 0x7FFFFFFFFFFFFFFF;
}

#endif
