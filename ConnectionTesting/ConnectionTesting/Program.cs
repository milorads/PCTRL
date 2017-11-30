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
            var url = "http://192.168.4.1/metoda3?polje_user=a&polje_password=a";
            //var url = "https://jsonplaceholder.typicode.com/posts/1";
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
            var presenceList = myResponse.Replace("\n", "").Split('\r');
            //var presenceList = pList.tol
            objectList = new List<Presence>();
            foreach (string line in presenceList)
            {
                var splitLine = line.Split('|');
                Presence currPresence = new Presence(splitLine[0], splitLine[1], splitLine[2], DateTime.ParseExact(splitLine[3], "yyyy-MM-ddTHH:mm", null), DateTime.ParseExact(splitLine[4], "yyyy-MM-ddTHH:mm", null));
                objectList.Add(currPresence);
            }
        }
    }
}
