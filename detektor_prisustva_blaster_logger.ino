#include <SD.h>
#include <DS1302.h>
#include <ESP8266WiFi.h>
#include <ESP8266Ping.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
//#include <DNSServer.h>
extern "C" 
{
  #include<user_interface.h>
}
#define countof(a) (sizeof(a)/sizeof(a[0]))
#define MAX 30
//unsigned char mac[6];
const char *ssid     = "ProbaBoban";
const char *webPage  = "Content-Type: text/html\r\n\r\n <!DOCTYPE HTML>\r\n <head> </head><html>\r\n <form name=\"form1\" id=\"txt_form\" method=\"get\" action=\"/metoda1\">\r\n <br>Ime:<input type=\"text\" name=\"polje_ime\" required = \"required\"><br> <br>Prezime:<input type=\"text\" name=\"polje_prezime\" required = \"required\"><br> <br>Id:<input type=\"text\" name=\"polje_id\" required = \"required\"><br> <button type=\"submit\">Continue</button>  </form>\r\n<br><br><br></html> \n";
const char *err_msg1 = "Content-Type: text/html\r\n\r\n <!DOCTYPE HTML>\r\n <head> </head><html>\r\n <font color=\"red\"> Greska pri unosu </font> \r\n<br><br><br></html> \n";
const char *err_msg2 = "Content-Type: text/html\r\n\r\n <!DOCTYPE HTML>\r\n <head> </head><html>\r\n <font color=\"green\"> Uspesan unos!! </font> \r\n<br><br><br></html> \n";
const char *err_msg3 = "Content-Type: text/html\r\n\r\n <!DOCTYPE HTML>\r\n <head> </head><html>\r\n <font color=\"green\"> Vec si registrovan, budalice mala! :D </font> \r\n<br><br><br></html> \n";
ESP8266WebServer server(80);
File myFile;
File myFile2;
File logFile;
//Set pins:  CE, IO,CLK
DS1302 rtc(0, 4, 5);
struct station_info *stat_info;
struct ip_addr *IPaddress;
IPAddress address;
struct registrovani
{
  uint8_t mac[6];
  String ime;
  String prezime;
  String id;
  String vreme_ulaska = "";
};
registrovani reg[MAX];
int reg_num = 0;
//dns server
/* const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 4, 1);
DNSServer dnsServer; */
void handleRoot()
{

  /***************************/
  /*loging*/
  logFile = SD.open("loging.txt",FILE_WRITE);
  if(logFile)
  {
    logFile.println("**POCETAK FUNKCIJE HANDLE ROOT**");
    logFile.println("Reguest za pocetnom stranicom");
    logFile.println("Provera da li je vec registrovan");      
  }
  /***************************/
  //Deo koda koji zabranjuje ponovnu registraciju
  for (int i = 0; i < reg_num; i++)
  {
    stat_info = wifi_softap_get_station_info();
    while (stat_info != NULL)
    {
      IPaddress = &stat_info->ip;
      address   = IPaddress->addr;
      if (address == server.client().remoteIP())
      {   
        if (reg[i].mac[0] == stat_info->bssid[0] && reg[i].mac[1] == stat_info->bssid[1] && reg[i].mac[2] == stat_info->bssid[2] &&
            reg[i].mac[3] == stat_info->bssid[3] && reg[i].mac[4] == stat_info->bssid[4] && reg[i].mac[5] == stat_info->bssid[5])
        {
          server.send(200, "text/html", err_msg3);
          /*...........................*/
          /*logovanje*/
          if(logFile)
          {
            logFile.println("Vec je prijavljen na ovoj mac adresi");
            logFile.println(reg[i].ime + " " + reg[i].prezime[i] + "|" + String(reg[i].mac[0]) + ":" + String(reg[i].mac[1]) + ":" + String(reg[i].mac[2]) + ":" + String(reg[i].mac[3]) + ":" + String(reg[i].mac[4]) + ":" + String(reg[i].mac[5]) + " je probao ponovo da se uloguje");
            logFile.close();
          }
          /*...........................*/       
          return;
        }
        break;
      }
      stat_info = STAILQ_NEXT(stat_info, next);
    }
    wifi_softap_free_station_info();
  }
    /*...........................*/
  /*logovanje*/
  if(logFile)
  {
    logFile.println("Zahtev za registracijom je regularan, salje se stranica za prijavu");
    logFile.close();
  }
  /*...........................*/
  
  
  server.send(200,"text/html",webPage);
}

void handleData()
{
    /*********************************/
    /*logovanje*/
    logFile = SD.open("loging.txt",FILE_WRITE);
    if(logFile)
    {
      logFile.println("**POCETAK FUNKCIJE HANDLE DATA**");
    }
    /********************************/
    
  bool err_flag = false;
  if ((server.arg("polje_ime") == "") || (server.arg("polje_prezime") == "") || (server.arg("polje_id") == ""))
  {
    logFile.println("GRESKA: neka od pollja su prazna!!");
    err_flag = true;
    
  }
  //provera flega za greske
  if (err_flag)
  {
    server.send(200, "text/html", err_msg1);
  }
  else
  {
    logFile.println("sva polja su uneta, prenos u kolekciju promenljivih");
    
    reg[reg_num].ime     = server.arg("polje_ime");
    reg[reg_num].prezime = server.arg("polje_prezime");
    reg[reg_num].id      = server.arg("polje_id");
    stat_info = wifi_softap_get_station_info();
    
    logFile.println("pocetak prolaska kroz listu stat_info");
    
    while (stat_info != NULL)
    {
      IPaddress = &stat_info->ip;
      address   = IPaddress->addr;
      if (address == server.client().remoteIP())
      {         
        break;
      }
      stat_info = STAILQ_NEXT(stat_info, next);
    }

    logFile.println("dealociranje liste station_info");
    logFile.println("upisivanje mac adrese u kolekciju registrovanih");
    
    reg[reg_num].mac[0]   = stat_info->bssid[0];
    reg[reg_num].mac[1]   = stat_info->bssid[1];
    reg[reg_num].mac[2]   = stat_info->bssid[2];
    reg[reg_num].mac[3]   = stat_info->bssid[3];
    reg[reg_num].mac[4]   = stat_info->bssid[4];
    reg[reg_num].mac[5]   = stat_info->bssid[5];
    wifi_softap_free_station_info();

    logFile.println("pocetak upisa podataka na sd karticu");
    logFile.close();
        
    myFile = SD.open("reg.txt", FILE_WRITE);
    
    String mac_str = mac2str(reg[reg_num].mac, 0);
    String reg_kor = mac_str + "|" + reg[reg_num].ime + "|" + reg[reg_num].prezime + "|" + reg[reg_num].id;

    if (myFile)
    {
      
      myFile.println(reg_kor);
      myFile.close();
    }
    else
    {
      Serial.println("error opening reg.txt");
    }
    logFile = SD.open("loging.txt",FILE_WRITE);
    reg_num++;
        Serial.print(reg_num);
    Serial.println(". registrovani korisnik: " + reg_kor);
    server.send(200, "text/html", err_msg2);

    logFile.println("poslata stranica za uspesno prijavljivanje");
    logFile.println("**KRAJ FUNKCIJE HANDLE DATA **");
    logFile.close();
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
  //dns config
  /* dnsServer.setTTL(300);
  dnsServer.setErrorReplyCode(DNSReplyCode::ServerFailure);
  dnsServer.start(DNS_PORT, "boban.coban", apIP); 
  server.onNotFound([](){
                          String message = "Hello World!\n\n";
                          message += "URI: ";
                          message += server.uri();
                          server.send(200, "text/plain", message);
                        }
  ); *///kraj dns-a
  struct softap_config config;
  wifi_softap_get_config(&config); // Get config first.
  config.max_connection = 30; // how many stations can connect to ESP8266 softAP at most.
  wifi_softap_set_config(&config);// Set ESP8266 softap config
  server.on("/", handleRoot);
  server.on("/metoda1", handleData);
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
      /***************************/
    /*loging*/
    logFile = SD.open("loging.txt",FILE_WRITE);
    if(logFile)
    {
      logFile.println("Serial monitor initialized");
      logFile.println("SD card  initialized");
      logFile.println("soft AP config started");
      logFile.println("Pocetak ucaitavanja podataka sa kartice");
      logFile.println("soft AP config finished succesfully");
      logFile.println("HTTP server started");
      logFile.close();      
    }
    /***************************/
  if (!reg_num)
  {
    myFile = SD.open("reg.txt");
    if (myFile)
    {
      while (myFile.available())
      {
        String mac_str       = myFile.readStringUntil(':');
        reg[reg_num].mac[0]  = str2mac(mac_str);
        mac_str              = myFile.readStringUntil(':');
        reg[reg_num].mac[1]  = str2mac(mac_str);
        mac_str              = myFile.readStringUntil(':');
        reg[reg_num].mac[2]  = str2mac(mac_str);
        mac_str              = myFile.readStringUntil(':');
        reg[reg_num].mac[3]  = str2mac(mac_str);
        mac_str              = myFile.readStringUntil(':');
        reg[reg_num].mac[4]  = str2mac(mac_str);
        mac_str              = myFile.readStringUntil('|');
        reg[reg_num].mac[5]  = str2mac(mac_str);
        reg[reg_num].ime     = myFile.readStringUntil('|');
        reg[reg_num].prezime = myFile.readStringUntil('|');
        reg[reg_num].id      = myFile.readStringUntil('\n');
        reg_num++;
      }
      myFile.close();
    }
    else
    {
      Serial.println("error opening reg.txt");
    }
  }
    /*loging*/
  logFile = SD.open("loging.txt",FILE_WRITE);
  if(logFile)
  {
    logFile.println("loading user info finished");
    logFile.println("rtc started");
    logFile.println("kraj setup funkcije");
    logFile.close();      
  }
  /***************************/
  // Set the clock to run-mode, and enable the write protection
  rtc.halt(false);
  rtc.writeProtect(true);
}
 
void loop(void)
{
  //dns processing
  //dnsServer.processNextRequest();
  server.handleClient();
  client_status();
}
void client_status()
{
  for (int i = 0; i <= reg_num; i++)
  {
    bool prisutan = false;
    String t = convertToStr(rtc.getTime()); 
    stat_info = wifi_softap_get_station_info();
    while (stat_info != NULL)
    {
      if ((stat_info->bssid[0] == reg[i].mac[0] && stat_info->bssid[1] == reg[i].mac[1] &&
           stat_info->bssid[2] == reg[i].mac[2] && stat_info->bssid[3] == reg[i].mac[3] &&
           stat_info->bssid[4] == reg[i].mac[4] && stat_info->bssid[5] == reg[i].mac[5]))
      {
        prisutan = true;
        if (reg[i].vreme_ulaska == "")
        {
          reg[i].vreme_ulaska = t;
          logFile=SD.open("loging.txt",FILE_WRITE);
          logFile.println(reg[i].ime + " " + reg[i].prezime + "|"+ reg[i].vreme_ulaska + "is in range");
          logFile.close();
          Serial.println(reg[i].ime + " " + reg[i].prezime + "|"+ reg[i].vreme_ulaska + "is in range");
        }
        break;
      }
      stat_info = STAILQ_NEXT(stat_info, next);
    }
    wifi_softap_free_station_info();
    if (!prisutan)
    {
      if (reg[i].vreme_ulaska != "")
      { 
        String datum = convertDateToStr(rtc.getTime());
        myFile2 = SD.open(datum + ".txt", FILE_WRITE);
        if (myFile2)
        {
          Serial.println(reg[i].ime + " " + reg[i].prezime + "|"+ reg[i].vreme_ulaska + "is out of range");
          myFile2.println(reg[i].ime + "|" + reg[i].prezime + reg[i].id+ "|" + reg[i].vreme_ulaska + "|" + t);
          myFile2.close();
          logFile=SD.open("loging.txt",FILE_WRITE);
          logFile.println(reg[i].ime + " " + reg[i].prezime + "|"+ reg[i].vreme_ulaska + "out of range");
          logFile.close();
          reg[i].vreme_ulaska = "";
        }         
        else
        {
          Serial.println("error opening " + datum + ".txt");
        }
      }
    }
  }
}
String convertToStr(const Time &tm)
{
  char datestring[21];
  snprintf_P(datestring, 
             countof(datestring),
             PSTR("%04u-%02u-%02uT%02u:%02u"),
             tm.year,
             tm.mon,
             tm.date,
             tm.hour,
             tm.min);
  return datestring;
}
String convertDateToStr(const Time &tm)
{
  char datestring[11];
  snprintf_P(datestring, 
             countof(datestring),
             PSTR("%02u_%02u"),
             tm.date,
             tm.mon);
  return datestring;
}
String mac2str(uint8_t *buf, uint8 i)
{
  char datestring[18];
  snprintf_P(datestring, 
             countof(datestring),
             PSTR("%02X:%02X:%02X:%02X:%02X:%02X"),
             buf[i],
             buf[i+1],
             buf[i+2],
             buf[i+3],
             buf[i+4],
             buf[i+5]);
  return datestring;
  }
uint8_t str2mac(String str)
{
  uint8_t mac = (str[0] < 65) ? 16*(str[0] - 48) : 16*(str[0] - 55);
  mac += (str[1] < 65) ? str[1] - 48 : str[1] - 55;
  return mac;
}

