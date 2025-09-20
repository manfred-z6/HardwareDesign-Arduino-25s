#include "Action_people.h"
#include "GlobalVars.h"
// 定义全局变量
const int MAX_CONCURRENT_SEQUENCES = 10;
const int MAX_ACTION_SINGLESEQ = 13;
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
          time_action_end = millis();
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
  RotationAction actions[4];
  actions[0] = {0, 0, 0, 100};
  actions[1] = {0, 0, 40, 1450};  
  actions[2] = {0, 1, 0, 200};  //舵机序号，方向，转动速度，转动时间
  actions[3] = {0, 1, 40, 1500}; 
  outslider_index = 1;
  return addActionSequence(actions, 4);
}
//吕布恢复
int action_lvbu_heal_front(){
  RotationAction actions[4];
  actions[0] = {0, 0, 0, 100};
  actions[1] = {0, 0, 10, 1200};  
  actions[2] = {0, 1, 0, 400};
  actions[3] = {0, 1, 10, 1350};
  outslider_index = 1; 
  return addActionSequence(actions, 4);
}
//吕布技能一
int action_lvbu_skill1_front(){
  RotationAction actions[8];
  actions[0] = {0, 0, 0, 100};
  actions[1] = {0, 0, 60, 900};  
  actions[2] = {0, 1, 0, 200};
  actions[3] = {0, 1, 60, 900}; 
  actions[4] = {0, 1, 0, 200};
  actions[5] = {0, 0, 70, 700};  
  actions[6] = {0, 1, 0, 200};
  actions[7] = {0, 1, 70, 700}; 
  outslider_index = 1;
  return addActionSequence(actions, 8);
}

int action_lvbu_skill1_back(){
  RotationAction actions[4];
  actions[0] = {1, 0, 0, 100};
  actions[1] = {1, 0, 0, 2200};    
  actions[2] = {1, 0, 40, 900};  
  actions[3] = {1, 1, 40, 700};  
  return addActionSequence(actions, 4);
}
//吕布技能二
int action_lvbu_skill2_front(){
  RotationAction actions[8];
  actions[0] = {0, 0, 0, 100};
  actions[1] = {0, 0, 40, 400};  
  actions[2] = {0, 1, 0, 200};
  actions[3] = {0, 1, 40, 400}; 
  actions[4] = {0, 0, 0, 200};
  actions[5] = {0, 0, 40, 800}; 
  actions[6] = {0, 0, 0, 200};
  actions[7] = {0, 1, 40, 800};
  outslider_index = 1;
  return addActionSequence(actions, 8);
}
int action_lvbu_skill2_back(){
  RotationAction actions[4];
  actions[0] = {1, 0, 0, 100};
  actions[1] = {1, 0, 0, 1200};  
  actions[2] = {1, 0, 50, 1000};
  actions[3] = {1, 1, 0, 800};  
   return addActionSequence(actions, 4);
}
//吕布阵亡
int action_lvbu_die_front(){
  RotationAction actions[4];
  actions[0] = {0, 0, 0, 100};
  actions[1] = {0, 0, 20, 1550};
  actions[2] = {0, 0, 0, 2500};  
  actions[3] = {0, 1, 20, 1600}; 
  return addActionSequence(actions, 4);
}
int action_lvbu_die_back(){
  RotationAction actions[3];
  actions[0] = {1, 0, 0, 100};
  actions[1] = {1, 0, 90, 4000};  
  actions[2] = {1, 0, 0, 1700}; 
  return addActionSequence(actions, 3);
}
//刘备攻击
int action_liubei_attack_front(){
  RotationAction actions[4];
  actions[0] = {2, 0, 0, 100};
  actions[1] = {2, 1, 40, 1300};  
  actions[2] = {2, 0, 0, 200};  //舵机序号，方向，转动速度，转动时间
  actions[3] = {2, 0, 40,1250}; 
  outslider_index = 2;
  return addActionSequence(actions, 4);
}
//刘备治疗
int action_liubei_heal_front(){
  RotationAction actions[4];
  actions[0] = {2, 0, 0, 100};
  actions[1] = {2, 1, 10, 1200};  
  actions[2] = {2, 0, 0, 400};
  actions[3] = {2, 0, 10, 1200}; 
  outslider_index = 2;
  return addActionSequence(actions, 4);
}
//刘备技能一
int action_liubei_skill1_front(){
  RotationAction actions[5];
  actions[0] = {2, 0, 0, 100};
  actions[1] = {2, 1, 0, 100};
  actions[2] = {2, 1, 40, 600};  
  actions[3] = {2, 0, 0, 4000};
  actions[4] = {2, 0, 40, 600}; 
  outslider_index = 2;
  return addActionSequence(actions, 5);
}
int action_liubei_skill1_back(){
  RotationAction actions[9];
  actions[0] = {3, 0, 0, 100};
  actions[1] = {3, 0, 0, 100};
  actions[2] = {3, 0, 0, 600};
  actions[3] = {3, 0, 30, 500};
  actions[4] = {3, 1, 30, 500};
  actions[5] = {3, 0, 30, 500};
  actions[6] = {3, 1, 30, 500};
  actions[7] = {3, 0, 30, 500};
  actions[8] = {3, 1, 30, 500};
  return addActionSequence(actions, 9);
}
//刘备技能二
int action_liubei_skill2_front(){
  RotationAction actions[4];
  actions[0] = {2, 0, 0, 100};
  actions[1] = {2, 1, 40, 1000};  
  actions[2] = {2, 0, 0, 2500};
  actions[3] = {2, 0, 40, 1050}; 
  outslider_index = 2;
  return addActionSequence(actions, 4);
}
int action_liubei_skill2_back(){
  RotationAction actions[5];
  actions[0] = {3, 0, 0, 100};
  actions[1] = {3, 0, 0, 1000};
  actions[2] = {3, 0, 45,1000};
  actions[3] = {3, 1, 45,1000};
  actions[4] = {3, 1, 0, 1000};
  return addActionSequence(actions, 5);
}
//刘备阵亡
int action_liubei_die_front(){
  RotationAction actions[4];
  actions[0] = {2, 0, 0, 100};
  actions[1] = {2, 1, 20, 1550};
  actions[2] = {2, 1, 0, 2500};  
  actions[3] = {2, 0, 20, 1550}; 
  return addActionSequence(actions, 4);
}
int action_liubei_die_back(){
  RotationAction actions[3];
  actions[0] = {3, 0, 0, 100};
  actions[1] = {3, 1, 90, 4000};  
  actions[2] = {3, 1, 0, 1700}; 
  return addActionSequence(actions, 3);
}

//关羽攻击
int action_guanyu_attack_front() {
  RotationAction actions[4];
  actions[0] = {4, 0, 0, 100};
  actions[1] = {4, 1, 40, 1200};  
  actions[2] = {4, 0, 0, 200};  //舵机序号，方向，转动速度，转动时间
  actions[3] = {4, 0, 40, 1200}; 
  outslider_index = 3;
  return addActionSequence(actions, 4);
}
//关羽恢复
int action_guanyu_heal_front() {
  RotationAction actions[4];
  actions[0] = {4, 0, 0, 100};
  actions[1] = {4, 1, 10, 800};  
  actions[2] = {4, 0, 0, 400};
  actions[3] = {4, 0, 10, 750}; 
  outslider_index = 3;
  return addActionSequence(actions, 4);
}
//关羽技能一
int action_guanyu_skill1_front(){
  RotationAction actions[8];
  actions[0] = {4, 0, 0, 100};
  actions[1] = {4, 0, 20, 600};  
  actions[2] = {4, 0, 0, 100};
  actions[3] = {4, 1, 20, 400};
  actions[4] = {4, 1, 0, 200};
  actions[5] = {4, 1, 40, 900};
  actions[6] = {4, 0, 0, 1000};
  actions[7] = {4, 0, 35, 1100};
  outslider_index = 3;
  return addActionSequence(actions, 8);
}
int action_guanyu_skill1_back(){
  RotationAction actions[6];
  actions[0] = {5, 0, 0, 100};
  actions[1] = {5, 0, 0, 100};
  actions[2] = {5, 0, 40, 900};
  actions[3] = {5, 1, 0, 650};
  actions[4] = {5, 1, 50, 1800};
  actions[5] = {5, 1, 0, 900};
  return addActionSequence(actions, 6);
}
//关羽技能二
int action_guanyu_skill2_front(){
  RotationAction actions[4];
  actions[0] = {4, 0, 0, 100};
  actions[1] = {4, 1, 20, 2000};  
  actions[2] = {4, 1, 0, 800};  
  actions[3] = {4, 0, 40, 1200}; 
  outslider_index = 3;
  return addActionSequence(actions, 4);
}
int action_guanyu_skill2_back(){
  RotationAction actions[5];
  actions[0] = {5, 0, 0, 100};
  actions[1] = {5, 1, 0, 1700};  
  actions[2] = {5, 1, 50,1000};
  actions[3] = {5, 1, 30,600};
  actions[4] = {5, 1, 0, 350}; 
  return addActionSequence(actions, 5);
}
//关羽阵亡
int action_guanyu_die_front(){
  RotationAction actions[4];
  actions[0] = {4, 0, 0, 100};
  actions[1] = {4, 1, 30, 1400};
  actions[2] = {4, 1, 0, 7000};  
  actions[3] = {4, 0, 30, 1330}; 
  return addActionSequence(actions, 4);
}
int action_guanyu_die_back(){
  RotationAction actions[3];
  actions[0] = {5, 0, 0, 100};
  actions[1] = {5, 1, 90, 7000};  
  actions[2] = {5, 1, 0, 1700}; 
  return addActionSequence(actions, 3);
}

//张飞攻击
int action_zhangfei_attack_front(){
  RotationAction actions[4];
  actions[0] = {6, 0, 0, 100};
  actions[1] = {6, 1, 40, 1200};  
  actions[2] = {6, 0, 0, 200};  //舵机序号，方向，转动速度，转动时间
  actions[3] = {6, 0, 40, 1100}; 
  outslider_index = 4;
  return addActionSequence(actions, 4);
}
//张飞恢复
int action_zhangfei_heal_front(){
  RotationAction actions[4];
  actions[0] = {6, 0, 0, 100};
  actions[1] = {6, 1, 10, 1100};  
  actions[2] = {6, 0, 0, 300};
  actions[3] = {6, 0, 10, 1000}; 
  outslider_index = 4;
  return addActionSequence(actions, 4);
}
//张飞技能一
int action_zhangfei_skill1_front(){
  RotationAction actions[4];
  actions[0] = {6, 0, 0, 100};
  actions[1] = {6, 1, 40, 950};  
  actions[2] = {6, 0, 0, 3000};
  actions[3] = {6, 0, 40, 875}; 
  outslider_index = 4;
  return addActionSequence(actions, 4);
}
int action_zhangfei_skill1_back(){
  RotationAction actions[4];
  actions[0] = {7, 0, 0, 100};
  actions[1] = {7, 0, 0, 1050};
  actions[2] = {7, 0, 50, 2500};
  actions[3] = {7, 0, 0, 2000};
  return addActionSequence(actions, 4);
}
//张飞技能二
int action_zhangfei_skill2_front(){
  RotationAction actions[4];
  actions[0] = {6, 0, 0, 100};
  actions[1] = {6, 1, 38, 1000};  
  actions[2] = {6, 0, 0, 4500};
  actions[3] = {6, 0, 38, 900}; 
  outslider_index = 4;
  return addActionSequence(actions, 4);
}
int action_zhangfei_skill2_back(){
  RotationAction actions[8];
  actions[0] = {7, 0, 0, 100};
  actions[1] = {7, 0, 0, 1200};
  actions[2] = {7, 0, 25, 500};
  actions[3] = {7, 1, 0, 500};
  actions[4] = {7, 0, 25, 500};
  actions[5] = {7, 1, 0, 1000};
  actions[6] = {7, 1, 50, 1500};
  actions[7] = {7, 1, 0, 1000};
  return addActionSequence(actions, 8);
}
//张飞阵亡
int action_zhangfei_die_front(){
  RotationAction actions[4];
  actions[0] = {6, 0, 0, 100};
  actions[1] = {6, 1, 30, 1450};
  actions[2] = {6, 1, 0, 7000};  
  actions[3] = {6, 0, 30, 1220}; 
  return addActionSequence(actions, 4);
}
int action_zhangfei_die_back(){
  RotationAction actions[2];
  actions[0] = {7, 0, 0, 100};
  actions[1] = {7, 1, 90, 7000};  
  return addActionSequence(actions, 2);
}