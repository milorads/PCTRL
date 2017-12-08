#include <SD.h>
#include <DS1302.h>
#include <ESP8266WiFi.h>
#include <ESP8266Ping.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <SPI.h>
#include "MD5.h"
#include <DNSServer.h>

extern "C" 
{
  #include<user_interface.h>
}

#define countof(a) (sizeof(a)/sizeof(a[0]))
#define MAX 5

const char *ssid     = "ProbaBoban";
const char *webPage  = "<!DOCTYPE HTML> <head><title>Registracija</title> </head><html> <form name=\"form1\" id=\"txt_form\" method=\"get\" action=\"/regData\"><h1>Registracija</h1><table><tr><td>Ime:</td><td><input type=\"text\" name=\"polje_ime\" required = \"required\"></td></tr> <tr><td>Prezime:</td><td><input type=\"text\" name=\"polje_prezime\" required = \"required\"></td></tr> <tr><td>Id:</td><td><input type=\"text\" name=\"polje_id\" required = \"required\"></td></tr> <tr><td><button type=\"submit\">Submit</button></td><td><button type=\"reset\">Reset</button></td></tr>  </form></html>";
//const char *webPage2 = "<!DOCTYPE HTML> <head><title>Admin</title> </head><html> <form name=\"form1\" id=\"txt_form\" method=\"get\" action=\"/getData\"><h1>Hello admin!</h1><table><tr><td>Code:</td><td><input type=\"text\" name=\"code\" required = \"required\"></td></tr> <tr><td>File name:</td><td><input type=\"text\" name=\"file\" required = \"required\"></td></tr> <tr><td><button type=\"submit\">Continue</button></td><td><button type=\"reset\">Reset</button></td></tr>  </form></html>";
//const char *webPage3 = "<!DOCTYPE HTML> <head><title>Brisanje</title> </head><html> <form name=\"form1\" id=\"txt_form\" method=\"get\" action=\"/deleteData\"><h1>Brisanje fajla</h1><table><tr><td>Code:</td><td><input type=\"text\" name=\"code\" required = \"required\"></td></tr><tr><td>Datum fajla:</td><td><input type=\"text\" name=\"file\" required = \"required\"></td></tr> <tr><td><button type=\"submit\">Continue</button></td><td><button type=\"reset\">Reset</button></td></tr>  </form></html>";
//const char *webPage4 = "<!DOCTYPE HTML> <head><title>Test</title> </head><html> <form name=\"form1\" id=\"txt_form\" method=\"get\" action=\"/apiTest\"><h1>Test</h1><table><tr><td>Code:</td><td><input type=\"text\" name=\"code\" required = \"required\"></td></tr> <tr><td><button type=\"submit\">Continue</button></td><td><button type=\"reset\">Reset</button></td></tr>  </form></html>";
//const char *webPage5 = "<!DOCTYPE HTML> <head><title>List</title> </head><html> <form name=\"form1\" id=\"txt_form\" method=\"get\" action=\"/listData\"><h1>List of files</h1><table><tr><td>Code:</td><td><input type=\"text\" name=\"code\" required = \"required\"></td></tr> <tr><td><button type=\"submit\">Continue</button></td><td><button type=\"reset\">Reset</button></td></tr>  </form></html>";
const char *err_msg1 = "<!DOCTYPE HTML> <head><title>Error</title> </head><html> <font color=\"red\"> Greska pri unosu! </font></html>";
const char *err_msg2 = "<!DOCTYPE HTML> <head><title>Success</title> </head><html> <font color=\"green\"> Uspesan unos! </font></html>";
const char *err_msg3 = "<!DOCTYPE HTML> <head><title>Warning</title> </head><html> <font color=\"green\"> Vec si registrovan! </font></html>";
const char *succ_remove = "<!DOCTYPE HTML> <head><title>Success</title> </head><html> <font color=\"green\"> Uspesno obrisan fajl! </font></html>";
ESP8266WebServer server(80);
File myFile;
File myFile2;
File logFile;
File brisanje;
File root;
//File root;
//Set pins:  CE, IO,CLK
DS1302 rtc(0, 4, 5);
struct station_info *stat_info;
struct ip_addr *IPaddress;
IPAddress address;
struct Registrovani
{
  uint8_t mac[6];
  String ime;
  String prezime;
  String id;
  String vreme_ulaska = "";
  uint8_t size_reg = MAX;
};
//registrovani reg[MAX];
Registrovani* reg = new Registrovani[MAX];
int reg_num = 0;
String salt = "PCTRL";
String md5 = "164803c7f69b529f116352cc0814084e";
//dns server
const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 4, 1);
DNSServer dnsServer;

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
  String mac_str;
  stat_info = wifi_softap_get_station_info();
  while (stat_info != NULL)
  {
    IPaddress = &stat_info->ip;
    address   = IPaddress->addr;
    if (address == server.client().remoteIP())
    {
      mac_str = mac2str(stat_info->bssid, 0);
      break;
    }
    stat_info = STAILQ_NEXT(stat_info, next);
  }
  wifi_softap_free_station_info();
  //F-ja koja zabranjuje ponovnu registraciju
  bool postoji =  checkRegClient(mac_str);
  if (postoji)
  {
    server.send(200, "text/html", err_msg3);
  }
  else
  {
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
}

/* void apiPage()
{
  server.send(200, "text/html", webPage4);
}

void adminPage()
{
	server.send(200, "text/html", webPage2);
}

void deletePage()
{
  server.send(200, "text/html", webPage3);
}

void listPage()
{
  server.send(200, "text/html", webPage5);
} */

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
  String mac_str;
  stat_info = wifi_softap_get_station_info(); 
  logFile.println("pocetak prolaska kroz listu stat_info"); 
  while (stat_info != NULL)
  {
    IPaddress = &stat_info->ip;
    address   = IPaddress->addr;
    if (address == server.client().remoteIP())
    {   
      mac_str = mac2str(stat_info->bssid, 0);      
      break;
    }
    stat_info = STAILQ_NEXT(stat_info, next);
  } 
  //F-ja koja zabranjuje ponovnu registraciju
  bool postoji = checkRegClient(mac_str);
  if (postoji)
  { 
    server.send(200, "text/html", err_msg3);
  }
  else
  {
    bool err_flag = false;
    if ((server.arg("polje_ime") == "") || (server.arg("polje_prezime") == "") || (server.arg("polje_id") == ""))
    {
      logFile.println("GRESKA: neka od polja su prazna!!");
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
      reg[reg_num].ime.replace("|", "");
      reg[reg_num].prezime.replace("|", "");
      reg[reg_num].id.replace("\n", "");
      reg[reg_num].id.replace("\r", "");
      reg[reg_num].id.replace("|", "");
      logFile.println("upisivanje mac adrese u kolekciju registrovanih"); 
      reg[reg_num].mac[0] = stat_info->bssid[0];
      reg[reg_num].mac[1] = stat_info->bssid[1];
      reg[reg_num].mac[2] = stat_info->bssid[2];
      reg[reg_num].mac[3] = stat_info->bssid[3];
      reg[reg_num].mac[4] = stat_info->bssid[4];
      reg[reg_num].mac[5] = stat_info->bssid[5];
      logFile.println("dealociranje liste station_info");
      wifi_softap_free_station_info();
      logFile.println("pocetak upisa podataka na sd karticu");
      logFile.close();       
      String mac_str = mac2str(reg[reg_num].mac, 0);
      String reg_kor = mac_str + "|" + reg[reg_num].ime + "|" + reg[reg_num].prezime + "|" + reg[reg_num].id;
      myFile = SD.open("reg.txt", FILE_WRITE);
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
      if (reg_num == reg->size_reg)
      {
        Registrovani* reg_pom = reg;
        reg = new Registrovani[reg->size_reg+MAX];
        for (int i = 0; i < reg_num; i++)
        {
          reg[i] = reg_pom[i];
        }
        delete[](reg_pom);
        reg->size_reg = reg->size_reg+MAX;
        Serial.println(reg->size_reg);
      }
      logFile.print(reg_num);
      logFile.println(". registrovani korisnik: " + reg_kor);
      server.send(200, "text/html", err_msg2);
  
      logFile.println("poslata stranica za uspesno prijavljivanje");
      logFile.println("**KRAJ FUNKCIJE HANDLE DATA **");
      logFile.close();
    } 
  }
}

void getData()
{
  bool err_flag = false;
  if ((server.arg("file") == "") || (server.arg("code") == ""))
  {
    //logFile.println("GRESKA: neka od polja su prazna!!");
    err_flag = true;  
  }
  //provera flega za greske
  if (err_flag)
  {
    server.send(200, "text/html", err_msg1);
  }
  else
  {
    String kod = server.arg("code");
    if (checkCode(kod))
    {
      String evidencija;
      String datum = server.arg("file");
      myFile2 = SD.open(datum + ".txt");
      if (myFile2)
      {
        evidencija = myFile2.readStringUntil('\0');
        myFile.close();
        server.send(200, "text/plain", evidencija);
      }
      else
      {
        Serial.println("error opening " + datum + ".txt");
        server.send(200, "text/html", err_msg1);
      }
    }
    else
    {
      server.send(200, "text/html", err_msg1);
    }
  }
}

void deleteData()
{
  bool err_flag = false;
  if (server.arg("file") == "" || (server.arg("code") == ""))
  {
    //logFile.println("GRESKA: neka od polja su prazna!!");
    err_flag = true;  
  }
  //provera flega za greske
  if (err_flag)
  {
    server.send(200, "text/html", err_msg1);
  }
  else
  {
    String kod = server.arg("code");
    if (checkCode(kod))
    {
      String polje_datum = server.arg("file");
      char datum[12];
      int i;
      for (i = 0; i < 8; i++)
      {
        datum[i] = polje_datum[i];
      }
      datum[i++] = '.';
      datum[i++] = 't';
      datum[i++] = 'x';
      datum[i++] = 't';
      datum[i]   = '\0';
      if (SD.remove(datum))
      {
        server.send(200, "text/html", succ_remove);
      }
      else
      {
        server.send(200, "text/html", err_msg1); 
        SD.exists("reg.txt");// stoji samo zbog baga biblioteke SD
      }
    }
    else
    {
      server.send(200, "text/html", err_msg1);
    }
  }
}

void apiTest()
{
  bool err_flag = false;
  if (server.arg("code") == "")
  {
    //logFile.println("GRESKA: neka od polja su prazna!!");
    err_flag = true;  
  }
  //provera flega za greske
  if (err_flag)
  {
    server.send(200, "text/html", err_msg1);
  }
  else
  {
    String kod = server.arg("code");
    if (checkCode(kod))
    {
      server.send(200, "text/html", "1");
    }
    else
    {
      server.send(200, "text/html", err_msg1);
    }
  }
}

void listData()
{
  bool err_flag = false;
  if (server.arg("code") == "")
  {
    //logFile.println("GRESKA: neka od polja su prazna!!");
    err_flag = true;  
  }
  //provera flega za greske
  if (err_flag)
  {
    server.send(200, "text/html", err_msg1);
  }
  else
  {
    String kod = server.arg("code");
    if(checkCode(kod))
    { 
      root = SD.open("/");
      String list;
      bool first = true;
      while (true) 
      {
        File entry =  root.openNextFile();
        if (!entry) 
        {
          // no more files
          break;
        }
        if(checkFile(entry))
        {
          if(!first)
          {
            list += ",";
          }
          first = false;
          //list += String(entry.name()); 
          String s = String(entry.name()); 
          list += s.substring(0, s.length()-4);
        }
        //list += String(entry.name());
        entry.close();
      }
      root.close();
      server.send(200, "text/plain", list);
    }
    else
    {
      //exception
      server.send(200, "text/html", err_msg1);
    }
  }
}

void setup(void)
{  
  delay(1000);
  Serial.begin(115200);
  Serial.println();
  Serial.print("Configuring access point...");
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(ssid);
  Serial.println("");
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());
  struct softap_config config;
  wifi_softap_get_config(&config); // Get config first.
  config.max_connection = 30; // how many stations can connect to ESP8266 softAP at most.
  wifi_softap_set_config(&config);// Set ESP8266 softap config
  server.on("/", handleRoot);
  server.on("/regData", handleData);
  //server.on("/file", adminPage);
  server.on("/getData", getData);
  //server.on("/delete", deletePage);
  server.on("/deleteData", deleteData);
  //server.on("/api", apiPage);
  server.on("/apiTest", apiTest);
  //server.on("/list", listPage);
  server.on("/listData", listData);
  //dns config
  dnsServer.setTTL(300);
  dnsServer.setErrorReplyCode(DNSReplyCode::ServerFailure);
  dnsServer.start(DNS_PORT, "prijavi.me", apIP); 
  server.onNotFound([](){
                          String message = "Hello World!\n\n";
                          message += "URI: ";
                          message += server.uri();
                          server.send(200, "text/plain", message);
                        }
  ); //kraj dns-a
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
        reg[reg_num].id.replace("\n", "");
        reg[reg_num].id.replace("\r", "");
        reg_num++;
        if (reg_num == reg->size_reg)
        {
          Registrovani* reg_pom = reg;
          reg = new Registrovani[reg->size_reg+MAX];
          for (int i = 0; i < reg_num; i++)
          {
            reg[i] = reg_pom[i];
          }
          delete[](reg_pom);
          reg->size_reg = reg->size_reg+MAX;
          Serial.println(reg->size_reg);
        }
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
  // Set the clock to run-mode, and disable the write protection
  rtc.halt(false);
  rtc.writeProtect(false);
}
 
void loop(void)
{
  //dns processing
  dnsServer.processNextRequest();
  server.handleClient();
  client_status();
}

void client_status()
{
  for (int i = 0; i < reg_num; i++)
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
          logFile = SD.open("loging.txt",FILE_WRITE);
          logFile.println(reg[i].ime + " " + reg[i].prezime + " " + reg[i].id + " is in range, time: " + reg[i].vreme_ulaska);
          logFile.close();
          Serial.println(reg[i].ime + " " + reg[i].prezime + " " + reg[i].id + " is in range, time: " + reg[i].vreme_ulaska);
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
          for (int idx = 0; idx < reg[i].ime.length(); idx++)
          {
            uint8_t c = reg[i].ime[idx];
            switch (c)
            {
              case 0x9A:
                reg[i].ime.replace(String(reg[i].ime[idx]), "&#353;");
                break;
              case 0x9E:
                reg[i].ime.replace(String(reg[i].ime[idx]), "&#382;");
                break;
              case 0x8A:
                reg[i].ime.replace(String(reg[i].ime[idx]), "&#352;");
                break;
              case 0x8E:
                reg[i].ime.replace(String(reg[i].ime[idx]), "&#381;");
                break;
            } 
          }
          for (int idx = 0; idx < reg[i].prezime.length(); idx++)
          {
            uint8_t c = reg[i].prezime[idx];
            switch (c)
            {
              case 0x9A:
                reg[i].prezime.replace(String(reg[i].prezime[idx]), "&#353;");
                break;
              case 0x9E:
                reg[i].prezime.replace(String(reg[i].prezime[idx]), "&#382;");
                break;
              case 0x8A:
                reg[i].prezime.replace(String(reg[i].prezime[idx]), "&#352;");
                break;
              case 0x8E:
                reg[i].prezime.replace(String(reg[i].prezime[idx]), "&#381;");
                break;
            } 
          }
          Serial.println(reg[i].ime + " " + reg[i].prezime + " " + reg[i].id + " is out of range, time: " + t);
          myFile2.println(reg[i].ime + "|" + reg[i].prezime + "|" + reg[i].id+ "|" + reg[i].vreme_ulaska + "|" + t);
          myFile2.close();
          logFile=SD.open("loging.txt", FILE_WRITE);
          logFile.println(reg[i].ime + " " + reg[i].prezime + " " + reg[i].id + " is out of range, time: " + t);
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
             PSTR("%02u_%02u_%02u"),
             tm.date,
             tm.mon,
             tm.year - 2000);
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

bool checkRegClient(String mac)
{
  for (int i = 0; i < reg_num; i++)
  {
    String mac_str_reg = mac2str(reg[i].mac, 0);
    if (mac_str_reg == mac)
    {   
      /*...........................*/
      /*logovanje*/
      if(logFile)
      {
        logFile.println("Vec je prijavljen na ovoj mac adresi");
        logFile.println(reg[i].ime + " " + reg[i].prezime[i] + "|" + mac + " je probao ponovo da se uloguje");
        logFile.close();
      }
      /*...........................*/    
      return true;
    }
  }
  return false;
}

bool checkCode(String str)
{
  String kod2 = str + salt;
  char kod1[kod2.length()];
  int i;
  for (i = 0; i < kod2.length(); i++)
  {
    kod1[i] = kod2[i];
  }
  kod1[i] = '\0';
  unsigned char* hash = MD5::make_hash(kod1);
  String md5str = MD5::make_digest(hash, 16);
  free(hash);
  if (md5str == md5)
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool checkFile(File entry)
{
  if(entry.isDirectory())
  {
    return false;
  }
  String n = entry.name();
  if (isDigit(n[0]) && isDigit(n[1]) && (n[2] == '_') && isDigit(n[3]) && isDigit(n[4]) && (n[5] == '_') && isDigit(n[6]) && isDigit(n[7]))
  {
    return true;
  }
  else
  {
    return false;
  }
}
