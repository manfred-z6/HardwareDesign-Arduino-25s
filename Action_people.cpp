#include "Action_people.h"

// 定义全局变量
const int MAX_CONCURRENT_SEQUENCES = 10; // 最多运行序列数
const int MAX_ACTION_SINGLESEQ = 20;  // 单个序列最多运行动作数
ActionSequenceState activeSequences[MAX_CONCURRENT_SEQUENCES] = {};
int activeSequenceCount = 0;  //当前占用的序列数

// 将微秒转换为PCA9685所需的PWM值
uint16_t getPulseWidth(unsigned long microseconds) {
  double pulselength = 1000000; // 1秒的微秒数
  pulselength /= 50; // PWM频率50Hz
  pulselength /= 4096; // 12位分辨率
  return microseconds / pulselength;
}

// 更新所有序列状态，需在loop中定期调用
void updateSequences() {
  for (int i = 0; i < MAX_CONCURRENT_SEQUENCES; i++) {
    ActionSequenceState* seq = &activeSequences[i];
    
    if (seq->isActive && seq->isRunning && seq->currentActionIndex >= 0 && seq->currentActionIndex < seq->actionCount) {
      RotationAction currentAction = seq->sequence[seq->currentActionIndex];
      
      // 检查当前动作是否完成
      if (millis() - seq->startTime >= currentAction.duration) {
        // 当前动作完成，停止这个动作控制的舵机
        pwm.setPWM(currentAction.servoChannel, 0, getPulseWidth(1500));
        
        // 移动到下一个动作
        seq->currentActionIndex++;
        
        if (seq->currentActionIndex < seq->actionCount) {
          // 执行下一个动作
          executeAction(i);
        } else {
          // 所有动作完成
          seq->isRunning = false;
          seq->currentActionIndex = -1;
          seq->isActive = false; // 释放槽位
          Serial.print("Sequence ");
          Serial.print(i);
          Serial.println(" completed and slot freed");
        }
      }
    }
  }
}

// 执行指定序列的当前动作
void executeAction(int sequenceIndex) {
  if (sequenceIndex < 0 || sequenceIndex >= MAX_CONCURRENT_SEQUENCES) return;
  
  ActionSequenceState* seq = &activeSequences[sequenceIndex];
  if (seq->currentActionIndex < 0 || seq->currentActionIndex >= seq->actionCount) return;
  
  RotationAction action = seq->sequence[seq->currentActionIndex];
  
  int pulseWidth;
  if (action.direction == 0) {
    pulseWidth = map(action.speed, 0, 100, 1500, 500); // 顺时针，脉宽变短
  } else {
    pulseWidth = map(action.speed, 0, 100, 1500, 2500); // 逆时针，脉宽变长
  }
  
  pulseWidth = constrain(pulseWidth, 500, 2500); // 确保脉宽在安全范围内

  // 使用PCA9685控制指定通道的舵机
  pwm.setPWM(action.servoChannel, 0, getPulseWidth(pulseWidth));
  seq->startTime = millis();
  
  Serial.print("Sequence ");
  Serial.print(sequenceIndex);
  Serial.print(" executing action ");
  Serial.print(seq->currentActionIndex + 1);
  Serial.print(" on servo ");
  Serial.print(action.servoChannel);
  Serial.print(": ");
  Serial.print(action.direction == 0 ? "Clockwise" : "Counter-clockwise");
  Serial.print(", Duration: ");
  Serial.print(action.duration);
  Serial.print("ms, Speed: ");
  Serial.print(action.speed);
  Serial.println("%");
}

// 添加一个动作序列并返回其索引
int addActionSequence(RotationAction actions[], int count) {
  
  if (count > MAX_ACTION_SINGLESEQ) {
    Serial.println("Too many actions for one sequence!");
    return -1;
  }
  //查找第一个空闲的序列槽位
  int freeSlotIndex = -1;
  for (int i = 0; i < MAX_CONCURRENT_SEQUENCES; i++) {
    if (!activeSequences[i].isActive) {
      freeSlotIndex = i;
      break;
    }
  }
  if (freeSlotIndex == -1) {
    Serial.println("Cannot add more sequences - maximum reached!");
    return -1;
  }
  // 初始化新序列到找到的空闲槽位
  activeSequences[freeSlotIndex].actionCount = count;
  activeSequences[freeSlotIndex].currentActionIndex = -1;
  activeSequences[freeSlotIndex].isRunning = false;
  activeSequences[freeSlotIndex].isActive = true; // 标记槽位已被占用
  
  // 复制动作数据
  for (int i = 0; i < count; i++) {
    activeSequences[freeSlotIndex].sequence[i] = actions[i];
  }
  
  return freeSlotIndex;
}

// 开始执行指定序列
void startSequence(int sequenceIndex) {
  if (sequenceIndex < 0 || sequenceIndex >= MAX_CONCURRENT_SEQUENCES) {
    Serial.println("Invalid sequence index!");
    return;
  }
  
  ActionSequenceState* seq = &activeSequences[sequenceIndex];
  if (seq->actionCount == 0) {
    Serial.println("No actions to execute in this sequence");
    return;
  }
  
  seq->currentActionIndex = 0;
  seq->isRunning = true;
  executeAction(sequenceIndex);
}

// 停止指定序列
void stopSequence(int sequenceIndex) {
  if (sequenceIndex < 0 || sequenceIndex >= MAX_CONCURRENT_SEQUENCES) return;
  
  ActionSequenceState* seq = &activeSequences[sequenceIndex];
  if (!seq->isActive) return; // 如果槽位本来就是空的，直接返回
  // 停止该序列控制的所有舵机
  for (int i = 0; i < seq->actionCount; i++) {
    pwm.setPWM(seq->sequence[i].servoChannel, 0, getPulseWidth(1500));
  }
  
  seq->isRunning = false;
  seq->currentActionIndex = -1;
  seq->isActive = false; // 释放槽位
  
  Serial.print("Sequence ");
  Serial.print(sequenceIndex);
  Serial.println(" stopped and slot freed");
}



// 具体动作序列组合 - 修改为返回序列索引
int action_attack_1() {
  RotationAction actions[2];
  actions[0] = {0, 1, 60, 400};  // 舵机0逆时针
  actions[1] = {0, 0, 100, 300}; // 舵机0顺时针
  
  return addActionSequence(actions, 2);
}

// 吕布剧情动作 - 修改为返回序列索引
int action_back_lb() {
  RotationAction actions[2];
  actions[0] = {1, 0, 50, 500};  // 舵机1顺时针
  actions[1] = {1, 1, 75, 1000}; // 舵机1逆时针
  
  return addActionSequence(actions, 2);
}