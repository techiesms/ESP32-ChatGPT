#include "CloudSpeechClient.h"
#include "network_param.h"
#include <base64.h>
#include <ArduinoJson.h>
#define USE_SERIAL Serial
#include <Arduino.h>
#include <HTTPClient.h>
//#define uart_en 15
 #define led_3 4
 #define led_1 15
#define led_2 2
//#include <SoftwareSerial.h>
////SoftwareSerial (D4, D2);
const char* chatgpt_token = "Your_ChatGPT_Token";
CloudSpeechClient::CloudSpeechClient(Authentication authentication) {
  this->authentication = authentication;
  WiFi.begin(ssid, password);
 //  while (WiFi.status() == WL_CONNECTED){ digitalWrite(led_3,1);}
  while (WiFi.status() != WL_CONNECTED) delay(1000);
  client.setCACert(root_ca);
 
 
  if (!client.connect(server, 443)) Serial.println("Connection failed!"); digitalWrite(led_3,1);digitalWrite(led_1,0);digitalWrite(led_2,0);
}

String ans;

CloudSpeechClient::~CloudSpeechClient() {
  client.stop();
  WiFi.disconnect();
}

void CloudSpeechClient::PrintHttpBody2(Audio* audio)
{
  String enc = base64::encode(audio->paddedHeader, sizeof(audio->paddedHeader));
  enc.replace("\n", "");  // delete last "\n"
  client.print(enc);      // HttpBody2
  char** wavData = audio->wavData;
  for (int j = 0; j < audio->wavDataSize / audio->dividedWavDataSize; ++j) {
    enc = base64::encode((byte*)wavData[j], audio->dividedWavDataSize);
    enc.replace("\n", "");// delete last "\n"
    client.print(enc);    // HttpBody2
  }
}

void CloudSpeechClient::Transcribe(Audio* audio) {
  String HttpBody1 = "{\"config\":{\"encoding\":\"LINEAR16\",\"sampleRateHertz\":16000,\"languageCode\":\"en-IN\"},\"audio\":{\"content\":\"";
  String HttpBody3 = "\"}}\r\n\r\n";
  int httpBody2Length = (audio->wavDataSize + sizeof(audio->paddedHeader)) * 4 / 3; // 4/3 is from base64 encoding
  String ContentLength = String(HttpBody1.length() + httpBody2Length + HttpBody3.length());
  String HttpHeader;
  // if (authentication == USE_APIKEY)
  HttpHeader = String("POST /v1/speech:recognize?key=") + ApiKey
               + String(" HTTP/1.1\r\nHost: speech.googleapis.com\r\nContent-Type: application/json\r\nContent-Length: ") + ContentLength + String("\r\n\r\n");
  //  else if (authentication == USE_ACCESSTOKEN)
  //    HttpHeader = String("POST /v1beta1/speech:syncrecognize HTTP/1.1\r\nHost: speech.googleapis.com\r\nContent-Type: application/json\r\nAuthorization: Bearer ")
  //   + AccessToken + String("\r\nContent-Length: ") + ContentLength + String("\r\n\r\n");
  client.print(HttpHeader);
  client.print(HttpBody1);
  PrintHttpBody2(audio);
  client.print(HttpBody3);
  String My_Answer="";
  while (!client.available());
   
  while (client.available())
  {
    char temp = client.read();
    My_Answer = My_Answer + temp;
   // Serial.write(client.read());
  }

 // Serial.print("My Answer - ");Serial.println(My_Answer);
  int postion = My_Answer.indexOf('{');
 // Serial.println(postion);
  ans = My_Answer.substring(postion);
  Serial.print("Json daata--");
  //Serial.print(ans);
  
 DynamicJsonDocument doc(384);

//StaticJsonDocument<384> doc;

DeserializationError error = deserializeJson(doc, ans);

if (error) {
  Serial.print("deserializeJson() failed: ");
  Serial.println(error.c_str());
  return;
}

JsonObject results_0 = doc["results"][0];
//const char* 
const char* chatgpt_Q = results_0["alternatives"][0]["transcript"];
const char* a= "light on";
const char* b= "light off";
//String chatgpt_Q = a+ans+b;
//Serial.println(ans);
Serial.print(chatgpt_Q);Serial.println("-");
///////////////////////////////////////////////////////////
pinMode(LED_BUILTIN,OUTPUT);
if(strstr(chatgpt_Q, "light on")){
  digitalWrite(LED_BUILTIN,HIGH);
  Serial.println("Light's On");
  digitalWrite(15, LOW);
delay(1);
Serial2.println("Turning Light on");
digitalWrite(led_1,1);
digitalWrite(led_3,0);
digitalWrite(led_2,1);
//digitalWrite(uart_en,HIGH);
  Serial.print("To ask again");
  }
if(strstr(chatgpt_Q, "light off")){
  digitalWrite(LED_BUILTIN,LOW);
  Serial.println("Light's Off");
  digitalWrite(15, LOW);
delay(1);
Serial2.println("Turning Light off");
digitalWrite(led_1,1);
digitalWrite(led_2,1);
//digitalWrite(uart_en,HIGH);
  Serial.print("To ask again");
  }  
 if(strstr(chatgpt_Q, "blink on")){
  HTTPClient http;

        USE_SERIAL.print("[HTTP] begin...\n");
        // configure traged server and url
        //http.begin("https://www.howsmyssl.com/a/check", ca); //HTTPS
        http.begin("http://example.com/index.html"); //HTTP

        USE_SERIAL.print("[HTTP] GET...\n");
        // start connection and send HTTP header
        int httpCode = http.GET();

        // httpCode will be negative on error
        if(httpCode > 0) {
            // HTTP header has been send and Server response header has been handled
            USE_SERIAL.printf("[HTTP] GET... code: %d\n", httpCode);

            // file found at server
            if(httpCode == HTTP_CODE_OK) {
                String payload = http.getString();
                USE_SERIAL.println(payload);
            }
        } else {
            USE_SERIAL.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }

        http.end();
  } 
 else if(strstr(chatgpt_Q, "light on")==0 && strstr(chatgpt_Q, "light off") == 0 && strstr(chatgpt_Q, "blink on") ==0 ){
  Serial.println("Asking Chat GPT");
  HTTPClient https;

    Serial.print("[HTTPS] begin...\n");
    if (https.begin("https://api.openai.com/v1/completions")) {  // HTTPS
      
      https.addHeader("Content-Type", "application/json"); 
      String token_key = String("Bearer ") + chatgpt_token;
      https.addHeader("Authorization", token_key);
      
      String payload = String("{\"model\": \"text-davinci-003\", \"prompt\": ") +"\""+ chatgpt_Q +"\"" + String(", \"temperature\": 0.2, \"max_tokens\": 40}"); //Instead of TEXT as Payload, can be JSON as Paylaod
      
      Serial.print("[HTTPS] GET...\n");
      
      // start connection and send HTTP header
      int httpCode = https.POST(payload);

      // httpCode will be negative on error      
      // file found at server
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        String payload = https.getString();
        Serial.println(payload);
       // Serial2.println(payload);
        //////////////////////////////////////////////////
        StaticJsonDocument<2000> doc2;

DeserializationError error = deserializeJson(doc2, payload);

if (error) {
  Serial.print("deserializeJson() failed: ");
  Serial.println(error.c_str());
  return;

}
JsonObject choices_0 = doc2["choices"][0];
const char* only_ans = choices_0["text"];
Serial.println("Only ans:-");Serial.print(only_ans);
Serial2.print(only_ans);
digitalWrite(led_1,1);
digitalWrite(led_2,1);
//digitalWrite(uart_en, LOW);
delay(1);

//digitalWrite(uart_en,HIGH);
        /////////////////////////////////////////////////////////
      }
      else {
        Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
      }
      https.end();
    }
    else {
      Serial.printf("[HTTPS] Unable to connect\n");
    }

  Serial.print("To ask again");
  //delay(10000);
  
  } 




///////////////////////////////////////////////////////////
/*

   
  */
}
