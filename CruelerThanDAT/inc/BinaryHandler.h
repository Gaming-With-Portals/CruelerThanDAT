#pragma once
#include "pch.hpp"

class BinaryReader {
public:
	BinaryReader(std::vector<char>& data, bool bigEndian = false);

	uint64_t ReadUINT64();
	uint32_t ReadUINT32();
	int32_t ReadINT32();
	std::vector<uint32_t> ReadUINT32Array(int count);
	int8_t ReadINT8();
	float ReadFloat();
	int16_t ReadINT16();
	uint16_t ReadUINT16();
	std::vector<uint16_t> ReadUINT16Array(int count);
	std::string ReadString(int length);
	std::string ReadNullTerminatedString();
	std::vector<char> ReadBytes(size_t size);
	bool EndOfBuffer() const;
	size_t Tell();
	void Reset();
	void Seek(size_t position);
	void Skip(size_t position);
	void SetEndianess(bool isBig);
	size_t GetSize();

	template <typename T>
	T ReadStruct() {
		T out;
		std::vector<char> raw = ReadBytes(sizeof(T));
		std::memcpy(&out, raw.data(), sizeof(T));
		return out;
	}

	template <typename T>
	std::vector<T> ReadStructs(int count) {
		std::vector<T> output;
		for (int x = 0; x < count; x++) {
			output.push_back(ReadStruct<T>());
		}
		return output;
	}
private:
	std::vector<char>& data;
	size_t offset;
	bool isBigEndian;

	// Function to reverse the endian-ness (for big-endian <-> little-endian conversion)
	template <typename T>
	T ReverseEndian(T value) {
		char* ptr = reinterpret_cast<char*>(&value);
		std::reverse(ptr, ptr + sizeof(T));
		return value;
	}
};

class BinaryWriter {
public:
	BinaryWriter(bool bigEndian = false);
	
	void WriteByteZero();
	void WriteBytes(std::vector<char> indata);
	void WriteString(const std::string& value);
	void WriteUINT32(uint32_t value);
	void WriteUINT16(uint16_t value);
	void WriteINT32(int32_t value);
	void WriteINT16(int16_t value);
	void WriteFloat(float value);
	std::vector<char> GetData();
	bool EndOfBuffer() const;
	size_t Tell();
	void Reset();
	void Seek(size_t position);
	void SetEndianess(bool isBig);
private:
	std::vector<char> data;
	size_t offset;
	bool isBigEndian;

	// Function to reverse the endian-ness (for big-endian <-> little-endian conversion)
	template <typename T>
	T ReverseEndian(T value) {
		char* ptr = reinterpret_cast<char*>(&value);
		std::reverse(ptr, ptr + sizeof(T));
		return value;
	}
};
