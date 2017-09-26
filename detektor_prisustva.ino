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

//mac i ip promenljive
unsigned char mac[6];
unsigned char ip[4];

const char *webPage = "Content-Type: text/html\r\n\r\n <!DOCTYPE HTML>\r\n <head> </head><html>\r\n <form name=\"form1\" id=\"txt_form\" method=\"get\" action=\"/metoda1\">\r\n <br>Ime:<input type=\"text\" name=\"polje_ime\" required = \"required\"><br> <br>Prezime:<input type=\"text\" name=\"polje_prezime\" required = \"required\"><br> <br>Id:<input type=\"text\" name=\"polje_id\" required = \"required\"><br> <button type=\"submit\">Continue</button>  </form>\r\n<br><br><br></html> \n";
const char *err_msg1 = "Content-Type: text/html\r\n\r\n <!DOCTYPE HTML>\r\n <head> </head><html>\r\n <font color=\"red\"> Greska pri unosu </font> \r\n<br><br><br></html> \n";
const char *err_msg2 = "Content-Type: text/html\r\n\r\n <!DOCTYPE HTML>\r\n <head> </head><html>\r\n <font color=\"green\"> Uspesan unos!! </font> \r\n<br><br><br></html> \n";

ESP8266WebServer server(80);

File myFile;
File myFile2;

String prijavljeni[MAX];

//DS1302 rtc(2, 3, 4);
RtcDS3231<TwoWire> rtc(Wire);

void handleRoot()
{
  server.send(200,"text/html",webPage);
  String add = server.client().remoteIP().toString();
  Serial.println(add);
}

void handleData()
{
  struct ip_addr *IPaddress;
  IPAddress address;

  bool err_flag = false;
  
  if((server.arg("polje_ime") == "") || (server.arg("polje_prezime") == "") || (server.arg("polje_id") == ""))
  {
    err_flag = true;
  }

  //provera flega za greske
  if(err_flag)
  {
    server.send(200,"text/html",err_msg1);
  }
  else
  {
    //ocitavanje podataka
    String par_ime = server.arg("polje_ime");
    String par_prezime = server.arg("polje_prezime");
    String par_id = server.arg("polje_id");
    
    struct station_info *stat_info;
    
    stat_info = wifi_softap_get_station_info();

    while ( STAILQ_NEXT(stat_info, next)!= NULL)
    {
      stat_info =  STAILQ_NEXT(stat_info, next);
    }
    IPaddress = &stat_info -> ip;
    address = IPaddress->addr;

    //zapisivanje adrese
    ip[0] = address[0];
    ip[1] = address[1];
    ip[2] = address[2];
    ip[3] = address[3];

    //zapisivanje fizicke adrese
    mac[0] = stat_info->bssid[0];
    mac[1] = stat_info->bssid[1];
    mac[2] = stat_info->bssid[2];
    mac[3] = stat_info->bssid[3];
    mac[4] = stat_info->bssid[4];
    mac[5] = stat_info->bssid[5];

    myFile2 = SD.open("reg.txt", FILE_WRITE);
    if(myFile2)
    {
      String ip_str = String(ip[0]) + String(".") + String(ip[1]) + String(".") + String(ip[2]) + String(".") + String(ip[3]);
      String mac_str = String(mac[0], HEX) + String(":") + String(mac[1], HEX) + String(":") + String(mac[2], HEX) + String(":") + String(mac[3], HEX) + String(":") + String(mac[4], HEX) + String(":") + String(mac[5], HEX);
      myFile2.println(ip_str + "|" + mac_str + "|" + par_ime + "|" + par_prezime + "|" + par_id);
      myFile2.close();
      Serial.println("Upis zavrsen");
    }
    else
    {
      Serial.println("greska pri otvaranju");
    }
    
    server.send(200,"text/html",err_msg2);
  }  
}

void setup(void)
{  
  delay(1000);
  Serial.begin(115200);
  Serial.println();
  Serial.print("Configuring access point...");
  WiFi.softAP(ssid);
  Serial.println("");
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());

  server.on("/",handleRoot);
  server.on("/metoda1",handleData);
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
        String mac1 = "";
        mac1 += String(stat_info->bssid[0],HEX);
        mac1 += String(stat_info->bssid[1],HEX);
        mac1 += String(stat_info->bssid[2],HEX);
        mac1 += String(stat_info->bssid[3],HEX);
        mac1 += String(stat_info->bssid[4],HEX);
        mac1 += String(stat_info->bssid[5],HEX);

        IPaddress = &stat_info->ip;
        address = IPaddress->addr;
        String ip1 = IpAddress2String(address);
        
        if (ip1 == line_ip)
        {
          prisutan = true;

          if (prijavljeni[br_korisnika-1] == "/")
          {
            novi = true;
            
            prijavljeni[br_korisnika-1] = line_ip + "|" + mac1 + "|" + t + "|1";
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
