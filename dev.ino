#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <OneButton.h>
#include "Action_people.h"

// 创建PCA9685对象，使用默认地址0x40
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

#define BUTTON_1 5
#define BUTTON_2 6

OneButton button1(BUTTON_1, true, true);
OneButton button2(BUTTON_2, true, true);

void setup() {
  Serial.begin(9600);
  
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
}

void loop() {
  // 更新所有动作序列状态
  updateSequences();
  
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