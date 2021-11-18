// WIRING SCHEME:
//
// 5V from charger goes to POWER SWITCH
// from switch power goes to INTERRUPT SWITCH
// from interrupt switch goes to 5V splitter
// split to 5V PIN
// split to DISPLAY VCC
// split to DHT11 VCC
// modePinTemp = 18 pin for Humidity switch
// modePinDate = 19 pin for Humidity switch
// resetButtonPin = 12 pin for checking if Reset button is pressed
// ledPinTemp = 17 LEDs to indicate mode
// ledPinHumi = 22;
// ledPinDate = 21;

#include <DHT.h>
#include <DHT_U.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <HTTPClient.h> //http client
#include <ezTime.h>     //excellent time library

#define MAX2719_DIN  27 //Pins connected to the dual LED display MAX chip 
#define MAX2719_CS  25
#define MAX2719_CLK 26
#define DHTTYPE DHT11   // DHT 11
#define DHTPIN 16       // Digital pin connected to the DHT sensor

DHT dht(DHTPIN, DHTTYPE);

// network settings
const char* ssid       = "moaiwlan";
const char* password   = "Ossi1Paavo234";
const char* serverName = "http://www.sabulo.com/humitime/ht.php";

//time service settings
Timezone myTZ;          //variable for maintaining timezone information

//server update loop control variables
unsigned long lastTime = 0;
// Set timer to 5 seconds (5000)
unsigned long timerDelay = 5000;

// variables
String myMonth, myDay, myHour, myMinute, mySecond; //date and time variables
int tempTens, tempOnes, tempDeciTens, tempDeciOnes, humiTens, humiOnes, humiDeciTens, humiDeciOnes; //temp and humidity variables
int modePinTemp = 18; //pin for temperature switch
int modePinDate = 19; //pin for Humidity switch
int resetButtonPin = 12; //pin for checking if Reset button is pressed
int ledPinTemp = 17; //LEDs to indicate mode
int ledPinHumi = 21;
int ledPinDate = 22;
long loopCounter = 0; //counter to send update to web every five minutes and reset the clock every five days
boolean boolTempPin, boolDatePin, boolResetButtonPin; //boolean for reading the mode switch and the reset button
byte ledLeft1, ledLeft2, ledLeft3, ledLeft4;     // these are the left segmented LED digits
byte ledRight1, ledRight2, ledRight3, ledRight4; // these are the right segmented LED digits
String strMyHour, strMyMinute, strMyDay, strMyMonth;
String humiStr = "";
String tempStr = "";
enum {TIME, DATE}; //required by the eztime library

void setup() {
  Serial.begin(115200); //start serial to device
  WiFi.begin(ssid, password);                     //wlan connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  waitForSync();                                          //get time from NTP server pool
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.println("UTC: " + UTC.dateTime());

  myTZ.setLocation("Europe/Helsinki");                    //timezone established
  Serial.println("Finland time: " + myTZ.dateTime());
  setDebug(INFO);                                         //print eztime debug data
  

  dht.begin();          //start the temp-humi sensor

  pinMode(modePinTemp, INPUT_PULLUP);
  pinMode(modePinDate, INPUT_PULLUP);
  pinMode(resetButtonPin, INPUT_PULLUP);
  pinMode(ledPinHumi, OUTPUT);
  pinMode(ledPinTemp, OUTPUT);
  pinMode(ledPinDate, OUTPUT);

  initialise();         // start LED displays

}

boolean checkModePinTemp() { //check if the mode switch is in TEMP setting
  if (digitalRead(modePinTemp) == HIGH) {
    Serial.println("Temp mode");
    return true;
  }
  else {
    return false;
  }
}

boolean checkmodePinDate() {  //check if the mode switch is in HUMIDITY setting
  if (digitalRead(modePinDate) == HIGH) {
    Serial.println("Date mode");
    return true;
  }
  else {
    return false;
  }
}

boolean checkResetButton() {  //check if the RESET button is pushed
  if (digitalRead(resetButtonPin) == HIGH) {
    Serial.println("Reset request");
    return true;
  }
  else {
    return false;
  }
}

void loop() {
  boolTempPin = checkModePinTemp(); //These check the position of the mode switch
  boolDatePin = checkmodePinDate(); //if neither is HIGH, the user wants to see date
  boolResetButtonPin = checkResetButton();
  float h = dht.readHumidity();   //Read humidity
  String humiStr = String(h);
  float t = dht.readTemperature();// Read temperature as Celsius (the default)
  String tempStr = String(t);
  float f = dht.readTemperature(true);// Read temperature as Fahrenheit (isFahrenheit = true)
  //error handling routine
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    delay(1000);
    return;
  }
  // these are for testing purposes, delete if not needed
  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.println(F("%  Temperature: "));
  Serial.print(t);
  Serial.println(F("°C "));

  humiTens = humiStr.substring(0, 1).toInt();     //this returns the tens of the humidity as String
  humiOnes = humiStr.substring(1, 2).toInt();     //this returns the ones of the humidity as String
  humiDeciTens = humiStr.substring(2, 3).toInt(); //this returns the decimal tens of the humidity as String
  humiDeciOnes = humiStr.substring(3, 4).toInt(); //this returns the decimal ones of the humidity as String
  tempTens = tempStr.substring(0, 1).toInt();     //this returns the tens of the temperature as String
  tempOnes = tempStr.substring(1, 2).toInt();     //this returns the ones of the temperature as String
  tempDeciOnes = tempStr.substring(2, 3).toInt(); //this returns the decimal tens of the temperature as String
  tempDeciTens = tempStr.substring(3, 4).toInt(); //this returns the decimal ones of the temperature as String

  //set the right LED values for time
  myHour = myTZ.dateTime("G");             //time hours with leading zero
  myMinute = myTZ.dateTime("i");           //time minutes with leading zero
  mySecond = myTZ.dateTime("s");           //time seconds just because I can
  ledRight1 = myHour.substring(0, 1).toInt();    //first digit of hours as byte
  ledRight2 = myHour.substring(1, 2).toInt()+128;    //second digit of hours as byte
  ledRight3 = myMinute.substring(0, 1).toInt();  //first digit of minutes as byte
  ledRight4 = myMinute.substring(1, 2).toInt();  //second digit of minutes as byte
  

  // for showing temperature
  if ((boolTempPin == true) && (boolDatePin == false)) {
    //if (loopCounter<10) {
    ledLeft1 = tempTens;                         //first digit of temp as byte
    ledLeft2 = tempOnes + 128;                   //second digit of temp as byte, 128 added to get decimal point
    ledLeft3 = tempDeciTens;                     //first digit of temp decimals as byte
    ledLeft4 = tempDeciOnes ;                    //second digit of temp decimals as byte
    digitalWrite(ledPinTemp, HIGH);              //light appropriate LED
    digitalWrite(ledPinHumi, LOW);
    digitalWrite(ledPinDate, LOW);
  }
  // for showing humidity
  else if ((boolTempPin == false) && (boolDatePin == true)) {
    //else if (loopCounter < 20){
    ledLeft1 = humiTens;                          //first digit of humidity as byte
    ledLeft2 = humiOnes + 128;                    //second digit of humidity as byte, 128 added to get decimal point
    ledLeft3 = humiDeciTens;                      //first digit of humidity decimals as byte
    ledLeft4 = humiDeciOnes;                      //second digit of humidity decimals as byte
    digitalWrite(ledPinTemp, LOW);                //light appropriate LED
    digitalWrite(ledPinHumi, LOW);
    digitalWrite(ledPinDate, HIGH);
  }
  //if both are off, show date
  else {
    myMonth = myTZ.dateTime("m");
    myDay = myTZ.dateTime("d");
    ledLeft1 = myDay.substring(0, 1).toInt();     //tens of days as byte
    ledLeft2 = myDay.substring(1, 2).toInt() + 128; //ones of days as byte, 128 added to get decimal point
    ledLeft3 = myMonth.substring(0, 1).toInt();   //tens of month as byte
    ledLeft4 = myMonth.substring(1, 2).toInt();   //ones of month as byte
    Serial.println(myMinute + " " + myHour  + " " +  myMonth + " " + myDay);
    digitalWrite(ledPinTemp, LOW);                //light appropriate LED
    digitalWrite(ledPinHumi, HIGH);
    digitalWrite(ledPinDate, LOW);
  }


  // send all data to LED units
  output(0x08, ledLeft1); // Left led 1 value
  output(0x07, ledLeft2); // Left led 2 value
  output(0x06, ledLeft3); // Left led 3 value
  output(0x05, ledLeft4); // Left led 4 value
  output(0x04, ledRight1); // Time value in hours, tens
  output(0x03, ledRight2); // Time value in hours, ones
  output(0x02, ledRight3); // Time value in minutes, tens
  output(0x01, ledRight4); // Time value in minutes, ones

  delay(1000);
  loopCounter++;
  Serial.println(loopCounter);
  if (loopCounter > 432000) { //five days of operation after which time is again read from the NTP
    loopCounter = 0;
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }

    events();
  }
  
  if (loopCounter % 300 == 0) {         //This is just to follow the time update cycle
    UpdateHumiTemp(tempStr, humiStr, loopCounter);
  }

}

void UpdateHumiTemp(String myT, String myH, int myC) {
  //sending data to the server:
  if ((millis() - lastTime) > timerDelay) {
    Serial.println("add data to web");
    Serial.printf("Connecting to %s ", ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }

    Serial.println(" CONNECTED");

    if (WiFi.status() == WL_CONNECTED) {
      WiFiClient client;
      HTTPClient http;

      // Your Domain name with URL path or IP address with path
      http.begin(serverName);

      // If you need an HTTP request with a content type: text/plain
      http.addHeader("Content-Type", "text/plain");
      String httpRequestData = "Temperature: " + myT + " Humidity: " + myH + " Counter: " + myC;
      // Send HTTP POST request
      int httpResponseCode = http.POST(httpRequestData);

      Serial.println("HTTP Response code: ");
      Serial.println(httpRequestData);

      // Free resources
      http.end();
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    //disconnect WiFi as it's no longer needed
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);

    lastTime = millis();
  }
}

void initialise()
{
  digitalWrite(MAX2719_CS, HIGH);
  pinMode(MAX2719_DIN, OUTPUT);
  pinMode(MAX2719_CS, OUTPUT);
  pinMode(MAX2719_CLK, OUTPUT);
  // For test mode (all digits on) set to 0x01. Normally we want this off (0x00)
  output(0x0f, 0x0);
  // Set all digits off initially
  output(0x0c, 0x0);
  // Set brightness for the digits to high(er) level than default minimum (Intensity Register Format)
  output(0x0a, 0x02);
  // Set decode mode for ALL digits to output actual ASCII chars rather than just
  // individual segments of a digit
  output(0x09, 0xFF);
  // Set first digit (right most) to '8'
  output(0x01, 0x08);
  // Set next digits to 7 6 5 4 3 2 1(Code B Font)
  output(0x02, 0x07);
  output(0x03, 0x06);
  output(0x04, 0x05);
  output(0x05, 0x04);
  output(0x06, 0x03);
  output(0x07, 0x02);
  output(0x08, 0x01);
  // Ensure ALL digits are displayed (Scan Limit Register)
  output(0x0b, 0x07);
  // Turn display ON (boot up = shutdown display)
  output(0x0c, 0x01);
}

void output(byte address, byte data)
{
  digitalWrite(MAX2719_CS, LOW);

  // Send out two bytes (16 bit)
  // parameters: shiftOut(dataPin, clockPin, bitOrder, value)
  shiftOut(MAX2719_DIN, MAX2719_CLK, MSBFIRST, address);
  shiftOut(MAX2719_DIN, MAX2719_CLK, MSBFIRST, data);
  digitalWrite(MAX2719_CS, HIGH);
}
