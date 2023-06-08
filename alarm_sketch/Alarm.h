#ifndef Alarm_h
#define Alarm_h
#include "AlarmType.h"

class Alarm {
  private:
    static unsigned int id;
    String name;
    bool active;
    unsigned int time; //milliseconds of the day
    bool[7] weekdays; //starting monday
    unsigned int duration; //in milliseconds
    unsigned long nextInitiation; //0 means there is no next
    AlarmType type;
    bool executed;

  public:
    Alarm(String name, unsigned int time, bool[7] days);
    Alarm(String name, unsigned int time, bool[7] days, unsigned int duration);
    void setName(String name);
    void setDays(bool[7] days);
    void setState(bool active);
    void setDuration(unsigned int duration);
    void setType(AlarmType type);
    unsigned int getDuration();
    AlarmType getType();
    unsigned long getNextInitiation();
    bool isRepeating();
    bool setExecuted(unsigned long executedAt);
};

#endif
