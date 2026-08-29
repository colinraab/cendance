#pragma once

class Transport {
public:
    void prepare(double sampleRate);
    void advance(int numSamples);      // Called every audio block
    void reset();
    double getPlayheadPosition() const; // In beats (e.g., 0.0 = bar start)
    int getCurrentBeat() const;         // 0-3 within current bar
    int getCurrentBar() const;
    bool isNewBeat() const;             // True if beat boundary crossed this block
    bool isNewBar() const;
    void setBpm(float bpm);

private:
    double sampleRate_ = 44100.0;
    float bpm_ = 120.0f;
    double samplesPerBeat_ = 0.0;
    double samplePosition_ = 0.0;
    int lastBeat_ = -1;
    int lastBar_ = -1;
    bool newBeat_ = false;
    bool newBar_ = false;

    void updateSamplesPerBeat();
};
