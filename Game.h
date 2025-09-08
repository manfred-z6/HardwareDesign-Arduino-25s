#ifndef GAME_H
#define GAME_H

#include <Arduino.h>

// 角色基类
class Character {
protected:
    String name;
    int maxHp;
    int hp;
    int attackDamage;
    int healAmount;
    bool isAlive;
    bool skillUsed;

public:
    Character(const String& n, int mHp, int atk, int heal);
    virtual ~Character() {}

    // 基础方法
    bool alive() const { return isAlive; }
    String getName() const { return name; }
    int getHp() const { return hp; }
    int getMaxHp() const { return maxHp; }
    bool isSkillUsed() const { return skillUsed; }
    int getAttackDamage() const { return attackDamage; }
    int getHealAmount() const { return healAmount; }
    
    void takeDamage(int damage);
    void restoreHp(int amount);
    void markSkillUsed() { skillUsed = true; }
    virtual void die();
    virtual void revive(); 
    
    // 纯虚函数 - 必须在派生类中实现
    virtual void attack(Character* target) = 0;
    virtual void heal() = 0;
    virtual bool skill1() = 0;
    virtual bool skill2(Character* target) = 0;
};

// 前向声明角色类
class LuBu;
class LiuBei;
class GuanYu;
class ZhangFei;

class Game {
private:
    LuBu* lubu;
    LiuBei* liuBei;
    GuanYu* guanYu;
    ZhangFei* zhangFei;
    
    bool isLuBuTurn;
    bool gameOver;

public:
    Game();
    ~Game();

    void initializeGame();
    void checkGameState();
    void nextTurn();
    bool isGameOver() const { return gameOver; }
    bool getIsLuBuTurn() const { return isLuBuTurn; }

    // 吕布的动作
    void lubuAttack(int heroIndex);
    void lubuHeal();
    void lubuSkill1();
    void lubuSkill2(int targetIndex);
    
    // 刘备的动作
    void liuBeiAttack();
    void liuBeiHeal();
    void liuBeiSkill1();
    void liuBeiSkill2(int targetIndex);
    
    // 关羽的动作
    void guanYuAttack();
    void guanYuHeal();
    void guanYuSkill1();
    void guanYuSkill2();
    
    // 张飞的动作
    void zhangFeiAttack();
    void zhangFeiHeal();
    void zhangFeiSkill1();
    void zhangFeiSkill2();

    // 获取状态信息
    String getLuBuStatus() const;
    String getLiuBeiStatus() const;
    String getGuanYuStatus() const;
    String getZhangFeiStatus() const;

    // 获取角色指针（用于测试和特殊操作）
    LuBu* getLuBu() const { return lubu; }
    LiuBei* getLiuBei() const { return liuBei; }
    GuanYu* getGuanYu() const { return guanYu; }
    ZhangFei* getZhangFei() const { return zhangFei; }

    // 添加获取角色状态信息的方法
    String getLuBuStatusForDisplay() const;
    String getLiuBeiStatusForDisplay() const;
    String getGuanYuStatusForDisplay() const;
    String getZhangFeiStatusForDisplay() const;

    // 在Game类中添加以下方法声明
    bool isLuBuAlive() const;
    bool isLiuBeiAlive() const;
    bool isGuanYuAlive() const;
    bool isZhangFeiAlive() const;
};

// 声明全局游戏对象
extern Game game;
// 显示游戏人物状态
void showGameStatus();

#endif