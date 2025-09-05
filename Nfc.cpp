#include "Nfc.h"

uint8_t lastUid[7] = {0};
uint8_t lastUidLength = 0;
unsigned long lastReadTime = 0;
const unsigned long DEBOUNCE_TIME = 2000;

const uint8_t targetUid[] = {0xEC, 0x45, 0x70, 0x6};
const uint8_t targetUidLength = 4;

unsigned long previousMillis = 0;
const long nfcInterval = 200;
bool waitingForCardRemoval = false;
const unsigned long removalDelay = 500;


void updatenfc() {
  unsigned long currentMillis = millis(); // 获取当前时间

  // 每隔nfcInterval毫秒检查一次NFC，而不是持续检查
  if (currentMillis - previousMillis >= nfcInterval && !waitingForCardRemoval) {
    previousMillis = currentMillis; // 更新上次读取时间

    uint8_t success;
    uint8_t uid[7] = {0};
    uint8_t uidLength;

    // 尝试读取NFC标签
    success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);

    if (success) {
      // 打印UID信息
      Serial.print("Found a card. UID: ");
      for (uint8_t i = 0; i < uidLength; i++) {
        Serial.print(" 0x"); Serial.print(uid[i], HEX);
      }
      Serial.println();

      // 检查是否是目标UID
      if (uidLength == targetUidLength && compareUid(uid, (uint8_t*)targetUid, targetUidLength)) {
        // 检查防抖时间是否已过
        if (currentMillis - lastReadTime > DEBOUNCE_TIME) {
          // 存储当前UID信息和时间
          memcpy(lastUid, uid, uidLength);
          lastUidLength = uidLength;
          lastReadTime = currentMillis;

          // 执行函数
          f(); // 执行你的操作

          // 设置标志位，进入等待卡片移开的状态
          waitingForCardRemoval = true;
          Serial.println("Waiting for card removal...");
        } else {
          Serial.println("Debounce time not elapsed, ignoring.");
        }
      } else {
        Serial.println("Not the target card.");
        // 如果不是目标卡，也等待移开，但可以根据需要修改
        waitingForCardRemoval = true;
      }
    }
  }

  // 处理等待卡片移开的状态
  if (waitingForCardRemoval) {
    uint8_t success;
    uint8_t uid[7] = {0};
    uint8_t uidLength;
    // 检查卡片是否已移开
    success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 10); // 短暂检测

    if (!success) { // 如果读不到卡，认为已移开
      waitingForCardRemoval = false;
      Serial.println("Card removed. Ready for next scan.");
    } else {
      // 如果还能读到卡，检查是否已经过了足够的等待时间
      if (currentMillis - lastReadTime >= removalDelay) {
        // 这里可以添加逻辑，如果卡片长时间未移走，也许可以重置状态或提示用户
        // waitingForCardRemoval = false; // 例如，强制重置（根据需求选择）
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