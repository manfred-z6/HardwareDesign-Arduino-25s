#include "Nfc.h"

// 定义全局变量
const unsigned long REQUIRED_DETECTION_TIME = 1000; // 需要连续检测到卡片500ms才算有效

NFCState nfcState = NFC_STATE_IDLE;
unsigned long stateEntryTime = 0;
uint8_t currentUid[7] = {0};
uint8_t currentUidLength = 0;
bool cardProcessed = false;

// 定义UID-函数映射表
const UidFunctionMap uidFunctionMap[] = {
  {{0xEC, 0x45, 0x70, 0x6}, 4, onUid1Detected},  
  {{0x4, 0x81, 0x8F, 0x8F, 0x2B, 0x2, 0x89}, 7, onUid2Detected},  
  {{0x4, 0xB1, 0xCE, 0x89, 0x2B, 0x2, 0x89}, 7, onUid3Detected},  
  {{0x4, 0x71, 0xB6, 0x8A, 0x2B, 0x2, 0x89}, 7, onUid4Detected},
  {{0x4, 0xF1, 0x42, 0x89, 0x2B, 0x2, 0x89}, 7, onUid5Detected},  
  {{0x4, 0x91, 0xF3, 0x87, 0x2B, 0x2, 0x89}, 7, onUid6Detected},  
  {{0x4, 0x1, 0xEF, 0x8A, 0x2B, 0x2, 0x89}, 7, onUid7Detected},  
  {{0x4, 0xC1, 0x6B, 0x8F, 0x2B, 0x2, 0x89}, 7, onUid8Detected},  
  {{0x4, 0x41, 0xA, 0x8C, 0x2B, 0x2, 0x89}, 7, onUid9Detected},  
  {{0x4, 0x91, 0x64, 0x81, 0x2B, 0x2, 0x89}, 7, onUid10Detected},  
  {{0x4, 0x41, 0x1C, 0x86, 0x2B, 0x2, 0x89}, 7, onUid11Detected},  
  {{0x4, 0x71, 0xB6, 0x8A, 0x2B, 0x2, 0x89}, 7, onUid12Detected},  
  {{0x4, 0x71, 0xB6, 0x8A, 0x2B, 0x2, 0x89}, 7, onUid13Detected},  
  {{0x4, 0x71, 0xB6, 0x8A, 0x2B, 0x2, 0x89}, 7, onUid14Detected},   
  {{0x4, 0x71, 0xB6, 0x8A, 0x2B, 0x2, 0x89}, 7, onUid15Detected},  
  {{0x4, 0x71, 0xB6, 0x8A, 0x2B, 0x2, 0x89}, 7, onUid16Detected},  
  {{0x4, 0x71, 0xB6, 0x8A, 0x2B, 0x2, 0x89}, 7, onUid17Detected},  
  {{0x4, 0x71, 0xB6, 0x8A, 0x2B, 0x2, 0x89}, 7, onUid18Detected},  
  {{0x4, 0x71, 0xB6, 0x8A, 0x2B, 0x2, 0x89}, 7, onUid20Detected},
  {{0x4, 0x71, 0xB6, 0x8A, 0x2B, 0x2, 0x89}, 7, onUid21Detected}
};

const int uidFunctionCount = sizeof(uidFunctionMap) / sizeof(uidFunctionMap[0]);

// 初始化PN532
void NFC_Init() {
  nfc.begin();
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (!versiondata) {
    Serial.println("Didn't find PN53x board. Please check your wiring.");
    while (1);
  }
  nfc.SAMConfig();
  Serial.println("NFC Initialized. Waiting for cards...");
  Serial.print("Registered ");
  Serial.print(uidFunctionCount);
  Serial.println(" UID-function mappings.");
}

// 非阻塞更新NFC状态
void updatenfc() {
  unsigned long currentMillis = millis();
  uint8_t success;
  uint8_t uid[7] = {0};
  uint8_t uidLength;

  switch (nfcState) {
    case NFC_STATE_IDLE:
      // 尝试检测卡片，超时时间极短（5ms）
      success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 20);
      if (success) {
        // 检测到卡片，转移到检测中状态
        memcpy(currentUid, uid, uidLength);
        currentUidLength = uidLength;
        nfcState = NFC_STATE_DETECTING;
        stateEntryTime = currentMillis;
        cardProcessed = false;
        
        Serial.print("Card detected. UID: ");
        for (uint8_t i = 0; i < uidLength; i++) {
          Serial.print(" 0x"); Serial.print(uid[i], HEX);
        }
        Serial.println();
      }
      break;

    case NFC_STATE_DETECTING:
      // 检查卡片是否仍然存在且停留时间足够
      if (currentMillis - stateEntryTime >= REQUIRED_DETECTION_TIME) {
        nfcState = NFC_STATE_PROCESSING;
        stateEntryTime = currentMillis;
      }
      break;

    case NFC_STATE_PROCESSING:
      if (!cardProcessed) {
        // 查找并执行对应的函数
        bool uidRecognized = false;
        
        for (int i = 0; i < uidFunctionCount; i++) {
          if (currentUidLength == uidFunctionMap[i].uidLength && 
              compareUid(currentUid, (uint8_t*)uidFunctionMap[i].uid, currentUidLength)) {
            Serial.print("Executing function for UID: ");
            for (uint8_t j = 0; j < currentUidLength; j++) {
              Serial.print(" 0x"); Serial.print(currentUid[j], HEX);
            }
            Serial.println();
            
            uidFunctionMap[i].action(); // 执行该UID对应的函数
            uidRecognized = true;
            break;
          }
        }
        
        if (!uidRecognized) {
          Serial.println("Unknown UID detected.");
          onUnknownUidDetected(); // 执行未知UID处理函数
        }
        
        cardProcessed = true;
        nfcState = NFC_STATE_WAIT_REMOVAL;
        stateEntryTime = currentMillis;
      }
      break;

    case NFC_STATE_WAIT_REMOVAL:
      // 检查卡片是否已移开
      success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 20);
      if (!success) {
        // 卡片已移除，返回空闲状态
        nfcState = NFC_STATE_IDLE;
        memset(currentUid, 0, sizeof(currentUid));
        currentUidLength = 0;
        cardProcessed = false;
        Serial.println("Card removed. Ready for next card.");
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

// UID对应的函数实现（空函数，你可以后续修改）
void onUid1Detected() {
  // 第一张卡的函数 - 后续可以修改为具体实现
  Serial.println("Function for UID 1 executed");
}

void onUid2Detected() {
  // 第二张卡的函数 - 后续可以修改为具体实现
  Serial.println("Function for UID 2 executed");
}

void onUid3Detected() {
  // 第三张卡的函数 - 后续可以修改为具体实现
  Serial.println("Function for UID 3 executed");
}

void onUid4Detected() {
  Serial.println("Function for UID 4 executed");
}

void onUid5Detected() {
  Serial.println("Function for UID 5 executed");
}

void onUid6Detected() {
  Serial.println("Function for UID 6 executed");
}

void onUid7Detected() {
  Serial.println("Function for UID 7 executed");
}

void onUid8Detected() {
  Serial.println("Function for UID 8 executed");
}

void onUid9Detected() {
  Serial.println("Function for UID 9 executed");
}

void onUid10Detected() {
  Serial.println("Function for UID 10 executed");
}

void onUid11Detected() {
  Serial.println("Function for UID 11 executed");
}

void onUid12Detected() {
  Serial.println("Function for UID 12 executed");
}

void onUid13Detected() {
  Serial.println("Function for UID 13 executed");
}

void onUid14Detected() {
  Serial.println("Function for UID 14 executed");
}

void onUid15Detected() {
  Serial.println("Function for UID 15 executed");
}

void onUid16Detected() {
  Serial.println("Function for UID 16 executed");
}

void onUid17Detected() {
  Serial.println("Function for UID 17 executed");
}

void onUid18Detected() {
  Serial.println("Function for UID 18 executed");
}

void onUid19Detected() {
  Serial.println("Function for UID 19 executed");
}

void onUid20Detected() {
  Serial.println("Function for UID 20 executed");
}

void onUid21Detected() {
  Serial.println("Function for UID 21 executed");
}

void onUnknownUidDetected() {
  // 未知UID的处理函数 - 后续可以修改为具体实现
  Serial.println("Unknown UID function executed");
}