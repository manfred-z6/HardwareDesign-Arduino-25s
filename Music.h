#ifndef MUSIC_H
#define MUSIC_H

#include <Arduino.h>
#include <DFRobotDFPlayerMini.h>

class MusicPlayer {
private:
  DFRobotDFPlayerMini myDFPlayer;
  uint8_t busyPin;
  bool isPlaying;
  unsigned long lastDebounceTime;
  const unsigned long debounceDelay = 50;
  
public:
  MusicPlayer(uint8_t busyPin = 2);
  
  bool begin(HardwareSerial& serial = Serial1);
  bool checkPlayingStatus();
  void playTrackOnce(uint16_t trackNumber);
  void setVolume(uint8_t volume);
  void stop();
  bool isCurrentlyPlaying() const { return isPlaying; }
};

extern MusicPlayer musicPlayer;

#endif  //MUSIC_H