#pragma once
#include "pch.hpp"

#ifndef HASH_DATA_CONTAINER_H
#define HASH_DATA_CONTAINER_H


class HashDataContainer {
public:
    std::vector<int> Hashes;
    std::vector<short> Indices;
    std::vector<short> Offsets;
    int Shift;
    int StructSize;
};

#endif // HASH_DATA_CONTAINER_H