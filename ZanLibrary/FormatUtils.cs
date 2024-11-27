using RisingFormats;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.Intrinsics.X86;
using System.Security.Cryptography;
using System.Text;
using System.Threading.Tasks;

namespace ZanLibrary
{
    public enum MGRFileFormat : int
    {
        UNK = 0, // ICON: V
        DAT, // ICON: V
        BXM, // ICON: V
        WTB, // ICON: V
        WMB, // ICON: V
        MOT, // ICON: X
        EST, // ICON: V
        BNK, // ICON: V
        SCR, // ICON: X
        SYN, // ICON: X
        LY2, // ICON: X
        UID, // ICON: X
        SOP, // ICON: X
        EXP, // ICON: X
        CTX, // ICON: X
        UVD, // ICON: X
        SAE, // ICON: X
        SAS, // ICON: X
        HKX, // ICON: X
        CPK, // ICON: X
        WEM, // ICON: V
        VCD, // ICON: X
        BRD  // ICON: X
    }
    public class FormatUtils
    {
        public static MGRFileFormat DetectFileFormat(byte[] data)
        {
            BinReader red = new BinReader(data);
            uint magic = red.ReadUInt32();
            switch (magic)
            {
                case 0x00544144:
                    return MGRFileFormat.DAT;
                case 0x004C4D58:
                case 0x004D5842:
                    return MGRFileFormat.BXM;
                case 0x00425457:
                    return MGRFileFormat.WTB;
                case 0x34424D57:
                    return MGRFileFormat.WMB;
                default:
                    return MGRFileFormat.UNK;
                    // return MGRFileFormat.MOT;
                    // return MGRFileFormat.EST;
                    // return MGRFileFormat.BNK;
                    // return MGRFileFormat.SCR;
                    // return MGRFileFormat.SYN;
                    // return MGRFileFormat.LY2;
                    // return MGRFileFormat.UID;
                    // return MGRFileFormat.SOP;
                    // return MGRFileFormat.EXP;
                    // return MGRFileFormat.CTX;
                    // return MGRFileFormat.UVD;
                    // return MGRFileFormat.SAE;
                    // return MGRFileFormat.SAS;
                    // return MGRFileFormat.HKX;
                    // return MGRFileFormat.CPK;
                    // return MGRFileFormat.WEM;
                    // return MGRFileFormat.VCD;
                    // return MGRFileFormat.BRD;

            }
        }
    }
}
