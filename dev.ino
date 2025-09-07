#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <Adafruit_PN532.h>
#include <OneButton.h>
#include "Action_people.h"
#include "Slider.h"
#include "Nfc.h"

// 创建硬件对象
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
Adafruit_PN532 nfc(255, 255); // 使用默认I2C引脚

// 按钮定义
#define BUTTON_1 5
#define BUTTON_2 6
OneButton button1(BUTTON_1, true, true);
OneButton button2(BUTTON_2, true, true);

void setup() {
  Serial.begin(115200);
  
  // 初始化PCA9685舵机驱动
  pwm.begin();
  pwm.setPWMFreq(50);  // 设置PWM频率为50Hz，适用于标准舵机
  // 初始化所有舵机到停止状态
  for (int i = 0; i < 16; i++) {
    pwm.setPWM(i, 0, getPulseWidth(1500));
  }
  
  // 初始化滑块电机
  initSliderMotors();
  Serial.println("Slider Motors Initialized.");

  // 初始化按钮
  button1.attachClick(onClick1);
  button2.attachClick(onClick2);
  
  // 初始化NFC模块
  NFC_Init(); // 使用Nfc.cpp中定义的初始化函数
  Serial.println("System Initialization Complete.");
}

void loop() {
  //更新所有滑块序列状态
  updateSliderSequences();

  //更新所有动作序列状态
  updateSequences();

  //更新NFC状态
  updatenfc();

  //处理按钮事件
  button1.tick();
  button2.tick();

}

// 按钮回调函数
void onClick1() {
  Serial.println("Button1 clicked - starting attack sequence");
  int seqIndex = action_attack_1();
  if (seqIndex >= 0) {
    startSequence(seqIndex);
  }
}

void onClick2() {
  Serial.println("Button2 clicked - starting back_lb sequence");
  int seqIndex = slide_motor3_sequence();
  if (seqIndex >= 0) {
    startSliderSequence(seqIndex);
  }
}