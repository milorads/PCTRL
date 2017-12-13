#ifndef _LOG_H_
#define _LOG_H_

#include <SD.h>
#include <Arduino.h>
#include <DS1302.h>

enum TLogSeverity {LogInfo1,LogInfo2,LogInfo3,LogWarning1,LogWarning2,LogWarning3,LogError1,LogError2,LogError3};
class Log
{
  public:
    Log();
    Log(TLogSeverity,String);
    void message(TLogSeverity,String,String);
    TLogSeverity getSeverity();
    void setSeverity(TLogSeverity);

    String getFileName();
    void setFileName(String);

  private:
    TLogSeverity globalSeverity;
    String fileName;
    File logFile;
};

#endif
