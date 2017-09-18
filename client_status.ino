void client_status()
{
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
