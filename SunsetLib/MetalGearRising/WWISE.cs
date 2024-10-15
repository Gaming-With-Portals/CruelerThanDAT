using RisingFormats;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Runtime.InteropServices.Marshalling;
using System.Text;
using System.Threading.Tasks;

namespace SunsetLib.MetalGearRising
{
    public class BNK_HEADER
    {
        public string header;
        public uint bnkID;
        public uint languageID;
        public uint isFeedbackInBNK;
        public uint projectID;
        public uint version;
        public byte[] padding;
        public byte[] unknown;


    }




    internal class WEM_FILE
    {
        public byte[] data;
        public string name;
        public WEM_FILE(string name, byte[] data)
        {
            this.data = data;
            this.name = name;

        }
    }

    public enum WwiseObjects
    {
        state,
        sound,
        action,
        evnt,
        randomSequence,
        soundSwitch,
        actorMixer,
        layerContainer,
        musicSegment,
        musicTrack,
        musicSwitch,
        musicPlaylist,
        attenuation,
        fxCustom,
        fxShareSet,
        unkn
    }


    public class BNK_HIRC_OBJ
    {
        public WwiseObjects type;
        uint size;
        public uint uid;
        public byte[] data;

        public BNK_HIRC_OBJ(BinReader reader)
        {
            type = WWISE.ByteToWwiseObject(reader.ReadByte());
            size = reader.ReadUInt32();
            uid = reader.ReadUInt32();
            data = reader.ReadByteArray(size - 4);

            Debug.WriteLine(type.ToString() + " ID:" + uid.ToString() + " SIZE: " + size.ToString() + " LOCATION: " + reader.Tell().ToString());

        }

    }


    public class BNK_HIRC
    {
        uint child_count;
        uint chunk_size;
        public List<BNK_HIRC_OBJ> objects;

        public BNK_HIRC(BinReader reader)
        {
            chunk_size = reader.ReadUInt32();
            child_count = reader.ReadUInt32();
            objects = new();
            Debug.WriteLine("Loading HIRC... Object Count: " + child_count.ToString());

            for (int i = 0; i < child_count; i++)
            {
                BNK_HIRC_OBJ obj = new BNK_HIRC_OBJ(reader);
                objects.Add(obj);

            }

        }
    }

    public class WWISE_BNK
    {
        public BNK_HEADER header;
        public BNK_HIRC hirc;
        public FileNode[] wem_files;

        public WWISE_BNK(BNK_HEADER header, FileNode[] wem_files, BNK_HIRC bnk_hirc)
        {
            this.header = header;
            this.wem_files = wem_files;
            this.hirc = bnk_hirc;
        }

    }



    public class WWISE
    {
        private BNK_HEADER BKHD_Chunk(BinReader reader)
        {
            BNK_HEADER header = new BNK_HEADER();

            uint chunk_size = reader.ReadUInt32();
            if (chunk_size < 20)
            {
                Debug.WriteLine("Ruh roh raggy, chunk size is less than 20");
                header.unknown = reader.ReadByteArray(chunk_size);
                return header;
            }

            header.version = reader.ReadUInt32();
            header.bnkID = reader.ReadUInt32();
            header.languageID = reader.ReadUInt32();
            header.isFeedbackInBNK = reader.ReadUInt32();
            header.projectID = reader.ReadUInt32();
            header.padding = reader.ReadByteArray(chunk_size - 20);



            return header;
        }

        private BNK_HIRC HIRC_Chunk(BinReader reader)
        {
            BNK_HIRC bnkhirc = new BNK_HIRC(reader);
            return bnkhirc;
        }

        public static WwiseObjects ByteToWwiseObject(int data)
        {
            switch (data)
            {
                case 0x01: return WwiseObjects.state;
                case 0x02: return WwiseObjects.sound;
                case 0x03: return WwiseObjects.action;
                case 0x04: return WwiseObjects.evnt;
                case 0x05: return WwiseObjects.randomSequence;
                case 0x06: return WwiseObjects.soundSwitch;
                case 0x07: return WwiseObjects.actorMixer;
                case 0x09: return WwiseObjects.layerContainer;
                case 0x0A: return WwiseObjects.musicSegment;
                case 0x0B: return WwiseObjects.musicTrack;
                case 0x0C: return WwiseObjects.musicSwitch;
                case 0x0D: return WwiseObjects.musicPlaylist;
                case 0x0E: return WwiseObjects.attenuation;
                case 0x12: return WwiseObjects.fxCustom;
                case 0x13: return WwiseObjects.fxShareSet;
                default: return WwiseObjects.unkn;
            }


        }


        private List<FileNode> DIDX_Chunk(BinReader reader)
        {
            List<FileNode> wem_files = new List<FileNode>();

            List<Tuple<uint, uint, uint>> filedat = new();

            uint chunk_size = reader.ReadUInt32();
            int child_count = (int)Math.Floor((double)((float)chunk_size / 12f));

            Debug.WriteLine("DIDX Chunk: " + child_count.ToString() + " file(s)");

            for (int i = 0; i < child_count; i++) {
                filedat.Add(Tuple.Create(reader.ReadUInt32(), reader.ReadUInt32(), reader.ReadUInt32()));
            }

            int cur_pos = reader.Tell();

            foreach (Tuple<uint, uint, uint> file in filedat)
            {
                
                reader.Seek((uint)file.Item2 + (uint)cur_pos + (uint)8);
                Debug.WriteLine("Loading WEM at: " + reader.Tell().ToString());
                FileNode tmp_node = new FileNode(file.Item1.ToString(), reader.ReadByteArray(file.Item3));
                wem_files.Add(tmp_node);
            }

            return wem_files;

        }


        public WWISE_BNK Read_BNK(byte[] bnk)
        {
            BinReader reader = new BinReader(bnk);

            BNK_HEADER bnk_header = null;
            List<FileNode> wem_files = new List<FileNode>();
            BNK_HIRC bnk_hirc = null;

            while (reader.Tell() < bnk.Length - 4)
            {
                string chunk_header = reader.ReadString(4);
                if (chunk_header == "BKHD")
                {
                    bnk_header = BKHD_Chunk(reader);
                }
                else if (chunk_header == "DIDX")
                {
                    wem_files = DIDX_Chunk(reader);
                }else if (chunk_header == "HIRC")
                {
                    bnk_hirc = HIRC_Chunk(reader);
                }

            }

            return new WWISE_BNK(bnk_header, wem_files.ToArray(), bnk_hirc);


        }

    }
}
