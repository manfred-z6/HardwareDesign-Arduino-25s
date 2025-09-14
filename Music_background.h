#ifndef __MUSIC_BACKGROUND_H
#define __MUSIC_BACKGROUND_H

#include <Arduino.h>
#include <DFRobotDFPlayerMini.h>

class MusicPlayer2 {
private:
  DFRobotDFPlayerMini myDFPlayer;
  uint8_t busyPin;
  bool isPlaying;
  unsigned long lastDebounceTime;
  const unsigned long debounceDelay = 50;
  uint16_t currentLoopTrack = 0;
  
public:
  MusicPlayer2(uint8_t busyPin = 3);
  
  bool begin(HardwareSerial& serial = Serial2);
  bool checkPlayingStatus();
  void playTrackOnce(uint16_t trackNumber);
  void playTrackLoop(uint16_t trackNumber);
  void setVolume(uint8_t volume);
  void stop();
  bool isCurrentlyPlaying() const { return isPlaying; }
};

extern MusicPlayer2 musicPlayer2;

#endif  //MUSIC_GROUND_H