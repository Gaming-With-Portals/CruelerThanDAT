#pragma once
#include "pch.hpp"

class BinaryReader {
public:
    BinaryReader(std::vector<char>& data, bool bigEndian = false)
        : data(data), offset(0), isBigEndian(bigEndian) {}

    // Read a UINT32 (4 bytes)
    uint32_t ReadUINT32() {
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

    int32_t ReadINT32() {
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

    std::vector<uint32_t> ReadUINT32Array(int count) {
        std::vector<uint32_t> output;
        for (int i = 0; i < count; i++) {
            output.push_back(ReadUINT32());
        }


        return output;

    }


    int8_t ReadINT8() {
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

    float ReadFloat() {
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


    int16_t ReadINT16() {
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

    // Read a UINT16 (2 bytes)
    uint16_t ReadUINT16() {
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

    std::vector<uint16_t> ReadUINT16Array(int count) {
        std::vector<uint16_t> output;
        for (int i = 0; i < count; i++) {
            output.push_back(ReadUINT16());
        }


        return output;

    }

    // Read a string
    std::string ReadString(int length) {
        std::string result;
        int x = 0;
        while (x < length) {
            x += 1;
            char ch = data[offset++];
            result += ch;
        }
        return result;
    }

    std::string ReadNullTerminatedString() {
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

    // Read raw bytes into a vector
    std::vector<char> ReadBytes(size_t size) {
        if (offset + size > data.size()) {
            throw std::runtime_error("Read beyond the buffer size.");
        }
        std::vector<char> buffer(size);
        std::memcpy(buffer.data(), &data[offset], size);
        offset += size;
        return buffer;
    }


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

    // Check if we are at the end of the data buffer
    bool EndOfBuffer() const {
        return offset >= data.size();
    }

    size_t Tell() {
        return offset;
    }

    // Reset the reader's position
    void Reset() {
        offset = 0;
    }

    void Seek(size_t position) {
        offset = position;
    }

    void Skip(size_t position) {
        offset += position;
    }

    void SetEndianess(bool isBig) {
        isBigEndian = isBig;
    }

    size_t GetSize() {
        return data.size();
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
    BinaryWriter(bool bigEndian = false)
        : offset(0), isBigEndian(bigEndian) {
    }
    
    void WriteByteZero() {
        data.push_back(static_cast<char>(0x00));
        offset += sizeof(char); 
    }

    void WriteBytes(std::vector<char> indata) {

        data.insert(data.end(), indata.begin(), indata.end());

        offset += indata.size();
    }

    void WriteString(const std::string& value) {
        uint32_t length = static_cast<uint32_t>(value.size());
        data.insert(data.end(), value.begin(), value.end());

        offset += length;
    }

    void WriteUINT32(uint32_t value) {
        char bytes[sizeof(uint32_t)];
        std::memcpy(bytes, &value, sizeof(uint32_t));
        if (offset + sizeof(uint32_t) > data.size()) {
            data.resize(offset + sizeof(uint32_t));
        }

        std::memcpy(data.data() + offset, bytes, sizeof(uint32_t));
        offset += sizeof(uint32_t);
    }

    void WriteUINT16(uint16_t value) {
        char bytes[sizeof(uint16_t)];
        std::memcpy(bytes, &value, sizeof(uint16_t));
        if (offset + sizeof(uint16_t) > data.size()) {
            data.resize(offset + sizeof(uint16_t));
        }

        std::memcpy(data.data() + offset, bytes, sizeof(uint16_t));
        offset += sizeof(uint16_t);
    }

    void WriteINT32(int32_t value) {
        char bytes[sizeof(int32_t)];
        std::memcpy(bytes, &value, sizeof(int32_t));
        if (offset + sizeof(int32_t) > data.size()) {
            data.resize(offset + sizeof(int32_t));
        }

        std::memcpy(data.data() + offset, bytes, sizeof(int32_t));
        offset += sizeof(int32_t);
    }

    void WriteINT16(int16_t value) {
        char bytes[sizeof(int16_t)];
        std::memcpy(bytes, &value, sizeof(int16_t));
        if (offset + sizeof(int16_t) > data.size()) {
            data.resize(offset + sizeof(int16_t));
        }

        std::memcpy(data.data() + offset, bytes, sizeof(int16_t));
        offset += sizeof(int16_t);
    }

    void WriteFloat(float value) {
        char bytes[sizeof(float)];
        std::memcpy(bytes, &value, sizeof(float));

        if (offset + sizeof(float) > data.size()) {
            data.resize(offset + sizeof(float));
        }

        std::memcpy(data.data() + offset, bytes, sizeof(float));

        offset += sizeof(float);
    }


    std::vector<char> GetData() {
        return data;
    }


    // Check if we are at the end of the data buffer
    bool EndOfBuffer() const {
        return offset >= data.size();
    }

    size_t Tell() {
        return offset;
    }

    // Reset the reader's position
    void Reset() {
        offset = 0;
    }

    void Seek(size_t position) {
        offset = position;
    }
    void SetEndianess(bool isBig) {
        isBigEndian = isBig;
    }

private:
    std::vector<char> data;
    size_t offset;
    bool isBigEndian;

    // Function to reverse the endian-ness (for big-endian <-> little-endian conversion)
    template <typename T>
    T ReverseEndian(T value) {
        char* data = reinterpret_cast<char*>(&value);
        std::reverse(data, data + sizeof(T));
        return value;
    }
};
