using RisingFormats;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Net.Http.Headers;
using System.Runtime.InteropServices;
using System.Runtime.Serialization.Formatters;
using System.Security.Cryptography;
using System.Text;
using System.Threading.Tasks;
using System.Xml;
using System.Xml.Linq;
using static System.Runtime.InteropServices.JavaScript.JSType;

namespace SunsetLib.MetalGearRising
{
    class BxmHeader
    {
        public string type;
        public int flags;
        public int nodeCount;
        public int dataCount;
        public int dataSize;
    }

    class BxmNodeInfo
    {
        public int childCount;
        public int firstChildIndex;
        public int attributeCount;
        public int dataIndex;

        public void read(BinReader_BE reader)
        {
            childCount = reader.ReadInt16();
            firstChildIndex = reader.ReadInt16();
            attributeCount = reader.ReadInt16();
            dataIndex = reader.ReadInt16();
        }

    }

    class BxmDataOffsets
    {
        public uint nameOffset;
        public uint valueOffset;

        public void read(BinReader_BE reader)
        {
            nameOffset = reader.ReadUInt16();
            valueOffset = reader.ReadUInt16();
        }

    }
    
    class BxmXmlNode
    {
        public string name;
        public string value;
        public Dictionary<string, string> attributes;
        public List<BxmXmlNode> children;
        public BxmXmlNode? parent;

        public int index;
        public int firstChildIndex;
        public int childCount;

        public BxmXmlNode()
        {
            name = "";
            value = "";
            attributes = new Dictionary<string, string>();
            children = new List<BxmXmlNode>();
            index = -1;
            firstChildIndex = -1;
            childCount = -1;
        }

        public XElement ToXml()
        {
            XElement node = new XElement(name);

            if (!string.IsNullOrEmpty(value))
            {
                node.Add(new XText(value));
            }

            foreach (var attribute in attributes)
            {
                node.SetAttributeValue(attribute.Key, attribute.Value);
            }

            foreach (var child in children)
            {
                node.Add(child.ToXml());
            }

            return node;
        }

    }

    public class BXM
    {
        private uint GetStringOffsetValueAndValidateData(Dictionary<string, int> stringToOffset, string SearchValue)
        {
            if (stringToOffset.ContainsKey(SearchValue))
            {
                return (uint)stringToOffset[SearchValue];
            }
            else
            {
                return 65535;
            }


        }
        
        private XElement[] getNodes(XElement element)
        {
            List<XElement> list = new List<XElement>();
            list.Add(element);
            foreach (var child in element.Elements())
            {
                list.AddRange(getNodes(child));
            }
            return list.ToArray();
        }

        private int[] FormatStringAsIntegerArray(string value)
        {
            
            byte[] utf8Bytes = Encoding.UTF8.GetBytes(value);

            int[] utf8Ints = new int[utf8Bytes.Length];
            for (int i = 0; i < utf8Bytes.Length; i++)
            {
                utf8Ints[i] = utf8Bytes[i];
            }

            return utf8Ints;
        }

        private string _getElementText(XElement element)
        {
            string TextNodes = "";
            foreach (var child in element.Elements())
            {
                try
                {
                    TextNodes = TextNodes + child.Value.ToString();
                }
                catch
                {

                }
            }
            return TextNodes;
        }


        private void tryAddString(string String, HashSet<string> uniqueStringsSet, List<(string, List<int>)> uniqueStrings)
        {
            if (!string.IsNullOrEmpty(String) && !uniqueStringsSet.Contains(String))
            {
                uniqueStrings.Add((String, FormatStringAsIntegerArray(String).ToList()));
                uniqueStringsSet.Add(String);
            }
        }

        public byte[] XmlToBXM(XElement element)
        {
            // Flattening the tree but in a cool C# sorta way
            List<XElement> nodes = new List<XElement>();
            nodes = getNodes(element).ToList();

            HashSet<string> uniqueStringsSet = new HashSet<string>();
            List<(string, List<int>)> uniqueStrings = new List<(string, List<int>)>();

            foreach (XElement node in nodes)
            {
                tryAddString(node.Name.LocalName, uniqueStringsSet, uniqueStrings);
                foreach (XAttribute attribute in node.Attributes())
                {
                    tryAddString(attribute.Name.LocalName, uniqueStringsSet, uniqueStrings);
                    tryAddString(attribute.Value, uniqueStringsSet, uniqueStrings);
                }

                tryAddString(_getElementText(node), uniqueStringsSet, uniqueStrings);
            }


            // Get ze string offsets
            Dictionary<string, int> stringToOffset = new Dictionary<string, int>();
            int curOffset = 0;
            foreach (var stringvar in uniqueStrings)
            {
                stringToOffset[stringvar.Item1] = curOffset;
                curOffset += stringvar.Item2.Count() + 1;
            }
            // Calculate data offsets for strings
            List<BxmDataOffsets> dataOffsets = new List<BxmDataOffsets>();
            Dictionary<XElement, int> nodeToDataIndex = new Dictionary<XElement, int>();
            foreach (var node in nodes)
            {
                BxmDataOffsets dataOffset = new BxmDataOffsets();
                dataOffset.nameOffset = GetStringOffsetValueAndValidateData(stringToOffset, node.Name.LocalName);
                dataOffset.valueOffset = GetStringOffsetValueAndValidateData(stringToOffset, _getElementText(node));

                nodeToDataIndex[node] = dataOffsets.Count();
                dataOffsets.Add(dataOffset);
                foreach (XAttribute attribute in node.Attributes())
                {
                    BxmDataOffsets attributeOffset = new BxmDataOffsets();
                    
                    attributeOffset.nameOffset = GetStringOffsetValueAndValidateData(stringToOffset, attribute.Name.LocalName);
                    attributeOffset.valueOffset = GetStringOffsetValueAndValidateData(stringToOffset, attribute.Value);
                    dataOffsets.Add(attributeOffset);
                }
                

            }

            // Make node infos
            Dictionary<BxmNodeInfo, XElement> nodeInfoToXmlNode = new Dictionary<BxmNodeInfo, XElement>();
            List<Tuple<BxmNodeInfo, XElement>> nodeCombos = new List<Tuple<BxmNodeInfo, XElement>>();
            Dictionary<XElement, XElement> parentMap = new Dictionary<XElement, XElement>();
            foreach (XElement parent in nodes)
            {
                foreach (var child in parent.Elements()) // If this doesnt work it might need to be a different function, I knew i should have used XmlElement instead of XElement
                {
                    parentMap[child] = parent;
                }
            }

            // xml nodes to node infos in correct order
            List<BxmNodeInfo> nodeInfos = new List<BxmNodeInfo>();

            nodeInfos.Add(nodeToNodeInfos(element, nodeToDataIndex, nodeInfoToXmlNode, nodeCombos));
            addNodeChildrenToInfos(element, nodeInfos, nodeToDataIndex, nodeInfoToXmlNode, nodeCombos);
            // set first child index / next sibling index
            foreach (BxmNodeInfo nodeInfo in nodeInfos)
            {
                int nextIndex = -1;
                if (nodeInfo.childCount > 0)
                {
                    XElement firstChild = nodeInfoToXmlNode[nodeInfo].Elements().First();
                    nextIndex = nodeCombos.FindIndex(combo => combo.Item2 == firstChild);
                }
                else
                {
                    XElement xmlNode = nodeInfoToXmlNode[nodeInfo]!;
                    XElement parent = parentMap[xmlNode]!;
                    XElement lastChild = parent.Elements().Last();
                    int lastChildIndex = nodeCombos.FindIndex(combo => combo.Item2 == lastChild);
                    nextIndex = lastChildIndex + 1;
                }
                nodeInfo.firstChildIndex = nextIndex;
            }

            BxmHeader header = new BxmHeader();
            header.type = "BXM\x00";
            header.flags = 0;
            header.nodeCount = nodeInfos.Count;
            header.dataCount = dataOffsets.Count;
            header.dataSize = uniqueStrings
                .Select(str => str.Item2.Count + 1)
                .Aggregate(0, (a, b) => a + b);


            byte[] data;
            BinWriter_BE reader = new();

            reader.WriteString(header.type);
            reader.WriteInt32(header.flags);
            reader.WriteInt16((short)header.nodeCount);
            reader.WriteInt16((short)header.dataCount);
            reader.WriteInt32((short)header.dataSize);

            foreach (BxmNodeInfo nodeInfo in nodeInfos)
            {
                reader.WriteInt16((short)nodeInfo.childCount);
                reader.WriteInt16((short)nodeInfo.firstChildIndex);
                reader.WriteInt16((short)nodeInfo.attributeCount);
                reader.WriteInt16((short)nodeInfo.dataIndex);
            }
            foreach (BxmDataOffsets dataOffset in dataOffsets)
            {
                reader.WriteUInt16((ushort)dataOffset.nameOffset);
                reader.WriteUInt16((ushort)dataOffset.valueOffset);
            }
            foreach (var stringobj in uniqueStrings)
            {
                foreach (int item in stringobj.Item2)
                {
                    reader.WriteByte((byte)(uint)item);
                }

                reader.WriteByte(0);
            }
            reader.WriteByte(0);
            return reader.GetArray();
        }
        
        private BxmNodeInfo nodeToNodeInfos(XElement node, Dictionary<XElement, int> nodeToDataIndex, Dictionary<BxmNodeInfo, XElement> nodeInfoToXmlNode, List<Tuple<BxmNodeInfo, XElement>> nodeCombos)
        {
            BxmNodeInfo nodeInfo = new BxmNodeInfo();
            nodeInfo.childCount = node.Elements().Count();
            nodeInfo.firstChildIndex = -1;
            nodeInfo.attributeCount = node.Attributes().Count();
            nodeInfo.dataIndex = nodeToDataIndex[node];


            

            nodeInfoToXmlNode[nodeInfo] = node;
            nodeCombos.Add(Tuple.Create(nodeInfo, node));
            return nodeInfo;

        }

        // another external function... damn you Dart! 
        private void addNodeChildrenToInfos(XElement node, List<BxmNodeInfo> nodeInfos, Dictionary<XElement, int> nodeToDataIndex, Dictionary<BxmNodeInfo, XElement> nodeInfoToXmlNode, List<Tuple<BxmNodeInfo, XElement>> nodeCombos)
        {
            foreach (var child in node.Elements())
            {
                nodeInfos.Add(nodeToNodeInfos(child, nodeToDataIndex, nodeInfoToXmlNode, nodeCombos));
            }
            foreach (var child in node.Elements())
            {
                addNodeChildrenToInfos(child, nodeInfos, nodeToDataIndex, nodeInfoToXmlNode, nodeCombos);
            }
        }


            public XElement read_bxm(byte[] file)
            {

                BinReader_BE reader = new BinReader_BE(file);

                BxmHeader header = new BxmHeader();
                header.type = reader.ReadString(4);
                header.flags = reader.ReadInt32();
                header.nodeCount = reader.ReadInt16();
                header.dataCount = reader.ReadInt16();
                header.dataSize = reader.ReadInt32();
                // Load Node Data
                List<BxmNodeInfo> nodeInfos = new List<BxmNodeInfo>();
                for (int i = 0; i < header.nodeCount; i++)
                {
                    BxmNodeInfo tmp_node_info = new BxmNodeInfo();
                    tmp_node_info.read(reader);
                    nodeInfos.Add(tmp_node_info);
                }


                // Load Offsets
                List<BxmDataOffsets> dataOffsets = new List<BxmDataOffsets>();
                for (int i = 0; i < header.dataCount; i++)
                {
                    BxmDataOffsets tmp_data_info = new BxmDataOffsets();
                    tmp_data_info.read(reader);
                    dataOffsets.Add(tmp_data_info);
                }

                var stringOffsets = 0x10 + 8 * header.nodeCount + 4 * header.dataCount;
                
                List<BxmXmlNode> nodes = new List<BxmXmlNode>();
                for (int i =  0; i < nodeInfos.Count; i++)
                {
                    BxmNodeInfo nodeInfo = nodeInfos[i];
                    BxmXmlNode node = new BxmXmlNode();
                    node.index = i;
                    node.firstChildIndex = nodeInfo.firstChildIndex;
                    node.childCount = nodeInfo.childCount;

                    uint nodeNameOffset = dataOffsets[nodeInfo.dataIndex].nameOffset;
                    if (nodeNameOffset != 0xFFFF)
                    {
                        reader.Seek((uint)(stringOffsets + nodeNameOffset));
                        node.name = reader.ReadStringZeroTerminated();
                    }
                    uint nodeValueOffset = dataOffsets[nodeInfo.dataIndex].valueOffset;
                    if (nodeValueOffset != 0xFFFF)
                    {
                        reader.Seek((uint)(stringOffsets + nodeValueOffset));
                        node.value = reader.ReadStringZeroTerminated();
                    }

                    for (int x = 0; x < nodeInfo.attributeCount; x++)
                    {
                        string attributeName = "";
                        string attributeValue = "";
                        uint attributeNameOffset = dataOffsets[nodeInfo.dataIndex + 1 + x].nameOffset;
                        if (attributeNameOffset != 0xFFFF)
                        {
                            reader.Seek((uint)(stringOffsets + attributeNameOffset));
                            attributeName = reader.ReadStringZeroTerminated();
                        }
                        uint attributeValueOffset = dataOffsets[nodeInfo.dataIndex + 1 + x].valueOffset;
                        if (attributeValueOffset != 0xFFFF)
                        {
                            reader.Seek ((uint)(stringOffsets + attributeValueOffset));
                            attributeValue = reader.ReadStringZeroTerminated();
                        }
                        node.attributes[attributeName] = attributeValue;

                    }
                    nodes.Add(node);
                }

                foreach (BxmXmlNode node in nodes)
                {
                    node.children = GetNodeChildren(node, nodes);
                    foreach (BxmXmlNode child in node.children)
                    {
                        child.parent = node;
                    }
                }

                return nodes[0].ToXml();
        }


        private List<BxmXmlNode> GetNodeChildren(BxmXmlNode node, List<BxmXmlNode> nodes)
        {
            if (node.childCount == 0)
                return new List<BxmXmlNode>();

            var firstChild = nodes[node.firstChildIndex];
            var otherChildren = GetNodeNextSiblings(firstChild, nodes);

            var children = new List<BxmXmlNode> { firstChild };
            children.AddRange(otherChildren);

            return children;
        }

        private List<BxmXmlNode> GetNodeNextSiblings(BxmXmlNode node, List<BxmXmlNode> nodes)
        {
            if (node.index + 1 >= node.firstChildIndex || node.firstChildIndex <= 0)
            {
                return new List<BxmXmlNode>();
            }

            return nodes.GetRange(node.index + 1, node.firstChildIndex - (node.index + 1));
        }
    }
}
