#include "Nfc.h"

// 定义全局变量
uint8_t lastUid[7] = {0};
uint8_t lastUidLength = 0;
unsigned long lastReadTime = 0;
const unsigned long DEBOUNCE_TIME = 2000; // 防抖时间2秒

const uint8_t targetUid[] = {0x4, 0x91, 0xF3, 0x87, 0x2B, 0x2, 0x89};
const uint8_t targetUidLength = 7;

// NFC状态机变量
NFCState nfcState = NFC_STATE_IDLE;
unsigned long stateEntryTime = 0;
uint8_t currentUid[7] = {0};
uint8_t currentUidLength = 0;
bool cardAlreadyProcessed = false;

// 初始化PN532
void NFC_Init() {
  nfc.begin();
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (!versiondata) {
    Serial.println("Didn't find PN53x board. Please check your wiring.");
    while (1);
  }
  nfc.SAMConfig();
  Serial.println("NFC Initialized. Waiting for an ISO14443A Card ...");
}

// 非阻塞更新NFC状态
void updatenfc() {
  unsigned long currentMillis = millis();

  switch (nfcState) {
    case NFC_STATE_IDLE:
      if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, currentUid, &currentUidLength, 20)) {
        Serial.print("Found a card. UID: ");
        for (uint8_t i = 0; i < currentUidLength; i++) {
          Serial.print(" 0x"); Serial.print(currentUid[i], HEX);
        }
        Serial.println();
        nfcState = NFC_STATE_CARD_PRESENT;
        stateEntryTime = currentMillis;
        cardAlreadyProcessed = false;
      }
      break;

    case NFC_STATE_CARD_PRESENT:
      // 检查是否是目标卡且防抖时间已过
      if (currentUidLength == targetUidLength && 
          compareUid(currentUid, (uint8_t*)targetUid, targetUidLength) &&
          (currentMillis - stateEntryTime > DEBOUNCE_TIME) &&
          !cardAlreadyProcessed) {
        
        memcpy(lastUid, currentUid, currentUidLength);
        lastUidLength = currentUidLength;
        lastReadTime = currentMillis;

        onTargetCardDetected(); // 执行目标卡触发的函数
        cardAlreadyProcessed = true;

        nfcState = NFC_STATE_WAIT_REMOVAL;
        stateEntryTime = currentMillis;
        Serial.println("Target card processed. Waiting for removal...");
      } 
      // 如果不是目标卡，或者防抖时间未过，直接进入等待移除状态
      else if (!(currentUidLength == targetUidLength && 
                compareUid(currentUid, (uint8_t*)targetUid, targetUidLength)) ||
                (currentMillis - stateEntryTime <= DEBOUNCE_TIME)) {
        nfcState = NFC_STATE_WAIT_REMOVAL;
        stateEntryTime = currentMillis;
        Serial.println("Non-target card or in debounce. Waiting for removal...");
      }
      break;

    case NFC_STATE_WAIT_REMOVAL:
      if (!nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, currentUid, &currentUidLength, 20)) {
        nfcState = NFC_STATE_IDLE;
        memset(currentUid, 0, sizeof(currentUid));
        currentUidLength = 0;
        cardAlreadyProcessed = false;
        Serial.println("Card removed. Ready for next scan.");
      }
      break;
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

// NFC触发函数
void onTargetCardDetected() {
  Serial.println("Target NFC card detected! Executing assigned function...");
  // 这里可以添加检测到目标卡后需要执行的动作序列，例如：
  // int seqIndex = action_attack_1();
  // if (seqIndex >= 0) {
  //   startSequence(seqIndex);
  // }
}