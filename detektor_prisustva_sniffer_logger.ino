#include <SD.h>
#include <DS1302.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <Ticker.h>
#include <DNSServer.h>
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
#define countof(a) (sizeof(a)/sizeof(a[0]))
#define MAX 30
#define MAX_BAFER_COMPARE 2000
#define MAX_CONN_NUM 30

#define ITERATION_NUM 10
// Delay of loop function in milli seconds
# define __delay__ 10
// Delay of channel changing in seconds
# define __dlay_ChannelChange__ 0.5
# define __serverPeriod__ 120
# define __regCheck__ 5
# define CHANNEL_SHIFT_NUM 48
//Ticker for channel hopping
Ticker tm;
//Promiscuous callback structures for storing package data, see Espressif SDK handbook
struct RxControl 
{
    signed rssi: 8;
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
const char* ssid     = "Proba";
const char *webPage  = "Content-Type: text/html\r\n\r\n <!DOCTYPE HTML>\r\n <head> </head><html>\r\n <form name=\"form1\" id=\"txt_form\" method=\"get\" action=\"/metoda1\">\r\n <br>Ime:<input type=\"text\" name=\"polje_ime\" required = \"required\"><br> <br>Prezime:<input type=\"text\" name=\"polje_prezime\" required = \"required\"><br> <br>Id:<input type=\"text\" name=\"polje_id\" required = \"required\"><br> <button type=\"submit\">Continue</button>  </form>\r\n<br><br><br></html> \n";
const char *err_msg1 = "Content-Type: text/html\r\n\r\n <!DOCTYPE HTML>\r\n <head> </head><html>\r\n <font color=\"red\"> Greska pri unosu </font> \r\n<br><br><br></html> \n";
const char *err_msg2 = "Content-Type: text/html\r\n\r\n <!DOCTYPE HTML>\r\n <head> </head><html>\r\n <font color=\"green\"> Uspesan unos!! </font> \r\n<br><br><br></html> \n";
const char *err_msg3 = "Content-Type: text/html\r\n\r\n <!DOCTYPE HTML>\r\n <head> </head><html>\r\n <font color=\"green\"> Vec si registrovan, budalice mala! </font> \r\n<br><br><br></html> \n";

ESP8266WebServer server(80);

File myFile;
File myFile2;
File logFile;

struct registrovani
{
    uint8_t mac[6];
    String ime;
    String prezime;
    String id;
    String vreme_ulaska = "";
    String vreme_izlaska="";
    uint8 odsutan;
};
struct registration_semaphore
{
    IPAddress address;
    bool flag;
};

registration_semaphore reg_semaphore_arr[MAX_CONN_NUM];
registrovani r[MAX];
uint8 semaphore_num = 0;
int reg_num = 0;
int seq_counter = 0;
uint8 direction_flag = 1;
struct bafer_compare_struct
{
    uint8_t mac[6];
};


bafer_compare_struct bafer_compare[MAX_BAFER_COMPARE];
int bafer_compare_num = 0;
char mode_flag         = 0;
bool registration_flag = false;
char timer_cnt         = 0;
char counter           = 0;
//Set pins:  CE, IO,CLK
DS1302 rtc(0, 4, 5);

//dns seerver
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
  //Deo koda koji zabranjuje ponovnu registraciju
  for (int i = 0; i < reg_num; i++)
  {
    struct station_info *stat_info;
    stat_info = wifi_softap_get_station_info();
    struct ip_addr *IPaddress;
    IPAddress address;
    while (stat_info != NULL)
    {
        IPaddress = &stat_info->ip;
        address   = IPaddress->addr;
        if (address == server.client().remoteIP())
        {
             
            if (r[i].mac[0] == stat_info->bssid[0] && r[i].mac[1] == stat_info->bssid[1] && r[i].mac[2] == stat_info->bssid[2] &&
                r[i].mac[3] == stat_info->bssid[3] && r[i].mac[4] == stat_info->bssid[4] && r[i].mac[5] == stat_info->bssid[5])
            { 
              server.send(200, "text/html", err_msg3);
              /*...........................*/
              /*logovanje*/
              if(logFile)
              {
                logFile.println("Vec je prijavljen na ovoj mac adresi korisnik:");
                logFile.println(r[i].ime + " " + r[i].prezime[i] + "|" + String(r[i].mac[0]) + ":" + String(r[i].mac[1]) + ":" + String(r[i].mac[2]) + ":" + String(r[i].mac[3]) + ":" + String(r[i].mac[4]) + ":" + String(r[i].mac[5]) + " je probao ponovo da se uloguje");
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
    logFile.println("Zahtev za registracijom je regularan, sledi setovanje flega");
  }
  /*...........................*/
  //setovanje flega
  bool flg = true;
  /**************************************/
  logFile.print("flg = ");
  logFile.println(String(flg));
  logFile.print("pocinje for petlja koja pocinje od 0 do (semaphore_num) ");
  logFile.println(semaphore_num);
  /**************************************/
  for(int i = 0;i<semaphore_num;i++)
  {
    if(reg_semaphore_arr[i].address == server.client().remoteIP())
    {
      logFile.println("ip adresa u iteraciji" + String(i) + "jednaka ip adresi klijenta, reg_semaphore_arr["+String(i)+"].flag = true; flg = false");
      reg_semaphore_arr[i].flag = true;
      flg = false;
    }
  }
  
  if(flg)
  {
    logFile.println("klijenta nema u kolekciji semafora, dodaje se na kraju!!");
    logFile.println("reg_semaphore_arr[" + String(semaphore_num) + "].flag = true;");
    reg_semaphore_arr[semaphore_num].flag = true;
    reg_semaphore_arr[semaphore_num].address = server.client().remoteIP();
    semaphore_num++;
    logFile.println("Semaphore_num se uvecava za jedan: " + semaphore_num); 
  }
  logFile.println("server salje web stranicu za prijavu");
  logFile.println("**KRAJ FUNKCIJE HandleRoot**");
  logFile.close();
  /******************************************/
  server.send(200, "text/html", webPage);
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
        r[reg_num].ime     = server.arg("polje_ime");
        r[reg_num].prezime = server.arg("polje_prezime");
        r[reg_num].id      = server.arg("polje_id");
        Serial.println(r[reg_num].ime);
        struct station_info *stat_info;
        stat_info = wifi_softap_get_station_info();
        struct ip_addr *IPaddress;
        IPAddress address;
        logFile.println("pocetak prolaska kroz listu stat_info");
        while (stat_info != NULL)
        {
            IPaddress = &stat_info->ip;
            address   = IPaddress->addr;
            if (address == server.client().remoteIP())
            {
                logFile.println("pronadjena odg ip adresa");
                logFile.println("pocetak for petlje kroz niz flegova, semaphore_num:" + String(semaphore_num));   
                for(int i = 0;i<semaphore_num;i++)
                {
                  logFile.println("i: "+ String(i));
                  if(reg_semaphore_arr[i].address == server.client().remoteIP())
                  {
                    logFile.println("flag korisnika sa ip adresom: " + String(reg_semaphore_arr[i].address)+"je setova na false");
                    logFile.println("izlazak iz petlje");
                    reg_semaphore_arr[i].flag = false;
                    Serial.print("false ");
                    Serial.println(i);
                  }
                }
                logFile.println("kraj for petlje kroz niz flegova");          
                break;
            }
            stat_info = STAILQ_NEXT(stat_info, next);
        }
        logFile.println("kraj prolaska kroz listu stat_info");
        wifi_softap_free_station_info();
        logFile.println("dealociranje liste station_info");
        logFile.println("upisivanje mac adrese u kolekciju r");
        r[reg_num].mac[0]   = stat_info->bssid[0];
        r[reg_num].mac[1]   = stat_info->bssid[1];
        r[reg_num].mac[2]   = stat_info->bssid[2];
        r[reg_num].mac[3]   = stat_info->bssid[3];
        r[reg_num].mac[4]   = stat_info->bssid[4];
        r[reg_num].mac[5]   = stat_info->bssid[5];
        r[reg_num].odsutan = false;
        logFile.println("pocetak upisa podataka na sd karticu");
        logFile.close();
        /*********************************************/
        myFile = SD.open("reg.txt", FILE_WRITE);
        if (myFile)
        {
            String mac_str = mac2str(r[reg_num].mac, 0);
            myFile.println(mac_str + "|" + r[reg_num].ime + "|" + r[reg_num].prezime + "|" + r[reg_num].id);
            myFile.close();
        }
        else
        {
            Serial.println("error opening reg.txt");
        }
        logFile = SD.open("loging.txt",FILE_WRITE);
        reg_num++;
        Serial.println("Upis zavrsen");
        
        logFile.println("Upis zavrsen");
        logFile.println("reg_num: " + String(reg_num));
        
        server.send(200, "text/html", err_msg2);

        logFile.println("poslata stranica za uspesno prijavljivanje");
        logFile.println("**KRAJ FUNKCIJE HANDLE DATA **");
        logFile.close();
    } 
}
//callback funkcija sniffera - poziva se nakon primljenog paketa
void promisc_cb(uint8_t *buf, uint16_t len)
{
    uint8_t* buffi;
    if (len == 12)
    {
        return; // Nothing to do for this package, see Espressif SDK documentation.
    }
    else if (len == 128) 
    {
        struct sniffer_buf2 *sniffer = (struct sniffer_buf2*) buf;
        buffi = sniffer->buf;
    } 
    else 
    {
        struct sniffer_buf *sniffer = (struct sniffer_buf*) buf;
        buffi = sniffer->buf;
    }
    if (bafer_compare_num == MAX_BAFER_COMPARE)
    {
        return;
    }
    if(!(buffi[10] == 0xFF && buffi[11] == 0xFF && buffi[12] == 0xFF && buffi[13] == 0xFF && buffi[14] == 0xFF && buffi[15] == 0xFF))
    {
      if(bafer_compare_num && !(buffi[10] == bafer_compare[bafer_compare_num - 1].mac[1] && buffi[11] == bafer_compare[bafer_compare_num - 1].mac[2] && buffi[12] == bafer_compare[bafer_compare_num - 1].mac[3] && buffi[13] == bafer_compare[bafer_compare_num - 1].mac[3] && buffi[14] == bafer_compare[bafer_compare_num - 1].mac[4] && buffi[15] == bafer_compare[bafer_compare_num - 1].mac[5]))
      {
        for(int i =0;i<6;i++)
        bafer_compare[bafer_compare_num].mac[i]  = buffi[10 + i];
        bafer_compare_num++; 
      }
      else if(!bafer_compare_num)
      {
        for(int i =0;i<6;i++)
        bafer_compare[bafer_compare_num].mac[i]  = buffi[10 + i];
        bafer_compare_num++;         
      }
      
    }
    
}
//callback funkcija tajmera, tu se proverava u kom je 
//stanju masina stanja kada tajmer generise callback fju
void channelCh(void)
{ 
    //server mode period od 2 minuta je istekao, provera da li su svi zavrsili sa registrovanjem
    if (!mode_flag)
    {
        logFile = SD.open("loging.txt",FILE_WRITE);
        logFile.println("**POCETAK CALLBACK FUNKCIJE Channel_ch, istekao period za server***"); 
        //ako je neko pokrenuo registraciju, tajmer ce na svakih 5 sekundi proveravati da li su zavrsene sve registracije
        //to ce raditi 60 puta (5 min) posle cegaa ce progrlasiti timeout i preci u sniffer mode
        //ovaj flag ce biti veci od nule ako je neko ucitao formu za prijavljivanje,
        //a nije submitovao
        //u tom slucaju prelazi u gore opisano stanje provere
            bool check = false;
            logFile.println("provera da li su svi semafori na false");
            for(int i=0;i < semaphore_num; i++)
            {
              if(reg_semaphore_arr[i].flag == true)
              {
                check = true;
                Serial.println("AA");
                Serial.println("check=true, nisu svi kliknuli submit");
                break;
              }
            }
            if(check)
            {
               logFile.println("check = true, produzava se vreme registracije");
                seq_counter++;
                logFile.println("seq_counter = " + String(seq_counter));
                if(seq_counter == 3)
                {
                  Serial.println("isteklo je produzeno vreme za registraciju!!");
                  logFile.println("isteklo je produzeno vreme za registraciju!!, seq_counter=0");
                  seq_counter = 0;
                  check = false;
                }
                else
                {
                  logFile.println("produzava se vreme za registraciju");
                  Serial.println("Produzava se server na jos 2 minuta"); 
                }
                             
            }
            if(!check)
            {
                logFile.println("Prelazak na sniffer, setovanje svih flagova na false");
                for(int i = 0;i<semaphore_num;i++)
                {
                  reg_semaphore_arr[i].flag = false;
                  reg_semaphore_arr[i].address = IPAddress(0,0,0,0);
                }
                
                semaphore_num = 0;
                mode_flag = 1;
                logFile.println("zavrseno setovanje, semaphore_num=0, mode_flag=1, timer detach, bafer_compare_num=0");
                tm.detach();
                bafer_compare_num = 0;
                tm.attach(__dlay_ChannelChange__, channelCh);
                logFile.println("timer attach channelCh delay");
                //setovanje espa da radi kao sniffer
                logFile.println("setovanje espa da radi kao sniffer");
                wifi_set_opmode(STATION_MODE);
                wifi_promiscuous_enable(1);
                Serial.println("Prelazi se sa Server na Sniffer mode");
                logFile.println("Prelazi se sa Server na Sniffer mode");
                             
            }
            logFile.println("**KRAJ CALLBACK FUNKCIJE channel_change**");
            logFile.close(); 
        
    }
    else
    //sniffing mode,u 36 sekundi proverava sve kanale
    // *TO DO: pokupiti te adrese u neku kolekciju podataka 
    {
        // Change the channels by modulo operation
        logFile = SD.open("loging.txt",FILE_WRITE);
        logFile.println("**POCETAK CALLBACK FUNKCIJE Channel_ch, poceo sniffing mode***");
        logFile.println("direction_flag : " + String(direction_flag));
        uint8 new_channel;
        
        if(direction_flag)
        {
          new_channel = wifi_get_channel() + 1;
          logFile.println("new_channel: " + String(new_channel));
          if(new_channel == 12)
            direction_flag = 0;
        }
        else
        { 
          new_channel = wifi_get_channel() - 1;
          logFile.println("new_channel: " + String(new_channel));
          if(new_channel == 1)
            direction_flag = 1;
        }
        wifi_set_channel(new_channel);
        timer_cnt ++;
        logFile.print("timer_cnt: ");
        logFile.println(timer_cnt);
        if(timer_cnt == CHANNEL_SHIFT_NUM)
        {
            timer_cnt = 0;
            logFile.print("timer_cnt:");
            logFile.print(timer_cnt);
            logFile.println("timer detach, counter = 0");
            tm.detach();
            counter = 0;
            
            //setovanje esp-a kao server
              wifi_promiscuous_enable(0);
              Serial.println("zavrseno snifovanje!");
              logFile.println("setovanje espa kao server, mode_flag = 0");
              mode_flag = 0;

              logFile.println("SODTAP_MODE");
              wifi_set_opmode(SOFTAP_MODE);
              Serial.println("Prelazi se sa Sniffer na Server mode");
              Serial.print("Broj kuraca u baferu: ");
              Serial.println(bafer_compare_num);
              logFile.println("bafer_compare_num: " + String(bafer_compare_num));
              logFile.println("timer attach :server period");
              tm.attach(__serverPeriod__, channelCh);
              logFile.println("poziv funkcije client_status");
              logFile.close();
              client_status();
              logFile = SD.open("loging.txt",FILE_WRITE);
              logFile.println("kraj fje client_status");
                          
              bafer_compare_num = 0;
              logFile.println("bafer_compare_num: " + String(bafer_compare_num));

        }     
        logFile.println("**KRAJ CALLBACK FUNKCIJE channel_change**");
        logFile.close();
    }
    
} 
void setup(void)
{  
    delay(1000);
    //Serial monitoring initialization
    Serial.begin(115200);
    Serial.println();

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
      logFile.close();      
    }
    /***************************/
    Serial.print("Configuring access point...");
    WiFi.softAP(ssid);
    //WiFi.hostname
    Serial.println("");
    Serial.print("IP address: ");
    Serial.println(WiFi.softAPIP());

    //dns config
     dnsServer.setTTL(300);
     dnsServer.setErrorReplyCode(DNSReplyCode::ServerFailure);
     dnsServer.start(DNS_PORT, "prijavi.me", apIP); 
    server.onNotFound([]() {
    String message = "DNS error";
    message += "URI: ";
    message += server.uri();

    server.send(200, "text/plain", message);
  });
    //kraj dns-a
    
    //po difoltu radi kao server
    Serial.println("AP mode");
    wifi_set_opmode(SOFTAP_MODE);
    //Set the promiscuous related options
    wifi_promiscuous_enable(0);
    wifi_set_promiscuous_rx_cb(promisc_cb);
    Serial.printf("Setup done!");
    tm.attach(__serverPeriod__, channelCh);
    //konfiguracija softAP moda
    struct softap_config config;
    wifi_softap_get_config(&config); // Get config first.
    config.max_connection = MAX_CONN_NUM; // how many stations can connect to ESP8266 softAP at most.
    wifi_softap_set_config(&config);// Set ESP8266 softap config

    /***************************/
    /*loging*/
    logFile = SD.open("loging.txt",FILE_WRITE);
    if(logFile)
    {
      logFile.println("soft AP config finished succesfully");
      logFile.println("HTTP server started");
      logFile.close();      
    }
    /***************************/
    server.on("/", handleRoot);
    server.on("/metoda1", handleData);
    server.begin();
    Serial.println("HTTP server started");
    
    if (!reg_num)
    {
        myFile = SD.open("reg.txt");
        if (myFile)
        {
            while (myFile.available())
            {
                String mac_str     = myFile.readStringUntil(':');
                r[reg_num].mac[0]  = str2mac(mac_str);
                mac_str            = myFile.readStringUntil(':');
                r[reg_num].mac[1]  = str2mac(mac_str);
                mac_str            = myFile.readStringUntil(':');
                r[reg_num].mac[2]  = str2mac(mac_str);
                mac_str            = myFile.readStringUntil(':');
                r[reg_num].mac[3]  = str2mac(mac_str);
                mac_str            = myFile.readStringUntil(':');
                r[reg_num].mac[4]  = str2mac(mac_str);
                mac_str            = myFile.readStringUntil('|');
                r[reg_num].mac[5]  = str2mac(mac_str);
                r[reg_num].ime     = myFile.readStringUntil('|');
                r[reg_num].prezime = myFile.readStringUntil('|');
                r[reg_num].id      = myFile.readStringUntil('\n');
                reg_num++;
            }
            myFile.close();
        }
        else
        {
            Serial.println("error opening reg.txt");
        }
    }
    /***************************/
    /*loging*/
    logFile = SD.open("loging.txt",FILE_WRITE);
    if(logFile)
    {
      logFile.println("loading user info finished");
      logFile.println("rtc started");
      logFile.close();      
    }
    /***************************/
    
    // Set the clock to run-mode, and enable the write protection
    rtc.halt(false);
    rtc.writeProtect(true);
}
 
void loop(void)
{
    if(mode_flag)
    {
        delay(100);      
    }
    else
    {
        //dns processing
        dnsServer.processNextRequest(); 
        server.handleClient();
        //client_status(); 
    }
}
void client_status()
{
    logFile=SD.open("loging.txt",FILE_WRITE);
    logFile.println("*FUNKCIJA client_status pozvana *");
    logFile.close();
    for (int br_korisnika = 1; br_korisnika <= reg_num; br_korisnika++)
    {
        boolean prisutan = false;
        boolean novi = false;
        String t = convertToStr(rtc.getTime()); 
        for (int i = 0; i < bafer_compare_num; i++)
        {
            if ((bafer_compare[i].mac[0] == r[br_korisnika-1].mac[0] && bafer_compare[i].mac[1] == r[br_korisnika-1].mac[1] &&
                 bafer_compare[i].mac[2] == r[br_korisnika-1].mac[2] && bafer_compare[i].mac[3] == r[br_korisnika-1].mac[3] &&
                 bafer_compare[i].mac[4] == r[br_korisnika-1].mac[4] && bafer_compare[i].mac[5] == r[br_korisnika-1].mac[5]))
            {
                prisutan = true;
                if (!r[br_korisnika-1].odsutan)
                {
                    novi = true; 
                    r[br_korisnika-1].vreme_ulaska = t;
                    r[br_korisnika-1].odsutan++;
                }
                else r[br_korisnika-1].odsutan = 1;
                break;
            }
        }
        if (novi)
        {
            String datum = convertDateToStr(rtc.getTime());
            myFile2 = SD.open(datum + ".txt", FILE_WRITE);
            if (myFile2)
            {
                Serial.println(r[br_korisnika-1].ime + " is in range.");
                //myFile2.println(r[br_korisnika-1].ime + "|" + r[br_korisnika-1].vreme + "|1");
                //myFile2.close();
                logFile = SD.open("loging.txt",FILE_WRITE);
                logFile.println(r[br_korisnika-1].ime + " " + r[br_korisnika-1].prezime + "|"+r[br_korisnika-1].vreme_ulaska + "is in range");
                logFile.close();
            }         
            else
            {
                Serial.println("error opening " + datum + ".txt");
            }
        }
        else if(!prisutan)
        {
            if(r[br_korisnika-1].odsutan == 1)
              r[br_korisnika-1].vreme_izlaska = t;
            if(r[br_korisnika-1].odsutan > 0)
            r[br_korisnika-1].odsutan++;
            if (r[br_korisnika-1].odsutan == ITERATION_NUM)
            { 
                String datum = convertDateToStr(rtc.getTime());
                myFile2 = SD.open(datum + ".txt", FILE_WRITE);
                if (myFile2)
                {
                    Serial.println(r[br_korisnika-1].ime + " is out of range.");
                    myFile2.println(r[br_korisnika-1].ime + " " + r[br_korisnika-1].prezime + "|" + r[br_korisnika-1].vreme_ulaska + "|" + r[br_korisnika-1].vreme_izlaska);
                    myFile2.close();
                    
                    logFile = SD.open("loging.txt",FILE_WRITE);
                    logFile.println(r[br_korisnika-1].ime + " " + r[br_korisnika-1].prezime + "|"+ r[br_korisnika-1].vreme_izlaska + "is out of range");
                    logFile.close();
                    
                    r[br_korisnika-1].odsutan = 0;
                    r[br_korisnika-1].vreme_ulaska = "";
                    r[br_korisnika-1].vreme_izlaska = "";
                }         
                else
                {
                    Serial.println("error opening " + datum + ".txt");
                }
            }
        }
    }
    logFile=SD.open("loging.txt",FILE_WRITE);
    logFile.println("*FUNKCIJA client_status zavrsena*");
    logFile.close();
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
    uint8_t mac;
    mac = (str[0] < 65)? 16*(str[0] - 48) : 16*(str[0] - 55);
    mac += (str[1] < 65)? str[1] - 48 : str[1] - 55;
    return mac;
}
