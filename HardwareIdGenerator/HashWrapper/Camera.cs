using System;
using System.Collections.Generic;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using Emgu.CV;
using Microsoft.Win32.SafeHandles;
using ZXing;
using ZXing.Common;

namespace IdGenerator
{
    public class Camera : IDisposable
    {
        public static void GetImage()
        {
            VideoCapture capture = new VideoCapture(); //create a camera captue
            Bitmap image = capture.QueryFrame().Bitmap; //take a picture
            capture.Dispose();
            image.Save(Directory.GetCurrentDirectory() + $"\\CameraCode_{Guid.NewGuid()}.png");
            SaveCapturedImage(image);
        }

        private static void SaveCapturedImage(Bitmap image)
        {
            //var barcodeWriter = new BarcodeWriter { Format = BarcodeFormat.QR_CODE };
            //barcodeWriter
            //    .Write(input)
            //    .Save(Directory.GetCurrentDirectory() + $"\\QrCode_{shortId}");
            //return Directory.GetCurrentDirectory() + $"\\QrCode_{shortId}";
            var barcodeReader = new BarcodeReader() { Options = new DecodingOptions(){TryHarder = true}};
            var a = barcodeReader.Decode(image);

        }

        private void tick(object sender, EventArgs evtArgs)
        {
            var bitmap = GetCurrentImage();
            if (bitmap == null)
                return;
            var reader = new BarcodeReader();
            var result = reader.Decode(bitmap);
            if (result != null)
            {
            }
        }

        public static Bitmap GetCurrentImage()
        {
            VideoCapture capture = new VideoCapture(); //create a camera captue
            Bitmap image = capture.QueryFrame().Bitmap; //take a picture
            capture.Dispose();
            return image;
        }

        bool disposed = false;
        // Instantiate a SafeHandle instance.
        SafeHandle handle = new SafeFileHandle(IntPtr.Zero, true);

        // Public implementation of Dispose pattern callable by consumers.
        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        // Protected implementation of Dispose pattern.
        protected virtual void Dispose(bool disposing)
        {
            if (disposed)
                return;

            if (disposing)
            {
                handle.Dispose();
                // Free any other managed objects here.
                //
            }

            // Free any unmanaged objects here.
            //
            disposed = true;
        }

        ~Camera()
        {
            Dispose(false);
            GC.SuppressFinalize(this);
        }
    }
}
