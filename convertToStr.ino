#define countof(a) (sizeof(a) / sizeof(a[0]))

String convertToStr(const RtcDateTime& dt)
{
    char datestring[21];
    String datum;

    snprintf_P(datestring, 
            countof(datestring),
            PSTR("%04u-%02u-%02uT%02u:%02u"),
            dt.Year(),
            dt.Month(),
            dt.Day(),
            dt.Hour(),
            dt.Minute());
    datum = datestring;
    return datum;
}
