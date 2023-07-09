
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>

#include <ArduinoJson.h>
#include "Audio.h"

const char* ssid = "SmS_jiofi";
const char* password = "sms123458956";
const char* chatgpt_token = "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";
const char* temperature = "0";
const char* max_tokens = "45";
String Question = "";

#define I2S_DOUT      25
#define I2S_BCLK      27
#define I2S_LRC       26

Audio audio;

void setup()
{
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();


  while (!Serial);

  // wait for WiFi connection
  WiFi.begin(ssid, password);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(100);
}

void loop()
{

  Serial.print("Ask your Question : ");
  while (!Serial.available())
  {
    audio.loop();
  }
  while (Serial.available())
  {
    char add = Serial.read();
    Question = Question + add;
    delay(1);
  }
  int len = Question.length();
  Question = Question.substring(0, (len - 1));
  Question = "\"" + Question + "\"";
  Serial.println(Question);

  HTTPClient https;

  //Serial.print("[HTTPS] begin...\n");
  if (https.begin("https://api.openai.com/v1/completions")) {  // HTTPS

    https.addHeader("Content-Type", "application/json");
    String token_key = String("Bearer ") + chatgpt_token;
    https.addHeader("Authorization", token_key);

    String payload = String("{\"model\": \"text-davinci-003\", \"prompt\": ") + Question + String(", \"temperature\": ") + temperature + String(", \"max_tokens\": ") + max_tokens + String("}"); //Instead of TEXT as Payload, can be JSON as Paylaod

    //Serial.print("[HTTPS] GET...\n");

    // start connection and send HTTP header
    int httpCode = https.POST(payload);

    // httpCode will be negative on error
    // file found at server
    if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
      String payload = https.getString();
      //Serial.println(payload);

      DynamicJsonDocument doc(1024);


      deserializeJson(doc, payload);
      String Answer = doc["choices"][0]["text"];
      Answer = Answer.substring(2);
      Serial.print("Answer : "); Serial.println(Answer);
      audio.connecttospeech(Answer.c_str(), "en");

    }
    else {
      Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
    }
    https.end();
  }
  else {
    Serial.printf("[HTTPS] Unable to connect\n");
  }

  Question = "";
}

void audio_info(const char *info) {
  Serial.print("audio_info: "); Serial.println(info);
}
