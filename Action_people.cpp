#include "Action_people.h"
#include "GlobalVars.h"
// 定义全局变量
const int MAX_CONCURRENT_SEQUENCES = 20;
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
  bool foundRunning = false; // 局部变量，用于检查当前是否有序列运行

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
          flag_slider = true;   //完成序列后，更改滑台标志，用于滑台返回
        }
      }
      foundRunning = true; // 发现有序列在运行
    }
  }

  if(foundRunning){
    isAnySequenceRunning = foundRunning;  // 更新全局状态标志
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
    Serial.print(F("Servo channel "));
    Serial.print(action.servoChannel);
    Serial.println(F(" is busy, waiting..."));
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
  
  Serial.print(F("Executing action on servo "));
  Serial.println(action.servoChannel);
}

// 添加一个动作序列并返回其索引
int addActionSequence(RotationAction actions[], int count) {
  if (count > MAX_ACTION_SINGLESEQ) {
    Serial.println(F("Too many actions for one sequence!"));
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
    Serial.println(F("Cannot add more sequences - maximum reached!"));
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
    Serial.println(F("Invalid sequence index!"));
    return;
  }
  
  ActionSequenceState* seq = &activeSequences[sequenceIndex];
  if (seq->actionCount == 0) {
    Serial.println(F("No actions to execute in this sequence"));
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
  
  Serial.print(F("Sequence "));
  Serial.print(sequenceIndex);
  Serial.println(F(" stopped"));
}

// 具体动作序列
//吕布攻击
int action_lvbu_attack_front(){
  RotationAction actions[3];
  actions[0] = {6, 0, 40, 800};  
  actions[1] = {6, 1, 0, 200};  //舵机序号，方向，转动速度，转动时间
  actions[2] = {6, 1, 40, 850}; 
  outslider_index = 1;
  return addActionSequence(actions, 3);
}
//吕布恢复
int action_lvbu_heal_front(){
  RotationAction actions[3];
  actions[0] = {6, 0, 10, 1200};  
  actions[1] = {6, 1, 0, 400};
  actions[2] = {6, 1, 10, 1350};
  outslider_index = 1; 
  return addActionSequence(actions, 3);
}
//吕布技能一
int action_lvbu_skill1_front(){
  RotationAction actions[12];
  actions[0] = {6, 0, 60, 900};  
  actions[1] = {6, 1, 0, 600};
  actions[2] = {6, 1, 60, 920}; 
  actions[3] = {6, 1, 0, 600};
  actions[4] = {6, 0, 70, 900};  
  actions[5] = {6, 1, 0, 600};
  actions[6] = {6, 1, 70, 920}; 
  actions[7] = {6, 1, 0, 600};
  actions[8] = {6, 0, 80, 900};  
  actions[9] = {6, 1, 0,600};
  actions[10] = {6, 1, 80, 900}; 
  actions[11] = {6, 1, 0, 600};
  outslider_index = 1;
  return addActionSequence(actions, 12);
}
int action_lvbu_skill1_back(){
  RotationAction actions[4];
  actions[0] = {7, 0, 0, 3000};  
  actions[1] = {7, 1, 0, 3000};  
  actions[2] = {7, 0, 40, 1600};  
  actions[3] = {7, 1, 40, 1600};  
  return addActionSequence(actions, 4);
}
//吕布技能二
int action_lvbu_skill2_front(){
  RotationAction actions[7];
  actions[0] = {6, 0, 40, 600};  
  actions[1] = {6, 1, 0, 1000};
  actions[2] = {6, 1, 40, 600}; 
  actions[3] = {6, 0, 0, 500};
  actions[4] = {6, 0, 20, 2500}; 
  actions[5] = {6, 0, 0, 1000};
  actions[6] = {6, 1, 80, 1200};
  outslider_index = 1;
  return addActionSequence(actions, 7);
}
int action_lvbu_skill2_back(){
  RotationAction actions[3];
  actions[0] = {7, 0, 0,5000};  
  actions[1] = {7, 0, 40,1200};
  actions[2] = {7, 1, 40, 1200};  
   return addActionSequence(actions, 3);
}
//吕布阵亡
int action_lvbu_die_front(){
   RotationAction actions[3];
  actions[0] = {6, 0, 20, 1550};
  actions[1] = {6, 0, 0, 4500};  
  actions[2] = {6, 1, 20, 1600}; 
  return addActionSequence(actions, 3);
}
int action_lvbu_die_back(){
  RotationAction actions[2];
  actions[0] = {7, 0, 90, 4000};  
  actions[1] = {7, 0, 0, 4000}; 
  return addActionSequence(actions, 2);
}
//关羽攻击
int action_guanyu_attack_front() {
  RotationAction actions[3];
  actions[0] = {0, 1, 40, 800};  
  actions[1] = {0, 0, 0, 200};  //舵机序号，方向，转动速度，转动时间
  actions[2] = {0, 0, 40, 800}; 
  outslider_index = 3;
  return addActionSequence(actions, 3);
}
//关羽恢复
int action_guanyu_heal_front() {
  RotationAction actions[3];
  actions[0] = {0, 1, 10, 1200};  
  actions[1] = {0, 0, 0, 400};
  actions[2] = {0, 0, 10, 1200}; 
  outslider_index = 3;
  return addActionSequence(actions, 3);
}
//关羽技能一
int action_guanyu_skill1_front(){
  RotationAction actions[7];
  actions[0] = {0, 0, 20, 400};  
  actions[1] = {0, 0, 0, 100};
  actions[2] = {0, 1, 20, 400};
  actions[3] = {0, 1, 0, 200};
  actions[4] = {0, 1, 25, 1100};
  actions[5] = {0, 0, 0, 200};
  actions[6] = {0, 0, 25, 1100};
  outslider_index = 3;
  return addActionSequence(actions, 7);
}
int action_guanyu_skill1_back(){
  RotationAction actions[5];
  actions[0] = {1, 0, 0, 100};
  actions[1] = {1, 0, 30, 700};
  actions[2] = {1, 1, 0, 650};
  actions[3] = {1, 1, 50, 1000};
  actions[4] = {1, 1, 0, 900};
  return addActionSequence(actions, 5);
}
//关羽技能二
int action_guanyu_skill2_front(){
  RotationAction actions[4];
  actions[0] = {0, 1, 10, 1100};  
  actions[1] = {0, 1, 0, 600};
  actions[2] = {0, 1, 20, 400};
  actions[3] = {0, 0, 40, 750}; 
  outslider_index = 3;
  return addActionSequence(actions, 4);
}
int action_guanyu_skill2_back(){
  RotationAction actions[3];
  actions[0] = {1, 1, 0, 1700};  
  actions[1] = {1, 1, 20, 400};
  actions[2] = {1, 1, 0, 350}; 
  return addActionSequence(actions, 3);
}
//关羽阵亡
int action_guanyu_die_front(){
  RotationAction actions[3];
  actions[0] = {0, 1, 20, 1550};
  actions[1] = {0, 1, 0, 4500};  
  actions[2] = {0, 0, 20, 1730}; 
  return addActionSequence(actions, 3);
}
int action_guanyu_die_back(){
  RotationAction actions[2];
  actions[0] = {1, 1, 90, 4000};  
  actions[1] = {1, 1, 0, 4000}; 
  return addActionSequence(actions, 2);
}
//关羽复活
int action_guanyu_revive_back(){}
//刘备攻击
int action_liubei_attack_front(){
  RotationAction actions[3];
  actions[0] = {2, 1, 40, 800};  
  actions[1] = {2, 0, 0, 200};  //舵机序号，方向，转动速度，转动时间
  actions[2] = {2, 0, 40, 800}; 
  outslider_index = 2;
  return addActionSequence(actions, 3);
}
//刘备治疗
int action_liubei_heal_front(){
  RotationAction actions[3];
  actions[0] = {2, 1, 10, 1200};  
  actions[1] = {2, 0, 0, 400};
  actions[2] = {2, 0, 10, 1200}; 
  outslider_index = 2;
  return addActionSequence(actions, 3);
}
//刘备技能一
int action_liubei_skill1_front(){
  RotationAction actions[3];
  actions[0] = {2, 1, 40, 600};  
  actions[1] = {2, 0, 0, 4000};
  actions[2] = {2, 0, 40, 600}; 
  outslider_index = 2;
  return addActionSequence(actions, 3);
}
int action_liubei_skill1_back(){
  RotationAction actions[8];
  actions[0] = {3, 0, 0, 600};
  actions[1] = {3, 0, 30, 500};
  actions[2] = {3, 1, 30, 500};
  actions[3] = {3, 0, 30, 500};
  actions[4] = {3, 1, 30, 500};
  actions[5] = {3, 0, 30, 500};
  actions[6] = {3, 1, 30, 500};
  actions[7] = {3, 1, 0, 600};
  return addActionSequence(actions, 9);
}
//刘备技能二
int action_liubei_skill2_front(){
  RotationAction actions[3];
  actions[0] = {2, 1, 40, 1000};  
  actions[1] = {2, 0, 0, 2500};
  actions[2] = {2, 0, 40, 1050}; 
  outslider_index = 2;
  return addActionSequence(actions, 3);
}
int action_liubei_skill2_back(){
  RotationAction actions[4];
  actions[0] = {3, 0, 0, 1000};
  actions[1] = {3, 0, 45,1000};
  actions[2] = {3, 1, 45,1000};
  actions[3] = {3, 1, 0, 1000};
  return addActionSequence(actions, 4);
}
//刘备阵亡
int action_liubei_die_front(){
  RotationAction actions[3];
  actions[0] = {2, 1, 20, 1550};
  actions[1] = {2, 1, 0, 4500};  
  actions[2] = {2, 0, 20, 1550}; 
  return addActionSequence(actions, 3);
}
int action_liubei_die_back(){
  RotationAction actions[2];
  actions[0] = {3, 1, 90, 4000};  
  actions[1] = {3, 1, 0, 4000}; 
  return addActionSequence(actions, 2);
}
//刘备归位
int action_liubei_revive_back(){
}
//张飞攻击
int action_zhangfei_attack_front(){
  RotationAction actions[3];
  actions[0] = {4, 1, 40, 900};  
  actions[1] = {4, 0, 0, 200};  //舵机序号，方向，转动速度，转动时间
  actions[2] = {4, 0, 40, 840}; 
  outslider_index = 4;
  return addActionSequence(actions, 3);
}
//张飞恢复
int action_zhangfei_heal_front(){
  RotationAction actions[3];
  actions[0] = {4, 1, 10, 1200};  
  actions[1] = {4, 0, 0, 400};
  actions[2] = {4, 0, 10, 1000}; 
  outslider_index = 4;
  return addActionSequence(actions, 3);
}
//张飞技能一
int action_zhangfei_skill1_front(){
   RotationAction actions[3];
  actions[0] = {4, 1, 40, 1030};  
  actions[1] = {4, 0, 0, 3000};
  actions[2] = {4, 0, 40, 950}; 
  outslider_index = 4;
  return addActionSequence(actions, 3);
}
int action_zhangfei_skill1_back(){
  RotationAction actions[3];
  actions[0] = {5, 0, 0, 1050};
  actions[1] = {5, 0,50, 2500};
  actions[2] = {5, 0,0, 2000};
  return addActionSequence(actions, 3);
}
//张飞技能二
int action_zhangfei_skill2_front(){
  RotationAction actions[3];
  actions[0] = {4, 1, 40, 1100};  
  actions[1] = {4, 0, 0, 4500};
  actions[2] = {4, 0, 40, 1000}; 
  outslider_index = 4;
  return addActionSequence(actions, 3);
}
int action_zhangfei_skill2_back(){
  RotationAction actions[8];
  actions[0] = {5, 0, 0, 1200};
  actions[1] = {5, 0, 30, 500};
  actions[2] = {5, 1, 0, 500};
  actions[3] = {5, 0, 30, 500};
  actions[4] = {5, 1, 0, 1000};
  actions[5] = {5, 1, 30, 1000};
  actions[6] = {5, 0, 60, 500};
  actions[7] = {5, 1, 0, 1000};
  return addActionSequence(actions, 8);
}
//张飞阵亡
int action_zhangfei_die_front(){
  RotationAction actions[3];
  actions[0] = {4, 1, 30, 1550};
  actions[1] = {4, 1, 0, 4500};  
  actions[2] = {4, 0, 30, 1300}; 
  return addActionSequence(actions, 3);
}
int action_zhangfei_die_back(){
  RotationAction actions[2];
  actions[0] = {5, 1, 90, 4000};  
  actions[1] = {5, 1, 0, 4000}; 
  return addActionSequence(actions, 2);
}