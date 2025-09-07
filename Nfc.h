#ifndef __NFC_H
#define __NFC_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_PN532.h>

extern Adafruit_PN532 nfc;

// NFC状态机状态定义
enum NFCState {
  NFC_STATE_IDLE,        // 空闲状态，等待卡片出现
  NFC_STATE_DETECTING,   // 检测到卡片，等待稳定
  NFC_STATE_PROCESSING,  // 处理卡片，执行对应函数
  NFC_STATE_WAIT_REMOVAL // 等待卡片移除
};

// 定义UID-函数映射结构
struct UidFunctionMap {
  uint8_t uid[7];        // 卡片的UID
  uint8_t uidLength;     // UID长度
  void (*action)();      // 该UID对应的函数指针
};

// 声明全局变量
extern const unsigned long REQUIRED_DETECTION_TIME;

extern NFCState nfcState;
extern unsigned long stateEntryTime;
extern uint8_t currentUid[];
extern uint8_t currentUidLength;
extern bool cardProcessed;

// 声明函数
void NFC_Init();
void updatenfc();
bool compareUid(uint8_t* uid1, uint8_t* uid2, uint8_t length);
void registerUidFunctions(); // 注册所有UID和对应的函数

// 声明各个UID对应的函数（你可以根据需要修改函数名和实现）
void onUid1Detected();
void onUid2Detected();
void onUid3Detected();
void onUid4Detected();
void onUid5Detected();
void onUid6Detected();
void onUid7Detected();
void onUid8Detected();
void onUid9Detected();
void onUid10Detected();
void onUid11Detected();
void onUid12Detected();
void onUid13Detected();
void onUid14Detected();
void onUid15Detected();
void onUid16Detected();
void onUid17Detected();
void onUid18Detected();
void onUid19Detected();
void onUid20Detected();
void onUid21Detected();
void onUnknownUidDetected();

#endif