#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <Adafruit_PN532.h>
#include <OneButton.h>
#include "Action_people.h"
#include "Slider.h"
#include "Nfc.h"
#include "Game.h"
#include "GlobalVars.h"

// 创建硬件对象
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
Adafruit_PN532 nfc(255, 255); // 使用默认I2C引脚

// 按钮定义
#define BUTTON_1 5
#define BUTTON_2 6
OneButton button1(BUTTON_1, true, true);
OneButton button2(BUTTON_2, true, true);

//定义全局变量
volatile bool isAnySequenceRunning = false; //判断是否有动作序列在执行
enum state_mode {
  MENU,
  GAME,
  PLOT
};
state_mode state = MENU;

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
  button1.attachLongPressStart(onLongPress1);
  button2.attachLongPressStart(onLongPress2);
  
  // 初始化NFC模块
  NFC_Init(); // 使用Nfc.cpp中定义的初始化函数
  Serial.println("System Initialization Complete.");
}

void loop() {  
  //处理按钮事件
  button1.tick();
  button2.tick();
  switch(state){
    case MENU:{
      menu();
      break;
    }
    case GAME:{
      // 更新游戏状态
      game.checkGameState();
      // 首先重置状态标志，假设没有序列运行
      isAnySequenceRunning = false;
      // 更新所有滑块序列状态
      updateSliderSequences();
      // 更新所有动作序列状态
      updateSequences();
      // 只有在没有序列运行时，才更新NFC状态
      if(!game.isGameOver()) {
        if (!isAnySequenceRunning) {
          updatenfc();
        }
      } else {
        Serial.println("游戏结束！");
        state = MENU;
      }
      break;
    }
    case PLOT:{
      //更新所有滑块序列状态
      updateSliderSequences();
      //更新所有动作序列状态
      updateSequences();
      break;
    }
    default: break;
  }
}

void menu() {
  
}

// 按钮回调函数
void onClick1() {
  Serial.println("Button1 clicked");
}
void onLongPress1() {
  Serial.println("Button1 long-pressed");
  switch(state){
    case MENU: {
      state = PLOT;
      Serial.println("已进入剧情模式");
      break;
    }
    default: break;
  }
}

void onClick2() {
  Serial.println("Button2 clicked");
}
void onLongPress2() {
  Serial.println("Button2 long-pressed");
  switch(state){
    case MENU: {
      state = GAME;
      // 初始化游戏
      game.initializeGame();
      // 显示初始状态
      showGameStatus();
      break;
    }
    case GAME: {
      state = MENU;
      Serial.println("已返回菜单");
      break;
    }
    default: break;
  }
}