using System;
using System.Buffers.Binary;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;

namespace RisingFormats
{
    public class BinWriter
    {
        List<byte> _data;
        int _pointer = 0;
        public BinWriter()
        {
            _data = new List<byte>();
        }
        public BinWriter(byte[] data)
        {
            _data = data.ToList();
        }
        public void Seek(uint offset)
        {
            _pointer = (int)offset;
        }
        public int Tell()
        {
            return _pointer;
        }
        // Writing
        public void WriteByte(byte value)
        {
            _data.Insert(_pointer, value);
            _pointer += 1;
        }
        public void WriteUInt32(uint value)
        {
            _data.InsertRange(_pointer, BitConverter.GetBytes(value));
            _pointer += 4;
        }
        public void WriteInt32(int value)
        {
            _data.InsertRange(_pointer, BitConverter.GetBytes(value));
            _pointer += 4;
        }
        public void WriteFloat32(float value)
        {
            _data.InsertRange(_pointer, BitConverter.GetBytes(value));
            _pointer += 4;
        }
        public void WriteInt16(short value)
        {
            _data.InsertRange(_pointer, BitConverter.GetBytes(value));
            _pointer += 2;
        }
        public void WriteUInt16(ushort value)
        {
            _data.InsertRange(_pointer, BitConverter.GetBytes(value));
            _pointer += 2;
        }
        public void WriteString(string value)
        {
            _data.InsertRange(_pointer, Encoding.ASCII.GetBytes(value));
            _pointer += value.Length;
        }
        public void WriteString(string value, int length)
        {
            int paddedLen = length - value.Length;
            _data.InsertRange(_pointer, Encoding.ASCII.GetBytes(value));
            _pointer += value.Length;
            for (int i = 0; i < paddedLen; i++)
            {
                WriteByte(0x00);
            }
        }
        public void WriteByteArray(byte[] value)
        {
            _data.InsertRange(_pointer, value);
            _pointer += value.Length;
        }

        // Replacing
        public void ReplaceByte(byte value)
        {
            _data.RemoveAt(_pointer);
            _data.Insert(_pointer, value);
            _pointer += 1;
        }
        public void ReplaceUInt32(uint value)
        {
            _data.RemoveRange(_pointer, 4);
            _data.InsertRange(_pointer, BitConverter.GetBytes(value));
            _pointer += 4;
        }
        public void ReplaceInt32(int value)
        {
            _data.RemoveRange(_pointer, 4);
            _data.InsertRange(_pointer, BitConverter.GetBytes(value));
            _pointer += 4;
        }
        public void ReplaceFloat32(float value)
        {
            _data.RemoveRange(_pointer, 4);
            _data.InsertRange(_pointer, BitConverter.GetBytes(value));
            _pointer += 4;
        }
        public void ReplaceInt16(short value)
        {
            _data.RemoveRange(_pointer, 2);
            _data.InsertRange(_pointer, BitConverter.GetBytes(value));
            _pointer += 2;
        }
        public void ReplaceUInt16(ushort value)
        {
            _data.RemoveRange(_pointer, 2);
            _data.InsertRange(_pointer, BitConverter.GetBytes(value));
            _pointer += 2;
        }

        public void ReplaceRange(byte[] range)
        {
            _data.RemoveRange(_pointer, range.Length);
            _data.InsertRange(_pointer, range);
            _pointer += range.Length;
        }

        public void RemoveRange(int amount)
        {
            _data.RemoveRange(_pointer, amount);
        }
        public byte[] GetArray()
        {
            return _data.ToArray();
        }
        public int Size()
        {
            return _data.Count;
        }
    }

    public class BinWriter_BE
    {
        List<byte> _data;
        int _pointer = 0;
        public BinWriter_BE()
        {
            _data = new List<byte>();
        }
        public BinWriter_BE(byte[] data)
        {
            _data = data.ToList();
        }
        public void Seek(uint offset)
        {
            _pointer = (int)offset;
        }
        public int Tell()
        {
            return _pointer;
        }
        // Writing
        public void WriteByte(byte value)
        {
            _data.Insert(_pointer, value);
            _pointer += 1;
        }
        public void WriteUInt32(uint value)
        {
            _data.InsertRange(_pointer, BitConverter.GetBytes(BinaryPrimitives.ReverseEndianness(value)));
            _pointer += 4;
        }
        public void WriteInt32(int value)
        {
            _data.InsertRange(_pointer, BitConverter.GetBytes(BinaryPrimitives.ReverseEndianness(value)));
            _pointer += 4;
        }

        public void WriteInt16(short value)
        {
            _data.InsertRange(_pointer, BitConverter.GetBytes(BinaryPrimitives.ReverseEndianness(value)));
            _pointer += 2;
        }
        public void WriteUInt16(ushort value)
        {
            _data.InsertRange(_pointer, BitConverter.GetBytes(BinaryPrimitives.ReverseEndianness(value)));
            _pointer += 2;
        }
        public void WriteString(string value)
        {
            _data.InsertRange(_pointer, Encoding.ASCII.GetBytes(value));
            _pointer += value.Length;
        }
        public void WriteString(string value, int length)
        {
            int paddedLen = length - value.Length;
            _data.InsertRange(_pointer, Encoding.ASCII.GetBytes(value));
            _pointer += value.Length;
            for (int i = 0; i < paddedLen; i++)
            {
                WriteByte(0x00);
            }
        }
        public void WriteByteArray(byte[] value)
        {
            _data.InsertRange(_pointer, value);
            _pointer += value.Length;
        }

        public byte[] GetArray()
        {
            return _data.ToArray();
        }
        public int Size()
        {
            return _data.Count;
        }
    }
}
