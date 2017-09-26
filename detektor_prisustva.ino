#include <SD.h>
//#include <DS1302.h>
#include <Wire.h> // must be included here so that Arduino library object file references work
#include <RtcDS3231.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

extern "C" {
#include<user_interface.h>
}

#define countof(a) (sizeof(a) / sizeof(a[0]))
#define MAX 3
#define date (convertDateToStr(rtc.GetDateTime()))

const char* ssid = "Proba";

ESP8266WebServer server(80);

String webPage = "";

File myFile;
File myFile2;

String prijavljeni[MAX];

//DS1302 rtc(2, 3, 4);
RtcDS3231<TwoWire> rtc(Wire);

void setup(void)
{  
  webPage += "<!DOCTYPE html><html lang='en'><head><meta charset='utf-8'><script>strText='';function SendText(){var request=new XMLHttpRequest();";
  webPage += "strText='Metoda1/'+document.getElementById('txt_form').form_text.value;request.open('GET',strText,true);request.send(null);}</script></head>";
  webPage += "<h1>Registracija</h1><body><form id='txt_form' name='frmText'><p>Broj indeksa: <br><textarea name='form_text' rows='1' cols='9' maxlength='9'></textarea>";
  webPage += "</p></form><input type='submit' value='Potvrdi' onclick='SendText()'/></body></html>";
  
  delay(1000);
  Serial.begin(115200);
  Serial.println();
  Serial.print("Configuring access point...");
  WiFi.softAP(ssid);
  Serial.println("");
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());
  
  server.on("/", [](){
    server.send(200, "text/html", webPage);
  });
  server.begin();
  Serial.println("HTTP server started");

  //SD Card Initialization
  if (SD.begin(SS))
  {
    Serial.println("SD card is ready to use.");
  }
  else
  {
    Serial.println("SD card initialization failed.");
    return;
  }

  for (int i = 0; i < MAX; prijavljeni[i++] = "/");
  
  rtc.Begin();
  rtc.Enable32kHzPin(false);
  rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone);
}
 
void loop(void)
{
  server.handleClient();
  delay(5000);
  client_status();
  delay(500);
}

void client_status()
{
  struct station_info *stat_info;

  struct ip_addr *IPaddress;
  IPAddress address;

  myFile = SD.open("reg.txt");
  if (myFile)
  {
    int br_korisnika = 1;
    
    while (myFile.available()) 
    {
      stat_info = wifi_softap_get_station_info();
      
      boolean prisutan = false;
      boolean novi = false;

      String t = convertToStr(rtc.GetDateTime());     
      String line_ip = myFile.readStringUntil('|');
     
      while (stat_info != NULL)
      {
        String mac = "";
        mac += ((String(stat_info->bssid[0],HEX).length()) == 2)? String(stat_info->bssid[0],HEX) : "0" + String(stat_info->bssid[0],HEX);
        mac += ((String(stat_info->bssid[1],HEX).length()) == 2)? String(stat_info->bssid[1],HEX) : "0" + String(stat_info->bssid[1],HEX);
        mac += ((String(stat_info->bssid[2],HEX).length()) == 2)? String(stat_info->bssid[2],HEX) : "0" + String(stat_info->bssid[2],HEX);
        mac += ((String(stat_info->bssid[3],HEX).length()) == 2)? String(stat_info->bssid[3],HEX) : "0" + String(stat_info->bssid[3],HEX);
        mac += ((String(stat_info->bssid[4],HEX).length()) == 2)? String(stat_info->bssid[4],HEX) : "0" + String(stat_info->bssid[4],HEX);
        mac += ((String(stat_info->bssid[5],HEX).length()) == 2)? String(stat_info->bssid[5],HEX) : "0" + String(stat_info->bssid[5],HEX);

        IPaddress = &stat_info->ip;
        address = IPaddress->addr;
        String ip = IpAddress2String(address);
        
        if (ip == line_ip)
        {
          prisutan = true;

          if (prijavljeni[br_korisnika-1] == "/")
          {
            novi = true;
            
            prijavljeni[br_korisnika-1] = line_ip + "|" + mac + "|" + t + "|1";
          }
        }
        stat_info = STAILQ_NEXT(stat_info, next); 
      }

      myFile2 = SD.open(date + ".txt", FILE_WRITE);
      if (myFile2)
      {
        if (novi)
        {
          Serial.print(br_korisnika);
          Serial.println(". student is in range.");
          myFile2.println(prijavljeni[br_korisnika-1]);
        }
        else if (!prisutan)
        {
          if (prijavljeni[br_korisnika-1] != "/")
          {
            int n = prijavljeni[br_korisnika-1].length();
            prijavljeni[br_korisnika-1][n-1] = '0';
            
            Serial.print(br_korisnika);
            Serial.println(". student is out of range.");
            myFile2.println(prijavljeni[br_korisnika-1]);
            prijavljeni[br_korisnika-1] = "/";
          }
        }
        myFile2.close();
      }         
      else
      {
        Serial.println("error opening " + date + ".txt");
      }
      br_korisnika++;

      line_ip = myFile.readStringUntil('\n');
    }
    myFile.close();
  }
  else
  {
    Serial.println("error opening reg.txt");
  }
  delay(500);
}

String convertToStr(const RtcDateTime& dt)
{
    char datestring[21];
    String datum;

    snprintf_P(datestring, 
            countof(datestring),
            PSTR("%04u-%02u-%02uT%02u:%02u"),
            dt.Year(),
            dt.Month(),
            dt.Day(),
            dt.Hour(),
            dt.Minute());
    datum = datestring;
    return datum;
}

String convertDateToStr(const RtcDateTime& dt)
{
  char datestring[11];
    String datum;

    snprintf_P(datestring, 
            countof(datestring),
            PSTR("%02u_%02u"),
            dt.Day(),
            dt.Month());
    datum = datestring;
    return datestring;
}

String IpAddress2String(const IPAddress& ipAddress)
{
  return String(ipAddress[0]) + "." + String(ipAddress[1]) + String(".") + String(ipAddress[2]) + "." + String(ipAddress[3]);
}
