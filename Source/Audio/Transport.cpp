#include "Transport.h"
#include <cmath>

void Transport::prepare(double sampleRate) {
    sampleRate_ = sampleRate;
    updateSamplesPerBeat();
    reset();
}

void Transport::reset() {
    samplePosition_ = 0.0;
    lastBeat_ = -1;
    lastBar_ = -1;
    newBeat_ = false;
    newBar_ = false;
}

void Transport::setBpm(float bpm) {
    if (bpm_ != bpm) {
        bpm_ = bpm;
        updateSamplesPerBeat();
    }
}

void Transport::updateSamplesPerBeat() {
    if (bpm_ > 0.0f) {
        samplesPerBeat_ = (60.0 / bpm_) * sampleRate_;
    } else {
        samplesPerBeat_ = sampleRate_; // Fallback to 60 BPM if 0 or negative
    }
}

void Transport::advance(int numSamples) {
    if (samplesPerBeat_ <= 0.0) return;

    samplePosition_ += numSamples;

    double playheadPositionBeats = samplePosition_ / samplesPerBeat_;
    int totalBeats = static_cast<int>(std::floor(playheadPositionBeats));
    
    int currentBeat = totalBeats % 4;
    int currentBar = totalBeats / 4;

    newBeat_ = (currentBeat != lastBeat_ && lastBeat_ != -1) || 
               (currentBeat == 0 && currentBar != lastBar_);
    newBar_ = (currentBar != lastBar_ && lastBar_ != -1);

    lastBeat_ = currentBeat;
    lastBar_ = currentBar;
}

double Transport::getPlayheadPosition() const {
    if (samplesPerBeat_ > 0.0) {
        return samplePosition_ / samplesPerBeat_;
    }
    return 0.0;
}

int Transport::getCurrentBeat() const {
    return lastBeat_ >= 0 ? lastBeat_ : 0;
}

int Transport::getCurrentBar() const {
    return lastBar_ >= 0 ? lastBar_ : 0;
}

bool Transport::isNewBeat() const {
    return newBeat_;
}

bool Transport::isNewBar() const {
    return newBar_;
}
