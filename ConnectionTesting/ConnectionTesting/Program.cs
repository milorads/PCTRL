using System;
using System.Collections.Generic;
using System.Net;
using System.Text;
using wifiConnectionTest;

namespace ConnectionTesting
{
    class Program
    {
        private static string myResponse;
        private static List<Presence> objectList;

        static void Main(string[] args)
        {
            //var url = "http://192.168.4.1/metoda3?polje_pass=wefwe&polje_user=evwv";
            var url = "https://jsonplaceholder.typicode.com/posts/1";
            string html = string.Empty;

            HttpWebRequest request = (HttpWebRequest)WebRequest.Create(url);//
            request.Method = "Get";
            //request.KeepAlive = true;
            //request.Timeout = 60;
            ////request.ContentType = "text/plain";
            ////request.Headers.Add("Content-Type", "text/plain");
            //request.ContentType = "application/x-www-form-urlencoded";

            HttpWebResponse response = (HttpWebResponse)request.GetResponse();
            myResponse = "";
            using (System.IO.StreamReader sr = new System.IO.StreamReader(response.GetResponseStream()))
            {
                myResponse = sr.ReadToEnd();
            }
            List<string> presenceList = myResponse.Replace("\n", "").Split('\r').ToList<string>();
            objectList = new List<Presence>();
            foreach (string line in presenceList)
            {
                var splitLine = line.Split('|');
                Presence currPresence = new Presence(splitLine[0], splitLine[1], splitLine[2], DateTime.ParseExact(splitLine[3], "yyyy-MM-ddTHH:mm", null), DateTime.ParseExact(splitLine[4], "yyyy-MM-ddTHH:mm", null));
                objectList.Add(currPresence);
            }
        }
        private static void ParseResult()
        {

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

        }
    }
}
