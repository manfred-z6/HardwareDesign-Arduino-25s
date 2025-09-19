#include "Game.h"
#include "Music.h"
#include "Music_background.h"
#include "GlobalVars.h"
#include "Action_people.h"
#include "Slider.h"
#include <Arduino.h>

unsigned long lastAudioEndTime = 0;
const unsigned long AUDIO_COOLDOWN = 4000; // 冷却时间

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
    Serial.println(F("已战败！"));
}

void Character::revive() {
    isAlive = true;
    hp = maxHp; // 复活后生命值为最大值
    skillUsed = true;
    Serial.print(name);
    Serial.println(F("已被复活！"));
}

// 吕布类
class LuBu : public Character {
private:
    int skillCount;
    bool nextAttackReduced;
    bool allAttacksReduced;

public:
    LuBu() : Character("吕布", 36, 10, 6), skillCount(2), 
             nextAttackReduced(false), allAttacksReduced(false) {}
    
    void attack(Character* target) override {
        int damage = attackDamage;
        
        if (allAttacksReduced) {
            damage = 8;
        } else if (nextAttackReduced) {
            damage = 4;
            nextAttackReduced = false;
        }
        
        Serial.print(F("吕布发动攻击，对"));
        Serial.print(target->getName());
        Serial.print(F("造成"));
        Serial.print(damage);
        Serial.println(F("点伤害！"));
        
        target->takeDamage(damage);

        startSliderSequence(slide_lvbu_out()); 
        musicPlayer.playTrackOnce(3);   //攻击音乐
        lastAudioEndTime = millis();
        startSequence(action_lvbu_attack_front());      //攻击动作     
    }
    
    void heal() override {
        Serial.println(F("吕布回血，恢复6点生命值！"));
        restoreHp(healAmount);  
    }
    
    bool skill1() override {
        if (skillCount <= 0) {
            Serial.println(F("技能使用次数已达上限！"));
            return false;
        }
        Serial.println(F("吕布发动技能1：横扫千军，对敌方全体造成8点伤害！"));
        skillCount--;
        return true;
    }
    
    bool skill2(Character* target) override {
        if (skillCount <= 0) {
            Serial.println(F("技能使用次数已达上限！"));
            return false;
        }
        Serial.print(F("吕布发动技能2：无双乱舞，对"));
        Serial.print(target->getName());
        Serial.println(F("造成12点伤害并恢复自身6点生命值！"));
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
        Serial.print(F("刘备发动攻击，对"));
        Serial.print(target->getName());
        Serial.println(F("造成4点伤害！"));
        target->takeDamage(attackDamage);

        startSliderSequence(slide_liubei_out());
        musicPlayer.playTrackOnce(7);       //攻击音乐
        lastAudioEndTime = millis();
        startSequence(action_liubei_attack_front());        //攻击动作
    }
    
    void heal() override {
        Serial.println(F("刘备回血，恢复8点生命值！"));
        restoreHp(healAmount);

        startSliderSequence(slide_liubei_out());
        musicPlayer.playTrackOnce(8);       //恢复音乐
        lastAudioEndTime = millis();
        startSequence(action_liubei_heal_front());      //恢复动作
    }
    
    bool skill1() override {
        if (isSkillUsed()) {
            Serial.println(F("技能已使用过，无法再次使用！"));
            return false;
        }
        Serial.println(F("刘备发动技能1：仁德之心，为我方全体恢复10点生命值！"));
        markSkillUsed();
        return true;
    }
    
    bool skill2(Character* target) override {
        if (isSkillUsed()) {
            Serial.println(F("技能已使用过，无法再次使用！"));
            return false;
        }
        if (!target->alive()) {
            Serial.print(F("刘备发动技能2：仁者无敌，复活"));
            Serial.print(target->getName());
            Serial.println("！");
            target->revive();
            markSkillUsed();
            if(target->getName() == "关羽"){
                flag3 = 1;
            }
            if(target->getName() == "张飞"){
                flag4 = 1;
            }
        } else {
            Serial.println(F("目标尚未死亡，无法复活！"));
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
        Serial.print(F("关羽发动攻击，对"));
        Serial.print(target->getName());
        Serial.print(F("造成"));
        Serial.print(damage);
        Serial.println(F("点伤害！"));
        
        target->takeDamage(damage);
        
        if (drinkStatus) {
            takeDamage(2);
            Serial.println(F("关羽因喝酒状态损失2点生命值！"));
        }
    }
    
    void heal() override {
        Serial.println(F("关羽回血，恢复4点生命值！"));
        restoreHp(healAmount);
    }
    
    bool skill1() override {
        if (isSkillUsed()) {
            Serial.println(F("技能已使用过，无法再次使用！"));
            return false;
        }
        Serial.println(F("关羽发动技能1：舍身一击，损失10点生命值，对吕布造成24点伤害！"));
        takeDamage(10);
        markSkillUsed();
        return true;
    }
    
    bool skill2(Character* target) override {
        if (isSkillUsed()) {
            Serial.println(F("技能已使用过，无法再次使用！"));
            return false;
        }
        Serial.println(F("关羽发动技能2：喝酒，之后攻击伤害增加但每次损失2点生命值！"));
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
        Serial.print(F("张飞发动攻击，对"));
        Serial.print(target->getName());
        Serial.println(F("造成8点伤害！"));
        target->takeDamage(attackDamage);
    }
    
    void heal() override {
        Serial.println(F("张飞回血，恢复6点生命值！"));
        restoreHp(healAmount);
    }
    
    bool skill1() override {
        if (isSkillUsed()) {
            Serial.println(F("技能已使用过，无法再次使用！"));
            return false;
        }
        Serial.println(F("张飞发动技能1：咆哮，强制吕布下次攻击伤害减为4点！"));
        markSkillUsed();
        return true;
    }
    
    bool skill2(Character* target) override {
        if (isSkillUsed()) {
            Serial.println(F("技能已使用过，无法再次使用！"));
            return false;
        }
        Serial.println(F("张飞发动技能2：威吓，之后吕布的所有攻击伤害减为8点！"));
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
    
    flag1 = 1;
    flag2 = 1;
    flag3 = 1;
    flag4 = 1;

    isLuBuTurn = false;
    gameOver = false;
    
    Serial.println(F("游戏初始化完成！"));
    Serial.println(F("三英战吕布开始！"));
    Serial.println(F("--- 三英的回合 ---"));
    musicPlayer2.playTrackLoop(1);
    musicPlayer.playTrackOnce(25);
}

void Game::checkGameState() {
    if(flag1&&!musicPlayer.isCurrentlyPlaying()&&!lubu->alive() && (millis() - lastAudioEndTime) > AUDIO_COOLDOWN)
    {
        musicPlayer.playTrackOnce(19);
        flag1 = 0;
        lastAudioEndTime = millis();
        startSequence(action_lvbu_die_front());
        startSequence(action_lvbu_die_back());
        return;
    }
    if(flag2&&!musicPlayer.isCurrentlyPlaying()&&!liuBei->alive() && (millis() - lastAudioEndTime) > AUDIO_COOLDOWN)
    {
        musicPlayer.playTrackOnce(20);
        flag2 = 0;
        lastAudioEndTime = millis();
        startSequence(action_liubei_die_front());
        startSequence(action_liubei_die_back());
        return;
    }
    if(flag3&&!musicPlayer.isCurrentlyPlaying()&&!guanYu->alive() && (millis() - lastAudioEndTime) > AUDIO_COOLDOWN)
    {
        musicPlayer.playTrackOnce(21);
        flag3 = 0;
        lastAudioEndTime = millis();
        startSequence(action_guanyu_die_front());
        startSequence(action_guanyu_die_back());
        return;
    }
    if(flag4&&!musicPlayer.isCurrentlyPlaying()&&!zhangFei->alive() && (millis() - lastAudioEndTime) > AUDIO_COOLDOWN)
    {
        musicPlayer.playTrackOnce(22);
        flag4 = 0;
        lastAudioEndTime = millis();
        startSequence(action_zhangfei_die_front());
        startSequence(action_zhangfei_die_back());
        return;
    }

    if (!lubu->alive() && !musicPlayer.isCurrentlyPlaying() && (millis() - lastAudioEndTime) > AUDIO_COOLDOWN) {
        Serial.println(F("吕布已败，三英胜利！"));
        gameOver = true;
        return;
    }
    
    if (!liuBei->alive() && !guanYu->alive() && !zhangFei->alive() && !musicPlayer.isCurrentlyPlaying() && (millis() - lastAudioEndTime) > AUDIO_COOLDOWN) {
        Serial.println(F("三英已全部战败，吕布胜利！"));
        gameOver = true;
        return;
    }
}

void Game::nextTurn() {
    isLuBuTurn = !isLuBuTurn;
    if (isLuBuTurn) {
        Serial.println(F("--- 吕布的回合 ---"));
    } else {
        Serial.println(F("--- 三英的回合 ---"));
    }
}

// 吕布动作实现
void Game::lubuAttack(int heroIndex) {
    Character* target = nullptr;
    switch (heroIndex) {
        case 0: target = liuBei; break;
        case 1: target = guanYu; break;
        case 2: target = zhangFei; break;
    }

    if (!isLuBuTurn || gameOver || !target->alive()) { 
        music_error();
        return;
    }
    
    if (target && target->alive()) {
        lubu->attack(target);
        nextTurn();
        checkGameState();
    } 
}

void Game::lubuHeal() {
    if (!isLuBuTurn || gameOver) {
        music_error();
        return;
    }

    startSliderSequence(slide_lvbu_out());
    musicPlayer.playTrackOnce(4);
    lastAudioEndTime = millis();
    startSequence(action_lvbu_heal_front());
    lubu->heal();
    nextTurn();
    checkGameState();

}

void Game::lubuSkill1() {
    if (!isLuBuTurn || gameOver || (lubu->getSkillCount() == 0)) {
        music_error();
        return;
    }    

    if (static_cast<LuBu*>(lubu)->skill1()) {
        startSliderSequence(slide_lvbu_out());
        musicPlayer.playTrackOnce(5);
        lastAudioEndTime = millis();
        startSequence(action_lvbu_skill1_front());
        startSequence(action_lvbu_skill1_back());
        // 对全体英雄造成伤害
        if (liuBei->alive()) liuBei->takeDamage(8);
        if (guanYu->alive()) guanYu->takeDamage(8);
        if (zhangFei->alive()) zhangFei->takeDamage(8);

        nextTurn();
        checkGameState();
    }  else {
        music_error();
    }
}

void Game::lubuSkill2(int targetIndex) {
    if (!isLuBuTurn || gameOver || (lubu->getSkillCount() == 0)) {
        music_error();
        return;
    }

    Character* target = nullptr;
    switch (targetIndex) {
        case 0: target = liuBei; break;
        case 1: target = guanYu; break;
        case 2: target = zhangFei; break;
    }
    
    if (target && target->alive() && static_cast<LuBu*>(lubu)->skill2(target)) {
        startSliderSequence(slide_lvbu_out());
        musicPlayer.playTrackOnce(6);
        lastAudioEndTime = millis();
        startSequence(action_lvbu_skill2_front());
        startSequence(action_lvbu_skill2_back());
        target->takeDamage(12);
        lubu->restoreHp(6);
        nextTurn();
        checkGameState();
    } 
}

// 刘备动作实现
void Game::liuBeiAttack() {
    if (isLuBuTurn || gameOver || !liuBei->alive()) {
        music_error();
        return;
    }

    liuBei->attack(lubu);
    nextTurn();
    checkGameState();

}

void Game::liuBeiHeal() {
    if (isLuBuTurn || gameOver || !liuBei->alive()) {
        music_error();
        return;
    }

    liuBei->heal();
    nextTurn();
    checkGameState();

}

void Game::liuBeiSkill1() {
    if (isLuBuTurn || gameOver || !liuBei->alive() || liuBei->isSkillUsed()) {
        music_error();
        return;
    }

    if (static_cast<LiuBei*>(liuBei)->skill1()) {
        // 为我方全体恢复10点生命值
        if (liuBei->alive()) liuBei->restoreHp(10);
        if (guanYu->alive()) guanYu->restoreHp(10);
        if (zhangFei->alive()) zhangFei->restoreHp(10);

        startSliderSequence(slide_liubei_out());
        musicPlayer.playTrackOnce(9);
        lastAudioEndTime = millis();
        startSequence(action_liubei_skill1_front());
        startSequence(action_liubei_skill1_back());
        nextTurn();
        checkGameState();
    } 
}

void Game::liuBeiSkill2(int targetIndex) {
    Character* target = nullptr;
    switch (targetIndex) {
        case 1: target = guanYu; break;
        case 2: target = zhangFei; break;
    }

    if (isLuBuTurn || gameOver || !liuBei->alive() || liuBei->isSkillUsed() || target->alive()) {
        music_error();
        return;
    }
    
    if (target && !target->alive() && static_cast<LiuBei*>(liuBei)->skill2(target)) {
        startSliderSequence(slide_liubei_out());
        musicPlayer.playTrackOnce(10);
        lastAudioEndTime = millis();
        startSequence(action_liubei_skill2_front());
        startSequence(action_liubei_skill2_back());
        nextTurn();
        checkGameState();
    } 
}

// 关羽动作实现
void Game::guanYuAttack() {
    if (isLuBuTurn || gameOver || !guanYu->alive()) {
        music_error();
        return;
    }

    startSliderSequence(slide_guanyu_out());
    musicPlayer.playTrackOnce(11);
    lastAudioEndTime = millis();
    startSequence(action_guanyu_attack_front());

    guanYu->attack(lubu);
    nextTurn();
    checkGameState();

}

void Game::guanYuHeal() {
    if (isLuBuTurn || gameOver || !guanYu->alive()) {
        music_error();
        return;
    }
    
    startSliderSequence(slide_guanyu_out());
    musicPlayer.playTrackOnce(12);
    lastAudioEndTime = millis();
    startSequence(action_guanyu_heal_front());

    guanYu->heal();
    nextTurn();
    checkGameState();

}

void Game::guanYuSkill1() {
    if (isLuBuTurn || gameOver || !guanYu->alive() || guanYu->isSkillUsed()) {
        music_error();
        return;
    }

    if (static_cast<GuanYu*>(guanYu)->skill1()) {
        startSliderSequence(slide_guanyu_out());
        musicPlayer.playTrackOnce(13);
        lastAudioEndTime = millis();
        startSequence(action_guanyu_skill1_front());
        startSequence(action_guanyu_skill1_back());
        lubu->takeDamage(24);
        nextTurn();
        checkGameState();
    } 
}

void Game::guanYuSkill2() {
    if (isLuBuTurn || gameOver || !guanYu->alive() || guanYu->isSkillUsed()) {
        music_error();
        return;
    }

    if (static_cast<GuanYu*>(guanYu)->skill2(nullptr)) {
        startSliderSequence(slide_guanyu_out());
        musicPlayer.playTrackOnce(14);
        lastAudioEndTime = millis();
        startSequence(action_guanyu_skill2_front());
        startSequence(action_guanyu_skill2_back());
        nextTurn();
        checkGameState();
    } 
}

// 张飞动作实现
void Game::zhangFeiAttack() {
    if (isLuBuTurn || gameOver || !zhangFei->alive()) {
        music_error();
        return;
    }
    
    startSliderSequence(slide_zhangfei_out());
    musicPlayer.playTrackOnce(15);
    lastAudioEndTime = millis();
    startSequence(action_zhangfei_attack_front());

    zhangFei->attack(lubu);
    nextTurn();
    checkGameState();

}

void Game::zhangFeiHeal() {
    if (isLuBuTurn || gameOver || !zhangFei->alive()) {
        music_error();
        return;
    }
    
    startSliderSequence(slide_zhangfei_out());
    musicPlayer.playTrackOnce(16);
    lastAudioEndTime = millis();
    startSequence(action_zhangfei_heal_front());

    zhangFei->heal();
    nextTurn();
    checkGameState();

}

void Game::zhangFeiSkill1() {
    if (isLuBuTurn || gameOver || !zhangFei->alive() || zhangFei->isSkillUsed()) {
        music_error();
        return;
    }

    if (static_cast<ZhangFei*>(zhangFei)->skill1()) {
        startSliderSequence(slide_zhangfei_out());
        musicPlayer.playTrackOnce(17);
        lastAudioEndTime = millis();
        startSequence(action_zhangfei_skill1_front());
        startSequence(action_zhangfei_skill1_back());
        static_cast<LuBu*>(lubu)->setNextAttackReduced(true);
        nextTurn();
        checkGameState();
    } 
}

void Game::zhangFeiSkill2() {
    if (isLuBuTurn || gameOver || !zhangFei->alive() || zhangFei->isSkillUsed()) {
        music_error();
        return;
    }

    if (static_cast<ZhangFei*>(zhangFei)->skill2(nullptr)) {
        startSliderSequence(slide_zhangfei_out());
        musicPlayer.playTrackOnce(18);
        lastAudioEndTime = millis();
        startSequence(action_zhangfei_skill2_front());
        startSequence(action_zhangfei_skill2_back());
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

String Game::getLuBuStatusForDisplay() const {
    String status = "LV:HP";
    status += lubu->getHp();
    status += "/";
    status += lubu->getMaxHp();
    status += " A:";
    status += lubu->getAttackDamage();
    status += " S:";
    status += static_cast<LuBu*>(lubu)->getSkillCount();
    return status;
}

String Game::getLiuBeiStatusForDisplay() const {
    String status = "LB:HP";
    status += liuBei->getHp();
    status += "/";
    status += liuBei->getMaxHp();
    status += " A:";
    status += liuBei->getAttackDamage();
    status += " S:";
    status += liuBei->isSkillUsed() ? "0" : "1";
    return status;
}

String Game::getGuanYuStatusForDisplay() const {
    String status = "GY:HP";
    status += guanYu->getHp();
    status += "/";
    status += guanYu->getMaxHp();
    status += " A:";
    status += guanYu->getAttackDamage();
    status += " S:";
    status += guanYu->isSkillUsed() ? "0" : "1";
    return status;
}

String Game::getZhangFeiStatusForDisplay() const {
    String status = "ZF:HP";
    status += zhangFei->getHp();
    status += "/";
    status += zhangFei->getMaxHp();
    status += " A:";
    status += zhangFei->getAttackDamage();
    status += " S:";
    status += zhangFei->isSkillUsed() ? "0" : "1";
    return status;
}

// 在Game.cpp文件末尾添加以下方法实现
bool Game::isLuBuAlive() const {
    return lubu->alive();
}

bool Game::isLiuBeiAlive() const {
    return liuBei->alive();
}

bool Game::isGuanYuAlive() const {
    return guanYu->alive();
}

bool Game::isZhangFeiAlive() const {
    return zhangFei->alive();
}

void music_error(){
    musicPlayer.playTrackOnce(2);
}