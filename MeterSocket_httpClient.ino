#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>

ESP8266WiFiMulti wifiMulti;
HTTPClient http;

void setup() {
  Serial.begin(115200);
  wifiMulti.addAP("#teamChitoge", "truenibba");
  wifiMulti.addAP("anon", "12345678");
  wifiMulti.addAP("Ganda & Cute Boarding House", "llaneta583");
  wifiMulti.addAP("Meter Socket", "FabLab2.0");

}

void loop() {
  if(wifiMulti.run() == WL_CONNECTED){
    String payload = "";
    char btnState[6];
    byte btnBool[5];
    http.begin("http://tiano.codes/metersocket/api/esp.php");
    int httpCode = http.GET();
    if(httpCode == HTTP_CODE_OK){
      payload = http.getString();
      payload.toCharArray(btnState, 6);
      btnBool[0] = (int)btnState[0]-48;
      btnBool[1] = (int)btnState[1]-48;
      btnBool[2] = (int)btnState[2]-48;
      btnBool[3] = (int)btnState[3]-48;
      btnBool[4] = (int)btnState[4]-48;
      Serial.write(btnBool, sizeof(btnBool));
      if(String(payload.charAt(4))=="1"){
        http.end();
        http.begin("http://tiano.codes/metersocket/api/update.php?id=5&status=0");
        httpCode = http.GET();
        http.end();
      }
    }
    http.end();
    if(Serial.available()){
      unsigned int curPos = 0;
      char postData[1000];
//      String postData = "";
      while(Serial.available() && curPos < 1000){
        char temp = (char)Serial.read();
        postData[curPos++] = temp;
      }
      postData[curPos] = '\0';
      http.begin("http://tiano.codes/metersocket/api/update.php?"+String(postData));
      httpCode = http.GET();
      http.end();
    }
  }
  delay(200);
}
