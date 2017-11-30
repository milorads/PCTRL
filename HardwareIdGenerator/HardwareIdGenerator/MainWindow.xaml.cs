using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Timers;
using System.Windows;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using IdGenerator;
using ZXing;
using Image = System.Windows.Controls.Image;

namespace HardwareIdGenerator
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
        }

        private void Button_Click(object sender, RoutedEventArgs e)
        {
            var qrFileName = IdGenerator.IdGenerator.GetId(out var id, out var hashedId);
            Image finalImage = new Image();
            BitmapImage logo = new BitmapImage();
            logo.BeginInit();
            logo.UriSource = new Uri(qrFileName);
            logo.EndInit();
            QrSlika.Source = logo;
            IdHashed.Content = hashedId;
            IdNonHashed.Content = id;
        }

        private void tick(object sender, EventArgs evtArgs)
        {
            var bitmap = Camera.GetCurrentImage();//GetCurrentImage();
            if (bitmap == null)
                return;
            var reader = new BarcodeReader();
            var result = reader.Decode(bitmap);
            if (result != null)
            {
                IdHashed.Content = result.Text;
            }
            bitmap.Dispose();
        }

        private bool active = false;
        private Timer webCamTimer;

        private void Button_Click_1(object sender, RoutedEventArgs e)
        {

            if (!active)
            {
                webCamTimer = new Timer();
                webCamTimer.Elapsed += tick;
                webCamTimer.Interval = 500;
                webCamTimer.Start();
            }
            else
            {
                webCamTimer.Stop();
                webCamTimer = null;
            }
            active = !active;
        }
    }
}
