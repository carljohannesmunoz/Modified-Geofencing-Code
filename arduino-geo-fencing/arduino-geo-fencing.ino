
#include <SoftwareSerial.h>

#include <AltSoftSerial.h>
#include <TinyGPS++.h>

const String PHONE = "+639636973459";
//--------------------------------------------------------------
//GSM Module RX pin to Arduino 3
//GSM Module TX pin to Arduino 2
#define rxPin 3
#define txPin 2
SoftwareSerial sim800(rxPin,txPin);
//--------------------------------------------------------------
//GPS Module RX pin to Arduino 9
//GPS Module TX pin to Arduino 8
AltSoftSerial neogps;
TinyGPSPlus gps;
//--------------------------------------------------------------

// Alarm
//int buzzer_timer = 0;/
//bool alarm = false;/
boolean send_alert_once = true;
int state = 0;
const int pin = 4;
//--------------------------------------------------------------
// Size of the geo fence (in meters)
const float maxDistance = 30;

//--------------------------------------------------------------
float initialLatitude = 0;
float initialLongitude = 0;

float latitude, longitude;
//--------------------------------------------------------------


void getGps(float& latitude, float& longitude);


/*****************************************************************************************
 * setup() function
 *****************************************************************************************/
void setup()
{
  //--------------------------------------------------------------
  //Serial.println("Arduino serial initialize");
  Serial.begin(9600);
  //--------------------------------------------------------------
  //Serial.println("SIM800L serial initialize");
  sim800.begin(115200);
  //--------------------------------------------------------------
  //Serial.println("NEO6M serial initialize");
  neogps.begin(9600);
  //--------------------------------------------------------------
//  pinMode(BUZZER, OUTPUT);/
  //--------------------------------------------------------------
  sim800.println("AT"); //Check GSM Module
  delay(1000);
  sim800.println("ATE1"); //Echo ON
  delay(1000);
  sim800.println("AT+CPIN?"); //Check SIM ready
  delay(1000);
  sim800.println("AT+CMGF=1"); //SMS text mode
  delay(1000);
  sim800.println("AT+CNMI=1,1,0,0,0"); /// Decides how newly arrived SMS should be handled
  delay(1000);
  //AT +CNMI = 2,1,0,0,0 - AT +CNMI = 2,2,0,0,0 (both are same)
  //--------------------------------------------------------------
  delay(20000);
//  buzzer_timer = millis();/
}





/*****************************************************************************************
 * loop() function
 *****************************************************************************************/
void loop()
{
  //--------------------------------------------------------------
  getGps(latitude, longitude);
  //--------------------------------------------------------------
  float distance = getDistance(latitude, longitude, initialLatitude, initialLongitude);
  //--------------------------------------------------------------
  Serial.print("Latitude= "); Serial.println(latitude, 6);
  Serial.print("Lngitude= "); Serial.println(longitude, 6);
  Serial.print("initialLatitude= "); Serial.println(initialLatitude, 6);
  Serial.print("initialLngitude= "); Serial.println(initialLongitude, 6);
  Serial.print("current Distance= "); Serial.println(distance);
  //--------------------------------------------------------------
  // Set alarm on?
  if(distance > maxDistance) {
    //------------------------------------------
    if(send_alert_once == true){
//      digitalWrite(BUZZER, HIGH);
      sendAlert();
//      alarm = true;/
      send_alert_once = false;
//      buzzer_timer = millis();/
    }
    //------------------------------------------
  }
  else{
    send_alert_once = true;
  }
  //--------------------------------------------------------------

 if (digitalRead(pin) == HIGH && state == 0) {
  
//      Serial.println("jfjfjf");
//      sgsm.listen();
//      sgsm.print("\r");
//      delay(1000);
//      sgsm.print("AT+CMGF=1\r");
//      delay(1000);
      /*Replace XXXXXXXXXX to 10 digit mobile number &
        ZZ to 2 digit country code*/
//      sgsm.print("AT+CMGS=\"+639603970762\"\r");
//      delay(1000);
//      //The text of the message to be sent.
//      sgsm.println("Alert!!!.\r");
//      sgsm.print("www.openstreetmap.org/?mlat=" + String(gpslat) + "&mlon=" + String(gpslon) + "#map=18/");
////      /
//      delay(1000);
//      sgsm.write(0x1A);
  sendAlert();
  state = 1;
    }
  if (digitalRead(pin) == LOW) {
      state = 0;
    }
  // Handle alarm
//  if (alarm == true) {
//    if (millis() - buzzer_timer > 5000) {
//      digitalWrite(BUZZER, LOW);
//      alarm = false;
//      buzzer_timer = 0;
//    }
//  }
  //--------------------------------------------------------------  
  while(sim800.available()){
    Serial.println(sim800.readString());
  }
  //--------------------------------------------------------------
  while(Serial.available())  {
    sim800.println(Serial.readString());
  }
  //--------------------------------------------------------------


}



/*****************************************************************************************
* getDistance() function
*****************************************************************************************/

// Calculate distance between two points
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

/*****************************************************************************************
 * parseData() function
 *****************************************************************************************/
void parseData(String buff){
  Serial.println(buff);

  unsigned int len, index;
  //--------------------------------------------------------------
  //Remove sent "AT Command" from the response string.
  index = buff.indexOf("\r");
  buff.remove(0, index+2);
  buff.trim();
  //NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
  if(buff != "OK"){
    //--------------------------------------------------------------
    index = buff.indexOf(":");
    String cmd = buff.substring(0, index);
    cmd.trim();
    
    buff.remove(0, index+2);
    //Serial.println(buff);
    //--------------------------------------------------------------
    if(cmd == "+CMTI"){
      //get newly arrived memory location and store it in temp
      //temp = 4
      index = buff.indexOf(",");
      String temp = buff.substring(index+1, buff.length()); 
      temp = "AT+CMGR=" + temp + "\r"; 
      //AT+CMGR=4 i.e. get message stored at memory location 4
      sim800.println(temp); 
    }
    //--------------------------------------------------------------
    else if(cmd == "+CMGR"){
      //extractSms(buff);
      //Serial.println(buff.indexOf(EMERGENCY_PHONE));
      if(buff.indexOf(PHONE) > 1){
        buff.toLowerCase();
        //Serial.println(buff.indexOf("get gps"));
        if(buff.indexOf("get gps") > 1){
          getGps(latitude, longitude);
          String sms_data;
          sms_data = "GPS Location Data\r";
//          sms_data += "www.openstreetmap.org/?mlat=";
          sms_data += String(latitude) + "&mlon=" + String(longitude);
          Serial.println("jfjfjf");
          sendSms(sms_data);
        }
      }
    }
    //--------------------------------------------------------------
  }
  else{
  //The result of AT Command is "OK"
  }
  //NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
}



/*****************************************************************************************
 * getGps() Function
*****************************************************************************************/
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
    Serial.println("No GPS data is available");
    latitude = 0;
    longitude = 0;
  }
}


/*****************************************************************************************
 * sendSms() function
 *****************************************************************************************/
 void sendSms(String text)
{
  //return;
  sim800.print("AT+CMGF=1\r");
  delay(1000);
  sim800.print("AT+CMGS=\""+PHONE+"\"\r");
  delay(1000);
  sim800.print(text);
  delay(100);
  sim800.write(0x1A); //ascii code for ctrl-26 //sim800.println((char)26); //ascii code for ctrl-26
  delay(1000);
  Serial.println("SMS Sent Successfully.");
}



/*****************************************************************************************
* sendAlert() function
*****************************************************************************************/
void sendAlert()
{
  //return;
//  String sms_data;
//  sms_data = "Alert! The object is outside the fense.\r";
//  sms_data += "http://maps.google.com/maps?q=loc:";
//  sms_data += String(latitude) + "," + String(longitude);
  Serial.println("jfjfjf");
  //return;
  sim800.print("AT+CMGF=1\r");
  delay(1000);
  sim800.print("AT+CMGS=\""+PHONE+"\"\r");
  delay(1000);
  sim800.println("Alert!!!.\r");
//  sim800.print("www.openstreetmap.org/?mlat=" + String(latitude) + "&mlon=" + String(longitude));
  delay(100);
  sim800.write(0x1A); //ascii code for ctrl-26 //sim800.println((char)26); //ascii code for ctrl-26
  delay(1000);
  Serial.println("SMS Sent Successfully.");
  
}

/*****************************************************************************************
 * SendAT() function
 *****************************************************************************************/
boolean SendAT(String at_command, String expected_answer, unsigned int timeout){

    uint8_t x=0;
    boolean answer=0;
    String response;
    unsigned long previous;
    
    //Clean the input buffer
    while( sim800.available() > 0) sim800.read();

    sim800.println(at_command);
    
    x = 0;
    previous = millis();

    //this loop waits for the answer with time out
    do{
        //if there are data in the UART input buffer, reads it and checks for the asnwer
        if(sim800.available() != 0){
            response += sim800.read();
            x++;
            // check if the desired answer (OK) is in the response of the module
            if(response.indexOf(expected_answer) > 0){
                answer = 1;
                break;
            }
        }
    }while((answer == 0) && ((millis() - previous) < timeout));

  Serial.println(response);
  return answer;
}
