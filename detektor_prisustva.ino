#include <SD.h>
//#include <DS1302.h>
#include <Wire.h> // must be included here so that Arduino library object file references work
#include <RtcDS3231.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
//#include <ESP8266mDNS.h>
//#include <Time.h>
//#include <SPI.h>

extern "C" {
#include<user_interface.h>
}

#define countof(a) (sizeof(a) / sizeof(a[0]))
#define MAX 2

//MDNSResponder mdns;

// Replace with your network credentials
const char* ssid = "Proba";

ESP8266WebServer server(80);

String webPage = "";

File myFile;

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
  //IPAddress ip(192, 168, 1, 7);
  //WiFi.hostname(prijava);
  //wifi_station_set_hostname( "test11" );
  WiFi.softAP(ssid);
  Serial.println("");
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());
  Serial.print("Hostname: ");
  //Serial.println(WiFi.hostname());

  //if (mdns.begin("prijava", WiFi.softAPIP())) {
    //Serial.println("MDNS responder started");
  //}
  
  server.on("/", [](){
    server.send(200, "text/html", webPage);
  });
  //server.on("/Metoda1", [](){
    //server.send(200, "text/html", webPage);
    //delay(1000); 
  //});
  server.begin();
  Serial.println("HTTP server started");

  //SD Card Initialization
  if (SD.begin(SS))
  {
    Serial.println("SD card is ready to use.");
  }
  else
  {
    Serial.println("SD card initialization failed");
    return;
  }
  
  for (int i = 0; i < MAX; prijavljeni[i++] = "0");
  
  rtc.Begin();
  rtc.Enable32kHzPin(false);
  rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone);

  String datum = convertDateToStr(rtc.GetDateTime()); 
  myFile = SD.open("ARP_" + datum + ".txt", FILE_WRITE);
  if (myFile)
  {
    Serial.println("Creating file ARP_" + datum + ".txt...");
    myFile.close();
    Serial.println("Done.");
  }
  else
  {
    Serial.println("error opening ARP_" + datum + ".txt");
  }
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
  //unsigned char number_client = wifi_softap_get_station_num();
  struct station_info *stat_info;
  stat_info = wifi_softap_get_station_info();

  struct ip_addr *IPaddress;
  IPAddress address;

  String datum = convertDateToStr(rtc.GetDateTime());

  while (stat_info != NULL)
  {
    String t = convertToStr(rtc.GetDateTime());
    String mac = "";
    mac += ((String(stat_info->bssid[0],HEX).length()) == 2)? String(stat_info->bssid[0],HEX) : "0" + String(stat_info->bssid[0],HEX);
    mac += ((String(stat_info->bssid[1],HEX).length()) == 2)? String(stat_info->bssid[1],HEX) : "0" + String(stat_info->bssid[1],HEX);
    mac += ((String(stat_info->bssid[2],HEX).length()) == 2)? String(stat_info->bssid[2],HEX) : "0" + String(stat_info->bssid[2],HEX);
    mac += ((String(stat_info->bssid[3],HEX).length()) == 2)? String(stat_info->bssid[3],HEX) : "0" + String(stat_info->bssid[3],HEX);
    mac += ((String(stat_info->bssid[4],HEX).length()) == 2)? String(stat_info->bssid[4],HEX) : "0" + String(stat_info->bssid[4],HEX);
    mac += ((String(stat_info->bssid[5],HEX).length()) == 2)? String(stat_info->bssid[5],HEX) : "0" + String(stat_info->bssid[5],HEX);

    IPaddress = &stat_info->ip;
    address = IPaddress->addr;

    boolean prisutan = false;

    myFile = SD.open("ARP_" + datum + ".txt");
    if (myFile)
    {
      int br_korisnika = 1;
      
      while (myFile.available() && !prisutan) 
      {     
        String line = myFile.readStringUntil('\n');
        
        String line_ip = "";
        int i = 0;
        for (; line[i] != '|'; line_ip[i] = line[i++]);
        
        String line_mac = "";
        i++;
        for (; line[i] != '|'; line_mac[i] = line[i++]);

        if (mac == line_mac)
        {
          prisutan = true;
          
          prijavljeni[br_korisnika-1] = "1";
        }
        else
        {
          if (prijavljeni[br_korisnika-1] == "1")
          {
            prijavljeni[br_korisnika-1] = line_ip + "|" + line_mac + "|" + convertToStr(rtc.GetDateTime()) + "|0";
          }
        }
      }
      myFile.close();
    }
    else
    {
      Serial.println("error opening ARP" + datum + ".txt");
    }
    
    if (!prisutan)
    {
      myFile = SD.open("ARP_" + datum + ".txt", FILE_WRITE);
      if (myFile)
      {
        Serial.println("Writing new students...");
        myFile.print(address);
        myFile.print("|");
        myFile.print(mac);
        myFile.print("|");
        myFile.print(t);
        myFile.println("|1");
        myFile.close();
        Serial.println("Done.");
      }
      else
      {
        Serial.println("error opening ARP" + datum + ".txt");
      }
    }

    stat_info = STAILQ_NEXT(stat_info, next);
  }

  myFile = SD.open("ARP_" + datum + ".txt", FILE_WRITE);
  if (myFile)
  {
    for (int i = 0; i < MAX; i++)
    {
      if (prijavljeni[i] != "1" || prijavljeni[i] != "0")
      {
        Serial.println("Writing an offline student...");
        myFile.println(prijavljeni[i]);
        Serial.println("Done.");
      }
    }
    
    myFile.close();
  }
  else
  {
    Serial.println("error opening ARP" + datum + ".txt");
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
            PSTR("%02u_%02u_%04u"),
            dt.Day(),
            dt.Month(),
            dt.Year());
    datum = datestring;
    return datum;
}
