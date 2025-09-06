#include "Nfc.h"

uint8_t lastUid[7] = {0};
uint8_t lastUidLength = 0;
unsigned long lastReadTime = 0;
const unsigned long DEBOUNCE_TIME = 2000;

const uint8_t targetUid[] = {0x4,0x91,0xF3,0x87,0x2B,0x2,0x89};
const uint8_t targetUidLength = 7;

unsigned long previousMillis = 0;
const long nfcInterval = 1000;    //nfc卡片读取间隔
bool waitingForCardRemoval = false;   //等待卡片离开标志

// 新增变量：跟踪当前卡片的UID和状态
uint8_t currentUid[7] = {0};
uint8_t currentUidLength = 0;
bool cardCurrentlyPresent = false;
bool cardAlreadyProcessed = false;    //卡片对应函数成功执行标志

void updatenfc() {
  unsigned long currentMillis = millis(); // 获取当前时间

  if (currentMillis - previousMillis >= nfcInterval && !waitingForCardRemoval) {
    previousMillis = currentMillis;
    
    uint8_t success;
    uint8_t uid[7] = {0};
    uint8_t uidLength;
    
    // 尝试读取NFC标签
    success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
    
    if (success) {
      // 检查是否是同一张卡
      bool isSameCard = (uidLength == currentUidLength);
      if (isSameCard) {
        for (uint8_t i = 0; i < uidLength; i++) {
          if (uid[i] != currentUid[i]) {
            isSameCard = false;
            break;
          }
        }
      }
      
      if (!isSameCard) {
        // 新卡片检测到
        memcpy(currentUid, uid, uidLength);
        currentUidLength = uidLength;
        cardCurrentlyPresent = true;
        cardAlreadyProcessed = false;
        
        // 打印UID信息
        Serial.print("Found a new card. UID: ");
        for (uint8_t i = 0; i < uidLength; i++) {
          Serial.print(" 0x"); Serial.print(uid[i], HEX);
        }
        Serial.println();
        
        // 检查是否是目标UID
        if (uidLength == targetUidLength && compareUid(uid, (uint8_t*)targetUid, targetUidLength)) {
          // 检查防抖时间是否已过
          if (currentMillis - lastReadTime > DEBOUNCE_TIME && !cardAlreadyProcessed) {
            // 存储当前UID信息和时间
            memcpy(lastUid, uid, uidLength);
            lastUidLength = uidLength;
            lastReadTime = currentMillis;
            
            // 执行函数
            f();
            cardAlreadyProcessed = true;
            
            // 设置标志位，进入等待卡片移开的状态
            waitingForCardRemoval = true;
            Serial.println("Waiting for card removal...");
          } else if (cardAlreadyProcessed) {
            Serial.println("Card already processed, waiting for removal.");
          } else {
            Serial.println("Debounce time not elapsed, ignoring.");
          }
        } else {
          Serial.println("Not the target card.");
          waitingForCardRemoval = true;
        }
      } else {
        // 同一张卡仍然在场，但已经处理过，不再重复处理
        if (!cardAlreadyProcessed) {
          // 如果是同一张卡但尚未处理（可能是之前未能识别）
          Serial.println("Same card still present, checking if it's target...");
          
          // 检查是否是目标UID
          if (uidLength == targetUidLength && compareUid(uid, (uint8_t*)targetUid, targetUidLength)) {
            if (currentMillis - lastReadTime > DEBOUNCE_TIME && !cardAlreadyProcessed) {
              memcpy(lastUid, uid, uidLength);
              lastUidLength = uidLength;
              lastReadTime = currentMillis;
              
              f();
              cardAlreadyProcessed = true;
              waitingForCardRemoval = true;
              Serial.println("Waiting for card removal...");
            }
          } else {
            Serial.println("Same non-target card still present.");
            waitingForCardRemoval = true;
          }
        } else {
          Serial.println("Same card still present, already processed.");
        }
      }
    } else {
      // 没有检测到卡片
      if (cardCurrentlyPresent) {
        // 卡片已移除
        memset(currentUid, 0, sizeof(currentUid));
        currentUidLength = 0;
        cardCurrentlyPresent = false;
        cardAlreadyProcessed = false;
        waitingForCardRemoval = false;
        Serial.println("Card removed. Ready for next scan.");
      }
    }
  }
  
  // 处理等待卡片移开的状态
  if (waitingForCardRemoval) {
    uint8_t success;
    uint8_t uid[7] = {0};
    uint8_t uidLength;
    
    // 检查卡片是否已移开
    success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 10);
    
    if (!success) {
      // 卡片已移除
      waitingForCardRemoval = false;
      memset(currentUid, 0, sizeof(currentUid));
      currentUidLength = 0;
      cardCurrentlyPresent = false;
      cardAlreadyProcessed = false;
      Serial.println("Card removed. Ready for next scan.");
    } else {
      // 卡片仍然存在
      bool isSameCard = (uidLength == currentUidLength);
      if (isSameCard) {
        for (uint8_t i = 0; i < uidLength; i++) {
          if (uid[i] != currentUid[i]) {
            isSameCard = false;
            break;
          }
        }
      }
      
      if (!isSameCard) {
        // 检测到新卡片，重置状态
        waitingForCardRemoval = false;
        memset(currentUid, 0, sizeof(currentUid));
        currentUidLength = 0;
        cardCurrentlyPresent = false;
        cardAlreadyProcessed = false;
      }
    }
  }
}

// 比较两个UID是否相同
bool compareUid(uint8_t* uid1, uint8_t* uid2, uint8_t length) {
  for (uint8_t i = 0; i < length; i++) {
    if (uid1[i] != uid2[i]) {
      return false;
    }
  }
  return true;
}

void f(){};