#include "Slider.h"
#include "GlobalVars.h"
// 定义全局变量
const int MAX_SLIDER_SEQUENCES = 10;
SliderSequenceState sliderSequences[MAX_SLIDER_SEQUENCES] = {};
bool motorChannelOccupied[MOTOR_COUNT] = {false};

// 电机引脚配置
MotorPins motorPins[MOTOR_COUNT] = {
  {22, 23, 24},  // 电机0: dirPin, stepPin, enaPin
  {25, 26, 27},  // 电机1
  {28, 29, 30},  // 电机2
  {31, 32, 33}   // 电机3
};

// 初始化电机引脚
void initSliderMotors() {
  for (int i = 0; i < MOTOR_COUNT; i++) {
    pinMode(motorPins[i].dirPin, OUTPUT);
    pinMode(motorPins[i].stepPin, OUTPUT);
    pinMode(motorPins[i].enaPin, OUTPUT);
    digitalWrite(motorPins[i].enaPin, HIGH); // 初始禁用电机
  }
}


// 更新所有滑块序列状态，需在loop中定期调用
void updateSliderSequences() {
  bool foundRunning = false; // 局部变量

  for (int i = 0; i < MAX_SLIDER_SEQUENCES; i++) {
    SliderSequenceState* seq = &sliderSequences[i];
    
    if (seq->isActive && seq->isRunning && seq->currentActionIndex >= 0 && seq->currentActionIndex < seq->actionCount) {
      SliderAction currentAction = seq->sequence[seq->currentActionIndex];
      
      // 检查当前动作是否完成
      if (millis() - seq->motorState.startTime >= currentAction.duration) {
        motorChannelOccupied[currentAction.motorID] = false;
        // 禁用当前电机
        digitalWrite(motorPins[currentAction.motorID].enaPin, HIGH);
        
        // 移动到下一个动作
        seq->currentActionIndex++;
        
        if (seq->currentActionIndex < seq->actionCount) {
          sliderExecuteAction(i);
        } else {
          // 序列完成
          seq->isRunning = false;
          seq->currentActionIndex = -1;
          seq->isActive = false;
          Serial.print(F("Slider Sequence "));
          Serial.print(i);
          Serial.println(F(" completed"));
        }
      } else {
        // 执行步进操作
        stepMotor(i);
      }
      foundRunning = true; // 发现有滑块序列在运行
    }
  }
  if (foundRunning) {
    isAnySequenceRunning = true;
    isSliderMoving = true;
  } else {
    isSliderMoving = false;
  }

}

// 执行指定序列的当前动作
void sliderExecuteAction(int sequenceIndex) {
  if (sequenceIndex < 0 || sequenceIndex >= MAX_SLIDER_SEQUENCES) return;
  
  SliderSequenceState* seq = &sliderSequences[sequenceIndex];
  if (seq->currentActionIndex < 0 || seq->currentActionIndex >= seq->actionCount) return;
  
  SliderAction action = seq->sequence[seq->currentActionIndex];
  int motorID = action.motorID;
 
  // 检查电机通道是否被占用
  if (motorChannelOccupied[motorID]) {
    Serial.print(F("Motor "));
    Serial.print(motorID);
    Serial.println(F(" is busy, waiting..."));
    return;
  }
  
  motorChannelOccupied[motorID] = true;

  // 设置方向
  digitalWrite(motorPins[motorID].dirPin, action.direction ? HIGH : LOW);
  
  // 计算步进延迟（转速转/秒 -> 微秒延迟）
  // 转速 (转/秒) -> 脉冲频率 (Hz) = 脉冲数/转 * 转/秒
  if (action.speed > 0) {
    seq->motorState.stepDelay = 500000 / (action.speed * 200); // 计算每个脉冲的间隔时间（微秒）
  } else {
    seq->motorState.stepDelay = 0; // 速度为0则不移动
  }
  
  // 记录开始时间
  seq->motorState.startTime = millis();
  seq->motorState.lastStepTime = micros();
  
  // 使能电机
  digitalWrite(motorPins[motorID].enaPin, LOW);
  
  Serial.print(F("Executing action on motor "));
  Serial.println(motorID);
}

// 步进电机函数（非阻塞）
void stepMotor(int sequenceIndex) {
  if (sequenceIndex < 0 || sequenceIndex >= MAX_SLIDER_SEQUENCES) return;
  
  SliderSequenceState* seq = &sliderSequences[sequenceIndex];
  if (seq->currentActionIndex < 0 || seq->currentActionIndex >= seq->actionCount) return;
  
  SliderAction action = seq->sequence[seq->currentActionIndex];
  int motorID = action.motorID;
  
  unsigned long currentMicros = micros();
  unsigned long stepInterval = seq->motorState.stepDelay;
  // 检查是否错过了多个脉冲（例如，由于循环阻塞）
  if (currentMicros - seq->motorState.lastStepTime >= stepInterval) {
    // 计算错过的脉冲数，并立即补发一个脉冲（避免积压）
    seq->motorState.lastStepTime = currentMicros; // 重置最后步进时间
    
    // 产生一个步进脉冲
    digitalWrite(motorPins[motorID].stepPin, HIGH);
    delayMicroseconds(5); // 保持短脉冲
    digitalWrite(motorPins[motorID].stepPin, LOW);
  }
}

// 添加一个滑块动作序列并返回其索引
int addSliderSequence(SliderAction actions[], int count) {
  const int MAX_ACTIONS_PER_SEQUENCE = 10;
  if (count > MAX_ACTIONS_PER_SEQUENCE) {
    //Serial.println("Too many actions for one sequence!");
    return -1;
  }

  int freeSlotIndex = -1;
  for (int i = 0; i < MAX_SLIDER_SEQUENCES; i++) {
    if (!sliderSequences[i].isActive) {
      freeSlotIndex = i;
      break;
    }
  }
  if (freeSlotIndex == -1) {
    //Serial.println("Cannot add more sequences - maximum reached!");
    return -1;
  }

  sliderSequences[freeSlotIndex].actionCount = count;
  sliderSequences[freeSlotIndex].currentActionIndex = -1;
  sliderSequences[freeSlotIndex].isRunning = false;
  sliderSequences[freeSlotIndex].isActive = true;
  sliderSequences[freeSlotIndex].motorState.currentMotion = 0;
  sliderSequences[freeSlotIndex].motorState.sequenceCompleted = false;
  sliderSequences[freeSlotIndex].motorState.isWaiting = false;
  
  for (int i = 0; i < count; i++) {
    sliderSequences[freeSlotIndex].sequence[i] = actions[i];
  }
  
  return freeSlotIndex;
}

// 开始执行指定序列
void startSliderSequence(int sequenceIndex) {
  if (sequenceIndex < 0 || sequenceIndex >= MAX_SLIDER_SEQUENCES) {
    //Serial.println("Invalid sequence index!");
    return;
  }
  
  SliderSequenceState* seq = &sliderSequences[sequenceIndex];
  if (seq->actionCount == 0) {
    //Serial.println("No actions to execute in this sequence");
    return;
  }
  
  seq->currentActionIndex = 0;
  seq->isRunning = true;
  sliderExecuteAction(sequenceIndex);
}

// 停止指定序列
void stopSliderSequence(int sequenceIndex) {
  if (sequenceIndex < 0 || sequenceIndex >= MAX_SLIDER_SEQUENCES) return;
  
  SliderSequenceState* seq = &sliderSequences[sequenceIndex];
  if (!seq->isActive) return;

  if (seq->currentActionIndex >= 0 && seq->currentActionIndex < seq->actionCount) {
    SliderAction currentAction = seq->sequence[seq->currentActionIndex];
    digitalWrite(motorPins[currentAction.motorID].enaPin, HIGH);
    motorChannelOccupied[currentAction.motorID] = false;
  }
  
  seq->isRunning = false;
  seq->currentActionIndex = -1;
  seq->isActive = false;
  
  Serial.print(F("Slider Sequence "));
  Serial.print(sequenceIndex);
  Serial.println(F(" stopped"));
}

// 预定义动作序列函数（每个序列只控制一个电机）
int slide_motor1_sequence() {
  SliderAction actions[4];
  actions[0] = {0, 1, 2.0, 1000, 500};    // 电机0顺时针，2转/秒，持续1秒，等待0.5秒
  actions[1] = {0, 0, 1.5, 2000, 1000};   // 电机0逆时针，1.5转/秒，持续2秒，等待1秒
  actions[2] = {0, 1, 3.0, 1500, 800};    // 电机0顺时针，3转/秒，持续1.5秒，等待0.8秒
  actions[3] = {0, 0, 0.5, 3000, 0};      // 电机0逆时针，0.5转/秒，持续3秒
  return addSliderSequence(actions, 4);
}

int slide_motor2_sequence() {
  SliderAction actions[3];
  actions[0] = {1, 1, 1.0, 1500, 300};    // 电机1顺时针，1转/秒，持续1.5秒，等待0.3秒
  actions[1] = {1, 0, 2.0, 1000, 700};    // 电机1逆时针，2转/秒，持续1秒，等待0.7秒
  actions[2] = {1, 1, 1.5, 2000, 0};      // 电机1顺时针，1.5转/秒，持续2秒
  return addSliderSequence(actions, 3);
}

int slide_motor3_sequence() {
  SliderAction actions[5];
  actions[0] = {3, 1, 1.8, 2000, 400};    // 电机2顺时针，0.8转/秒，持续2秒，等待0.4秒
  actions[1] = {3, 0, 2, 1800, 600};    // 电机2逆时针，1.2转/秒，持续1.8秒，等待0.6秒
  actions[2] = {3, 1, 2.0, 1500, 300};    // 电机2顺时针，2转/秒，持续1.5秒，等待0.3秒
  actions[3] = {3, 0, 3.0, 2500, 500};    // 电机2逆时针，1转/秒，持续2.5秒，等待0.5秒
  actions[4] = {3, 1, 2.5, 1200, 0};      // 电机2顺时针，1.5转/秒，持续1.2秒
  return addSliderSequence(actions, 5);
}

int slide_motor4_out() {
  SliderAction actions[2];
  actions[0] = {3, 1, 2.0, 2000, 0};    
  actions[1] = {3, 1, 2.0, 1500, 0};    
  return addSliderSequence(actions, 2);
}
