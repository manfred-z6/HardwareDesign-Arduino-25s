#ifndef __NFC_H
#define __NFC_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_PN532.h>

extern Adafruit_PN532 nfc; 

// 用于存储上次检测到的UID和长度
extern uint8_t lastUid[7];
extern uint8_t lastUidLength;
extern unsigned long lastReadTime;
extern const unsigned long DEBOUNCE_TIME; // 防抖时间

// 目标UID
extern const uint8_t targetUid[];
extern const uint8_t targetUidLength;

// --- 非阻塞控制变量 ---
extern unsigned long previousMillis;     // 存储上次读取NFC的时间
extern const long nfcInterval;         // 读取NFC的时间间隔（毫秒），替代 delay(100)
extern bool waitingForCardRemoval;   // 标志位，用于防止卡片未离开时的重复读取

void updatenfc();
bool compareUid(uint8_t* uid1, uint8_t* uid2, uint8_t length);
void f();

#endif //__NFC_H