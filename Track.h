/*
  Track.h - Library for containing a track model.
  Created by Tim Coyne
*/

#ifndef Track_h
#define Track_h

#include "Energia.h"

class Track {
  public:
    Track();
    int update(byte state);
    void setMicrosPerCount(int mpc);
    int getAverageTimePerStep();
    int getTargetMicrosPerCount();
    int getCount();
    void init(byte initState);
    static const int num_samples = 5;
  private:
    static const byte ORDERED_STATES[];
    int _times[num_samples];
    int _state;
    int _count;
    int _target;
    long _totalTime;
    unsigned long _prevTime;
    int stateToVal(byte state);
    void updateTimes(long timeDif);
};

#endif
