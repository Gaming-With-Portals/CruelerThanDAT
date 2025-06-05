#include "pch.hpp"
#include "BinaryHandler.h"

BinaryReader::BinaryReader(std::vector<char>& data, bool bigEndian)
	: data(data), offset(0), isBigEndian(bigEndian) {}

uint64_t BinaryReader::ReadUINT64() {
	if (offset + sizeof(uint64_t) > data.size()) {
		throw std::runtime_error("Read beyond the buffer size.");
	}
	uint64_t value = *reinterpret_cast<uint64_t*>(&data[offset]);
	offset += sizeof(uint64_t);

	if (isBigEndian) {
		value = ReverseEndian(value);
	}

	return value;
}

uint32_t BinaryReader::ReadUINT32() {
	if (offset + sizeof(uint32_t) > data.size()) {
		throw std::runtime_error("Read beyond the buffer size.");
	}
	uint32_t value = *reinterpret_cast<uint32_t*>(&data[offset]);
	offset += sizeof(uint32_t);

	if (isBigEndian) {
		value = ReverseEndian(value);
	}

	return value;
}

int32_t BinaryReader::ReadINT32() {
	if (offset + sizeof(int32_t) > data.size()) {
		throw std::runtime_error("Read beyond the buffer size.");
	}
	int32_t value = *reinterpret_cast<int32_t*>(&data[offset]);
	offset += sizeof(int32_t);

	if (isBigEndian) {
		value = ReverseEndian(value);
	}

	return value;
}

std::vector<uint32_t> BinaryReader::ReadUINT32Array(int count) {
	std::vector<uint32_t> output;
	for (int i = 0; i < count; i++) {
		output.push_back(ReadUINT32());
	}
	return output;
}

int8_t BinaryReader::ReadINT8() {
	if (offset + sizeof(int8_t) > data.size()) {
		throw std::runtime_error("Read beyond the buffer size.");
	}
	int8_t value = *reinterpret_cast<int8_t*>(&data[offset]);
	offset += sizeof(int8_t);

	if (isBigEndian) {
		value = ReverseEndian(value);
	}

	return value;
}

float BinaryReader::ReadFloat() {
	if (offset + sizeof(float) > data.size()) {
		throw std::runtime_error("Read beyond the buffer size.");
	}

	float value;
	std::memcpy(&value, &data[offset], sizeof(float));
	offset += sizeof(float);

	if (isBigEndian) {
		uint32_t temp = ReverseEndian(*reinterpret_cast<uint32_t*>(&value));
		std::memcpy(&value, &temp, sizeof(float));
	}

	return value;
}

int16_t BinaryReader::ReadINT16() {
	if (offset + sizeof(int16_t) > data.size()) {
		throw std::runtime_error("Read beyond the buffer size.");
	}
	int16_t value = *reinterpret_cast<int16_t*>(&data[offset]);
	offset += sizeof(int16_t);

	if (isBigEndian) {
		value = ReverseEndian(value);
	}

	return value;
}

uint16_t BinaryReader::ReadUINT16() {
	if (offset + sizeof(uint16_t) > data.size()) {
		throw std::runtime_error("Read beyond the buffer size.");
	}
	uint16_t value = *reinterpret_cast<uint16_t*>(&data[offset]);
	offset += sizeof(uint16_t);

	if (isBigEndian) {
		value = ReverseEndian(value);
	}

	return value;
}

std::vector<uint16_t> BinaryReader::ReadUINT16Array(int count) {
	std::vector<uint16_t> output;
	for (int i = 0; i < count; i++) {
		output.push_back(ReadUINT16());
	}
	return output;
}

std::string BinaryReader::ReadString(int length) {
	std::string result;
	int x = 0;
	while (x < length) {
		x += 1;
		char ch = data[offset++];
		result += ch;
	}
	return result;
}

std::string BinaryReader::ReadNullTerminatedString() {
	std::string result;
	char ch = 'x';
	while (ch != '\0') {
		ch = data[offset++];
		if (ch != '\0') {
			result += ch;
		}
	}

	return result;
}

std::vector<char> BinaryReader::ReadBytes(size_t size) {
	if (offset + size > data.size()) {
		throw std::runtime_error("Read beyond the buffer size.");
	}
	std::vector<char> buffer(size);
	std::memcpy(buffer.data(), &data[offset], size);
	offset += size;
	return buffer;
}

bool BinaryReader::EndOfBuffer() const {
	return offset >= data.size();
}

size_t BinaryReader::Tell() {
	return offset;
}

void BinaryReader::Reset() {
	offset = 0;
}

void BinaryReader::Seek(size_t position) {
	offset = position;
}

void BinaryReader::Skip(size_t position) {
	offset += position;
}

void BinaryReader::SetEndianess(bool isBig) {
	isBigEndian = isBig;
}

size_t BinaryReader::GetSize() {
	return data.size();
}

BinaryWriter::BinaryWriter(bool bigEndian)
	: offset(0), isBigEndian(bigEndian) {}

void BinaryWriter::WriteByteZero() {
	data.push_back(static_cast<char>(0x00));
	offset += sizeof(char); 
}

void BinaryWriter::WriteBytes(std::vector<char> indata) {
	data.insert(data.end(), indata.begin(), indata.end());
	offset += indata.size();
}

void BinaryWriter::WriteString(const std::string& value) {
	uint32_t length = static_cast<uint32_t>(value.size());
	data.insert(data.end(), value.begin(), value.end());
	offset += length;
}

void BinaryWriter::WriteUINT32(uint32_t value) {
	if (isBigEndian) {
		value = ReverseEndian(value);
	}

	char bytes[sizeof(uint32_t)];
	std::memcpy(bytes, &value, sizeof(uint32_t));
	if (offset + sizeof(uint32_t) > data.size()) {
		data.resize(offset + sizeof(uint32_t));
	}

	std::memcpy(data.data() + offset, bytes, sizeof(uint32_t));
	offset += sizeof(uint32_t);
}

void BinaryWriter::WriteUINT16(uint16_t value) {
	if (isBigEndian) {
		value = ReverseEndian(value);
	}

	char bytes[sizeof(uint16_t)];
	std::memcpy(bytes, &value, sizeof(uint16_t));
	if (offset + sizeof(uint16_t) > data.size()) {
		data.resize(offset + sizeof(uint16_t));
	}

	std::memcpy(data.data() + offset, bytes, sizeof(uint16_t));
	offset += sizeof(uint16_t);
}

void BinaryWriter::WriteINT32(int32_t value) {
	if (isBigEndian) {
		value = ReverseEndian(value);
	}

	char bytes[sizeof(int32_t)];
	std::memcpy(bytes, &value, sizeof(int32_t));
	if (offset + sizeof(int32_t) > data.size()) {
		data.resize(offset + sizeof(int32_t));
	}

	std::memcpy(data.data() + offset, bytes, sizeof(int32_t));
	offset += sizeof(int32_t);
}

void BinaryWriter::WriteINT16(int16_t value) {
	if (isBigEndian) {
		value = ReverseEndian(value);
	}

	char bytes[sizeof(int16_t)];
	std::memcpy(bytes, &value, sizeof(int16_t));
	if (offset + sizeof(int16_t) > data.size()) {
		data.resize(offset + sizeof(int16_t));
	}

	std::memcpy(data.data() + offset, bytes, sizeof(int16_t));
	offset += sizeof(int16_t);
}

void BinaryWriter::WriteFloat(float value) {
	if (isBigEndian) {
		value = ReverseEndian(value);
	}

	char bytes[sizeof(float)];
	std::memcpy(bytes, &value, sizeof(float));

	if (offset + sizeof(float) > data.size()) {
		data.resize(offset + sizeof(float));
	}

	std::memcpy(data.data() + offset, bytes, sizeof(float));

	offset += sizeof(float);
}

std::vector<char> BinaryWriter::GetData() {
	return data;
}

bool BinaryWriter::EndOfBuffer() const {
	return offset >= data.size();
}

size_t BinaryWriter::Tell() {
	return offset;
}

void BinaryWriter::Reset() {
	offset = 0;
}

void BinaryWriter::Seek(size_t position) {
	offset = position;
}

void BinaryWriter::SetEndianess(bool isBig) {
	isBigEndian = isBig;
}
