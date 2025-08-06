#include "pch.hpp"
#include "Criware.h"

std::variant<
	std::monostate,
	uint8_t,
	uint16_t,
	uint32_t,
	uint64_t,
	float,
	std::string,
	std::vector<char>
> CriRow::GetValue() const {
	switch (type) {
	case 0:
	case 1: return uint8;
	case 2:
	case 3: return uint16;
	case 4:
	case 5: return uint32;
	case 6:
	case 7: return uint64;
	case 8:     return ufloat;
	case 0xA:   return str;
	case 0xB:   return data;
	default:    return std::monostate{};
	}
}