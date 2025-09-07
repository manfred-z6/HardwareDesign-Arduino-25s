#include "Action_people.h"

// 定义全局变量
const int MAX_CONCURRENT_SEQUENCES = 10;
const int MAX_ACTION_SINGLESEQ = 20;
ActionSequenceState activeSequences[MAX_CONCURRENT_SEQUENCES] = {};
int activeSequenceCount = 0;
bool servoChannelOccupied[16] = {false};

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
        servoChannelOccupied[currentAction.servoChannel] = false;
        // 停止当前舵机
        pwm.setPWM(currentAction.servoChannel, 0, getPulseWidth(1500));
        
        // 移动到下一个动作
        seq->currentActionIndex++;
        
        if (seq->currentActionIndex < seq->actionCount) {
          executeAction(i);
        } else {
          // 序列完成
          seq->isRunning = false;
          seq->currentActionIndex = -1;
          seq->isActive = false;
          Serial.print("Sequence ");
          Serial.print(i);
          Serial.println(" completed");
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
 
  // 检查舵机通道是否被占用
  if (servoChannelOccupied[action.servoChannel]) {
    Serial.print("Servo channel ");
    Serial.print(action.servoChannel);
    Serial.println(" is busy, waiting...");
    return;
  }
  
  servoChannelOccupied[action.servoChannel] = true;

  int pulseWidth;
  if (action.direction == 0) {
    pulseWidth = map(action.speed, 0, 100, 1500, 500); // 顺时针
  } else {
    pulseWidth = map(action.speed, 0, 100, 1500, 2500); // 逆时针
  }
  
  pulseWidth = constrain(pulseWidth, 500, 2500);

  pwm.setPWM(action.servoChannel, 0, getPulseWidth(pulseWidth));
  seq->startTime = millis();
  
  Serial.print("Executing action on servo ");
  Serial.println(action.servoChannel);
}

// 添加一个动作序列并返回其索引
int addActionSequence(RotationAction actions[], int count) {
  if (count > MAX_ACTION_SINGLESEQ) {
    Serial.println("Too many actions for one sequence!");
    return -1;
  }

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

  activeSequences[freeSlotIndex].actionCount = count;
  activeSequences[freeSlotIndex].currentActionIndex = -1;
  activeSequences[freeSlotIndex].isRunning = false;
  activeSequences[freeSlotIndex].isActive = true;
  
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
  if (!seq->isActive) return;

  if (seq->currentActionIndex >= 0 && seq->currentActionIndex < seq->actionCount) {
    RotationAction currentAction = seq->sequence[seq->currentActionIndex];
    pwm.setPWM(currentAction.servoChannel, 0, getPulseWidth(1500));
    servoChannelOccupied[currentAction.servoChannel] = false;
  }
  
  seq->isRunning = false;
  seq->currentActionIndex = -1;
  seq->isActive = false;
  
  Serial.print("Sequence ");
  Serial.print(sequenceIndex);
  Serial.println(" stopped");
}

// 具体动作序列
int action_attack_1() {
  RotationAction actions[2];
  actions[0] = {0, 1, 60, 400};  // 舵机0逆时针
  actions[1] = {0, 0, 100, 300}; // 舵机0顺时针
  return addActionSequence(actions, 2);
}

int action_back_lb() {
  RotationAction actions[2];
  actions[0] = {1, 0, 50, 500};  // 舵机1顺时针
  actions[1] = {1, 1, 75, 1000}; // 舵机1逆时针
  return addActionSequence(actions, 2);
}