#include <PZEM004Tv30.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <LiquidCrystal_I2C.h>

#define btnReset D3
#define btnToggle D4

struct BTN{
  int main;
  int s1;
  int s2;
  int s3;
  int rst;
};
struct BTN state = {1,1,1,1,1};
struct BTN relay = {D0,D5,D6,D7,7};

String payload = "";
char btnState[6], str[30];
bool lcdBool = true;
float Voltage=0.0, Current=0.0, Frequency = 0.0, Power=0.0, Energy = 0.0, PF = 0.0;
int httpCode;

PZEM004Tv30 pzem(&Serial);
LiquidCrystal_I2C lcd(0x27, 20, 4);
ESP8266WiFiMulti wifiMulti;
HTTPClient http;

void setup() {
  lcd.begin();
  lcd.backlight();
  lcd.noCursor();
  lcd.clear();
  lcd.setCursor(0,1);
  lcd.print("Booting up");
  wifiMulti.addAP("#teamChitoge", "truenibba");
  wifiMulti.addAP("anon", "12345678");
  wifiMulti.addAP("Ganda & Cute Boarding House", "llaneta583");
  wifiMulti.addAP("Meter Socket", "FabLab2.0");
  pinMode(D5, OUTPUT);
  pinMode(D6, OUTPUT);
  pinMode(D7, OUTPUT);
  pinMode(D0, OUTPUT);
  pinMode(D3, INPUT);
  pinMode(D4, INPUT);
  delay(2000);
}

void loop() {
  static unsigned long lastMillis = 0;
  unsigned long currentMillis = millis();
  if (currentMillis - lastMillis > 150) {
    lastMillis = currentMillis;
    if(wifiMulti.run() == WL_CONNECTED){
      http.begin("http://tiano.codes/metersocket/api/esp.php");
      int httpCode = http.GET();
      if(httpCode == HTTP_CODE_OK){
        payload = http.getString();
        payload.toCharArray(btnState, 6);
        state.main = (int)btnState[0]-48;
        state.s1 = (int)btnState[1]-48;
        state.s2 = (int)btnState[2]-48;
        state.s3 = (int)btnState[3]-48;
        state.rst = (int)btnState[4]-48;
        setRelay();
        if(state.rst){
          http.end();
          http.begin("http://tiano.codes/metersocket/api/update.php?id=5&status=0");
          httpCode = http.GET();
          pzem.resetEnergy();
        }
        sendRawData();
      }
      http.end();
      updateValues();
      lcdPrint();
    }else{
      lcd.setCursor(0,0);
      lcd.print("Connecting to wifi");
    }
  }
  scanButtons();
}

void setRelay(){
  digitalWrite(relay.main, !state.main);
  digitalWrite(relay.s1, !state.s1);
  digitalWrite(relay.s2, !state.s2);
  digitalWrite(relay.s3, !state.s3);
}

void sendRawData() {
  static unsigned long lastMillis = 0;
  unsigned long currentMillis = millis();
  if (currentMillis - lastMillis > 1000) {
    lastMillis = currentMillis;
    String postStr = "voltage="+String(Voltage,1)+"&frequency="+String(Frequency,2)+"&amperage="+String(Current,3)+"&energy="+String(Energy,3)+"&pFactor="+String(PF,2);
    http.begin("http://tiano.codes/metersocket/api/update.php?"+postStr);
    httpCode = http.GET();
    http.end();
  }
}

void updateValues() {
  static unsigned long lastMillis = 0;
  unsigned long currentMillis = millis();
  if (currentMillis - lastMillis > 1000) {
    lastMillis = currentMillis;
    Voltage = pzem.voltage();
    if (!isnan(Voltage))
    {
      Current = pzem.current();
      Energy  = pzem.energy();
      Power = pzem.power();
      PF = pzem.pf();
      Frequency = pzem.frequency();
    } else {
      Voltage = 0.0;
      Current = 0.0;
      Power = 0.0;
      PF = 0.0;
      Frequency = 0.0;
    }
  }
}

void lcdPrint(){
  char valBuffer[15];
  char valBuffer2[15];
  lcd.setCursor(0,0);
  dtostrf(Voltage, 5, 1, valBuffer);
  dtostrf(PF, 5, 2, valBuffer2);
  sprintf(str, "Volt: %s PF:%s", valBuffer, valBuffer2);
  lcd.print(str);
  lcd.setCursor(0,1);
  dtostrf(Frequency, 5, 2, valBuffer);
  sprintf(str, "Freq: %s", valBuffer);
  
  lcd.print(str);
  lcd.setCursor(0,2);
  dtostrf(Current, 4, 3, valBuffer);
  sprintf(str, "Amp:  %s    ", valBuffer);
  lcd.print(str);
  lcd.setCursor(0,3);
  dtostrf(Energy, 4, 3, valBuffer);
  sprintf(str, "kWh:  %s    ",valBuffer);
  lcd.print(str);
}

void btn_toggleMain_handler()
{
 static unsigned long last_interrupt_time = 0;
 unsigned long interrupt_time = millis();
 
 if (interrupt_time - last_interrupt_time > 500)
 {
   lcdBool ? lcd.noBacklight() : lcd.backlight();
   lcdBool = !lcdBool;
 }
 last_interrupt_time = interrupt_time;
}

void btn_reset_handler()
{
 static unsigned long last_interrupt_time = 0;
 unsigned long interrupt_time = millis();
 if (interrupt_time - last_interrupt_time > 2000)
 {
   pzem.resetEnergy();
 }
 last_interrupt_time = interrupt_time;
}

void scanButtons() {
  static bool lastStateReset = 0;
  static bool lastStateToggle = 0;

  bool currentStateReset = digitalRead(btnReset);
  bool currentStateToggle = digitalRead(btnToggle);
  if (lastStateReset != currentStateReset)
  {
    if (currentStateReset == 0) {
      btn_reset_handler();
    }
    lastStateReset = currentStateReset;
  }
  if (lastStateToggle != currentStateToggle)
  {
    if (currentStateToggle == 0) {
      btn_toggleMain_handler();
    }
    lastStateToggle = currentStateToggle;
  }
}
