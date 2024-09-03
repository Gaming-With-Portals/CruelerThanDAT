using CruelerThanDAT.Systems;
using System;
using System.Collections.Generic;
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
using DDSReader;
using System.Diagnostics;
using SunsetLib.MetalGearRising;
using System.IO;
using SunsetLib;

namespace CruelerThanDAT.Tabs
{
    /// <summary>
    /// Interaction logic for dds_view.xaml
    /// </summary>
    public partial class dds_view : Page
    {
        private bool flipped_x = false;
        private bool flipped_y = false;
        private FileNode f_node;
        public dds_view(FileNode image_data)
        {
            InitializeComponent();
            f_node = image_data;

            // DDS Check
            Stream stream = new MemoryStream(image_data.Data);
            BinaryReader reader = new BinaryReader(stream);
            uint header = reader.ReadUInt32();


            if (header == 542327876)
            {
                reader.BaseStream.Seek(84, SeekOrigin.Begin);
                string DXTTYPE = new string(reader.ReadChars(4));
                if (DXTTYPE == "DXT5")
                {

                    DDSImage d_image = new DDSImage(image_data.Data);
                    Pfim.IImage img = d_image._image;

                    var buffer = img.Data;
                    Data.Content = img.Width.ToString() + "x" + img.Height.ToString() + " | TYPE: DDS, " + DXTTYPE;
                    WriteableBitmap bmp = new WriteableBitmap(img.Width, img.Height, img.Width, img.Height, PixelFormats.Bgra32, null);
                    bmp.Lock();

                    try
                    {
                        for (var x = 0; x < img.Width; x++)
                        {
                            for (var y = 0; y < img.Height; y++)
                            {
                                var offset = (y * img.Width + x) * 4;

                                IntPtr backbuffer = bmp.BackBuffer;
                                backbuffer += offset;

                                var r = buffer[offset];
                                var g = buffer[offset + 1];
                                var b = buffer[offset + 2];
                                var a = buffer[offset + 3];
                                // Hack-y color space fix for DDS files
                                int color = a << 24 | b << 16 | g << 8 | r;

                                System.Runtime.InteropServices.Marshal.WriteInt32(backbuffer, color);
                            }
                        }
                        bmp.AddDirtyRect(new Int32Rect(0, 0, img.Width, img.Height));
                    }
                    finally
                    {
                        bmp.Unlock();

                        DisplayImage.Source = bmp;
                    }


                }
                else if (DXTTYPE == "DXT3")
                {

                    DDSImage d_image = new DDSImage(image_data.Data);
                    Pfim.IImage img = d_image._image;

                    var buffer = img.Data;
                    Data.Content = img.Width.ToString() + "x" + img.Height.ToString() + " | TYPE: DDS, " + DXTTYPE;
                    WriteableBitmap bmp = new WriteableBitmap(img.Width, img.Height, img.Width, img.Height, PixelFormats.Bgra32, null);
                    bmp.Lock();

                    try
                    {
                        for (var x = 0; x < img.Width; x++)
                        {
                            for (var y = 0; y < img.Height; y++)
                            {
                                var offset = (y * img.Width + x) * 4;

                                IntPtr backbuffer = bmp.BackBuffer;
                                backbuffer += offset;

                                var r = buffer[offset];
                                var g = buffer[offset + 1];
                                var b = buffer[offset + 2];
                                var a = buffer[offset + 3];
                                // Hack-y color space fix for DDS files
                                int color = a << 24 | b << 16 | g << 8 | r;

                                System.Runtime.InteropServices.Marshal.WriteInt32(backbuffer, color);
                            }
                        }
                        bmp.AddDirtyRect(new Int32Rect(0, 0, img.Width, img.Height));
                    }
                    finally
                    {
                        bmp.Unlock();

                        DisplayImage.Source = bmp;
                    }


                }
                else if (DXTTYPE == "DXT1")
                {
                    DDSImage d_image = new DDSImage(image_data.Data);
                    Pfim.IImage img = d_image._image;

                    var buffer = img.Data;
                    Data.Content = img.Width.ToString() + "x" + img.Height.ToString() + " | TYPE: DDS, " + DXTTYPE;

                    using (var fs = new FileStream("F:\\image.bin", FileMode.Create, FileAccess.Write))
                    {
                        fs.Write(buffer, 0, buffer.Length);

                    }

                    WriteableBitmap bmp = new WriteableBitmap(img.Width, img.Height, img.Width, img.Height, PixelFormats.Rgb24, null);
                    bmp.Lock();

                    try
                    {
                        for (var x = 0; x < img.Width; x++)
                        {
                            for (var y = 0; y < img.Height; y++)
                            {
                                var offset = (y * img.Width + x) * 3;

                                IntPtr backbuffer = bmp.BackBuffer;
                                backbuffer += offset;

                                var r = buffer[offset];
                                var g = buffer[offset + 1];
                                var b = buffer[offset + 2];
                                // Hack-y color space fix for DDS files
                                int color = b << 16 | g << 8 | r;

                                // System.Runtime.InteropServices.Marshal.WriteInt32(backbuffer, color);
                                System.Runtime.InteropServices.Marshal.WriteByte(backbuffer, b);
                                System.Runtime.InteropServices.Marshal.WriteByte(backbuffer + 1, g);
                                System.Runtime.InteropServices.Marshal.WriteByte(backbuffer + 2, r);


                            }
                        }
                        bmp.AddDirtyRect(new Int32Rect(0, 0, img.Width, img.Height));
                    }
                    finally
                    {
                        bmp.Unlock();

                        DisplayImage.Source = bmp;
                    }
                }
            }
            else if (header == 1196314761)
            {
                BitmapImage bmp_png = LoadImage(image_data.Data);
                DisplayImage.Source = bmp_png;

                Data.Content = "0x0 (PNG uses PPI instead of Resolution)" + "  | TYPE: PNG, UNCOMPRESSED";

            }

        }

        public BitmapSource GetBitmapAndReturn()
        {
            return (BitmapSource)DisplayImage.Source;
        }

        private static BitmapImage LoadImage(byte[] imageData)
        {
            if (imageData == null || imageData.Length == 0) return null;
            var image = new BitmapImage();
            using (var mem = new MemoryStream(imageData))
            {
                mem.Position = 0;
                image.BeginInit();
                image.CreateOptions = BitmapCreateOptions.PreservePixelFormat;
                image.CacheOption = BitmapCacheOption.OnLoad;
                image.UriSource = null;
                image.StreamSource = mem;
                image.EndInit();
            }
            image.Freeze();

            return image;
        }

        private void Alpha_Channel_Click(object sender, RoutedEventArgs e)
        {
            
        }

        private void Flip_X_Click(object sender, RoutedEventArgs e)
        {
            DisplayImage.RenderTransformOrigin = new Point(0.5, 0.5);
            ScaleTransform flipTrans = new ScaleTransform();

            if (!flipped_x)
            {
                flipTrans.ScaleX = -1;
                flipped_x = true;
            }
            else
            {
                flipTrans.ScaleX = 1;
                flipped_x = false;
            }
            if (flipped_y)
            {
                flipTrans.ScaleY = -1;
            }
            else
            {
                flipTrans.ScaleY = 1;
            }

            DisplayImage.RenderTransform = flipTrans;
        }

        private void Flip_Y_Click(object sender, RoutedEventArgs e)
        {
            DisplayImage.RenderTransformOrigin = new Point(0.5, 0.5);
            ScaleTransform flipTrans = new ScaleTransform();

            if (flipped_x)
            {
                flipTrans.ScaleX = -1;
            }
            else
            {
                flipTrans.ScaleX = 1;
            }
            if (!flipped_y)
            {
                flipTrans.ScaleY = -1;
                flipped_y = true;
            }
            else
            {
                flipTrans.ScaleY = 1;
                flipped_y = false;
            }

            DisplayImage.RenderTransform = flipTrans;
        }

        private void BMP_Export_Click(object sender, RoutedEventArgs e)
        {
            // Actually saves as PNG, ignore the name
            var dialog = new Microsoft.Win32.SaveFileDialog();
            dialog.FileName = System.IO.Path.GetFileNameWithoutExtension(f_node.Name) + ".png";
            dialog.DefaultExt = System.IO.Path.GetExtension(".png"); // Default file extension

            dialog.Filter = "Portable Network Graphics Format (.png)|*.png"; // Filter files by extension
            dialog.Title = "Export file";
            // Show open file dialog box
            bool? result = dialog.ShowDialog();

            // Process open file dialog box results
            if (result == true)
            {
                var encoder = new PngBitmapEncoder();
                encoder.Frames.Add(BitmapFrame.Create((BitmapSource)DisplayImage.Source));
                using (FileStream stream = new FileStream(dialog.FileName, FileMode.Create))
                    encoder.Save(stream);
 

            }



        }
    }


}


