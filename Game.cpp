#include "Game.h"
#include <Arduino.h>

// 定义全局游戏对象
Game game;

// Character 基类实现
Character::Character(const String& n, int mHp, int atk, int heal) 
    : name(n), maxHp(mHp), hp(mHp), attackDamage(atk), 
      healAmount(heal), isAlive(true), skillUsed(false) {}

void Character::takeDamage(int damage) {
    hp -= damage;
    if (hp <= 0) {
        hp = 0;
        die();
    }
}

void Character::restoreHp(int amount) {
    hp += amount;
    if (hp > maxHp) {
        hp = maxHp;
    }
}

void Character::die() {
    isAlive = false;
    Serial.print(name);
    Serial.println("已战败！");
}

void Character::revive() {
    isAlive = true;
    hp = maxHp / 2; // 复活后生命值为最大值的一半
    skillUsed = false;
    Serial.print(name);
    Serial.println("已被复活！");
}

// 吕布类
class LuBu : public Character {
private:
    int skillCount;
    bool nextAttackReduced;
    bool allAttacksReduced;

public:
    LuBu() : Character("吕布", 40, 10, 6), skillCount(2), 
             nextAttackReduced(false), allAttacksReduced(false) {}
    
    void attack(Character* target) override {
        int damage = attackDamage;
        
        if (allAttacksReduced) {
            damage = 8;
        } else if (nextAttackReduced) {
            damage = 4;
            nextAttackReduced = false;
        }
        
        Serial.print("吕布发动攻击，对");
        Serial.print(target->getName());
        Serial.print("造成");
        Serial.print(damage);
        Serial.println("点伤害！");
        
        target->takeDamage(damage);
    }
    
    void heal() override {
        Serial.println("吕布回血，恢复6点生命值！");
        restoreHp(healAmount);
    }
    
    bool skill1() override {
        if (skillCount <= 0) {
            Serial.println("技能使用次数已达上限！");
            return false;
        }
        Serial.println("吕布发动技能1：横扫千军，对敌方全体造成8点伤害！");
        skillCount--;
        return true;
    }
    
    bool skill2(Character* target) override {
        if (skillCount <= 0) {
            Serial.println("技能使用次数已达上限！");
            return false;
        }
        Serial.print("吕布发动技能2：无双乱舞，对");
        Serial.print(target->getName());
        Serial.println("造成12点伤害并恢复自身6点生命值！");
        skillCount--;
        return true;
    }
    
    void setNextAttackReduced(bool reduced) { nextAttackReduced = reduced; }
    void setAllAttacksReduced(bool reduced) { allAttacksReduced = reduced; }
    int getSkillCount() const { return skillCount; }
};

// 刘备类
class LiuBei : public Character {
public:
    LiuBei() : Character("刘备", 14, 4, 8) {}
    
    void attack(Character* target) override {
        Serial.print("刘备发动攻击，对");
        Serial.print(target->getName());
        Serial.println("造成4点伤害！");
        target->takeDamage(attackDamage);
    }
    
    void heal() override {
        Serial.println("刘备回血，恢复8点生命值！");
        restoreHp(healAmount);
    }
    
    bool skill1() override {
        if (isSkillUsed()) {
            Serial.println("技能已使用过，无法再次使用！");
            return false;
        }
        Serial.println("刘备发动技能1：仁德之心，为我方全体恢复10点生命值！");
        markSkillUsed();
        return true;
    }
    
    bool skill2(Character* target) override {
        if (isSkillUsed()) {
            Serial.println("技能已使用过，无法再次使用！");
            return false;
        }
        if (!target->alive()) {
            Serial.print("刘备发动技能2：仁者无敌，复活");
            Serial.print(target->getName());
            Serial.println("！");
            target->revive();
            markSkillUsed();
            return true;
        } else {
            Serial.println("目标尚未死亡，无法复活！");
            return false;
        }
    }
};

// 关羽类
class GuanYu : public Character {
private:
    bool drinkStatus;

public:
    GuanYu() : Character("关羽", 16, 9, 4), drinkStatus(false) {}
    
    void attack(Character* target) override {
        int damage = drinkStatus ? 13 : 9;
        Serial.print("关羽发动攻击，对");
        Serial.print(target->getName());
        Serial.print("造成");
        Serial.print(damage);
        Serial.println("点伤害！");
        
        target->takeDamage(damage);
        
        if (drinkStatus) {
            takeDamage(2);
            Serial.println("关羽因喝酒状态损失2点生命值！");
        }
    }
    
    void heal() override {
        Serial.println("关羽回血，恢复4点生命值！");
        restoreHp(healAmount);
    }
    
    bool skill1() override {
        if (isSkillUsed()) {
            Serial.println("技能已使用过，无法再次使用！");
            return false;
        }
        Serial.println("关羽发动技能1：舍身一击，损失10点生命值，对吕布造成24点伤害！");
        takeDamage(10);
        markSkillUsed();
        return true;
    }
    
    bool skill2(Character* target) override {
        if (isSkillUsed()) {
            Serial.println("技能已使用过，无法再次使用！");
            return false;
        }
        Serial.println("关羽发动技能2：喝酒，之后攻击伤害增加但每次损失2点生命值！");
        drinkStatus = true;
        markSkillUsed();
        return true;
    }
};

// 张飞类
class ZhangFei : public Character {
public:
    ZhangFei() : Character("张飞", 20, 8, 6) {}
    
    void attack(Character* target) override {
        Serial.print("张飞发动攻击，对");
        Serial.print(target->getName());
        Serial.println("造成8点伤害！");
        target->takeDamage(attackDamage);
    }
    
    void heal() override {
        Serial.println("张飞回血，恢复6点生命值！");
        restoreHp(healAmount);
    }
    
    bool skill1() override {
        if (isSkillUsed()) {
            Serial.println("技能已使用过，无法再次使用！");
            return false;
        }
        Serial.println("张飞发动技能1：咆哮，强制吕布下次攻击伤害减为4点！");
        markSkillUsed();
        return true;
    }
    
    bool skill2(Character* target) override {
        if (isSkillUsed()) {
            Serial.println("技能已使用过，无法再次使用！");
            return false;
        }
        Serial.println("张飞发动技能2：威吓，之后吕布的所有攻击伤害减为8点！");
        markSkillUsed();
        return true;
    }
};

// Game 类成员函数实现
Game::Game() : lubu(nullptr), liuBei(nullptr), guanYu(nullptr), zhangFei(nullptr), 
               isLuBuTurn(true), gameOver(false) {}

Game::~Game() {
    delete lubu;
    delete liuBei;
    delete guanYu;
    delete zhangFei;
}

void Game::initializeGame() {
    lubu = new LuBu();
    liuBei = new LiuBei();
    guanYu = new GuanYu();
    zhangFei = new ZhangFei();
    
    isLuBuTurn = true;
    gameOver = false;
    
    Serial.println("游戏初始化完成！");
    Serial.println("三英战吕布开始！");
}

void Game::checkGameState() {
    if (!lubu->alive()) {
        Serial.println("吕布已败，三英胜利！");
        gameOver = true;
        return;
    }
    
    if (!liuBei->alive() && !guanYu->alive() && !zhangFei->alive()) {
        Serial.println("三英已全部战败，吕布胜利！");
        gameOver = true;
        return;
    }
}

void Game::nextTurn() {
    isLuBuTurn = !isLuBuTurn;
    if (isLuBuTurn) {
        Serial.println("--- 吕布的回合 ---");
    } else {
        Serial.println("--- 三英的回合 ---");
    }
}

// 吕布动作实现
void Game::lubuAttack(int heroIndex) {
    if (!isLuBuTurn || gameOver) return;
    
    Character* target = nullptr;
    switch (heroIndex) {
        case 0: target = liuBei; break;
        case 1: target = guanYu; break;
        case 2: target = zhangFei; break;
    }
    
    if (target && target->alive()) {
        lubu->attack(target);
        nextTurn();
        checkGameState();
    }
}

void Game::lubuHeal() {
    if (!isLuBuTurn || gameOver) return;
    
    lubu->heal();
    nextTurn();
    checkGameState();
}

void Game::lubuSkill1() {
    if (!isLuBuTurn || gameOver) return;
    
    if (static_cast<LuBu*>(lubu)->skill1()) {
        // 对全体英雄造成伤害
        if (liuBei->alive()) liuBei->takeDamage(8);
        if (guanYu->alive()) guanYu->takeDamage(8);
        if (zhangFei->alive()) zhangFei->takeDamage(8);
        
        nextTurn();
        checkGameState();
    }
}

void Game::lubuSkill2(int targetIndex) {
    if (!isLuBuTurn || gameOver) return;
    
    Character* target = nullptr;
    switch (targetIndex) {
        case 0: target = liuBei; break;
        case 1: target = guanYu; break;
        case 2: target = zhangFei; break;
    }
    
    if (target && target->alive() && static_cast<LuBu*>(lubu)->skill2(target)) {
        target->takeDamage(12);
        lubu->restoreHp(6);
        nextTurn();
        checkGameState();
    }
}

// 刘备动作实现
void Game::liuBeiAttack() {
    if (isLuBuTurn || gameOver) return;
    
    liuBei->attack(lubu);
    nextTurn();
    checkGameState();
}

void Game::liuBeiHeal() {
    if (isLuBuTurn || gameOver) return;
    
    liuBei->heal();
    nextTurn();
    checkGameState();
}

void Game::liuBeiSkill1() {
    if (isLuBuTurn || gameOver) return;
    
    if (static_cast<LiuBei*>(liuBei)->skill1()) {
        // 为我方全体恢复10点生命值
        if (liuBei->alive()) liuBei->restoreHp(10);
        if (guanYu->alive()) guanYu->restoreHp(10);
        if (zhangFei->alive()) zhangFei->restoreHp(10);
        
        nextTurn();
        checkGameState();
    }
}

void Game::liuBeiSkill2(int targetIndex) {
    if (isLuBuTurn || gameOver) return;
    
    Character* target = nullptr;
    switch (targetIndex) {
        case 1: target = guanYu; break;
        case 2: target = zhangFei; break;
    }
    
    if (target && !target->alive() && static_cast<LiuBei*>(liuBei)->skill2(target)) {
        nextTurn();
        checkGameState();
    }
}

// 关羽动作实现
void Game::guanYuAttack() {
    if (isLuBuTurn || gameOver) return;
    
    guanYu->attack(lubu);
    nextTurn();
    checkGameState();
}

void Game::guanYuHeal() {
    if (isLuBuTurn || gameOver) return;
    
    guanYu->heal();
    nextTurn();
    checkGameState();
}

void Game::guanYuSkill1() {
    if (isLuBuTurn || gameOver) return;
    
    if (static_cast<GuanYu*>(guanYu)->skill1()) {
        lubu->takeDamage(24);
        nextTurn();
        checkGameState();
    }
}

void Game::guanYuSkill2() {
    if (isLuBuTurn || gameOver) return;
    
    if (static_cast<GuanYu*>(guanYu)->skill2(nullptr)) {
        nextTurn();
        checkGameState();
    }
}

// 张飞动作实现
void Game::zhangFeiAttack() {
    if (isLuBuTurn || gameOver) return;
    
    zhangFei->attack(lubu);
    nextTurn();
    checkGameState();
}

void Game::zhangFeiHeal() {
    if (isLuBuTurn || gameOver) return;
    
    zhangFei->heal();
    nextTurn();
    checkGameState();
}

void Game::zhangFeiSkill1() {
    if (isLuBuTurn || gameOver) return;
    
    if (static_cast<ZhangFei*>(zhangFei)->skill1()) {
        static_cast<LuBu*>(lubu)->setNextAttackReduced(true);
        nextTurn();
        checkGameState();
    }
}

void Game::zhangFeiSkill2() {
    if (isLuBuTurn || gameOver) return;
    
    if (static_cast<ZhangFei*>(zhangFei)->skill2(nullptr)) {
        static_cast<LuBu*>(lubu)->setAllAttacksReduced(true);
        nextTurn();
        checkGameState();
    }
}

// 状态获取方法
String Game::getLuBuStatus() const {
    String status = "吕布: HP: ";
    status += lubu->getHp();
    status += "/";
    status += lubu->getMaxHp();
    status += ", 技能次数: ";
    status += static_cast<LuBu*>(lubu)->getSkillCount();
    status += ", 状态: ";
    status += lubu->alive() ? "存活" : "战败";
    return status;
}

String Game::getLiuBeiStatus() const {
    String status = "刘备: HP: ";
    status += liuBei->getHp();
    status += "/";
    status += liuBei->getMaxHp();
    status += ", 技能使用: ";
    status += liuBei->isSkillUsed() ? "是" : "否";
    status += ", 状态: ";
    status += liuBei->alive() ? "存活" : "战败";
    return status;
}

String Game::getGuanYuStatus() const {
    String status = "关羽: HP: ";
    status += guanYu->getHp();
    status += "/";
    status += guanYu->getMaxHp();
    status += ", 技能使用: ";
    status += guanYu->isSkillUsed() ? "是" : "否";
    status += ", 状态: ";
    status += guanYu->alive() ? "存活" : "战败";
    return status;
}

String Game::getZhangFeiStatus() const {
    String status = "张飞: HP: ";
    status += zhangFei->getHp();
    status += "/";
    status += zhangFei->getMaxHp();
    status += ", 技能使用: ";
    status += zhangFei->isSkillUsed() ? "是" : "否";
    status += ", 状态: ";
    status += zhangFei->alive() ? "存活" : "战败";
    return status;
}


void showGameStatus() {
  // 显示状态函数
  Serial.println(game.getLuBuStatus());
  Serial.println(game.getLiuBeiStatus());
  Serial.println(game.getGuanYuStatus());
  Serial.println(game.getZhangFeiStatus());
}