#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <Adafruit_PN532.h>
#include <OneButton.h>
#include <U8glib.h>
#include "Action_people.h"
#include "Slider.h"
#include "Nfc.h"
#include "Game.h"
#include "GlobalVars.h"
#include "Music.h"
#include "Music_background.h"

// 音乐播放器全局变量
#define MUSIC_SERIAL Serial1    //使用Mega的Serial1硬件串口
#define MUSIC_SERIAL2 Serial2   //使用Mega的Serial2硬件串口
// 创建硬件对象
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
Adafruit_PN532 nfc(255, 255); // 使用默认I2C引脚
U8GLIB_SH1106_128X64 u8g(U8G_I2C_OPT_NONE|U8G_I2C_OPT_DEV_0);	// I2C / TWI 


// 按钮定义
#define BUTTON_1 5
#define BUTTON_2 6
OneButton button1(BUTTON_1, true, true);
OneButton button2(BUTTON_2, true, true);



enum state_mode {
  MENU,
  GAME,
  PLOT
};
state_mode state = MENU;


// OLED更新计时器
unsigned long lastOledUpdateTime = 0;
const unsigned long OLED_UPDATE_INTERVAL = 200; // OLED更新间隔(毫秒)

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
  Serial.println(F("Slider Motors Initialized."));

  // 初始化按钮
  button1.attachClick(onClick1);
  button2.attachClick(onClick2);
  button1.attachLongPressStart(onLongPress1);
  button2.attachLongPressStart(onLongPress2);
  
  // 初始化NFC模块
  NFC_Init(); // 使用Nfc.cpp中定义的初始化函数

  // 初始化音乐播放器
  MUSIC_SERIAL.begin(9600);
  if (!musicPlayer.begin(MUSIC_SERIAL)) {
    Serial.println(F("音乐播放器初始化失败，系统继续运行但无音频功能"));
  } else {
    Serial.println(F("音乐播放器初始化成功"));
  }

  // 初始化音乐播放器2
  MUSIC_SERIAL2.begin(9600);
  if (!musicPlayer2.begin(MUSIC_SERIAL2)) {
    Serial.println(F("音乐播放器2初始化失败，系统继续运行但无音频功能"));
  } else {
    Serial.println(F("音乐播放器2初始化成功"));
  }

  Serial.println(F("System Initialization Complete."));
}



void loop() {  
  //处理按钮事件
  button1.tick();
  button2.tick();

  // 检查音乐播放状态
  musicPlayer.checkPlayingStatus();

  unsigned long currentTime = millis();

  switch(state){
    case MENU:{
      menu_main();
      // 更新所有滑块序列状态
      updateSliderSequences();
      // 更新所有动作序列状态
      updateSequences();
      break;
    }
    case GAME:{
      // 更新游戏状态
      game.checkGameState();
      // 首先重置状态标志，假设没有序列运行
      isAnySequenceRunning = false;
      // 更新所有滑块序列状态
      updateSliderSequences();
      // 在没有滑台移动时更新所有动作序列状态
      if(!isSliderMoving){
        updateSequences();
      }
      // 定期更新OLED显示，避免频繁更新影响NFC读取
      if (currentTime - lastOledUpdateTime >= OLED_UPDATE_INTERVAL) {
        displayGameStatus();
        lastOledUpdateTime = currentTime;
      }

      // 只有在没有序列运行时，才更新NFC状态
      if(!game.isGameOver()) {
        if (!isAnySequenceRunning && (millis() - lastAudioEndTime >= AUDIO_COOLDOWN)) {
          updatenfc();
        }
      } else {
        Serial.println(F("游戏结束！"));
        musicPlayer2.stop();
        if(!game.isLuBuAlive()){
          musicPlayer.playTrackOnce(24);
        } else {
          musicPlayer.playTrackOnce(23);
        }
        state = MENU;
      }

      break;
    }
    case PLOT:{
      menu_plot();
      //更新所有滑块序列状态
      updateSliderSequences();
      //更新所有动作序列状态
      updateSequences();
      break;
    }
    default: break;
  }
}

void menu_main() {
  u8g.firstPage();
  do{
    u8g.setFont(u8g_font_6x10);
    u8g.drawStr(8, 15, "1.PLOT MODE");
    u8g.drawStr(8, 30, "(Long Press Button1)");
    u8g.drawStr(8, 45, "2.GAME MODE");
    u8g.drawStr(8, 60, "(Long Press Button2)");
  }while(u8g.nextPage());
}

void displayGameStatus(){
  u8g.firstPage();
  do {
    u8g.setFont(u8g_font_6x10);
    
    // 显示吕布状态
    u8g.drawStr(0, 20, game.getLuBuStatusForDisplay().c_str());
    
    // 显示刘备状态
    u8g.drawStr(0, 34, game.getLiuBeiStatusForDisplay().c_str());
    
    // 显示关羽状态
    u8g.drawStr(0, 48, game.getGuanYuStatusForDisplay().c_str());
    
    // 显示张飞状态
    u8g.drawStr(0, 62, game.getZhangFeiStatusForDisplay().c_str());
    
    // 显示当前回合
    u8g.setFont(u8g_font_6x10);
    if (game.getIsLuBuTurn()) {
      u8g.drawStr(32, 10, "LVBU Turn");
    } else {
      u8g.drawStr(32, 10, "HERO Turn");
    }
    
    // 显示游戏状态（如果游戏结束）
    if (game.isGameOver()) {
      u8g.setFont(u8g_font_6x10);
      if (!game.isLuBuAlive()) {
        u8g.drawStr(90, 22, "Hero Win!");
      } else if (!game.isLiuBeiAlive() && 
                 !game.isGuanYuAlive() && 
                 !game.isZhangFeiAlive()) {
        u8g.drawStr(90, 22, "LVBU Win!");
      }
    }
  } while(u8g.nextPage());
}

void menu_plot(){
  u8g.firstPage();
  do{
    u8g.setFont(u8g_font_10x20);
    u8g.drawStr(4, 32, "PLOT MODE");
  }while(u8g.nextPage());
}

// 按钮回调函数
void onClick1() {
  Serial.println(F("Button1 clicked"));
  startSliderSequence(slide_motor4_out());
}
void onLongPress1() {
  Serial.println(F("Button1 long-pressed"));
  switch(state){
    case MENU: {
      state = PLOT;
      Serial.println(F("已进入剧情模式"));  
      break;
    }
    default: break;
  }
}

void onClick2() {
  Serial.println(F("Button2 clicked"));
}
void onLongPress2() {
  Serial.println(F("Button2 long-pressed"));
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
      Serial.println(F("已返回菜单"));
      musicPlayer2.stop();
      break;
    }
    default: break;
  }
}
