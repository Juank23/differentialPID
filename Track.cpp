/*
  Track.cpp - a model for controlling motors with quadrature encoders
  Written by Tim Coyne, 2014
*/

#include "Energia.h"
#include "Track.h"

Track::Track() {
  _count = 0;
  _prevTime = micros();
  _lastCount = 0;
  _target = 0;
}

int Track::update(int state) {
  if (state > 0) {
    _count++;
  } else if (state < 0) {
    _count--;
  }
}

void Track::setMicrosPerCount(int mpc) {
  _target = mpc;
}

int Track::getMicrosPerCount() {
  return _lastCount;
}

int Track::getTargetMicrosPerCount() {
  return _target;
}

int Track::getCount() {
  return _count;
}
