#ifndef __NFC_H
#define __NFC_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_PN532.h>

extern Adafruit_PN532 nfc; // 使用默认I2C引脚，参数无意义

// 用于存储上次检测到的UID和长度
extern uint8_t lastUid[7];
extern uint8_t lastUidLength;
extern unsigned long lastReadTime;
extern const unsigned long DEBOUNCE_TIME; // 2秒防抖时间

// 目标UID: 0xEC 0x45 0x70 0x6 (注意: 最后一个字节可能需要调整)
extern const uint8_t targetUid[];
extern const uint8_t targetUidLength;

// --- 非阻塞控制变量 ---
extern unsigned long previousMillis;     // 存储上次读取NFC的时间
extern const long nfcInterval;         // 读取NFC的时间间隔（毫秒），替代 delay(100)
extern bool waitingForCardRemoval;   // 标志位，用于防止卡片未离开时的重复读取
extern const unsigned long removalDelay; // 检测到卡后等待卡片移开的时间

void updatenfc();
bool compareUid(uint8_t* uid1, uint8_t* uid2, uint8_t length);
void f();

#endif //__NFC_H