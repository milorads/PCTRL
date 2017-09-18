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
