using RisingFormats.Dat;
using SunsetLib.MetalGearRising;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;


namespace SunsetLib
{
    public enum NodeType
    {
        standard,
        texture
    }

    public class FileUtils
    {
        public static DDSFile[] FileNodeToDDS(FileNode[] fnodes) {

            List<DDSFile> result = new List<DDSFile>();
            foreach (FileNode node in fnodes)
            {
                DDSFile dfile = new DDSFile();
                dfile.ID = node.Name.Substring(0, node.Name.Length-4);
                dfile.Data = node.Data;
                result.Add(dfile);

            }

            return result.ToArray();
        }

    }

   
    public class FileNode
    {
        private string[] VALID_DAT_EXTENSIONS = { ".dat", ".dtt", ".eff", ".evn", ".eft" };

        public string Name;
        public byte[] Data;
        public FileNode[] children;
        public DDSFile[] texture_children;
        public NodeType nodetype = NodeType.standard;

        public bool has_children;
        public bool locked = false;
        public byte[] ConvertedData; // Only initalized if needed, i.e for WEMs or DDS files
        public FileNode(string f_name, byte[] f_data)
        {
            has_children = false;
            Name = f_name;
            Data = f_data;
            

            if (VALID_DAT_EXTENSIONS.Contains(System.IO.Path.GetExtension(f_name).ToLower()))
            {
                try
                {
                    DatFileEntry[] tmp_sub_nodes;
                    tmp_sub_nodes = RisingFormats.Dat.DatFile.Load(f_data);
                    List<FileNode> child_nodes_tmp = new List<FileNode>();
                    has_children = true;
                    foreach (DatFileEntry node in tmp_sub_nodes)
                    {

                        FileNode tmp_node = new FileNode(node.Name, node.Data);

                        child_nodes_tmp.Add(tmp_node);
                    }
                    children = child_nodes_tmp.ToArray();
                }
                catch
                {
                    has_children = false;
                }

            }

        }

        public string GetExt()
        {
            return Path.GetExtension(Name);
        }

    }
}
