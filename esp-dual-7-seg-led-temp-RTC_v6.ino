#define MAX2719_DIN  27   //Pins connected to the MAX chip 
#define MAX2719_CS  25
#define MAX2719_CLK 26
#define DHTTYPE DHT11   // DHT 11
#define DHTPIN 16     // Digital pin connected to the DHT sensor

#include "DHT.h"    //temp and humidity sensor library
#include <WiFi.h>   //wifi library
#include "time.h"   //time functions library

DHT dht(DHTPIN, DHTTYPE);
// network settings
const char* ssid       = "YOUR_SSID_HERE";
const char* password   = "YOUR_PASSWORD_HERE";

//time service settings
const char* ntpServer = "pool.ntp.org"; //server address
const long  gmtOffset_sec = 7200; //timezone setting. GMT+1 = 3600 GMT+2 = 7200 etc.
const int   daylightOffset_sec = 3600; //in effect when Daylight Saving Time is in effect
struct tm timeinfo;

// variables
int dayTens, dayOnes, monthTens, monthOnes, hourTens, hourOnes, minuteTens, minuteOnes; //date and time variables
int tempTens, tempOnes, tempDeciTens, tempDeciOnes, humiTens, humiOnes, humiDeciTens, humiDeciOnes; //temp and humidity variables
int modePinTemp = 18; //pin for temperature switch
int modePinHumi = 19; //pin for Humidity switch
int resetButtonPin = 12; //pin for checking if Reset button is pressed
int ledPinTemp = 17; //LEDs to indicate mode
int ledPinHumi = 20;
int ledPinDate = 21;
long loopCounter = 0; //counter to reset the clock every five days
boolean boolTempPin, boolHumiPin, boolResetButtonPin; //boolean for reading the mode switch and the reset button
byte ledLeft1, ledLeft2, ledLeft3, ledLeft4;     // these are the left segmented LED digits
byte ledRight1, ledRight2, ledRight3, ledRight4; // these are the right segmented LED digits

void printLocalTime() { //update time

  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
}

void setup() {
  dht.begin();          //start the temp-humi sensor
  Serial.begin(115200); //start serial to device
  pinMode(modePinTemp, INPUT_PULLUP);
  pinMode(modePinHumi, INPUT_PULLUP);
  pinMode(resetButtonPin, INPUT_PULLUP);
  pinMode(ledPinHumi, OUTPUT);
  pinMode(ledPinTemp, OUTPUT); 
  pinMode(ledPinDate, OUTPUT);
  
  initialise();         // start LED displays 
  
  goGetTime();          // update system time from Network Time
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

boolean checkModePinHumi() {  //check if the mode switch is in HUMIDITY setting
  if (digitalRead(modePinHumi) == HIGH) {
    Serial.println("Humidity mode");
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


void goGetTime() {

  //connect to WiFi 
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" CONNECTED");

  //init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
  //disconnect WiFi as it's no longer needed
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
}


void loop() {
  boolTempPin = checkModePinTemp(); //These check the position of the mode switch
  boolHumiPin = checkModePinHumi(); //if neither is HIGH, the user wants to see date
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
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F("°C "));
  Serial.println(f);
  
  dayTens = timeinfo.tm_mday / 10; //tens digit of day
  dayOnes = timeinfo.tm_mday % 10; //ones digit of day
  monthTens = timeinfo.tm_mon / 10;  //tens digit of month
  monthOnes = timeinfo.tm_mon % 10 + 1;  //ones digit of month, +1 needed to adjust
  hourTens = timeinfo.tm_hour / 10; //tens digit of hour
  hourOnes = timeinfo.tm_hour % 10; //ones digit of hour
  minuteTens = timeinfo.tm_min / 10; //tens digit of minutes
  minuteOnes = timeinfo.tm_min % 10; //ones digit of minutes
  humiTens = humiStr.substring(0, 1).toInt(); //this returns the tens of the humidity as String
  humiOnes = humiStr.substring(1, 2).toInt(); //this returns the ones of the humidity as String
  humiDeciTens = humiStr.substring(2, 3).toInt(); //this returns the decimal tens of the humidity as String
  humiDeciOnes = humiStr.substring(3, 4).toInt(); //this returns the decimal ones of the humidity as String
  tempTens = tempStr.substring(0, 1).toInt(); //this returns the tens of the temperature as String
  tempOnes = tempStr.substring(1, 2).toInt(); //this returns the ones of the temperature as String
  tempDeciOnes = tempStr.substring(2, 3).toInt(); //this returns the decimal tens of the temperature as String
  tempDeciTens = tempStr.substring(3, 4).toInt(); //this returns the decimal ones of the temperature as String 

  // for showing temperature
  if ((boolTempPin == true) && (boolHumiPin == false)) {
  //if (loopCounter<10) {
    ledLeft1 = tempTens;
    ledLeft2 = tempOnes + 128;
    ledLeft3 = tempDeciTens;
    ledLeft4 = tempDeciOnes ;
    digitalWrite(ledPinTemp, HIGH);
    digitalWrite(ledPinHumi, LOW);
    digitalWrite(ledPinDate, LOW);    
  }
  // for showing humidity
  else if ((boolTempPin == false) && (boolHumiPin == true)) {
  //else if (loopCounter < 20){  
    ledLeft1 = humiTens;
    ledLeft2 = humiOnes + 128;
    ledLeft3 = humiDeciTens;
    ledLeft4 = humiDeciOnes;
    digitalWrite(ledPinTemp, LOW);
    digitalWrite(ledPinHumi, HIGH);
    digitalWrite(ledPinDate, LOW);    
  }
  //if both are off, show date
  else {
    ledLeft1 = dayTens;
    ledLeft2 = dayOnes + 128;
    ledLeft3 = monthTens;
    ledLeft4 = monthOnes + 128;
    digitalWrite(ledPinTemp, LOW);
    digitalWrite(ledPinHumi, LOW);
    digitalWrite(ledPinDate, HIGH);    
  }
  //set the right LED values for time
  ledRight1 = hourTens;
  ledRight2 = hourOnes + 128;
  ledRight3 = minuteTens;
  ledRight4 = minuteOnes;

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
  if (loopCounter >432000) { //five days of operation after which time is again read from the NTP
    loopCounter=0;
    goGetTime();
  }
  printLocalTime();
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
