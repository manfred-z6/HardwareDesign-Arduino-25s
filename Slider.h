#ifndef SLIDER_H
#define SLIDER_H

#include <Arduino.h>

// 定义电机数量
const int MOTOR_COUNT = 4;

// 运动参数结构体
struct SliderAction {
  uint8_t motorID : 2;        // 电机ID (0-3)
  uint8_t direction : 1;      // 转动方向 (0:逆时针, 1:顺时针)
  float speed;                // 转速 (转/秒)
  unsigned long duration;     // 转动时间 (毫秒)
  unsigned long waitTime;     // 等待时间 (毫秒)
};

// 电机状态结构体
struct MotorState {
  int currentMotion;
  bool sequenceCompleted;
  unsigned long startTime;
  unsigned long lastStepTime;
  unsigned long waitStartTime;
  bool isWaiting;
  int stepDelay;
};

// 定义每个独立动作序列的状态结构体
struct SliderSequenceState {
  SliderAction sequence[10];  // 该序列的动作数组
  int actionCount;            // 该序列的动作数量
  int currentActionIndex;     // 该序列当前执行的动作索引
  bool isRunning;             // 该序列是否正在运行
  bool isActive;              // 标记该序列槽位是否已被分配并使用
  MotorState motorState;      // 电机的实时状态
};

// 电机引脚配置结构体
struct MotorPins {
  int dirPin;
  int stepPin;
  int enaPin;
};

// 声明全局变量
extern const int MAX_SLIDER_SEQUENCES;
extern SliderSequenceState sliderSequences[];
extern bool motorChannelOccupied[MOTOR_COUNT];
extern MotorPins motorPins[MOTOR_COUNT];

// 声明函数
void sliderExecuteAction(int sequenceIndex);
int addSliderSequence(SliderAction actions[], int count);
void startSliderSequence(int sequenceIndex);
void stopSliderSequence(int sequenceIndex);
void updateSliderSequences();
void initSliderMotors();
int slide_forward();
int slide_back();
int slide_rotate();
int slide_custom_pattern();
void stepMotor(int sequenceIndex);

int slide_lvbu_out();
int slide_lvbu_back();
int slide_liubei_out();
int slide_liubei_back();
int slide_guanyu_out();
int slide_guanyu_back();
int slide_zhangfei_out();
int slide_zhangfei_back();

#endif