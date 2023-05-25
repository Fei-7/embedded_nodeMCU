#include <Arduino.h>
#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>

//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "123"
#define WIFI_PASSWORD "1234qwer"

// Insert Firebase project API Key
#define API_KEY "AIzaSyBwXE6K4i3FXlvYoHPlxHJuiKOBQ8rjnzg"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "esp-firebase-22c67-default-rtdb.asia-southeast1.firebasedatabase.app/" 

//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

// unsigned long sendDataPrevMillis = 0;
// int intValue;
// float floatValue;
// int count = 0;
bool signupOK = false;

char c[32];
uint8_t a[4];
// 0:d1, 1:d2, 2:rotation, 3:humid
// d1:dry, d2:wet
int idx = 0;
int dry, wet;
const int maxD = 25;
//uint8_t Humidity;
//uint8_t Temperature;

void setup(){
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  while (1) {
    Serial.println("wet1");
    if (Firebase.ready() && signupOK) {
      Serial.println("wet2");
      if (Firebase.RTDB.getInt(&fbdo, "/data/Wet")) {
        Serial.println("wet3");
          if (fbdo.dataType() == "int") {
            Serial.println("wet4");
            wet = fbdo.intData();
            break;
          }
      }
    }
    delay(1000);
  }

  while (1) {
    if (Firebase.ready() && signupOK) {
      if (Firebase.RTDB.getInt(&fbdo, "/data/Dry")) {
          if (fbdo.dataType() == "int") {
            dry = fbdo.intData();
            break;
          }
      }
    }
    delay(100);
  }
}

void loop(){
  // Read data from STM32 UART
  //pingping
  //Serial.println("Yes\n");
  if (Serial.available()) {
    Serial.println("Yes ");
    c[idx++] = Serial.read();
    Serial.println(c[idx-1]);
    if (c[idx-1] == ':') {
      while (!Serial.available()) {}
      c[idx++] = Serial.read();
      a[c[idx-3] - '0'] = c[idx-1];
      Serial.println(a[c[idx-3] - '0']);
      if (c[idx-3] == '2') {
        if (c[idx-1] == 'W') {
          wet++;
          while (!(Firebase.ready() && signupOK)) {}
          if (Firebase.RTDB.setInt(&fbdo, "data/Wet", wet)) {
            Serial.println("Dry count sent to Firebase: " + wet);
          } else {
            Serial.println("Failed to send Wet to Firebase");
            Serial.println("Reason: " + fbdo.errorReason());
          }
        }
        if (c[idx-1] == 'D') {
          dry++;
          while (!(Firebase.ready() && signupOK)) {}
          if (Firebase.RTDB.setInt(&fbdo, "data/Dry", dry)) {
          Serial.println("Dry count sent to Firebase: " + dry);
        } else {
          Serial.println("Failed to send Dry to Firebase");
          Serial.println("Reason: " + fbdo.errorReason());
        }
        }
      }
      // Serial.println("get " + String(c[idx-3]) + " : " + String(a[c[idx-3] - '0']));
      // if (a[0] >= maxD) {
      //   dry = 0;
      //   while (!(Firebase.ready() && signupOK)) {}
      //   if (Firebase.RTDB.setInt(&fbdo, "data/Dry", dry)) {
      //     Serial.println("Dry count sent to Firebase: " + dry);
      //   } else {
      //     Serial.println("Failed to send Dry to Firebase");
      //     Serial.println("Reason: " + fbdo.errorReason());
      //   }
      //   // idx=0;
      //   // delay(50);
      //   // continue;
      // }
      // if (a[1] >= maxD) {
      //   wet = 0;
      //   while (!(Firebase.ready() && signupOK)) {}
      //   if (Firebase.RTDB.setInt(&fbdo, "data/Wet", wet)) {
      //     Serial.println("Dry count sent to Firebase: " + wet);
      //   } else {
      //     Serial.println("Failed to send Wet to Firebase");
      //     Serial.println("Reason: " + fbdo.errorReason());
      //   }
      //   // idx=0;
      //   // delay(50);
      //   // continue;
      // }
      if (maxD <= a[0]) a[0] = maxD;
      if (maxD <= a[1]) a[1] = maxD;
      // Send data to Firebase
      if (c[idx-3] == '0') {
        while (!(Firebase.ready() && signupOK)) {}
        String d1String = String(a[0]);
        if (Firebase.RTDB.setString(&fbdo, "data/Distance1", d1String)) {
          Serial.println("Distance1 sent to Firebase: " + d1String);
        } else {
          Serial.println("Failed to send Distance1 to Firebase");
          Serial.println("Reason: " + fbdo.errorReason());
        }
      }
      if (c[idx-3] == '1') {
        while (!(Firebase.ready() && signupOK)) {}
        String d2String = String(a[1]);
        if (Firebase.RTDB.setString(&fbdo, "data/Distance2", d2String)) {
          Serial.println("Distance2 sent to Firebase: " + d2String);
        } else {
          Serial.println("Failed to send Distance2 to Firebase");
          Serial.println("Reason: " + fbdo.errorReason());
        }
      }
      idx=0;
    }
  }


  delay(50);

}