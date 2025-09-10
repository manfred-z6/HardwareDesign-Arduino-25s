#include "Music.h"
#include <DFRobotDFPlayerMini.h>

MusicPlayer musicPlayer(2); // 默认使用引脚2作为BUSY引脚

MusicPlayer::MusicPlayer(uint8_t busyPin) 
  : busyPin(busyPin), isPlaying(false), lastDebounceTime(0) {
}

bool MusicPlayer::begin(HardwareSerial& serial) {
  pinMode(busyPin, INPUT_PULLUP);
  
  Serial.println(F("初始化DFPlayer Mini..."));
  
  if (!myDFPlayer.begin(serial)) {
    Serial.println(F("无法初始化DFPlayer Mini:"));
    Serial.println(F("1. 请检查电源和接线!"));
    Serial.println(F("2. 请确认SD卡已插入!"));
    return false;
  }
  
  Serial.println(F("DFPlayer Mini 就绪!"));
  myDFPlayer.volume(30);
  myDFPlayer.EQ(DFPLAYER_EQ_NORMAL);
  
  return true;
}

bool MusicPlayer::checkPlayingStatus() {
  int reading = digitalRead(busyPin);
  
  if (reading != isPlaying) {
    lastDebounceTime = millis();
  }
  
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != isPlaying) {
      isPlaying = (reading == LOW);
    }
  }
  
  return isPlaying;
}

void MusicPlayer::playTrackOnce(uint16_t trackNumber) {
  Serial.print(F("播放曲目: "));
  Serial.println(trackNumber);
  myDFPlayer.play(trackNumber);
}

void MusicPlayer::setVolume(uint8_t volume) {
  myDFPlayer.volume(constrain(volume, 0, 30));
}

void MusicPlayer::stop() {
  myDFPlayer.stop();
  isPlaying = false;
}