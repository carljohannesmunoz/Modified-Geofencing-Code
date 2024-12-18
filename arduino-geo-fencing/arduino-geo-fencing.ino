

#include <SoftwareSerial.h> //Libraries required for Serial communication
#include <AltSoftSerial.h>
#include <TinyGPS++.h>


char Received_SMS; //Here we store the full received SMS (with phone sending number and date/time) as char
short GET_GPS=-1;
short SET_UP=-1;

String Data_SMS; //Here's the SMS that we gonna send to the phone number


SoftwareSerial sim800l(3, 2); // RX,TX for Arduino and for the module it's TXD RXD, they should be inverted

AltSoftSerial neogps;
TinyGPSPlus gps;
String Phone = "+639691682984";
boolean send_alert_once = true;
int state = 0;
const int pin = 12;
bool isSetup = false;
int count = 0;
int timeout = 0;
//--------------------------------------------------------------
// Size of the geo fence (in meters)
const float maxDistance = 30;

//--------------------------------------------------------------
float initialLatitude = 0;
float initialLongitude = 0;

float latitude, longitude;

void getGps(float& latitude, float& longitude);

void setup()
{
neogps.begin(9600);
sim800l.begin(9600); //Begin all the communications needed Arduino with PC serial and Arduino with all devices
Serial.begin(9600);
Serial.println("Starting ...");
delay(6000); //Delay to let the module connect to network, can be removed
ReceiveMode(); //Calling the function that puts the SIM800L moduleon receiving SMS mode

}




void loop() {

String RSMS; //We add this new variable String type, and we put it in loop so everytime gets initialized
//This is where we put the Received SMS, yes above there's Recevied_SMS variable, we use a trick below
//To concatenate the "char Recevied_SMS" to "String RSMS" which makes the "RSMS" contains the SMS received but as a String
//The recevied SMS cannot be stored directly as String

getGps(latitude, longitude);

float distance = getDistance(latitude, longitude, initialLatitude, initialLongitude);

Serial.println(distance);
while(sim800l.available()>0){ //When SIM800L sends something to the Arduino... problably the SMS received... if something else it's not a problem

Received_SMS=sim800l.read(); //"char Received_SMS" is now containing the full SMS received
//Serial.print(Received_SMS); //Show it on the serial monitor (optional)
RSMS.concat(Received_SMS); //concatenate "char received_SMS" to RSMS which is "empty"
GET_GPS=RSMS.indexOf("GET-GPS"); //And this is why we changed from char to String, it's to be able to use this function "indexOf"
SET_UP=RSMS.indexOf("SET-UP");
}
  
if(isSetup == true && count > 20) {
  
  if(distance > maxDistance) {
      //------------------------------------------
      if(send_alert_once == true){
        isInside();
        send_alert_once = false;
        isSetup = false;
        initialLatitude = 0;
        initialLongitude = 0;
      }
      //------------------------------------------
    }
    else{
      send_alert_once = true;
    }
  } else {
    isSetup = false;
  }


if(GET_GPS!=-1){
  //Show it on the serial monitor also optional
  sim800l.print("AT+CMGF=1\r");
  delay(100);
  sim800l.print("AT+CMGS=\""+Phone+"\"\r");
  delay(1000);
  sim800l.println("maps.google,com/maps?q=");
  sim800l.println("Alert..... www.openstreetmap.org/?mlat=" + String(latitude, 6) + "&mlon=" + String(longitude, 6));
  sim800l.println("distance to fence: " + String(distance));
  sim800l.print("initialLat " + String(initialLatitude) + ", " + "initialLon" + String(initialLongitude));
  delay(100);
  sim800l.write(0x1A); //ascii code for ctrl-26 //sim800.println((char)26); //ascii code for ctrl-26
  delay(100);
  Serial.println("SMS Sent Successfully."); 
  ReceiveMode(); //Come back to Receving SMS mode and wait for other SMS
  
  GET_GPS=-1;
  }

 if(SET_UP!=-1){ //If "SET-UP" word is found within the SMS, it means that SET_UP have other value than -1 so we can proceed
  initialLatitude = gps.location.lat();
  initialLongitude = gps.location.lng();
  sim800l.print("AT+CMGF=1\r");
  delay(100);
  sim800l.print("AT+CMGS=\""+Phone+"\"\r");
  delay(100);
  sim800l.println("Fence is deployed at: www.openstreetmap.org/?mlat=" + String(latitude, 6) + "&mlon=" + String(longitude, 6));
  delay(100);
  sim800l.write(0x1A); //ascii code for ctrl-26 //sim800.println((char)26); //ascii code for ctrl-26
  delay(100);
  
//  sendAlert();
  ReceiveMode(); //Come back to Receving SMS mode and wait for other SMS
  isSetup = true;
  SET_UP=-1;
  }
  
 if (digitalRead(pin) == HIGH && state == 0) {
  pushAlert();
  state = 1;
    }
  if (digitalRead(pin) == LOW) {
      state = 0;
    }
if(send_alert_once == false) {
  timeout += 1;
  if(timeout == 60) {
    send_alert_once == true;
    timeoutAvail();
  }
}
count += 1;
Serial.println(count);
delay(5);
}


void Serialcom() //This is used with ReceiveMode function, it's okay to use for tests with Serial monitor
{
delay(500);
while(Serial.available())
{
sim800l.write(Serial.read());//Forward what Serial received to Software Serial Port
}
while(sim800l.available())
{
Serial.write(sim800l.read());//Forward what Software Serial received to Serial Port
}
}

void ReceiveMode(){ //Set the SIM800L Receive mode

sim800l.println("AT"); //If everything is Okay it will show "OK" on the serial monitor
Serialcom();
sim800l.println("AT+CMGF=1"); // Configuring TEXT mode
Serialcom();
sim800l.println("AT+CNMI=2,2,0,0,0"); //Configure the SIM800L on how to manage the Received SMS... Check the SIM800L AT commands manual
Serialcom();
}

float getDistance(float flat1, float flon1, float flat2, float flon2) {

  // Variables
  float dist_calc=0;
  float dist_calc2=0;
  float diflat=0;
  float diflon=0;

  // Calculations
  diflat  = radians(flat2-flat1);
  flat1 = radians(flat1);
  flat2 = radians(flat2);
  diflon = radians((flon2)-(flon1));

  dist_calc = (sin(diflat/2.0)*sin(diflat/2.0));
  dist_calc2 = cos(flat1);
  dist_calc2*=cos(flat2);
  dist_calc2*=sin(diflon/2.0);
  dist_calc2*=sin(diflon/2.0);
  dist_calc +=dist_calc2;

  dist_calc=(2*atan2(sqrt(dist_calc),sqrt(1.0-dist_calc)));
  
  dist_calc*=6371000.0; //Converting to meters

  return dist_calc;
}

void getGps(float& latitude, float& longitude)
{
  // Can take up to 60 seconds
  boolean newData = false;
  for (unsigned long start = millis(); millis() - start < 2000;){
    while (neogps.available()){
      if (gps.encode(neogps.read())){
        newData = true;
        break;
      }
    }
  }
  
  if (newData) //If newData is true
  {
    latitude = gps.location.lat();
    longitude = gps.location.lng();
    newData = false;
  }
  else {
//    Send_Data();
    latitude = 0;
    longitude = 0;
  }
}

void pushAlert() 
{
  sim800l.print("AT+CMGF=1\r");
  delay(100);
  sim800l.print("AT+CMGS=\""+Phone+"\"\r");
  delay(100);
  sim800l.println("maps.google,com/maps?q=");
  sim800l.print("Alert buttton pressed..... www.openstreetmap.org/?mlat=" + String(latitude, 6) + "&mlon=" + String(longitude, 6));
  delay(100);
  sim800l.write(0x1A); //ascii code for ctrl-26 //sim800.println((char)26); //ascii code for ctrl-26
  delay(100);
  Serial.println("SMS Sent Successfully.");
  
}

void isInside()
{
  sim800l.print("AT+CMGF=1\r");
  delay(1000);
  sim800l.print("AT+CMGS=\""+Phone+"\"\r");
  delay(1000);
  sim800l.println("maps.google,com/maps?q=");
  sim800l.print("Alert outside fence..... www.openstreetmap.org/?mlat=" + String(latitude, 6) + "&mlon=" + String(longitude, 6));
  sim800l.println("You can deploy Geofence after 1 mins");
  delay(100);
  sim800l.write(0x1A); //ascii code for ctrl-26 //sim800.println((char)26); //ascii code for ctrl-26
  delay(1000);
}

//void Send_Data()
//{
//Serial.println("Sending Data..."); //Displays on the serial monitor...Optional
//sim800l.print("AT+CMGF=1\r"); //Set the module to SMS mode
//delay(100);
//sim800l.print("AT+CMGS=\"+639859205850\"\r"); //Your phone number don't forget to include your country code example +212xxxxxxxxx"
//delay(500);
//sim800l.print("No GPS Dtected"); //This string is sent as SMS
//delay(500);
//sim800l.print((char)26);//Required to tell the module that it can send the SMS
//delay(500);
//sim800l.println();
//Serial.println("Data Sent.");
//delay(500);
//
//}

void timeoutAvail()
{
  //return;
//  Serial.println("jfjfjf");
  //return;
  sim800l.print("AT+CMGF=1\r");
  delay(100);
  sim800l.print("AT+CMGS=\""+Phone+"\"\r");
  delay(100);
  sim800l.println("Geofence available");
  delay(100);
  sim800l.write(0x1A); //ascii code for ctrl-26 //sim800.println((char)26); //ascii code for ctrl-26
  delay(100);
  Serial.println("SMS Sent Successfully.");
}
