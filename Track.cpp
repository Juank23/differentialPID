/*
  Track.cpp - a model for controlling motors with quadrature encoders
  Written by Tim Coyne, 2014
*/

#include "Energia.h"
#include "Track.h"

const byte Track::ORDERED_STATES[] = {B00000001, B00000011, B00000010, B00000000, B00000001, B00000011};

Track::Track() {
  _count = 0;
  _target = 0;
  _sampleIndex = 0;
  _totalTime = 0;
}

int Track::update(byte state) {
  byte actualState = state & 0x03;
  unsigned long time = micros();
  
  if (actualState == ORDERED_STATES[_state + 1]) {
    _state++;
  } else if (actualState == ORDERED_STATES[_state - 1]) {
    if (_state == 2) {
      _count--;
      updateTimes(time - _prevTime);
    }
    _state--;
  } else if (actualState != ORDERED_STATES[_state]) {
    return -1;
  }
  
  if (_state >= 5) {
    _state = 1;
    _count++;
    updateTimes(time - _prevTime);
  }
  if (_state <= 0) {
    _state = 4;
  }
  
  _prevTime = time;
  return 0;
}

void Track::setMicrosPerCount(int mpc) {
  _target = mpc;
}

int Track::getAverageTimePerStep() {
  return _totalTime / num_samples;
}

int Track::getTargetMicrosPerCount() {
  return _target;
}

int Track::getCount() {
  return _count;
}

void Track::init(byte initState) {
  _state = stateToVal(initState & 0x03);
  _prevTime = micros();
}

int Track::stateToVal(byte state) {
  switch (state) {
    case B00000001:
      return 1;
    case B00000011:
      return 2;
    case B00000010:
      return 3;
    case B00000000:
      return 4;
    default:
      return -1;
  }
}

void Track::updateTimes(long timeDif) {
  int dif = 0;
  if (timeDif <= 0x0000FFFF) {
    dif = timeDif;
  }
  
  _totalTime -= _times[_sampleIndex];
  _times[_sampleIndex] = dif;
  _totalTime += dif;
  _sampleIndex++;
  if (_sampleIndex >= num_samples) {
    _sampleIndex = 0;
  }
}
  
