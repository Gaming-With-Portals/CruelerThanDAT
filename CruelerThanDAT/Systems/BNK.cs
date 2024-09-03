using RisingFormats.Dat;
using RisingFormats;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Diagnostics;
using ZanLibrary.Bxm;
using System.IO;
using System.Reflection;
using SunsetLib;

namespace CruelerThanDAT.Systems
{
    internal class BNK_HEADER
    {
        public string magic;
        public uint headerLength;
        public uint version;
        public uint soundbankid;
    }

    internal class DIDX_HEADER
    {
        public string magic;
        public uint chunkLength;
    }

    internal class WEM_FILE
    {
        public uint fileID;
        public uint DataOffset;
        public uint DataLength;
    }


    public class BNK
    {
        public FileNode[] Load_BNK(byte[] data)
        {
            BinReader bin_reader = new BinReader(data);
            BNK_HEADER header = new BNK_HEADER();
            header.magic = bin_reader.ReadString(4);
            header.headerLength = bin_reader.ReadUInt32();
            header.version = bin_reader.ReadUInt32();
            header.soundbankid = bin_reader.ReadUInt32();

            int currentPos = 8;
            while (currentPos < header.headerLength)
            {
                bin_reader.ReadUInt32();
                currentPos = currentPos + 4;
            }

            DIDX_HEADER dx_header = new DIDX_HEADER();
            dx_header.magic = bin_reader.ReadString(4);
            dx_header.chunkLength = bin_reader.ReadUInt32();

            List<FileNode> WEM_OUTPUT = new List<FileNode>();
            List<WEM_FILE> WEM_OFFSET_INFO = new List<WEM_FILE>();

            if (dx_header.chunkLength > 0)
            {
                int WemCount = (int)(dx_header.chunkLength / 12);
                for (int i = 0; i < WemCount; i++)
                {
                    WEM_FILE tmp_wem = new WEM_FILE();
                    tmp_wem.fileID = bin_reader.ReadUInt32();
                    tmp_wem.DataOffset = bin_reader.ReadUInt32();
                    tmp_wem.DataLength = bin_reader.ReadUInt32(); ;
                    WEM_OFFSET_INFO.Add(tmp_wem);

                }
            }

            foreach (WEM_FILE wem in WEM_OFFSET_INFO)
            {
                bin_reader.Seek(wem.DataOffset);
                uint riff_header = 0;
                while (riff_header != 1179011410)
                {
                    riff_header = bin_reader.ReadUInt32();
                }
                bin_reader.Seek((uint)(bin_reader.Tell() - 4));

                byte[] wem_data = bin_reader.ReadByteArray(wem.DataLength);



                FileNode wem_file = new FileNode(wem.fileID.ToString(), wem_data);
                WEM_OUTPUT.Add(wem_file);


            }
            return WEM_OUTPUT.ToArray();

        }

    }
}
