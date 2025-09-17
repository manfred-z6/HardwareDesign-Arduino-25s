#include "GlobalVars.h"

// 定义全局变量
volatile bool isAnySequenceRunning = false;
volatile bool isSliderMoving = false;
volatile bool flag1 = false, flag2 = false, flag3 = false, flag4 = false;
//滑台控制相关变量
volatile bool flag_slider = false;
volatile uint8_t outslider_index = 5;
volatile unsigned long lasttime_action_people = 0;
const unsigned long ACTION_PEOPLE_INTERVAL = 2000;