using RisingFormats;
using SunsetLib;
using SunsetLib.MetalGearRising;
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

namespace CruelerThanDAT.Tabs
{
    /// <summary>
    /// Interaction logic for bnk_view.xaml
    /// </summary>
    public partial class bnk_view : Page
    {
        public bnk_view(FileNode file)
        {
            InitializeComponent();

            SunsetLib.MetalGearRising.WWISE wwise = new SunsetLib.MetalGearRising.WWISE();

            SunsetLib.MetalGearRising.WWISE_BNK bnk = wwise.Read_BNK(file.Data);

            foreach (BNK_HIRC_OBJ obj in bnk.hirc.objects)
            {
                if (obj.type == SunsetLib.MetalGearRising.WwiseObjects.evnt)
                {
                    BNK_EVENTS.Items.Add(obj.uid.ToString());
                }
            }

        }
    }
}
