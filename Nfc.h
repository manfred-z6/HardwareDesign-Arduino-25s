#ifndef __NFC_H
#define __NFC_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_PN532.h>

extern Adafruit_PN532 nfc; 

// NFC状态机状态定义
enum NFCState {
  NFC_STATE_IDLE,        // 空闲状态，等待卡片出现
  NFC_STATE_CARD_PRESENT, // 卡片在场，进行判断
  NFC_STATE_WAIT_REMOVAL  // 等待当前卡片移除
};

// 声明全局变量
extern uint8_t lastUid[];
extern uint8_t lastUidLength;
extern unsigned long lastReadTime;
extern const unsigned long DEBOUNCE_TIME;

extern const uint8_t targetUid[];
extern const uint8_t targetUidLength;

extern NFCState nfcState;
extern unsigned long stateEntryTime;
extern uint8_t currentUid[];
extern uint8_t currentUidLength;
extern bool cardAlreadyProcessed;

// 声明函数
void NFC_Init();
void updatenfc();
bool compareUid(uint8_t* uid1, uint8_t* uid2, uint8_t length);
void onTargetCardDetected(); // 检测到目标卡时执行的函数

#endif