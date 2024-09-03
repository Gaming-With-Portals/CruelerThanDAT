using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Windows.Xps;
using System.Xml.Linq;
using CruelerThanDAT.Systems;
using SunsetLib.MetalGearRising;
using SunsetLib;

namespace CruelerThanDAT.Tabs
{
    /// <summary>
    /// Interaction logic for bxm_view.xaml
    /// </summary>
    public partial class bxm_view : Page
    {
        private MemoryStream _bxmFile;
        private string XML_Text;
        private string fname;
        private TreeViewItem assoc_file;
        public bxm_view(byte[] file_in, string filename, TreeViewItem assoc_item)
        {
            fname = filename;
            InitializeComponent();
            BXM bxm = new BXM();
            _bxmFile = new MemoryStream();
            assoc_file = assoc_item;

            XElement xml_file = bxm.read_bxm(file_in);
            MemoryStream ms = new MemoryStream();
            xml_file.Save(ms);
            string tmp_text = ASCIIEncoding.UTF8.GetString(ms.ToArray());
            XML.Text = tmp_text;
            string number_string = "";
            foreach (string num in (XML.Text.ToString().Split("\n"))) {

                number_string +=  "-\n";
            }
            NumberBar.Content = number_string;



        }

        public void WriteXMLToDisk(object sender, RoutedEventArgs e)
        {
            var dialog = new Microsoft.Win32.SaveFileDialog();
            dialog.FileName = System.IO.Path.GetFileNameWithoutExtension(fname) + ".xml";
            dialog.DefaultExt = ".xml";
            dialog.Filter = "eXtensible Markup Language (*.xml)|*.xml"; // Filter files by extension
            dialog.Title = "Export file";
            // Show open file dialog box
            bool? result = dialog.ShowDialog();

            // Process open file dialog box results
            if (result == true)
            {

                // Open document
                string filename = dialog.FileName;
                using (var fs = new FileStream(filename, FileMode.Create, FileAccess.Write))
                {
                    fs.Write(Encoding.ASCII.GetBytes(XML.Text).Skip(1).ToArray(), 0, Encoding.ASCII.GetBytes(XML.Text).Length - 1);

                }
            }
            
        }

        private void Save_Click(object sender, RoutedEventArgs e)
        {
            
            BXM bxm = new BXM();
            string x_text = XML.Text;
            string _byteOrderMarkUtf8 = Encoding.UTF8.GetString(Encoding.UTF8.GetPreamble());
            if (x_text.StartsWith(_byteOrderMarkUtf8))
            {
                x_text = x_text.Remove(0, _byteOrderMarkUtf8.Length);
            }
            byte[] data = bxm.XmlToBXM(XElement.Parse(x_text));

            ((FileNode)assoc_file.Tag).Data = data;
        }

        private void ImportXML(object sender, RoutedEventArgs e)
        {
            var dialog = new Microsoft.Win32.OpenFileDialog();
            dialog.Filter = "XML/BXM Files (*.xml;*.bxm)|*.xml;*.bxm"; // Filter files by extension
            dialog.Title = "Import file";
            // Show open file dialog box
            bool? result = dialog.ShowDialog();

            // Process open file dialog box results
            if (result == true)
            {

                // Open document
                string filename = dialog.FileName;
                if (System.IO.Path.GetExtension(filename) == ".bxm")
                {
                    BXM bxm = new BXM();
                    XElement xml_file = bxm.read_bxm(File.ReadAllBytes(filename));
                    MemoryStream ms = new MemoryStream();
                    xml_file.Save(ms);
                    XML.Text = ASCIIEncoding.UTF8.GetString(ms.ToArray());
                }else if (System.IO.Path.GetExtension(filename) == ".xml")
                {
                    XML.Text = File.ReadAllText(filename);
                }
            }
        }
    }
}
