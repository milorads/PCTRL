#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>


extern "C" {
#include<user_interface.h>
}

//ssid i pass
const char *ssid = "IoT";
const  char *pass = "sparkfun";

//mac i ip
unsigned char mac[6];
unsigned char ip[4];

//web stranica
  const char *form_str = "Content-Type: text/html\r\n\r\n <!DOCTYPE HTML>\r\n <head> </head><html>\r\n <form name=\"form1\" id=\"txt_form\" method=\"get\" action=\"/metoda1\">\r\n <br>Ime:<input type=\"text\" name=\"polje_ime\" required = \"required\"><br> <br>Prezime:<input type=\"text\" name=\"polje_prezime\" required = \"required\"><br> <br>Id:<input type=\"text\" name=\"polje_id\" required = \"required\"><br> <button type=\"submit\">Continue</button>  </form>\r\n<br><br><br></html> \n";
  const char *err_msg1 = "Content-Type: text/html\r\n\r\n <!DOCTYPE HTML>\r\n <head> </head><html>\r\n <font color=\"red\"> Greska pri unosu </font> \r\n<br><br><br></html> \n";
  const char *err_msg2 = "Content-Type: text/html\r\n\r\n <!DOCTYPE HTML>\r\n <head> </head><html>\r\n <font color=\"green\"> Uspesan unos!! </font> \r\n<br><br><br></html> \n";
//instanciranje servera na portu 80
ESP8266WebServer server(80);

void handleRoot()
{
  server.send(200,"text/html",form_str);
  String add = server.client().remoteIP().toString();
  Serial.println(add);
}
void takeData()
{ 
  struct ip_addr *IPaddress;
  IPAddress address;
  String par_ime = server.arg("polje_ime");
  String par_prezime = server.arg("polje_prezime");
  String par_id = server.arg("polje_id");
  //Serial.println("<<" + par_ime + "|" + par_prezime + "|" + par_id + ">>");
  bool err_flag = false;
  //provera!!
  if(err_flag)
  {
    server.send(200,"text/html",err_msg1);
  }
  else
  {
    struct station_info *stat_info;
    stat_info = wifi_softap_get_station_info();
    IPaddress = &stat_info -> ip;
    address = IPaddress->addr;
    ip[0] = address[0];
    ip[1] = address[1];
    ip[2] = address[2];
    ip[3] = address[3];
    
    mac[0] = stat_info->bssid[0];
    mac[1] = stat_info->bssid[1];
    mac[2] = stat_info->bssid[2];
    mac[3] = stat_info->bssid[3];
    mac[4] = stat_info->bssid[4];
    mac[5] = stat_info->bssid[5];


    
    server.send(200,"text/html",err_msg2);
    Serial.println("<<" + par_ime + "|" + par_prezime + "|" + par_id + ">>");
    Serial.print(ip[0]);
    Serial.print(".");
    Serial.print(ip[1]);
    Serial.print(".");
    Serial.print(ip[2]);
    Serial.print(".");
    Serial.println(ip[3]);
    Serial.print(mac[0],HEX);
    Serial.print(mac[1],HEX);
    Serial.print(mac[2],HEX);
    Serial.print(mac[3],HEX);
    Serial.print(mac[4],HEX);
    Serial.print(mac[5],HEX);
    
  }
  
}
void setup() {
  delay(1000);
  Serial.begin(115200);
  Serial.println();
  Serial.print("Configuring access point...");

  //DODAJ I PASSWORD!!
  WiFi.softAP(ssid);
  IPAddress myIP = WiFi.softAPIP();
  Serial.println("AP IP address: ");
  Serial.println(myIP);
  server.on("/",handleRoot);
  server.on("/metoda1",takeData);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}




