/* ----------------------------------------------------------

      ARDUINO SENSOR AND 433MHZ SWITCH TRANSMITTER SKETCH
                   VERION 0.2 - 6-7 OCT. 2013
                  AUTHOR MICHIEL VAN DER VELDE

-------------------------------------------------------------*/

// ------------------------
//  INCLUDES
// ------------------------
#include "DHT.h"
#include <RCSwitch.h>

#include <IRremote.h>

// ------------------------
//  DEFINES
// ------------------------
#define DHTPIN 2
#define DHTTYPE DHT22

#define ENVCHECKINTERVALMS 60000
int DEVID = 1001;

int RECV_PIN = 11;

// ------------------------
//  OBJECT CREATION
// ------------------------
DHT dht(DHTPIN, DHTTYPE);
RCSwitch mySwitch = RCSwitch();

IRrecv irrecv(RECV_PIN);
decode_results results;
byte previous_ir_code;

// ------------------------
//  GLOBAL VARS - SENSORS
// ------------------------
boolean envCheckStartup = true;
long lastEnvCheckMs = 0;
boolean soundHeard = false;

float lastTemperature;
float lastHumidity;

// ------------------------
//  GLOBAL VARS - SWITCHH
// ------------------------
boolean newCmd = false;
int groupID = 0;
int switchID = 0;
int switchCmd = 0;

// ------------------------
//  GLOBAL VARS - LDR
// ------------------------
int LDR_Pin = A0;
int lastLDRValue = 0;

// ------------------------
//  MAIN ARDUINO METHODS
// ------------------------

void setup() {
  Serial.begin(9600);

  // Setup sensors
  attachInterrupt(1, flipSoundHeard, RISING);
  dht.begin();

  // Setup switch
  mySwitch.enableTransmit(10);
  //Serial.println("DEVID 1001");

  // Setup IR
  irrecv.enableIRIn();

  // ---------------------------------
}

void loop() {
  if(envCheckStartup || (lastEnvCheckMs + ENVCHECKINTERVALMS) <= millis()) {
    if(envCheckStartup)
      envCheckStartup = false;
    float newTemperature = dht.readTemperature();
    float newHumidity = dht.readHumidity();
    int newLDRValue = analogRead(LDR_Pin);

    if(!isnan(newTemperature) && lastTemperature != newTemperature) {
      Serial.print("TMP ");
      Serial.print(newTemperature);
      Serial.print(" ");
      lastTemperature = newTemperature;
		}

    if(!isnan(newHumidity) && lastHumidity != newHumidity) {
      Serial.print("HUM ");
      Serial.print(newHumidity);
      Serial.print(" ");
      lastHumidity = newHumidity;
    }

    Serial.print("LDR ");
    Serial.println(newLDRValue);

    lastEnvCheckMs = millis();
  }

  if(soundHeard) {
    soundHeard = false;
    Serial.println("SND 1");
  }

  // IR
  if (irrecv.decode(&results)) {
    byte send_ir = results.value;
    if(send_ir == 255)
      send_ir = previous_ir_code;
    else
      previous_ir_code = results.value;
    Serial.print("IR ");
    Serial.println(send_ir, HEX);
    irrecv.resume();
  }

  // Run switch
  processSerialForSwitch();
  if(newCmd) {
    switch(switchCmd) {
      case 0:
        mySwitch.switchOff(groupID, switchID);
      break;
      case 1:
        mySwitch.switchOn(groupID, switchID);
      break;
    }
    newCmd = false;
  }
}

// ------------------------
//  HELPER METHODS - SWITCH
// ------------------------

void processSerialForSwitch() {
  if(Serial.available() == 5) {
    Serial.read(); // discard first byte (S)
    Serial.read(); // discard second byte (space)
    groupID = Serial.read() - 48;
    switchID = Serial.read() - 48;
    switchCmd = Serial.read() - 48;
    newCmd = true;
  }
}

// ------------------------
//  INTERRUPT HANDLER
// ------------------------

void flipSoundHeard() {
  soundHeard = true;
}
