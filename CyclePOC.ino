
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <Arduino_JSON.h>
#include <Keypad.h>

#define LED D8

const char* ssid = "wifi_name";
const char* password = "wifi_password";

//Your Domain name with URL path or IP address with path
const char* serverName = "http://192.168.0.107:5000/otp_get/101";   // cycle api link end point (change the host to your private ip address (ipconfig on cmd))
const char* serverName2 = "http://192.168.0.107:5000/information/101";  // information end point


 
void setup() {
  // Setup serial monitor
  // esp8266 connects to the wifi
  // gets otp from the user through numpad
  // gets generated otp from the server
  // compares the otp
  // unlocks the bike (D8 pin set to high)
  
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
  bool correctOTPEntered = false;

  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
 
  Serial.println("Timer set to 5 seconds (timerDelay variable), it will take 5 seconds before publishing the first reading.");

  int otp_keypad = otpGetKeypad();  // functions end only when 4 digits are entered on the keypad
  
  JSONVar object = otpGetServer();  // sends https request to the server to get the otp
  int otp_server = int(object["otp"]); // extracting otp from the response file from the server

  if(otp_keypad == otp_server){
    Serial.println();
    Serial.println("Cycle Unlocked!");
    digitalWrite(LED, HIGH);
  }
  else{
    Serial.println();
    Serial.println("OTP you entered is invalid, please try again.");  // esp needs to be restarted to enter the otp again. no trial and exception used.
  }
  
  
}
 
void loop() {
  // Get key value if pressed
  JSONVar information = httpGETRequest(serverName2);
  JSONVar infoObject = JSON.parse(information);

  if(int(infoObject["endTrip"]) == 1){
    digitalWrite(LED, LOW);
  }
  delay(1000);
}

int convertToInt(char* a, int size)  // from string to int
{
    int i;
    String s = "";
    for (i = 0; i < size; i++) {
        s = s + a[i];
    }
    return s.toInt();
}

int otpGetKeypad(){
    const byte ROWS = 4;
    const byte COLS = 4;
     
    // Array to represent keys on keypad
    char hexaKeys[ROWS][COLS] = {
      {'1', '2', '3', 'A'},
      {'4', '5', '6', 'B'},
      {'7', '8', '9', 'C'},
      {'*', '0', '#', 'D'}
    };
    
    char keysPressed[4];
    int count = 0;
     
    // Connections to Arduino
    byte rowPins[ROWS] = {D7, D6, D5, D4};
    byte colPins[COLS] = {D3, D2, D1, D0};
     
    // Create keypad object
    Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);
    
    while(count <= 3){
        char keyPress = customKeypad.getKey();
        if (keyPress){
        if (keyPress == '#'){
        break;
        }
        Serial.print(keyPress);
        keysPressed[count] = keyPress;
        count++;
        }
        delay(0);
      }
      
      count = 0;
      int size_ = sizeof(keysPressed) / sizeof(char);
      int otp = convertToInt(keysPressed, size_);
      
      return otp;
}

JSONVar otpGetServer(){           // GET request to the server. Used a function just to make the code more readable.
  JSONVar otpJson = httpGETRequest(serverName);
  JSONVar myObject = JSON.parse(otpJson);


  return myObject;
}

void endTrip(JSONVar myObject){  // unnecessary function, will remove later
  if(bool(myObject["endTrip"]) == true){
    digitalWrite(LED, LOW);
  }
}


String httpGETRequest(const char* serverName) {  // ye code chapa hai
  WiFiClient client;
  HTTPClient http;
    
  http.begin(client, serverName);
  
  int httpResponseCode = http.GET();
  
  String payload = "{}"; 
  
  if (httpResponseCode>0) {
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }

  http.end();

  return payload;
}
