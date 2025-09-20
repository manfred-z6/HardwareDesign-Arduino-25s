#include "Action_people.h"
#include "GlobalVars.h"

// 定义全局变量（添加2后缀）
const int MAX_CONCURRENT_SEQUENCES2 = 10;
const int MAX_ACTION_SINGLESEQ2 = 13;
ActionSequenceState2 activeSequences2[MAX_CONCURRENT_SEQUENCES2] = {};
int activeSequenceCount2 = 0;
bool servoChannelOccupied2[16] = {false};

// 将微秒转换为PCA9685所需的PWM值（添加2后缀）
uint16_t getPulseWidth2(unsigned long microseconds) {
  double pulselength2 = 1000000; // 1秒的微秒数
  pulselength2 /= 50; // PWM频率50Hz
  pulselength2 /= 4096; // 12位分辨率
  return microseconds / pulselength2;
}

// 更新所有序列状态，需在loop中定期调用（添加2后缀）
void updateSequences2() {
  bool foundRunning2 = false; // 局部变量，用于检查当前是否有序列运行
   
  for (int i = 0; i < MAX_CONCURRENT_SEQUENCES2; i++) {
    ActionSequenceState2* seq2 = &activeSequences2[i];
    
    if (seq2->isActive && seq2->isRunning && seq2->currentActionIndex >= 0 && seq2->currentActionIndex < seq2->actionCount) {
      RotationAction2 currentAction2 = seq2->sequence[seq2->currentActionIndex];
      
      // 检查当前动作是否完成
      if (millis() - seq2->startTime >= currentAction2.duration) {
        servoChannelOccupied2[currentAction2.servoChannel] = false;
        // 停止当前舵机
        pwm2.setPWM(currentAction2.servoChannel, 0, getPulseWidth2(1500));
        
        // 移动到下一个动作
        seq2->currentActionIndex++;
        
        if (seq2->currentActionIndex < seq2->actionCount) {
          executeAction2(i);
        } else {
          // 序列完成
          seq2->isRunning = false;
          seq2->currentActionIndex = -1;
          seq2->isActive = false;
          Serial.print("Sequence ");
          Serial.print(i);
          Serial.println(" completed");
          flag_slider2 = true;   //完成序列后，更改滑台标志，用于滑台返回
          time_action_end2 = millis();
        }
      }
      foundRunning2 = true; // 发现有序列在运行
    }
  }

  if(foundRunning2){
    isAnySequenceRunning2 = foundRunning2;  // 更新全局状态标志
  }
}

// 执行指定序列的当前动作（添加2后缀）
void executeAction2(int sequenceIndex) {
  if (sequenceIndex < 0 || sequenceIndex >= MAX_CONCURRENT_SEQUENCES2) return;
  
  ActionSequenceState2* seq2 = &activeSequences2[sequenceIndex];
  if (seq2->currentActionIndex < 0 || seq2->currentActionIndex >= seq2->actionCount) return;
  
  RotationAction2 action2 = seq2->sequence[seq2->currentActionIndex];
 
  // 检查舵机通道是否被占用
  if (servoChannelOccupied2[action2.servoChannel]) {
    Serial.print(F("Servo channel "));
    Serial.print(action2.servoChannel);
    Serial.println(F(" is busy, waiting..."));
    return;
  }
  
  servoChannelOccupied2[action2.servoChannel] = true;

  int pulseWidth2;
  if (action2.direction == 0) {
    pulseWidth2 = map(action2.speed, 0, 100, 1500, 500); // 顺时针
  } else {
    pulseWidth2 = map(action2.speed, 0, 100, 1500, 2500); // 逆时针
  }
  
  pulseWidth2 = constrain(pulseWidth2, 500, 2500);

  pwm2.setPWM(action2.servoChannel, 0, getPulseWidth2(pulseWidth2));
  seq2->startTime = millis();
  
  Serial.print(F("Executing action on servo "));
  Serial.println(action2.servoChannel);
}

// 添加一个动作序列并返回其索引（添加2后缀）
int addActionSequence2(RotationAction2 actions[], int count) {
  if (count > MAX_ACTION_SINGLESEQ2) {
    Serial.println(F("Too many actions for one sequence!"));
    return -1;
  }

  int freeSlotIndex2 = -1;
  for (int i = 0; i < MAX_CONCURRENT_SEQUENCES2; i++) {
    if (!activeSequences2[i].isActive) {
      freeSlotIndex2 = i;
      break;
    }
  }
  if (freeSlotIndex2 == -1) {
    Serial.println(F("Cannot add more sequences - maximum reached!"));
    return -1;
  }

  activeSequences2[freeSlotIndex2].actionCount = count;
  activeSequences2[freeSlotIndex2].currentActionIndex = -1;
  activeSequences2[freeSlotIndex2].isRunning = false;
  activeSequences2[freeSlotIndex2].isActive = true;
  
  for (int i = 0; i < count; i++) {
    activeSequences2[freeSlotIndex2].sequence[i] = actions[i];
  }
  
  return freeSlotIndex2;
}

// 开始执行指定序列（添加2后缀）
void startSequence2(int sequenceIndex) {
  if (sequenceIndex < 0 || sequenceIndex >= MAX_CONCURRENT_SEQUENCES2) {
    Serial.println(F("Invalid sequence index!"));
    return;
  }
  
  ActionSequenceState2* seq2 = &activeSequences2[sequenceIndex];
  if (seq2->actionCount == 0) {
    Serial.println(F("No actions to execute in this sequence"));
    return;
  }
  
  seq2->currentActionIndex = 0;
  seq2->isRunning = true;
  executeAction2(sequenceIndex);
}

// 停止指定序列（添加2后缀）
void stopSequence2(int sequenceIndex) {
  if (sequenceIndex < 0 || sequenceIndex >= MAX_CONCURRENT_SEQUENCES2) return;
  
  ActionSequenceState2* seq2 = &activeSequences2[sequenceIndex];
  if (!seq2->isActive) return;

  if (seq2->currentActionIndex >= 0 && seq2->currentActionIndex < seq2->actionCount) {
    RotationAction2 currentAction2 = seq2->sequence[seq2->currentActionIndex];
    pwm2.setPWM(currentAction2.servoChannel, 0, getPulseWidth2(1500));
    servoChannelOccupied2[currentAction2.servoChannel] = false;
  }
  
  seq2->isRunning = false;
  seq2->currentActionIndex = -1;
  seq2->isActive = false;
  
  Serial.print(F("Sequence "));
  Serial.print(sequenceIndex);
  Serial.println(F(" stopped"));
}
