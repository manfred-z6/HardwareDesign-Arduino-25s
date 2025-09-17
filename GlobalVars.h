#ifndef GLOBAL_VARS_H  
#define GLOBAL_VARS_H

#include <Arduino.h>

// 使用 extern 关键字声明全局变量，告诉编译器其定义在其他文件中
extern volatile bool isAnySequenceRunning;
extern volatile bool isSliderMoving;
extern volatile bool flag1, flag2, flag3, flag4;
extern volatile bool flag_slider;
extern volatile uint8_t outslider_index;  
extern volatile unsigned long lasttime_action_people;
extern const unsigned long ACTION_PEOPLE_INTERVAL;
#endif  //GLOBALVARS_H
