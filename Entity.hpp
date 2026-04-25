#pragma once
#include "Offsets.hpp"
#include "Structs.hpp"
#include "Memory/Interface.hpp"



inline std::vector<uint64_t> GetEntityList(uint64_t BaseAddr)
{
    std::vector<uint64_t> entities;

    uint64_t localPlayer = I::Read<uint64_t>(BaseAddr + OFF_LOCAL_PLAYER);

    for (int i = 0; i < 15000; i++)
    {
        uint64_t entity = I::Read<uint64_t>(BaseAddr + OFF_ENTITYLIST + (i * 0x20));
        if (!entity)
            continue;

        
        if (localPlayer && entity == localPlayer)
            continue;

        
        if (entity < 0x10000 || entity > 0x7FFFFFFFFFFFFFFF)
            continue;

        
        if (entity >= BaseAddr && entity < BaseAddr + 0x10000000)
            continue;

        entities.push_back(entity);
    }

    return entities;
}
