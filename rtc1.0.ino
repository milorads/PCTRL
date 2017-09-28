#include <DS1302RTC.h>
#include <Time.h>
#include <TimeLib.h>


// Set pins:  CE, IO,CLK
DS1302RTC RTC(15, 13, 12);

void setup()
{

  // Setup Serial connection
  Serial.begin(115200);
  
  Serial.println("RTC module activated");
  Serial.println();
  delay(500);
  
  if (RTC.haltRTC()) {
    Serial.println("The DS1302 is stopped.  Please run the SetTime");
    Serial.println("example to initialize the time and begin running.");
    Serial.println();
  }
  if (!RTC.writeEN()) {
    Serial.println("The DS1302 is write protected. This normal.");
    Serial.println();
  }
  
  delay(5000);
}
unsigned getCurrTime()
{
  tmElements_t tm;
  if(!RTC.read(tm))
  {
    return tm.Hour*60 + tm.Minute;
    Serial.println(tm.Hour*60 + tm.Minute);
  }
  else
  {
    Serial.println("DS1302 greska pri citanju!!");
    Serial.println();
    return 0;  
  }
}
void loop()
{

  /*tmElements_t tm;
  Serial.print("UNIX Time: ");
  Serial.print(RTC.get());

  if (! RTC.read(tm)) {
    Serial.print("  Time = ");
    print2digits(tm.Hour);
    Serial.write(':');
    print2digits(tm.Minute);
    Serial.write(':');
    print2digits(tm.Second);
    Serial.print(", Date (D/M/Y) = ");
    Serial.print(tm.Day);
    Serial.write('/');
    Serial.print(tm.Month);
    Serial.write('/');
    Serial.print(tmYearToCalendar(tm.Year));
    Serial.print(", DoW = ");
    Serial.print(tm.Wday);
    Serial.println();
  } else {
    Serial.println("DS1302 read error!  Please check the circuitry.");
    Serial.println();
    delay(9000);
  }
  
  // Wait one second before repeating :)
  delay (1000);*/

  Serial.print("MInutes: ");
  Serial.print(getCurrTime());
  delay(10000);
}

void print2digits(int number) {
  if (number >= 0 && number < 10)
    Serial.write('0');
  Serial.print(number);
}

