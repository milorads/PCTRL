#include "Log.h"
extern DS1302 rtc;


String getRtcTime()
{
  Time tm = rtc.getTime();
  char datestring[21];
  snprintf_P(datestring, 
             sizeof(datestring)/sizeof(datestring[0]),
             PSTR("%04u-%02u-%02uT%02u:%02u"),
             tm.year,
             tm.mon,
             tm.date,
             tm.hour,
             tm.min);
  return datestring;
}
String printSeverity(TLogSeverity sev,String code)
{
  String output;
  switch (sev)
  {
    case LogInfo1:
      output = "INFO1";
      break;
    case LogInfo2:
      output = "INFO2";
      break;
    case LogInfo3:
      output = "INFO3";
      break;
    case LogWarning1:
      output = "WARNING1";
      break;
    case LogWarning2:
      output = "WARNING2";
      break;
    case LogWarning3:
      output = "WARNING3";
      break;
    case LogError1:
      output = "ERROR1";
      break;
    case LogError2:
      output = "ERROR2";
      break;
    case LogError3:
      output = "ERROR3";
      break;
  }
  output = output + code;
  return output;
}


Log::Log()
{
  this->globalSeverity = LogError3;
  fileName = "logging.txt";

  this->message(LogInfo1,"Inicijalizacija logging monitora.Globani severity je "+printSeverity(globalSeverity," ")+" Naziv fajla: " + fileName,"A");

}

Log::Log(TLogSeverity severity,String filename)
{
  globalSeverity = severity;
  fileName = filename;

  this->message(LogInfo1,"Inicijalizacija logging monitora.Globani severity je "+printSeverity(globalSeverity," ")+" Naziv fajla: " + fileName,"A");
}

void Log::message(TLogSeverity severity,String msg,String code)
{
  if(severity <= this->globalSeverity)
  {
    this->logFile = SD.open(this->fileName,FILE_WRITE);
    if(this->logFile)
    {
         logFile.println(printSeverity(severity,code) + "[" +getRtcTime() +  "]" + msg);
         logFile.close();
    }    
  }
}
TLogSeverity Log::getSeverity()
{
  return this->globalSeverity;
}
void Log::setSeverity(TLogSeverity severity)
{
  this->globalSeverity = severity;
}
String Log::getFileName()
{
  return this->fileName;
}
void Log::setFileName(String fname)
{
  this->fileName = fname;
}

