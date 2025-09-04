#ifndef __ACTION_PEOPLE_H
#define __ACTION_PEOPLE_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

// 声明PCA9685对象（在主文件中定义）
extern Adafruit_PWMServoDriver pwm;

// 定义转动动作结构体
struct RotationAction {
  uint8_t servoChannel : 4;  // 要控制的舵机通道 (0-15)
  uint8_t direction : 1;     // 方向：0顺时针，1逆时针
  uint8_t speed : 7;         // 速度：0-100%
  unsigned long duration;  // 持续时间：毫秒
};

// 定义每个独立动作序列的状态结构体 [4](@ref)
struct ActionSequenceState {
  RotationAction sequence[10]; // 该序列的动作数组
  int actionCount;             // 该序列的动作数量
  int currentActionIndex;      // 该序列当前执行的动作索引
  bool isRunning;              // 该序列是否正在运行
  bool isActive;               // 标记该序列槽位是否已被分配并使用
  unsigned long startTime;     // 当前动作的开始时间
};

// 声明全局变量
extern const int MAX_CONCURRENT_SEQUENCES; // 最大运行的序列数
extern ActionSequenceState activeSequences[]; // 正在运行的序列数组
extern int activeSequenceCount; // 当前活跃的序列数量

// 声明函数
void executeAction(int sequenceIndex);    // 执行指定序列的当前动作
int addActionSequence(RotationAction actions[], int count);   // 添加一个动作序列并返回其索引
void startSequence(int sequenceIndex);   // 开始执行指定序列
void stopSequence(int sequenceIndex);    // 停止指定序列
void updateSequences();                  // 更新所有序列状态，需在loop中调用
int action_attack_1();                  // 动作函数1
int action_back_lb();                   // 动作函数2

// 新增辅助函数
uint16_t getPulseWidth(unsigned long microseconds);

#endif //__ACTION_PEOPLE_H