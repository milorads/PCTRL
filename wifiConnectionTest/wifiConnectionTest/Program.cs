using System;
using System.Collections.Generic;
using System.Data;
using System.IO;
using System.Linq;
using System.Net;
using System.Runtime.InteropServices;
using System.Text;
using ClosedXML.Excel;
using NativeWifi;

namespace wifiConnectionTest
{
    class Program
    {

        private static string myResponse = String.Empty;
        private static List<Presence> objectList;
        static void Main(string[] args)
        {
            //firstConnectionTest();
            //ConnectToWifi();
            MakeRequest();
            ParseResult(); // needs work
            //WriteListToExcel(); // needs work
            WriteToExcel(); // needs work
        }

        private static void WriteToExcel()
        {
            Microsoft.Office.Interop.Excel.Application xlApp = new Microsoft.Office.Interop.Excel.Application();
            if (xlApp == null)
            {
                Console.WriteLine("Excel not installed");
            }
            object misValue = System.Reflection.Missing.Value;
            var xlWorkBook = xlApp.Workbooks.Add(misValue);
           var xlWorkSheet = (Microsoft.Office.Interop.Excel.Worksheet)xlWorkBook.Worksheets.Item[1];
            xlWorkSheet.Cells[1, 1] = "ID";
            xlWorkSheet.Cells[1, 2] = "Name";
            xlWorkSheet.Cells[1, 3] = "Prezime";
            xlWorkSheet.Cells[1, 4] = "Ulaz";
            xlWorkSheet.Cells[1, 5] = "Izlaz";
            xlWorkSheet.Cells[1, 6] = "Zadržavanje(u minutima)";
            int i = 2;
            foreach (var obj in objectList)
            {
                xlWorkSheet.Cells[i, 1] = obj.Id;
                xlWorkSheet.Cells[i, 2] = obj.Ime;
                xlWorkSheet.Cells[i, 3] = obj.Prezime;
                xlWorkSheet.Cells[i, 4] = obj.Ulaz.ToOADate();
                xlWorkSheet.Cells[i, 5] = obj.Izlaz.ToOADate();
                DateTime startTime = obj.Ulaz;
                DateTime endTime = obj.Izlaz;
                xlWorkSheet.Cells[i++, 6] = endTime.Subtract(startTime).Minutes;
            }

            Microsoft.Office.Interop.Excel.Range ulazRange = xlWorkSheet.get_Range("D:D");
            ulazRange.NumberFormat = "DD/MM/YYYY HH:mm";
            Marshal.FinalReleaseComObject(ulazRange);
            Microsoft.Office.Interop.Excel.Range izlazRange = xlWorkSheet.get_Range("E:E");
            izlazRange.NumberFormat = "DD/MM/YYYY HH:mm";
            Marshal.FinalReleaseComObject(izlazRange);

            // we will have full data report - all the data
            // concatenated all the presences of same person into length of his presence for the day
            // same as above but only calculating the class time which is input
            xlWorkBook.SaveAs(Directory.GetCurrentDirectory()+$"\\FullDataReport_DD-MM-YY-{ShortId.GetBase36(5)}.xls",
                Microsoft.Office.Interop.Excel.XlFileFormat.xlWorkbookNormal, misValue, misValue, misValue, misValue,
                Microsoft.Office.Interop.Excel.XlSaveAsAccessMode.xlExclusive, misValue, misValue, misValue, misValue, misValue);
            xlWorkBook.Close(true, misValue, misValue);
            xlApp.Quit();

            Marshal.ReleaseComObject(xlWorkSheet);
            Marshal.ReleaseComObject(xlWorkBook);
            Marshal.ReleaseComObject(xlApp);
        }

        private static void WriteListToExcel()
        {
            //closed excel library
            var workbook = new XLWorkbook();
            workbook.AddWorksheet("sheetName");
            var ws = workbook.Worksheet("sheetName");

            int row = 1;
            foreach (Presence item in objectList)
            {
                ws.Cell("A" + row.ToString()).Value = item.ToString();
                row++;
            }

            workbook.SaveAs("filename.xlsx");
        }

        private static void ParseResult()
        {
            List<string> presenceList = myResponse.Replace("\n", "").Split('\r').ToList<string>();
            objectList = new List<Presence>();
            foreach (string line in presenceList)
            {
                if (!string.IsNullOrEmpty(line))
                {
                    var splitLine = line.Split('|');
                    Presence currPresence = new Presence(System.Web.HttpUtility.HtmlDecode(splitLine[0]), System.Web.HttpUtility.HtmlDecode(splitLine[1]), splitLine[2], DateTime.ParseExact(splitLine[3], "yyyy-MM-ddTHH:mm", null), DateTime.ParseExact(splitLine[4], "yyyy-MM-ddTHH:mm", null));
                    objectList.Add(currPresence);
                }
            }
            //foreach (var line in presenceList)
            //{
            //    var splitLine = line.Split('|');
            //    //rucno
            //    DateTime oDate = DateTime.ParseExact(splitLine[1], "yyyy-MM-ddTHH:mm", null);
            //    bool oBool = Boolean.Parse(splitLine[2]);
            //    PresenceTemp presT = new PresenceTemp()
            //    {
            //        Ime = splitLine[0],
            //        UlazIzlaz = oBool,
            //        Vrijeme = oDate
            //    };
            //    //putem metoda
            //    PresenceTemp presT2 = new PresenceTemp() { Ime = splitLine[0]};
            //    presT.SetUlazIzlaz(splitLine[2]);
            //    presT.SetVrijeme(splitLine[1]);
            //    //putem konstruktora #1
            //    PresenceTemp presT3 = new PresenceTemp(line);
            //    //putem konstruktora #2
            //    PresenceTemp presT4 = new PresenceTemp(splitLine[0],splitLine[1],splitLine[2]);
            //    //putem konstruktora #3
            //    PresenceTemp presT5 = new PresenceTemp(splitLine);
            //    //dodajemo objekat u listu
                //objectList.Add(presT); // svejedno koji jer su svi isti
            //}
            // get reg list
            //write to excel
        }

        public static void MakeRequest()
        {
            var url = "http://192.168.4.1/metoda3?polje_user=a&polje_password=a";
            //var url = "https://jsonplaceholder.typicode.com/posts/1";
            string html = string.Empty;

            HttpWebRequest request = (HttpWebRequest)WebRequest.Create(url);//
            request.Method = "Get";
            //request.ContentType =  "text/html; charset=utf-8";
            //request.KeepAlive = true;
            //request.Timeout = 60;
            ////request.ContentType = "text/plain";
            //request.Headers.Add("Content-Type", "text/plain");
            //request.ContentType = "application/x-www-form-urlencoded";

            HttpWebResponse response = (HttpWebResponse)request.GetResponse();
            //response.ContentType = "text/html; charset=utf-8";
            myResponse = "";
            using (System.IO.StreamReader sr = new System.IO.StreamReader(response.GetResponseStream()))
            {
                myResponse = sr.ReadToEnd();
            }
        }

        public static void ConnectToWifi()
        {
            WlanClient client = new WlanClient();
            foreach (WlanClient.WlanInterface wlanIface in client.Interfaces)
            {
                Wlan.WlanAvailableNetwork[] networks = wlanIface.GetAvailableNetworkList(Wlan.WlanGetAvailableNetworkFlags.IncludeAllManualHiddenProfiles);
                foreach (Wlan.WlanAvailableNetwork network in networks)
                {
                        var ssid = Encoding.UTF8.GetString(network.dot11Ssid.SSID).ToCharArray().Where(x => x != '\0').Aggregate("", (current, x) => current + x);
                        Console.WriteLine("Found network with SSID {0}.", ssid);
                }

                // Retrieves XML configurations of existing profiles.
                // This can assist you in constructing your own XML configuration
                // (that is, it will give you an example to follow).
                var profileName = "";
                var profileXml = "";
                foreach (Wlan.WlanProfileInfo profileInfo in wlanIface.GetProfiles())
                {
                    string name = profileInfo.profileName; // this is typically the network's SSID
                    string xml = wlanIface.GetProfileXml(profileInfo.profileName);
                    if (name == "Proba")
                    {
                        profileName = name;
                        profileXml = xml;
                    }
                }
                wlanIface.SetProfile(Wlan.WlanProfileFlags.AllUser, profileXml, true);
                wlanIface.Connect(Wlan.WlanConnectionMode.Profile, Wlan.Dot11BssType.Any, profileName);


            }
        }
    }
}
