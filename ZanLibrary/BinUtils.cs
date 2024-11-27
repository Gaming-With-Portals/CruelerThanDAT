using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace RisingFormats
{
    public class BinUtils
    {
        public static uint CalcPadding(uint BlockSize, uint Length)
        {
	        return BlockSize - (Length % BlockSize);
        }
        public static int CalcPadding(int BlockSize, int Length)
        {
	        return BlockSize - (Length % BlockSize);
        }
    }
}
