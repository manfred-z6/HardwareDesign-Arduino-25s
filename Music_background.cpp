#include "Music_background.h"
#include <DFRobotDFPlayerMini.h>

MusicPlayer2 musicPlayer2(3); // 默认使用引脚3作为BUSY引脚

MusicPlayer2::MusicPlayer2(uint8_t busyPin) 
  : busyPin(busyPin), isPlaying(false), lastDebounceTime(0),  currentLoopTrack(0) {
}

bool MusicPlayer2::begin(HardwareSerial& serial) {
  pinMode(busyPin, INPUT_PULLUP);
  
  Serial.println(F("初始化DFPlayer Mini 2..."));
  
  if (!myDFPlayer.begin(serial)) {
    Serial.println(F("无法初始化DFPlayer Mini 2:"));
    Serial.println(F("1. 请检查电源和接线!"));
    Serial.println(F("2. 请确认SD卡已插入!"));
    return false;
  }
  
  Serial.println(F("DFPlayer Mini 2 就绪!"));
  myDFPlayer.volume(25);
  myDFPlayer.EQ(DFPLAYER_EQ_NORMAL);
  
  return true;
}

bool MusicPlayer2::checkPlayingStatus() {
  int reading = digitalRead(busyPin);
  
  if (reading != isPlaying) {
    lastDebounceTime = millis();
  }
  
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != isPlaying) {
      isPlaying = (reading == LOW);
      if (currentLoopTrack != 0 && !isPlaying) {
        myDFPlayer.play(currentLoopTrack);
      }
    }
  }
  
  return isPlaying;
}

void MusicPlayer2::playTrackOnce(uint16_t trackNumber) {
  Serial.print(F("播放曲目: "));
  Serial.println(trackNumber);
  myDFPlayer.play(trackNumber);
}

void MusicPlayer2::playTrackLoop(uint16_t trackNumber) {
  Serial.print(F("Loop track: "));
  Serial.println(trackNumber);
  currentLoopTrack = trackNumber; // 设置要循环的曲目
  myDFPlayer.enableLoop(); // 启用模块的单曲循环
  myDFPlayer.play(trackNumber); // 开始播放
}



void MusicPlayer2::setVolume(uint8_t volume) {
  myDFPlayer.volume(constrain(volume, 0, 30));
}

void MusicPlayer2::stop() {
  Serial.println(F("Stop playback."));
  myDFPlayer.stop();
  isPlaying = false;
  currentLoopTrack = 0;
}