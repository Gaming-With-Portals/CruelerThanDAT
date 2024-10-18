using System;
using System.Linq;
using System.Windows;
using AdonisUI.Controls;
using System.Windows.Shapes;
using CruelerThanDAT.Systems;
using System.Windows.Controls;
using System.Runtime.CompilerServices;
using System.Windows.Media.Imaging;
using System.Collections;
using System.IO;
using RisingFormats.Dat;
using System.Collections.Generic;
using SunsetLib.MetalGearRising;
using System.Xml.Linq;
using CruelerThanDAT.Tabs;
using System.Windows.Controls.Ribbon;
using System.Windows.Media;
using System.Text;
using System.Net;
using System.Data;
using System.Runtime.InteropServices.JavaScript;
using CruelerThanDAT.ui;
using SunsetLib;
using System.Xml;
using System.Formats.Tar;
using System.Diagnostics;
using System.Printing.IndexedProperties;

namespace CruelerThanDAT
{
    public static class Application_Data
    {
        public static string version = "BETA 6";
        public static int internal_version = 0; // for ze update server
        public static string optimized_for = "Metal Gear Rising: Revengence";


    }


    public partial class MainWindow
    {
        StreamWriter log_file;
        TreeViewItem selected_object;
        string[] VALID_DAT_EXTENSIONS = { ".dat", ".dtt", ".eff", ".evn", ".eft" };
        FileNode _base_node;
        Dictionary<string, BitmapImage> icon_map;
        string loaded_file_path = "";
        bool edited_wta_file = false;
        bool dtt_save_required = false;
        bool dat_save_required = false;
        DDSFile[] wtp_wta_data;
        public MainWindow()
        {
            InitializeComponent();
            Title = "CruelerThan.DAT | Version: " + Application_Data.version + " | " + Application_Data.optimized_for;
            icon_map = new Dictionary<string, BitmapImage>();
            icon_map.Add(".dat", new BitmapImage(new Uri(System.AppDomain.CurrentDomain.BaseDirectory + "images\\dat.png")));
            icon_map.Add(".dtt", new BitmapImage(new Uri(System.AppDomain.CurrentDomain.BaseDirectory + "images\\dat.png")));
            icon_map.Add(".eff", new BitmapImage(new Uri(System.AppDomain.CurrentDomain.BaseDirectory + "images\\dat.png")));
            icon_map.Add(".evn", new BitmapImage(new Uri(System.AppDomain.CurrentDomain.BaseDirectory + "images\\dat.png")));
            icon_map.Add(".est", new BitmapImage(new Uri(System.AppDomain.CurrentDomain.BaseDirectory + "images\\est.png")));
            icon_map.Add(".bin", new BitmapImage(new Uri(System.AppDomain.CurrentDomain.BaseDirectory + "images\\bin.png")));
            icon_map.Add(".wmb", new BitmapImage(new Uri(System.AppDomain.CurrentDomain.BaseDirectory + "images\\wmb.png")));
            icon_map.Add(".scr", new BitmapImage(new Uri(System.AppDomain.CurrentDomain.BaseDirectory + "images\\wmb.png")));
            icon_map.Add(".ly2", new BitmapImage(new Uri(System.AppDomain.CurrentDomain.BaseDirectory + "images\\ly2.png")));
            icon_map.Add(".bxm", new BitmapImage(new Uri(System.AppDomain.CurrentDomain.BaseDirectory + "images\\bxm.png")));
            icon_map.Add(".wta", new BitmapImage(new Uri(System.AppDomain.CurrentDomain.BaseDirectory + "images\\wta.png")));
            icon_map.Add(".wtp", new BitmapImage(new Uri(System.AppDomain.CurrentDomain.BaseDirectory + "images\\wtb.png")));
            icon_map.Add(".wtb", new BitmapImage(new Uri(System.AppDomain.CurrentDomain.BaseDirectory + "images\\dat.png")));
            icon_map.Add(".mot", new BitmapImage(new Uri(System.AppDomain.CurrentDomain.BaseDirectory + "images\\mot.png")));
            icon_map.Add(".mcd", new BitmapImage(new Uri(System.AppDomain.CurrentDomain.BaseDirectory + "images\\mcd.png")));
            icon_map.Add("default", new BitmapImage(new Uri(System.AppDomain.CurrentDomain.BaseDirectory + "images\\default.png")));
            icon_map.Add(".hkx", new BitmapImage(new Uri(System.AppDomain.CurrentDomain.BaseDirectory + "images\\havok.png")));
            Update_Crueler_Status(icon_map.Count.ToString() + " icon(s) loaded!");
            log_file = new StreamWriter("CruelerThanDAT.log");
            string base_dir = System.AppDomain.CurrentDomain.BaseDirectory;
            log_file.WriteLine("CRUELERTHANDAT, V" + Application_Data.version + " FOR " + Application_Data.optimized_for);
            log_file.WriteLine("Initalized successfully!");

            if (!System.IO.Path.Exists(System.IO.Path.Join(base_dir, "plugins"))){
                System.IO.Directory.CreateDirectory(System.IO.Path.Join(base_dir, "plugins"));
            }
            if (!System.IO.Path.Exists(System.IO.Path.Join(base_dir, "userdata")))
            {
                System.IO.Directory.CreateDirectory(System.IO.Path.Join(base_dir, "userdata"));
            }
            if (!System.IO.Path.Exists(System.IO.Path.Join(base_dir, "userdata", Environment.UserName)))
            {
                System.IO.Directory.CreateDirectory(System.IO.Path.Join(base_dir, "userdata", Environment.UserName));
            }
            log_file.Flush();
        }

        private void Update_Crueler_Status(string status)
        {
            CruelerStatus.Content = " STATUS: " + status;
        }
        private void Crueler_DragOver(object sender, DragEventArgs e)
        {
            e.Effects = DragDropEffects.Copy;
            e.Handled = true;
        }


        private void Crueler_Drop(object sender, DragEventArgs e)
        {
            if (e.Data.GetDataPresent(DataFormats.FileDrop))
            {
                string[] files = (string[])e.Data.GetData(DataFormats.FileDrop);
                log_file.WriteLine("User dropped file: " + files[0]);
                Open_File_From_String(files[0]);
                log_file.Flush();
            }


        }

        private string Get_File_Extension_From_Text(byte[] file)
        {
            Stream stream = new MemoryStream(file);
            BinaryReader reader = new BinaryReader(stream);
            uint header = reader.ReadUInt32();

            if (header == 1196314761)
            {
                return ".texture";
            }else if (header == 542327876)
            {
                return ".texture";
            }
            else
            {
                return ".unknown";
            }


        }

        private void Open_File_From_String(string file_path)
        {
            System.Diagnostics.Debug.WriteLine(file_path);
            if (VALID_DAT_EXTENSIONS.Contains(System.IO.Path.GetExtension(file_path)))
            {
                loaded_file_path = file_path;
                Update_Crueler_Status("Opening DAT " + System.IO.Path.GetFileName(file_path));
                _base_node = new FileNode(System.IO.Path.GetFileName(file_path), System.IO.File.ReadAllBytes(file_path));
                TreeViewItem base_item = new TreeViewItem();

                base_item.Header = System.IO.Path.GetFileName(file_path);
                Build_Treeview(_base_node, base_item);
                base_item.Tag = _base_node;
                TreeView1.Items.Add(base_item);
                Update_Crueler_Status("Ready.");
            }
            else if (System.IO.Path.GetExtension(file_path) == ".bxm") {
                
                Update_Crueler_Status("Converting...");
                BXM bxm = new BXM();
                XElement xml_file = bxm.read_bxm(File.ReadAllBytes(file_path));
                MemoryStream ms = new MemoryStream();
                xml_file.Save(ms);
                using (var fs = new FileStream(System.IO.Path.GetDirectoryName(file_path) + "\\" + System.IO.Path.GetFileNameWithoutExtension(file_path) + ".xml", FileMode.Create, FileAccess.Write))
                {
                    fs.Write(ms.ToArray(), 0, ms.ToArray().Length - 1);

                }
                Update_Crueler_Status("Converted a BXM file to XML!");
            }
            else if (System.IO.Path.GetExtension(file_path) == ".xml")

            {
                Update_Crueler_Status("Converting...");
                System.Diagnostics.Debug.WriteLine("XML to BXM");
                BXM bxm = new BXM();
                string x_text = File.ReadAllText(file_path);
                string _byteOrderMarkUtf8 = Encoding.UTF8.GetString(Encoding.UTF8.GetPreamble());
                if (x_text.StartsWith("\xEE" + "\xBB" + "\xBF"))
                {
                    x_text = x_text.Remove(0, 3);
                }
                byte[] data = bxm.XmlToBXM(XElement.Parse(x_text));
                using (var fs = new FileStream(System.IO.Path.GetDirectoryName(file_path) + "\\" + System.IO.Path.GetFileNameWithoutExtension(file_path) + ".bxm", FileMode.Create, FileAccess.Write))
                {
                    fs.Write(data, 0, data.Length - 1);

                }
                Update_Crueler_Status("Converted an XML file to BXM!");
            }



        }

        private BitmapImage GetIconFromMap(string extension)
        {
            if (icon_map.ContainsKey(extension))
            {
                return icon_map[extension];
            }
            else
            {
                return icon_map["default"];
            }

        }

        private void Build_Treeview(FileNode node, TreeViewItem parent_item)
        {

            foreach (var child in node.children)
            {
                TreeViewItem tmp_item = new TreeViewItem();
                StackPanel file_stack_panel = new StackPanel();
                file_stack_panel.Orientation = Orientation.Horizontal;
                Image image = new Image();
                image.Source = GetIconFromMap(System.IO.Path.GetExtension(child.Name));
                
                

                TextBlock lbl = new TextBlock
                {
                    Text = child.Name,
                    TextWrapping = TextWrapping.NoWrap,
                    Width = 300
                };
                file_stack_panel.Children.Add(image);
                file_stack_panel.Children.Add(lbl);

                tmp_item.Header = file_stack_panel;
                tmp_item.Tag = child;


                if (child.has_children)
                {
                    Build_Treeview(child, tmp_item);
                }else if (System.IO.Path.GetExtension(child.Name) == ".bnk")
                {
                    log_file.WriteLine("Extracting WEM files from BNK.");
                    try
                    {
                        Systems.BNK bnk = new Systems.BNK();
                        TreeViewItem bnk_audio = new TreeViewItem();
                        bnk_audio.Header = "WEM Files";

                        foreach (FileNode bnk_obj in bnk.Load_BNK(child.Data))
                        {


                            TreeViewItem tmp_bnk_item = new TreeViewItem();
                            tmp_bnk_item.Header = bnk_obj.Name + ".wem";
                            tmp_bnk_item.Tag = bnk_obj;
                            ((FileNode)tmp_bnk_item.Tag).locked = true;
                            bnk_audio.Items.Add(tmp_bnk_item);

                        }
                        tmp_item.Items.Add(bnk_audio);
                    }
                    catch
                    {
                        System.Diagnostics.Debug.WriteLine("Error loading BNK");
                        log_file.WriteLine("! Error loading BNK");
                    }

                }else if (child.Name == System.IO.Path.GetFileNameWithoutExtension(loaded_file_path) + ".wta")
                {
                    // we gotta load the DTT now
                    string dtt_path = System.IO.Path.Join(System.IO.Path.GetDirectoryName(loaded_file_path), System.IO.Path.GetFileNameWithoutExtension(loaded_file_path) + ".dtt");
                    if (System.IO.Path.Exists(dtt_path))
                    {
                        log_file.WriteLine("Loading DTT.");
                        DatFileEntry[] dttfiles = DatFile.Load(File.ReadAllBytes(dtt_path));
                        DatFileEntry wtpfile;
                        foreach ( DatFileEntry datfile in dttfiles)
                        {
                            if (datfile.Name == System.IO.Path.GetFileNameWithoutExtension(loaded_file_path) + ".wtp")
                            {
                                wtpfile = datfile;
                                Texture texture = new Texture();
                                DDSFile[] dds_texture = texture.LoadWTA_WTP(child.Data, wtpfile.Data);
                                Update_Crueler_Status("Fetching texture image data from associated DTT");
                                List<FileNode> dds_children = new List<FileNode>();
                                foreach (DDSFile ddsobject in dds_texture)
                                {
                                    log_file.WriteLine("Ripping texture: " + ddsobject.ID.ToString());
                                    TreeViewItem tmp_dds_item = new TreeViewItem();
                                    file_stack_panel = new StackPanel();
                                    file_stack_panel.Orientation = Orientation.Horizontal;
                                    image = new Image();
                                    WriteableBitmap dds = DDSHandler.DDSToBitmap(ddsobject.Data);
                                    image.Source = new TransformedBitmap(dds,
                                    new ScaleTransform(
                                        32 / dds.Width,
                                        32 / dds.Height));

                                    string extension = Get_File_Extension_From_Text(ddsobject.Data);

                                    lbl = new TextBlock
                                    {
                                        Text = ddsobject.ID + extension,
                                        TextWrapping = TextWrapping.NoWrap,
                                        Width = 300
                                    };
                                    file_stack_panel.Children.Add(image);
                                    file_stack_panel.Children.Add(lbl);
                                    FileNode dds_node = new FileNode(ddsobject.ID + extension, ddsobject.Data);

                                    dds_children.Add(dds_node);
                                    tmp_dds_item.Header = file_stack_panel;
                                    tmp_dds_item.Tag = dds_node;

                                    tmp_item.Items.Add(tmp_dds_item);
                                }
                                GetFileNodeFromTag(tmp_item).nodetype = NodeType.texture;
                                GetFileNodeFromTag(tmp_item).children = dds_children.ToArray();
                                break;
                            }
                        }

                    }
                    else
                    {
                        System.Diagnostics.Debug.WriteLine("Unable to load DTT file");
                        Update_Crueler_Status("Texture load error.");
                    }
                    


                }
                else if (child.Name == System.IO.Path.GetFileNameWithoutExtension(loaded_file_path) + ".wtp")
                {
                    // we gotta load the DTT now
                    log_file.WriteLine("Loading DAT.");
                    string dtt_path = System.IO.Path.Join(System.IO.Path.GetDirectoryName(loaded_file_path),System.IO.Path.GetFileNameWithoutExtension(loaded_file_path) + ".dat");
                    if (System.IO.Path.Exists(dtt_path))
                    {

                        DatFileEntry[] dttfiles = DatFile.Load(File.ReadAllBytes(dtt_path));
                        DatFileEntry wtafile;
                        foreach (DatFileEntry datfile in dttfiles)
                        {
                            if (datfile.Name == System.IO.Path.GetFileNameWithoutExtension(loaded_file_path) + ".wta")
                            {
                                wtafile = datfile;
                                Texture texture = new Texture();
                                DDSFile[] dds_texture = texture.LoadWTA_WTP(wtafile.Data, child.Data);
                                Update_Crueler_Status("Fetching texture image data from associated DTT");
                                List<FileNode> dds_children = new List<FileNode>();
                                foreach (DDSFile ddsobject in dds_texture)
                                {
                                    log_file.WriteLine("Ripping Texture: " + ddsobject.ID.ToString());
                                    TreeViewItem tmp_dds_item = new TreeViewItem();
                                    file_stack_panel = new StackPanel();
                                    file_stack_panel.Orientation = Orientation.Horizontal;
                                    image = new Image();
                                    WriteableBitmap dds = DDSHandler.DDSToBitmap(ddsobject.Data);
                                    image.Source = new TransformedBitmap(dds,
                                    new ScaleTransform(
                                        32 / dds.Width,
                                        32 / dds.Height));

                                    string extension = Get_File_Extension_From_Text(ddsobject.Data);

                                    lbl = new TextBlock
                                    {
                                        Text = ddsobject.ID + extension,
                                        TextWrapping = TextWrapping.NoWrap,
                                        Width = 300
                                    };
                                    file_stack_panel.Children.Add(image);
                                    file_stack_panel.Children.Add(lbl);
                                    FileNode dds_node = new FileNode(ddsobject.ID + extension, ddsobject.Data);

                                    dds_children.Add(dds_node);
                                    tmp_dds_item.Header = file_stack_panel;
                                    tmp_dds_item.Tag = dds_node;

                                    tmp_item.Items.Add(tmp_dds_item);
                                }
                                GetFileNodeFromTag(tmp_item).nodetype = NodeType.texture;
                                GetFileNodeFromTag(tmp_item).children = dds_children.ToArray();
                                break;
                            }
                        }

                    }
                    else
                    {
                        System.Diagnostics.Debug.WriteLine("Unable to load DTT file");
                        Update_Crueler_Status("Texture load error.");
                    }



                }
                else if (child.Name == System.IO.Path.GetFileNameWithoutExtension(loaded_file_path) + "scr.wtb")
                {
                    log_file.WriteLine("Loading WTB Textures.");
                    Texture texture = new Texture();
                    DDSFile[] dds_texture = texture.LoadWTA_WTP(child.Data, child.Data);
                    Update_Crueler_Status("Fetching texture metdata from associated DAT");
                    GetFileNodeFromTag(tmp_item).texture_children = dds_texture;
                    List<FileNode> dds_children = new List<FileNode>();
                    foreach (DDSFile ddsobject in dds_texture)
                    {
                        log_file.WriteLine("Ripping Texture: " + ddsobject.ID.ToString());
                        TreeViewItem tmp_dds_item = new TreeViewItem();
                        file_stack_panel = new StackPanel();
                        file_stack_panel.Orientation = Orientation.Horizontal;
                        image = new Image();
                        WriteableBitmap dds = DDSHandler.DDSToBitmap(ddsobject.Data);
                        image.Source = new TransformedBitmap(dds,
                        new ScaleTransform(
                            32 / dds.Width,
                            32 / dds.Height));

                        string extension = Get_File_Extension_From_Text(ddsobject.Data);

                        lbl = new TextBlock
                        {
                            Text = ddsobject.ID + extension,
                            TextWrapping = TextWrapping.NoWrap,
                            Width = 300
                        };
                        file_stack_panel.Children.Add(image);
                        file_stack_panel.Children.Add(lbl);
                        FileNode dds_node = new FileNode(ddsobject.ID + extension, ddsobject.Data);

                        dds_children.Add(dds_node);
                        tmp_dds_item.Header = file_stack_panel;
                        tmp_dds_item.Tag = dds_node;

                        tmp_item.Items.Add(tmp_dds_item);
                    }
                    GetFileNodeFromTag(tmp_item).nodetype = NodeType.texture;
                    GetFileNodeFromTag(tmp_item).children = dds_children.ToArray();



                }
                else
                    {
                        System.Diagnostics.Debug.WriteLine("Unable to load DTT file");
                    }



               
                parent_item.Items.Add(tmp_item);

            }



        }

        private void Application_Exit(object sender, ExitEventArgs e)
        {
            log_file.Flush();
            log_file.Close();
        }

        private string Get_Icon(string Extension)
        {
            switch (Extension)
            {
                case ".xml":
                    return "/xml.png";

                default:
                    return "/xml.png";
            }
        }

        private void TreeView1_PreviewMouseDoubleClick(object sender, System.Windows.Input.MouseButtonEventArgs e)
        {
            // Cast the sender to a TreeView
            TreeView treeView = sender as TreeView;

            if (treeView != null && treeView.SelectedItem != null)
            {
                // Get the selected item
                var selectedItem = treeView.SelectedItem;

                // You can cast this to the specific type if needed
                TreeViewItem treeViewItem = selectedItem as TreeViewItem;


            }

        }

        private FileNode GetFileNodeFromTag(TreeViewItem treeViewItem)
        {
            return (FileNode)treeViewItem.Tag;
        }

        private void TreeView1_SelectedItemChanged(object sender, RoutedPropertyChangedEventArgs<object> e)
        {
            TreeView treeView = sender as TreeView;

            if (treeView != null && treeView.SelectedItem != null)
            {
                // Get the selected item
                var selectedItem = treeView.SelectedItem;

                // You can cast this to the specific type if needed
                TreeViewItem treeViewItem = selectedItem as TreeViewItem;
                selected_object = treeViewItem;

                if (System.IO.Path.GetExtension(GetFileNodeFromTag(treeViewItem).Name) == ".bxm")
                {

                    // Create a new TabItem
                    CloseableTabItem tabItem = new CloseableTabItem();
                    tabItem.SetHeader(GetFileNodeFromTag(treeViewItem).Name);

                    // Create a Frame to host the Page
                    Frame frame = new Frame();
                    frame.NavigationUIVisibility = System.Windows.Navigation.NavigationUIVisibility.Hidden;
                    frame.Content = new bxm_view(GetFileNodeFromTag(treeViewItem).Data, GetFileNodeFromTag(treeViewItem).Name, selected_object);  // Replace YourPage with your Page class

                    // Set the Frame as the content of the TabItem
                    tabItem.Content = frame;

                    tab_control.Items.Add(tabItem);
                }
                else if (System.IO.Path.GetExtension(GetFileNodeFromTag(treeViewItem).Name) == ".bnk")
                {

                    // Create a new TabItem
                    CloseableTabItem tabItem = new CloseableTabItem();
                    tabItem.SetHeader(GetFileNodeFromTag(treeViewItem).Name);

                    // Create a Frame to host the Page
                    Frame frame = new Frame();
                    frame.NavigationUIVisibility = System.Windows.Navigation.NavigationUIVisibility.Hidden;
                    frame.Content = new bnk_view(GetFileNodeFromTag(treeViewItem));  // Replace YourPage with your Page class

                    // Set the Frame as the content of the TabItem
                    tabItem.Content = frame;

                    //tab_control.Items.Add(tabItem);

                }
                else if (System.IO.Path.GetExtension(GetFileNodeFromTag(treeViewItem).Name) == ".texture")
                {




                    // Create a new TabItem
                    CloseableTabItem tabItem = new CloseableTabItem();
                    tabItem.SetHeader(GetFileNodeFromTag(treeViewItem).Name);

                    // Create a Frame to host the Page
                    Frame frame = new Frame();
                    frame.NavigationUIVisibility = System.Windows.Navigation.NavigationUIVisibility.Hidden;
                    dds_view dds_tab = new dds_view(GetFileNodeFromTag(treeViewItem));
                    frame.Content = dds_tab;  // Replace YourPage with your Page class

                    // Set the Frame as the content of the TabItem
                    tabItem.Content = frame;


                    tab_control.Items.Add(tabItem);

                    TreeViewItem tmp_item = new TreeViewItem();
                    StackPanel file_stack_panel = new StackPanel();
                    file_stack_panel.Orientation = Orientation.Horizontal;
                    Image image = new Image();

                    try
                    {
                        image.Source = new TransformedBitmap(dds_tab.GetBitmapAndReturn(),
                        new ScaleTransform(
                            32 / dds_tab.GetBitmapAndReturn().Width,
                            32 / dds_tab.GetBitmapAndReturn().Height));
                        TextBlock lbl = new TextBlock
                        {
                            Text = " " + GetFileNodeFromTag(treeViewItem).Name,
                            TextWrapping = TextWrapping.NoWrap,
                            Width = 300
                        };
                        file_stack_panel.Children.Add(image);
                        file_stack_panel.Children.Add(lbl);

                        selected_object.Header = file_stack_panel;
                    }
                    catch
                    {

                    }



                }

            }
        }

        private void Export_Click(object sender, RoutedEventArgs e)
        {
            if (selected_object.Tag != null)
            {
                var dialog = new Microsoft.Win32.SaveFileDialog();
                dialog.FileName = GetFileNodeFromTag(selected_object).Name;
                dialog.DefaultExt = System.IO.Path.GetExtension(GetFileNodeFromTag(selected_object).Name); // Default file extension
                
                dialog.Filter = "Suggested File Extension (" + System.IO.Path.GetExtension(GetFileNodeFromTag(selected_object).Name) + ")|*" + System.IO.Path.GetExtension(GetFileNodeFromTag(selected_object).Name); // Filter files by extension
                dialog.Title = "Export file";
                // Show open file dialog box
                bool? result = dialog.ShowDialog();

                // Process open file dialog box results
                if (result == true)
                {
                    byte[] data = ((FileNode)selected_object.Tag).Data;


                    if (VALID_DAT_EXTENSIONS.Contains(System.IO.Path.GetExtension(GetFileNodeFromTag(selected_object).Name)))
                    {
                        System.Diagnostics.Debug.WriteLine("Repacking " + GetFileNodeFromTag(selected_object).Name + " before export");
                        Save((string)dialog.FileName, selected_object);
                        return;

                    }else if (System.IO.Path.GetExtension(GetFileNodeFromTag(selected_object).Name) == ".wta")
                    {
                        
                        System.Diagnostics.Debug.WriteLine("Compiling Textures, Count: " + GetFileNodeFromTag(selected_object).children.Length.ToString());
                        DDSFile[] output = Texture.SaveWTA_WTP(GetFileNodeFromTag(selected_object).children);
                        

                        data = output[1].Data;
                        using (var fs = new FileStream(System.IO.Path.GetDirectoryName(dialog.FileName) + "\\" + System.IO.Path.GetFileNameWithoutExtension(dialog.FileName) + ".wtp", FileMode.Create, FileAccess.Write))
                        {
                            fs.Write(data, 0, data.Length);


                        }
                        data = output[0].Data;
                    }




                    // Open document
                    string filename = dialog.FileName;
                    using (var fs = new FileStream(filename, FileMode.Create, FileAccess.Write))
                    {
                        fs.Write(data, 0, data.Length);

                    }

                }
            }


        }
        private void Replace_Click(object sender, RoutedEventArgs e)
        {
            if (selected_object.Tag != null)
            {
                if (!((FileNode)selected_object.Tag).locked)
                {
                    if (selected_object.Items.Count > 1)
                    {

                        AdonisUI.Controls.MessageBox.Show("Replacing container files will not do anything\nConsider using 'Add' file instead and deleting the file you wish to replace", "CruelerThan.DAT", AdonisUI.Controls.MessageBoxButton.OK, AdonisUI.Controls.MessageBoxImage.Information);
                        return;
                    }
                }
                else
                {
                    AdonisUI.Controls.MessageBox.Show("Cannot replace this file, the application has locked it\n(This is most likely because the file is in an unrepackable container)", "CruelerThan.DAT", AdonisUI.Controls.MessageBoxButton.OK, AdonisUI.Controls.MessageBoxImage.Error);
                    return;
                }

                string filter_ext = System.IO.Path.GetExtension(GetFileNodeFromTag(selected_object).Name);


                var dialog = new Microsoft.Win32.OpenFileDialog();
                if (filter_ext == ".texture")
                {
                    edited_wta_file = true;
                    filter_ext = ".dds";
                    dialog.FilterIndex = 2;

                }

                dialog.FileName = GetFileNodeFromTag(selected_object).Name;
                dialog.DefaultExt = System.IO.Path.GetExtension(GetFileNodeFromTag(selected_object).Name); // Default file extension
                dialog.Filter = "Suggested File Extension (" + filter_ext + ")|*" + filter_ext + "|MGR/Nier Textures (*.dds;*.png)|*.dds;*.png|MGR/Nier Models (*.wmb)|*.wmb|All files (*.*)|*.*"; // Filter files by extension
                dialog.Title = "Replace file";
                // Show open file dialog box
                bool? result = dialog.ShowDialog();

                // Process open file dialog box results
                if (result == true)
                {
                    ((FileNode)selected_object.Tag).Data = File.ReadAllBytes(dialog.FileName);
                    
                }

            }




        }

        private void Delete_Click(object sender, RoutedEventArgs e)
        {
            if (((FileNode)selected_object.Tag).locked)
            {
                AdonisUI.Controls.MessageBox.Show("Cannot delete this file, the application has locked it\n(This is most likely because the file is in an unrepackable container)", "CruelerThan.DAT", AdonisUI.Controls.MessageBoxButton.OK, AdonisUI.Controls.MessageBoxImage.Error);
                return;
            }
            try
            {
                System.Diagnostics.Debug.WriteLine("Removed " + GetFileNodeFromTag(selected_object).Name);
                string filter_ext = System.IO.Path.GetExtension(GetFileNodeFromTag(selected_object).Name);


                var dialog = new Microsoft.Win32.OpenFileDialog();
                if (filter_ext == ".texture")
                {
                    edited_wta_file = true;

                }
                TreeViewItem parent = (TreeViewItem)selected_object.Parent;
                parent.Items.Remove(selected_object);
            }
            catch (Exception ex)
            {
                AdonisUI.Controls.MessageBox.Show("Error (You can most likely ignore this)\n" + ex.ToString(), "CruelerThan.DAT", AdonisUI.Controls.MessageBoxButton.OK, AdonisUI.Controls.MessageBoxImage.Error);
            }

        }

        private void Save(string file_location, TreeViewItem file)
        {
            byte[] out_data = SaveDATRecursive(file);


            using (var fs = new FileStream(file_location, FileMode.Create, FileAccess.Write))
            {
                fs.Write(out_data, 0, out_data.Length);

            }
            // Saving WTP files to DTT
            if (dtt_save_required)
            {
                Debug.WriteLine("Saving DTT.");
                // Convert to support both DAT and DTT later.
                string dtt_path = System.IO.Path.GetDirectoryName(loaded_file_path) + "\\" + System.IO.Path.GetFileNameWithoutExtension(loaded_file_path) + ".dtt";

                DatFileEntry[] dttfiles = DatFile.Load(File.ReadAllBytes(dtt_path));
                int wtp_file = -1;
                foreach (DatFileEntry dttfile in dttfiles)
                {
                    Debug.WriteLine(dttfile.Name);
                    wtp_file++;
                    if (dttfile.Name == System.IO.Path.GetFileNameWithoutExtension(loaded_file_path) + ".wtp")
                    {

                        break;
                    }
                    
                }
                if (wtp_file == -1)
                {
                    AdonisUI.Controls.MessageBox.Show("Error saving DTT! (WTP file was not found)", "CruelerThan.DAT", AdonisUI.Controls.MessageBoxButton.OK, AdonisUI.Controls.MessageBoxImage.Error);
                    return;
                }

                dttfiles[wtp_file].Data = wtp_wta_data[1].Data;
                byte[] dtt_data = DatFile.Save(dttfiles);

                using (var fs = new FileStream(System.IO.Path.GetDirectoryName(file_location) + "\\" + System.IO.Path.GetFileNameWithoutExtension(file_location) + ".dtt", FileMode.Create, FileAccess.Write))
                {
                    fs.Write(dtt_data, 0, dtt_data.Length);

                }
            } // Saving WTA files to DAT
            if (dat_save_required)
            {
                Debug.WriteLine("Saving DAT.");
                // Convert to support both DAT and DTT later.
                string dtt_path = System.IO.Path.GetDirectoryName(loaded_file_path) + "\\" + System.IO.Path.GetFileNameWithoutExtension(loaded_file_path) + ".dat";

                DatFileEntry[] dttfiles = DatFile.Load(File.ReadAllBytes(dtt_path));
                int wtp_file = 0;
                foreach (DatFileEntry dttfile in dttfiles)
                {
                    wtp_file++;
                    if (dttfile.Name == System.IO.Path.GetFileNameWithoutExtension(loaded_file_path) + ".wta")
                    {

                        break;
                    }
                    
                }
                dttfiles[wtp_file].Data = wtp_wta_data[0].Data;
                byte[] dtt_data = DatFile.Save(dttfiles);

                using (var fs = new FileStream(System.IO.Path.GetDirectoryName(file_location) + "\\" + System.IO.Path.GetFileNameWithoutExtension(file_location) + ".dat", FileMode.Create, FileAccess.Write))
                {
                    fs.Write(dtt_data, 0, dtt_data.Length);

                }
            }
        }

            
        private Byte[] SaveDATRecursive(TreeViewItem Tree_View_Item)
        {
            List<DatFileEntry> files_out = new List<DatFileEntry>();

            foreach (TreeViewItem Node in Tree_View_Item.Items)
            {
                if (Node.Items.Count > 1)
                {
                    if (VALID_DAT_EXTENSIONS.Contains(System.IO.Path.GetExtension(((FileNode)Node.Tag).Name)))
                    {
                        Byte[] tmp_bytes = SaveDATRecursive(Node);
                        DatFileEntry tmp_file = new DatFileEntry();
                        tmp_file.Name = ((FileNode)Node.Tag).Name;
                        tmp_file.Data = tmp_bytes;
                        files_out.Add(tmp_file);
                    }
                    else if (((FileNode)Node.Tag).Name == System.IO.Path.GetFileNameWithoutExtension(loaded_file_path) + ".wta" && edited_wta_file)
                    {
                        DDSFile[] tex_output = Texture.SaveWTA_WTP(((FileNode)Node.Tag).children);
                        wtp_wta_data = tex_output;
                        DatFileEntry tmp_file = new DatFileEntry();
                        tmp_file.Name = ((FileNode)Node.Tag).Name;
                        tmp_file.Data = tex_output[0].Data;
                        files_out.Add(tmp_file);
                        dtt_save_required = true;

                    }
                    else if (((FileNode)Node.Tag).Name == System.IO.Path.GetFileNameWithoutExtension(loaded_file_path) + ".wtp" && edited_wta_file)
                    {
                        DDSFile[] tex_output = Texture.SaveWTA_WTP(((FileNode)Node.Tag).children);
                        wtp_wta_data = tex_output;
                        DatFileEntry tmp_file = new DatFileEntry();
                        tmp_file.Name = ((FileNode)Node.Tag).Name;
                        tmp_file.Data = tex_output[1].Data;
                        files_out.Add(tmp_file);
                        dat_save_required = true;

                    }
                    else if (((FileNode)Node.Tag).Name == System.IO.Path.GetFileNameWithoutExtension(loaded_file_path) + "scr.wtb" && edited_wta_file)
                    {
                        byte[] tex_output = Texture.SaveWTB(((FileNode)Node.Tag).children);
                        DatFileEntry tmp_file = new DatFileEntry();
                        tmp_file.Name = ((FileNode)Node.Tag).Name;
                        tmp_file.Data = tex_output;
                        files_out.Add(tmp_file);

                    }
                    else
                    {
                        DatFileEntry tmp_file = new DatFileEntry();
                        tmp_file.Name = ((FileNode)Node.Tag).Name;
                        tmp_file.Data = ((FileNode)Node.Tag).Data;
                        files_out.Add(tmp_file);
                    }


                }
                else
                {
                    DatFileEntry tmp_file = new DatFileEntry();
                    tmp_file.Name = ((FileNode)Node.Tag).Name;
                    tmp_file.Data = ((FileNode)Node.Tag).Data;
                    files_out.Add(tmp_file);
                }

            }

            Byte[] output_bytes = DatFile.Save(files_out.ToArray());

            return output_bytes;
        }
    }
}
