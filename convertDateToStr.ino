#define countof(a) (sizeof(a) / sizeof(a[0]))

String convertDateToStr(const RtcDateTime& dt)
{
  char datestring[11];
    String datum;

    snprintf_P(datestring, 
            countof(datestring),
            PSTR("%02u_%02u_%04u"),
            dt.Day(),
            dt.Month(),
            dt.Year());
    datum = datestring;
    return datum;
}
