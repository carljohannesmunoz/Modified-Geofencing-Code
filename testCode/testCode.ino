#include <TinyGPS++.h>
#include <SoftwareSerial.h>

static const int GPSBaud = 9600;
static const int SerialBaud = 9600;

static const int GPS_RXPin = 8, GPS_TXPin = 9; // GPS pins
static const int CELLULAR_RXPin = 3, CELLULAR_TXPin = 2; // GPS pins

static const int RecButton = 4; // Record Button Pin

static const int Duration = 5000; // Milliseconds between samples

static const String TraccarServer = "demo.traccar.org";
static const String APN = "internet";
static const String id = "1122334455";


uint32_t lasttime = 0;

TinyGPSPlus gps; // The TinyGPS++ object
SoftwareSerial GPSSerial(GPS_RXPin, GPS_TXPin); // The serial connection to the GPS device
SoftwareSerial CellularSerial(CELLULAR_RXPin, CELLULAR_TXPin); // The serial connection to the Cellular module

void setup()
{
  Serial.begin(SerialBaud);
  while (!Serial);
  GPSSerial.begin(GPSBaud);
  CellularSerial.begin(115200);
  CellularSerial.println("AT+CIPSHUT");
  CellularSerial.println("AT+CIPMUX=0");
  CellularSerial.println("AT+CGATT=1");
  CellularSerial.println("AT+CSTT=\"uinternet\",\"\",\"\"");
  CellularSerial.println("AT+CIICR");
  CellularSerial.println("AT+CIFSR");

  GPSSerial.listen();

  pinMode(RecButton, INPUT_PULLUP);

  lasttime = millis();
}

void loop()
{
  while (GPSSerial.available() > 0)
    if (gps.encode(GPSSerial.read())) sendDataToServer();

  if (millis() > 5000 && gps.charsProcessed() < 10)
  {
    Serial.println(F("No GPS detected: check wiring."));
    while (true);
  }
  // Button pushed and location valid and Time between samples bigger the Duration THEN send point to TRACCAR Server
  if ((digitalRead(RecButton) == 0) && gps.location.isValid() && (millis() - lasttime > Duration)) {
    // send point
    sendDataToServer();

    lasttime = millis();

    Serial.println(F("Sent point to Server"));
    Serial.print(gps.location.lat(), 8);
    Serial.print(F(":"));
    Serial.println(gps.location.lng(), 8);
    Serial.print(gps.date.value());
    Serial.print("-");
    Serial.println(gps.time.value());
  }
}

void sendDataToServer()
{
  CellularSerial.print("AT+CIPSTART=\"TCP\",\"");
  CellularSerial.print(TraccarServer);
  CellularSerial.println("\", 5055");
  CellularSerial.println("AT+CIPSEND");
  CellularSerial.print("GET /?id=");
  CellularSerial.print(id);
  CellularSerial.print("&lat=");
  CellularSerial.print(gps.location.lat(), 8);
  CellularSerial.print("&lon=");
  CellularSerial.print(gps.location.lng(), 8);
  CellularSerial.print("&timestamp=");
  CellularSerial.print(gps.date.value());
  CellularSerial.print("-");
  CellularSerial.print(gps.time.value());
  CellularSerial.println(" HTTP/1.1");
  CellularSerial.print("Host: ");
  CellularSerial.print(TraccarServer);
  CellularSerial.println("\r\n");
  CellularSerial.write(0x1A);
  delay(1000);
}
