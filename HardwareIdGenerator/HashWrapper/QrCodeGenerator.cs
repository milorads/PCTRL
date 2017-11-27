using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ZXing;

namespace IdGenerator
{
    public class QrCodeGenerator
    {
        internal static string GenerateQrCode(string input, string shortId)
        {
            var barcodeWriter = new BarcodeWriter {Format = BarcodeFormat.QR_CODE};
            barcodeWriter
                .Write(input)
                .Save(Directory.GetCurrentDirectory()+$"\\QrCode_{shortId}");
            return Directory.GetCurrentDirectory() + $"\\QrCode_{shortId}";
        }
    }
}
