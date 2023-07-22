#include "Arduino.h"
#include "WiFi.h"
#include "Audio.h"
#define uart_en 15
#define RXp2 16
#define TXp2 17
#define I2S_DOUT      25
#define I2S_BCLK      27
#define I2S_LRC       26

Audio audio;


void setup()
{

  Serial.begin(115200);
    Serial2.begin(115200, SERIAL_8N1, RXp2,TXp2);

  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.begin( "SSID", "PASSWORD");

  while (WiFi.status() != WL_CONNECTED)
    delay(1500);

  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(100);
  audio.connecttospeech("Starting ", "en"); // Google TTS
}


void loop()

{
if (Serial2.available()){
  String Answer = Serial2.readString();
  Serial.println(Answer);
 audio.connecttospeech(Answer.c_str(), "en");
  }
  audio.loop();

}

void audio_info(const char *info) {
  Serial.print("audio_info: "); Serial.println(info);}
