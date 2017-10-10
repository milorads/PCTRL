#include <SD.h>
#include <DS1302.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <Ticker.h>

extern "C" 
{
  #include "user_interface.h"
  #include "ets_sys.h"
  #include "osapi.h"
  #include "gpio.h"
  #include "os_type.h"
  #include "mem.h"
  #include "user_config.h"
}

#define countof(a) (sizeof(a) / sizeof(a[0]))
#define MAX 5
#define MAX_BAFER_OUT 5
#define MAX_BAFER_COMPARE 50
#define MAX_RANGE 50
// Delay of loop function in milli seconds
# define __delay__ 10
// Delay of channel changing in seconds
# define __dlay_ChannelChange__ 0.5

//Ticker for channel hopping
Ticker ts;

//Promiscuous callback structures for storing package data, see Espressif SDK handbook
struct RxControl 
{
  signed rssi:8;
  unsigned rate:4;
  unsigned is_group:1;
  unsigned:1;
  unsigned sig_mode:2;
  unsigned legacy_length:12;
  unsigned damatch0:1;
  unsigned damatch1:1;
  unsigned bssidmatch0:1;
  unsigned bssidmatch1:1;
  unsigned MCS:7;
  unsigned CWB:1;
  unsigned HT_length:16;
  unsigned Smoothing:1;
  unsigned Not_Sounding:1;
  unsigned:1;
  unsigned Aggregation:1;
  unsigned STBC:2;
  unsigned FEC_CODING:1;
  unsigned SGI:1;
  unsigned rxend_state:8;
  unsigned ampdu_cnt:8;
  unsigned channel:4;
  unsigned:12;
};

struct LenSeq 
{
  uint16_t length;
  uint16_t seq;
  uint8_t  address3[6];
};

struct sniffer_buf 
{
  struct RxControl rx_ctrl;
  uint8_t buf[36];
  uint16_t cnt;
  struct LenSeq lenseq[1];
};

struct sniffer_buf2
{
  struct RxControl rx_ctrl;
  uint8_t buf[112];
  uint16_t cnt;
  uint16_t len;
};

const char* ssid = "Proba";

//mac promenljiva
//unsigned char mac[6];

const char *webPage = "Content-Type: text/html\r\n\r\n <!DOCTYPE HTML>\r\n <head> </head><html>\r\n <form name=\"form1\" id=\"txt_form\" method=\"get\" action=\"/metoda1\">\r\n <br>Ime:<input type=\"text\" name=\"polje_ime\" required = \"required\"><br> <br>Prezime:<input type=\"text\" name=\"polje_prezime\" required = \"required\"><br> <br>Id:<input type=\"text\" name=\"polje_id\" required = \"required\"><br> <button type=\"submit\">Continue</button>  </form>\r\n<br><br><br></html> \n";
const char *err_msg1 = "Content-Type: text/html\r\n\r\n <!DOCTYPE HTML>\r\n <head> </head><html>\r\n <font color=\"red\"> Greska pri unosu </font> \r\n<br><br><br></html> \n";
const char *err_msg2 = "Content-Type: text/html\r\n\r\n <!DOCTYPE HTML>\r\n <head> </head><html>\r\n <font color=\"green\"> Uspesan unos!! </font> \r\n<br><br><br></html> \n";

ESP8266WebServer server(80);

File myFile;
File myFile2;

struct registrovani
{
  String mac;
  String ime;
  String prezime;
  String id;
  String vreme;
  bool prisutan;
};
registrovani r[MAX];
int reg_num = 0;

String all_in_range[MAX_RANGE];

String bafer_out[MAX_BAFER_OUT];
int bafer_out_num = 0;

String bafer_compare[MAX_BAFER_COMPARE];
int bafer_compare_num = 0;

//Set pins:  CE, IO,CLK
DS1302 rtc(0, 4, 5);

void handleRoot()
{
  server.send(200, "text/html", webPage);
}

void handleData()
{
  bool err_flag = false;
  
  if ((server.arg("polje_ime") == "") || (server.arg("polje_prezime") == "") || (server.arg("polje_id") == ""))
  {
    err_flag = true;
  }

  //provera flega za greske
  if (err_flag)
  {
    server.send(200, "text/html", err_msg1);
  }
  else
  {
    r[reg_num].ime = server.arg("polje_ime");
    r[reg_num].prezime = server.arg("polje_prezime");
    r[reg_num].id = server.arg("polje_id");

    struct station_info *stat_info;
    stat_info = wifi_softap_get_station_info();

    struct ip_addr *IPaddress;
    IPAddress address;

    while (stat_info != NULL)
    {
      IPaddress = &stat_info->ip;
      address = IPaddress->addr;

      if (address == server.client().remoteIP())
      {
        break;
      }
      
      stat_info = STAILQ_NEXT(stat_info, next);
    }

    wifi_softap_free_station_info();
    r[reg_num].mac = mac2str(stat_info->bssid, 0);
    r[reg_num].prisutan = false;
    reg_num++;
    Serial.println("Upis zavrsen");
    server.send(200, "text/html", err_msg2);
  }  
}

void promisc_cb(uint8_t *buf, uint16_t len)
{
  uint8_t* buffi;
  
  if (len == 12)
  {
    return; // Nothing to do for this package, see Espressif SDK documentation.
  }
  else if (len == 128) 
  {
    struct sniffer_buf2 *sniffer = (struct sniffer_buf2*)buf;
    buffi = sniffer->buf;
  } 
  else 
  {
    struct sniffer_buf *sniffer = (struct sniffer_buf*)buf;
    buffi = sniffer->buf;
  }
   
  String mac_str = mac2str(buffi, 4);
                   
  for (int i = 0; i < MAX_RANGE; i++)
  {
    if (mac_str == all_in_range[i])
    {
      break;
    }
    else if (all_in_range[i] == "/" && mac_str != "ff:ff:ff:ff:ff:ff")
    {
      all_in_range[i] = mac_str;
      bafer_compare[bafer_compare_num] = mac_str;
      bafer_compare_num++;
      break;
    }
  }
}

// Change the WiFi channel
void channelCh(void) 
{ 
  // Change the channels by modulo operation
  uint8 new_channel = wifi_get_channel()%12 + 1; 
  //Serial.printf("** Hop to %d **\n", new_channel); 
  wifi_set_channel(new_channel); 
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

  for (int i = 0; i < MAX_RANGE; all_in_range[i++] = "/");
  
  // Set the clock to run-mode, and disable the write protection
  rtc.halt(false);
  rtc.writeProtect(true);
}
 
void loop(void)
{
  server.handleClient();
  client_status();
}

void client_status()
{
  if (!reg_num)
  {
    myFile = SD.open("reg.txt");
    if (myFile)
    {
      while (myFile.available())
      {
        r[reg_num].mac = myFile.readStringUntil('|');
        r[reg_num].ime = myFile.readStringUntil('|');
        r[reg_num].prezime = myFile.readStringUntil('|');
        r[reg_num].id = myFile.readStringUntil('\n');
        reg_num++;
      }
    }
    else
    {
      Serial.println("error opening reg.txt");
    }
  }

  for (int br_korisnika = 1; br_korisnika <= reg_num; br_korisnika++)
  {
    boolean prisutan = false;
    boolean novi = false;
  
    String t = convertToStr(rtc.getTime()); 
   
    for (int i = 0; i < bafer_compare_num; i++)
    { 
      if (bafer_compare[i] == r[br_korisnika-1].mac)
      {
        prisutan = true;
  
        if (!r[br_korisnika-1].prisutan)
        {
          novi = true; 
          r[br_korisnika-1].vreme = t;
          r[br_korisnika-1].prisutan = true;
        }

        break;
      }
    }

    if (novi)
    {
      Serial.print(br_korisnika);
      Serial.println(". student is in range.");
      bafer_out[bafer_out_num] = r[br_korisnika-1].ime + "|" + r[br_korisnika-1].vreme + "|1";
      bafer_out_num++;
    }
    else if (!prisutan)
    {
      if (r[br_korisnika-1].prisutan)
      { 
        Serial.print(br_korisnika);
        Serial.println(". student is out of range.");
        bafer_out[bafer_out_num] = r[br_korisnika-1].ime + "|" + t + "|0";
        bafer_out_num++;
        r[br_korisnika-1].prisutan = false;
      }
    }
  }

  if (bafer_out_num == MAX_BAFER_OUT)
  {
    String datum = convertDateToStr(rtc.getTime());
  
    myFile2 = SD.open(datum + ".txt", FILE_WRITE);
    if (myFile2)
    {
      while (bafer_out_num)
      {
        myFile2.println(bafer_out[MAX_BAFER_OUT-bafer_out_num]);
        bafer_out_num--;
      }
      myFile2.close();
    }         
    else
    {
      Serial.println("error opening " + datum + ".txt");
    }
  }

  bafer_compare_num = 0;
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

