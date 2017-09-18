#define MAX 2

//Replace with your network credentials
const char* ssid = "Proba";

ESP8266WebServer server(80);

String webPage = "";

File myFile;

String prijavljeni[MAX];

RtcDS3231<TwoWire> rtc(Wire);
