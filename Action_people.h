#ifndef __ACTION_PEOPLE_H
#define __ACTION_PEOPLE_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

extern Adafruit_PWMServoDriver pwm;

// 定义转动动作结构体
struct RotationAction {
  uint8_t servoChannel : 4;  // 要控制的舵机通道 (0-15)
  uint8_t direction : 1;     // 方向：0顺时针，1逆时针
  uint8_t speed : 7;         // 速度：0-100%
  unsigned long duration;    // 持续时间：毫秒
};

// 定义每个独立动作序列的状态结构体
struct ActionSequenceState {
  RotationAction sequence[10]; // 该序列的动作数组
  int actionCount;             // 该序列的动作数量
  int currentActionIndex;      // 该序列当前执行的动作索引
  bool isRunning;              // 该序列是否正在运行
  bool isActive;               // 标记该序列槽位是否已被分配并使用
  unsigned long startTime;     // 当前动作的开始时间
};

// 声明全局变量
extern const int MAX_CONCURRENT_SEQUENCES;
extern ActionSequenceState activeSequences[];
extern int activeSequenceCount;
extern bool servoChannelOccupied[16];

// 声明函数
void executeAction(int sequenceIndex);
int addActionSequence(RotationAction actions[], int count);
void startSequence(int sequenceIndex);
void stopSequence(int sequenceIndex);
void updateSequences();
int action_attack_1();
int action_back_lb();
uint16_t getPulseWidth(unsigned long microseconds);

#endif