
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;


namespace CruelerThanDAT.ui
{


    public class CloseableTabItem : TabItem
    {
        public void SetHeader(string header)
        {
            Background = new SolidColorBrush(Color.FromRgb(61, 61, 76));
             
            
            // Container for header controls
            var dockPanel = new DockPanel();
            
            var text = new TextBlock { Text = header + "  " };
            text.Foreground = new SolidColorBrush(Color.FromRgb(255, 255, 255));
            text.Background = new SolidColorBrush(Color.FromRgb(61, 61, 76));
            dockPanel.Children.Add(text);
            
            // Close button to remove the tab
            var closeButton = new TabCloseButton();
            closeButton.Click +=
                (sender, e) =>
                {
                    var tabControl = Parent as ItemsControl;
                    tabControl.Items.Remove(this);
                };
            dockPanel.Children.Add(closeButton);

            // Set the header
            Header = dockPanel;
        }
    }
}
