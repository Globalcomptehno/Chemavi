#define CAYENNE_PRINT Serial  // Comment this out to disable prints and save space
#include "utilities.h"
#include <axp20x.h> 
#define TINY_GSM_MODEM_SIM800
#define SerialMon Serial
#define SerialAT Serial1
#define TINY_GSM_DEBUG SerialMon
#define TINY_GSM_USE_GPRS true
#define TINY_GSM_USE_WIFI false
#include <CayenneMQTTGSM.h>
#include <TinyGsmClient.h>
#include <NewPing.h>
#define SONAR_NUM 2                                     
#define TRIGGER_PIN_1 13
#define ECHO_PIN_1 13
#define TRIGGER_PIN_2 12
#define ECHO_PIN_2 12
#define MAX_DISTANCE 200
float count = 0, dist1 = 0, dist2 = 0, tank1 = 0, tank2 = 0, level1, level2, litr1, litr2;
float r1 = 65, r2 = 79, depth1 = 76, depth2 = 70;
float s1 = PI*sq(r1);
float s2 = PI*sq(r2);
int switchPin = 34;
int ledPinb = 0;
int ledPing = 2;
int ledPinr = 15;
int er = 0;
NewPing sonar1(TRIGGER_PIN_1, ECHO_PIN_1, MAX_DISTANCE);
NewPing sonar2(TRIGGER_PIN_2, ECHO_PIN_2, MAX_DISTANCE);

char apn[] = "internet.orange.md"; // Access point name. Leave empty if it is not needed.
char gprsLogin[] = ""; // GPRS username. Leave empty if it is not needed.
char gprsPassword[] = ""; // GPRS password. Leave empty if it is not needed.
char pin[] = ""; // SIM pin number. Leave empty if it is not needed.
char username[] = "a3debf00-2bc1-11ed-baf6-35fab7fd0ac8";
char password[] = "92efff80b98ce08716d9ec67693f78505f5eb9ab";
char clientID[] = "851eb800-2c42-11ed-baf6-35fab7fd0ac8";   // TricouFantasy

void setup() {
	Serial.begin(115200);
	setupModem();
  pinMode(ledPinr, OUTPUT);
  pinMode(ledPing, OUTPUT);
  pinMode(ledPinb, OUTPUT);
  pinMode(switchPin, INPUT);
	SerialAT.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);
	Cayenne.begin(username, password, clientID, SerialAT, apn, gprsLogin, gprsPassword, pin);
}
void loop() {
	Cayenne.loop();
  delay(100);                     
  unsigned int dist1 = sonar1.ping_median(5);
  dist1 = sonar1.convert_cm(dist1);
  level1 = depth1 - dist1;
  litr1 = (s1 * level1) / 1000;
  tank1 = (level1 / depth1) * 100;  
  delay(100); 
  unsigned int dist2 = sonar2.ping_median(5);
  dist2 = sonar2.convert_cm(dist2);
  level2 = depth2 - dist2;
  litr2 = (s2 * level2) / 1000;
  tank2 = (level2 / depth2) * 100;
  if (digitalRead(switchPin) == HIGH) {count = count + 10;}
}

CAYENNE_OUT_DEFAULT() {
	Cayenne.virtualWrite(0, millis()/1000/60/60);
	float vbus_v = axp.getVbusVoltage();
	Cayenne.virtualWrite(1, vbus_v,"Utility");
  float vbus_c = axp.getVbusCurrent();
  Cayenne.virtualWrite(2, vbus_c, "Curent IN");
  float batt_v = axp.getBattVoltage();
  Cayenne.virtualWrite(3, batt_v, "Battery Power");
  float batt_c = axp.getBattChargeCurrent();
  Cayenne.virtualWrite(4, batt_c, "Battery + Curent");
  float batt_d = axp.getBattDischargeCurrent();
  Cayenne.virtualWrite(5, batt_d, "Battery - Curent");
  float temp = axp.getTemp();
  Cayenne.virtualWrite(6, temp, "CPU Temp");
  Cayenne.virtualWrite(13, tank1);
  Cayenne.virtualWrite(12, tank2);
  Cayenne.virtualWrite(14, count);
  if (vbus_v > 4) {er = 0;}
  if (vbus_v < 3) {er = 2;}
  if (tank1 < 33 or tank2 < 33) {er = 1;} else {er = 0;}
  if (er == 0) {digitalWrite(ledPing, HIGH), digitalWrite(ledPinb, LOW), digitalWrite(ledPinr, LOW);}
  if (er == 1) {digitalWrite(ledPinr, HIGH), digitalWrite(ledPinb, LOW), digitalWrite(ledPing, LOW);}
  if (er == 2) {digitalWrite(ledPinb, HIGH), digitalWrite(ledPinr, LOW), digitalWrite(ledPing, LOW);}
  Cayenne.virtualWrite(20, er, "Error");
}
CAYENNE_IN(9)
{
  CAYENNE_LOG("Channel %u, value %s", request.channel, getValue.asString());
  er = 0, count = 0;
}
