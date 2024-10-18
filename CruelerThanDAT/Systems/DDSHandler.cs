using DDSReader;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Media.Imaging;
using System.Windows.Media;
using System.Windows;
using System.Xml.Linq;

namespace CruelerThanDAT.Systems
{
    public class DDSHandler
    {
        public static WriteableBitmap DDSToBitmap(byte[] data)
        {
            byte[] image_data = data;

            // DDS Check
            Stream stream = new MemoryStream(image_data);
            BinaryReader reader = new BinaryReader(stream);
            uint header = reader.ReadUInt32();


            if (header == 542327876)
            {
                reader.BaseStream.Seek(84, SeekOrigin.Begin);
                string DXTTYPE = new string(reader.ReadChars(4));
                if (DXTTYPE == "DXT5")
                {

                    DDSImage d_image = new DDSImage(image_data);
                    Pfim.IImage img = d_image._image;

                    var buffer = img.Data;
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

                        
                    }
                    return bmp;


                }
                else if (DXTTYPE == "DXT3")
                {

                    DDSImage d_image = new DDSImage(image_data);
                    Pfim.IImage img = d_image._image;

                    var buffer = img.Data;
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


                    }

                    return bmp;


                }
                else if (DXTTYPE == "DXT1")
                {
                    DDSImage d_image = new DDSImage(image_data);
                    Pfim.IImage img = d_image._image;

                    var buffer = img.Data;

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

                    }
                    return bmp;
                }
            }
            else if (header == 1196314761)
            {
                BitmapImage bmp_png = LoadImage(image_data);

                return new WriteableBitmap(bmp_png);


            }

            return null;

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
    }

}

