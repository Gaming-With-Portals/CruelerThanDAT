using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Security.Cryptography.X509Certificates;
using System.Text;
using System.Threading.Tasks;
using static System.Runtime.InteropServices.JavaScript.JSType;
using SunsetLib;

namespace SunsetLib.MetalGearRising
{
    
    public class DDSFile
    {
        public string ID;
        public byte[] Data;
    }

    public class Texture
    {

        public uint version;
        public uint num_files;
        public uint offsetTexturesOffsets;
        public uint offsetTextureSizes;
        public uint offsetTextureFlags;
        public uint offsetTextureIdx;
        public uint offsetIndexInfo;
        public List<uint> offsets = new List<uint>();
        public List<uint> sizes = new List<uint>();
        public List<uint> flags = new List<uint>();
        public List<uint> idx = new List<uint>();

        public byte[] wtp_file;

        private static void swap_and_write_offset(RisingFormats.BinWriter file, uint offset_pos)
        {
            uint tmp_pos = (uint)file.Tell();
            file.Seek(offset_pos);
            file.WriteUInt32(tmp_pos);
            file.Seek(tmp_pos);


        }

        private static void swap_and_write_offset(RisingFormats.BinWriter file, uint offset_pos, uint value)
        {
            uint tmp_pos = (uint)file.Tell();
            file.Seek(offset_pos);
            file.WriteUInt32(value);
            file.Seek(tmp_pos);


        }

        public static byte[] SaveWTB(FileNode[] texture_data)
        {
            RisingFormats.BinWriter WTAFile = new();



            uint tx_count = (uint)texture_data.Length;

            WTAFile.WriteString("WTB");
            WTAFile.WriteByte(0);
            WTAFile.WriteUInt32(1);
            WTAFile.WriteUInt32(tx_count);

            // Reserve space for headers

            // Offsets
            WTAFile.WriteUInt32(32);

            // Sizes
            WTAFile.WriteUInt32(32 + (tx_count * 4));

            // Flags
            WTAFile.WriteUInt32(32 + (tx_count * 4) + (tx_count * 4));

            // Index
            WTAFile.WriteUInt32(32 + (tx_count * 4) + (tx_count * 4) + (tx_count * 4));

            // Unused
            WTAFile.WriteUInt32(0);


            int size_shifter = (int)(32 + (tx_count * 4) + (tx_count * 4) + (tx_count * 4) + (tx_count * 4)); // weird offset calculation that somehow works probably hopefully please im begging you
            for (int i = 0; i < texture_data.Count(); i++)
            {

                WTAFile.WriteUInt32((uint)(size_shifter));
                size_shifter += texture_data[i].Data.Length;
            }


            for (int i = 0; i < texture_data.Count(); i++)
            {
                WTAFile.WriteUInt32((uint)texture_data[i].Data.Length);
            }


            for (int i = 0; i < texture_data.Count(); i++)
            {
                WTAFile.WriteUInt32(536870944);
            }



            for (int i = 0; i < texture_data.Count(); i++)
            {
                WTAFile.WriteUInt32((uint)Int32.Parse(Path.GetFileNameWithoutExtension(texture_data[i].Name)));
            }


            for (int i = 0; i < texture_data.Count(); i++)
            {
                WTAFile.WriteByteArray(texture_data[i].Data);
            }




            return WTAFile.GetArray();
        }

        public static DDSFile[] SaveWTA_WTP(FileNode[] texture_data)
        {
            RisingFormats.BinWriter WTAFile = new();



            uint tx_count = (uint)texture_data.Length;

            WTAFile.WriteString("WTB");
            WTAFile.WriteByte(0);
            WTAFile.WriteUInt32(1);
            WTAFile.WriteUInt32(tx_count);

            // Reserve space for headers

            // Offsets
            WTAFile.WriteUInt32(32);

            // Sizes
            WTAFile.WriteUInt32(32 + (tx_count * 4));

            // Flags
            WTAFile.WriteUInt32(32 + (tx_count * 4) + (tx_count * 4));

            // Index
            WTAFile.WriteUInt32(32 + (tx_count * 4) + (tx_count * 4) + (tx_count * 4));

            // Unused
            WTAFile.WriteUInt32(0);


            int size_shifter = 0;
            for (int i = 0; i < texture_data.Count(); i++)
            {
                
                WTAFile.WriteUInt32((uint)(size_shifter));
                size_shifter += texture_data[i].Data.Length;
            }


            for (int i = 0; i < texture_data.Count(); i++)
            {
                WTAFile.WriteUInt32((uint)texture_data[i].Data.Length);
            }


            for (int i = 0; i < texture_data.Count(); i++)
            {
                WTAFile.WriteUInt32(536870944);
            }

  

            for (int i = 0; i < texture_data.Count(); i++)
            {
                WTAFile.WriteUInt32((uint)Int32.Parse(Path.GetFileNameWithoutExtension(texture_data[i].Name)));
            }



            List<Byte> wtp_byte_data = new List<byte>();

            for (int i = 0; i < texture_data.Count(); i++)
            {

                wtp_byte_data.AddRange(texture_data[i].Data);

            }


            DDSFile wta_file = new DDSFile();
            wta_file.Data = WTAFile.GetArray();
            wta_file.ID = "";

            DDSFile wtp_file = new DDSFile();
            wtp_file.Data = wtp_byte_data.ToArray();
            wta_file.ID = "";

            DDSFile[] output_array = new DDSFile[2];
            output_array[0] = wta_file;
            output_array[1] = wtp_file;

            return output_array;
        }

        public DDSFile[] LoadWTA_WTP(byte[] WTA, byte[] WTP)
        {
            RisingFormats.BinReader WTAFile = new (WTA);
            RisingFormats.BinReader WTPFile = new RisingFormats.BinReader(WTP);
            Debug.WriteLine("Loading WTA_WTP");
            WTAFile.Seek(4);
            version = WTAFile.ReadUInt32();
            num_files = WTAFile.ReadUInt32();
            offsetTexturesOffsets = WTAFile.ReadUInt32();
            offsetTextureSizes = WTAFile.ReadUInt32();
            offsetTextureFlags = WTAFile.ReadUInt32();
            offsetTextureIdx = WTAFile.ReadUInt32();
            offsetIndexInfo = WTAFile.ReadUInt32();

            Debug.WriteLine("WTA/WTP Header Info");
            Debug.WriteLine("File Count: " + num_files.ToString());

            WTAFile.Seek(offsetTexturesOffsets);
            offsets = new List<uint>();
            for (int i = 0; i < num_files; i++)
            {
                offsets.Add(WTAFile.ReadUInt32());
            }

            WTAFile.Seek(offsetTextureSizes);
            sizes = new List<uint>();
            for (int i = 0; i < num_files; i++)
            {
                sizes.Add(WTAFile.ReadUInt32());
            }

            WTAFile.Seek(offsetTextureFlags);
            flags = new List<uint>();
            for (int i = 0; i < num_files; i++)
            {
                flags.Add(WTAFile.ReadUInt32());
            }

            WTAFile.Seek(offsetTextureIdx);
            idx = new List<uint>();
            for (int i = 0; i < num_files; i++)
            {
                idx.Add(WTAFile.ReadUInt32());
            }

            wtp_file = WTP;


            List<DDSFile> files = new List<DDSFile>();

            for (int i = 0; i < num_files; i++)
            {
                DDSFile tmp_file = new DDSFile();
                byte[] output = new byte[sizes[i]];
                Debug.WriteLine("Grabbing Texture @ Offset: " + offsets[i].ToString() + " with size: " + sizes[i].ToString());
                Array.Copy(wtp_file, offsets[i], output, 0, sizes[i]);
                tmp_file.Data = output;
                tmp_file.ID = idx[i].ToString();
                files.Add(tmp_file);
            }

            return files.ToArray();
        }




    }
}
