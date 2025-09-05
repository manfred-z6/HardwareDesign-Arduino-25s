#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <Adafruit_PN532.h>
#include <OneButton.h>
#include "Action_people.h"
#include "Nfc.h"

// 创建PCA9685对象，使用默认地址0x40
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
Adafruit_PN532 nfc(255, 255); // 使用默认I2C引脚，参数无意义

#define BUTTON_1 5
#define BUTTON_2 6

OneButton button1(BUTTON_1, true, true);
OneButton button2(BUTTON_2, true, true);

void setup() {
  
  // 初始化PCA9685
  pwm.begin();
  pwm.setPWMFreq(50);  // 设置PWM频率为50Hz，适用于标准舵机
  
  // 初始化所有舵机到停止状态
  for (int i = 0; i < 16; i++) {
    pwm.setPWM(i, 0, getPulseWidth(1500));
  }
  
  button1.attachClick(onClick1);
  button2.attachClick(onClick2);
  
  Serial.println("Initialization complete");

  //初始化nfc
  Serial.begin(115200);
  Serial.println("Hello! Testing PN532 with Adafruit Library...");

  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (!versiondata) {
    Serial.print("Didn't find PN53x board. Please check your wiring.");
    while (1); // halt
  }
  // 打印固件版本信息
  Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX);
  Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC);
  Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);

  nfc.SAMConfig(); // 配置模块读取标签
  Serial.println("Waiting for an ISO14443A Card ...");
}

void loop() {
  // 更新所有动作序列状态
  updateSequences();

  // 更新读取到的nfc卡
  updatenfc();

  button1.tick();
  button2.tick();
  
  // 可以添加其他代码，不会阻塞序列执行
}

void onClick1(){
  Serial.println("button1 clicked - starting attack sequence");
  int seqIndex = action_attack_1();
  if (seqIndex >= 0) {
    startSequence(seqIndex);
  }
}

void onClick2(){
  Serial.println("button2 clicked - starting back_lb sequence");
  int seqIndex = action_back_lb();
  if (seqIndex >= 0) {
    startSequence(seqIndex);
  }
}