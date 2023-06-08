#include "Alarm.h"
#include "AlarmType.h"
#define STANDARD_DURATION = 30000;

static unsigned int nextId = 1;

Alarm::Alarm(String name, unsigned int time, bool[7] days) {
    Alarm::Alarm(name, time, days, STANDARD_DURATION, init);
}
Alarm::Alarm(String name, unsigned int time, bool[7] days, unsigned int duration) {
    id = nextId;
    nextId++;
    this.name = name;
    this.isActive = true;
    this.time = time;
    this.weekdays = days;
    this.duration = duration;
    this.nextInitiation = calcNextInit;
    this.riseToMax = AlarmType::RISE_TO_MAX;
    this.executed = false;
}
void Alarm::setName(String name) {
    this.name = name;
}
void Alarm::setDays(bool[7] days) {
    this.weekdays = days;
}
void Alarm::setState(bool active) {
    this.active = active;
}
void Alarm::setDuration(unsigned int duration) {
    this.duration = duration;
}
unsigned int Alarm::getDuration() {
    return this.duration;
}
AlarmType getType() {
    return this.type;
}
unsigned long Alarm::getNextInitiation() {
    if(!active || (nextInitiation < millis() && !isRepeating)) {
        return 0;
    } else if(nextInitiation < millis() && executed) {

    }
    return this.nextInitiation;
}
bool isRepeating(){
    bool repeat = false;
    for(int i = 0; i < days.size(); i++) {
        if(days[i] == true) { this.repeat = true;}
    }
    return repeat;
}
unsigned long calcNextInit(unsigned long last, unsigned int time, bool[7] days) {
    int currentWeekday = weekday();
    int currentDayTimestamp = timestamp(date());
    if(days[currentWeekday] && time >)
    for(int i = 0; i < 8; i++) {
        day = (currentWeekday + i) % 7;

    }
    if(millis() < nextInitiation) {
    return;
    }

}
bool setExecuted(unsigned long executedAt) {
}
