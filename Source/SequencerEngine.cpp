#include "SequencerEngine.h"

SequencerEngine::SequencerEngine()
{
    // Set to 15 so the first advanceStep() in play() lands on Step 0
    currentStep = 15;

    for (auto& step : pattern)
    {
        step.noteOffset = 0;
        step.gate = false;
    }
}

SequencerEngine::~SequencerEngine()
{
    stopTimer();
    sendNoteOff();
}

// --- Transport ---

void SequencerEngine::play()
{
    if (playing) return;
    
    playing = true;
    
    // Start by playing the "next" step immediately (Step 0 if reset)
    advanceStep();
    
    // Then start the timer for the steps that follow
    updateTimerInterval();
}

void SequencerEngine::stop()
{
    stopTimer();
    sendNoteOff();
    playing = false;
}

void SequencerEngine::reset()
{
    stop();
    // Set to 15 so the first advanceStep() in play() lands on Step 0
    currentStep = 15;
    
    if (stepCallback)
    {
        juce::MessageManager::callAsync([this]() {
             if (stepCallback) stepCallback(0); 
        });
    }
}

bool SequencerEngine::isPlaying() const
{
    return playing;
}

// --- Configuration ---

void SequencerEngine::setBpm(double newBpm)
{
    if (newBpm < 20.0) newBpm = 20.0;
    if (newBpm > 300.0) newBpm = 300.0;
    
    bpm = newBpm;
    if (playing)
    {
        updateTimerInterval();
    }
}

double SequencerEngine::getBpm() const
{
    return bpm;
}

void SequencerEngine::setBaseNote(int midiNoteNumber)
{
    baseNote = juce::jlimit(0, 127, midiNoteNumber);
}

int SequencerEngine::getBaseNote() const
{
    return baseNote;
}

// --- Pattern Management ---

void SequencerEngine::setStep(int index, int noteOffset, bool gate)
{
    if (index >= 0 && index < 16)
    {
        pattern[index].noteOffset = noteOffset;
        pattern[index].gate = gate;
    }
}

SequencerEngine::Step SequencerEngine::getStep(int index) const
{
    if (index >= 0 && index < 16)
        return pattern[index];
    return {};
}

void SequencerEngine::setMidiCallback(MidiCallback callback)
{
    midiCallback = callback;
}

void SequencerEngine::setStepCallback(StepCallback callback)
{
    stepCallback = callback;
}

// --- Internal Implementation ---

void SequencerEngine::updateTimerInterval()
{
    double msPerStep = (60000.0 / bpm) / 4.0;
    startTimer(static_cast<int>(msPerStep));
}

void SequencerEngine::sendNoteOff()
{
    if (lastNotePlayed != -1)
    {
        if (midiCallback)
            midiCallback(juce::MidiMessage::noteOff(1, lastNotePlayed));
        lastNotePlayed = -1;
    }
}

void SequencerEngine::hiResTimerCallback()
{
    advanceStep();
}

void SequencerEngine::advanceStep()
{
    // 1. Silence previous
    sendNoteOff();

    // 2. Advance
    currentStep = (currentStep + 1) % 16;

    // 3. UI Callback
    if (stepCallback)
    {
        juce::MessageManager::callAsync([this, step = currentStep]() {
             if (stepCallback) stepCallback(step); 
        });
    }

    // 4. Play New
    const auto& step = pattern[currentStep];
    if (step.gate)
    {
        int noteToPlay = juce::jlimit(0, 127, baseNote + step.noteOffset);
        lastNotePlayed = noteToPlay;
        if (midiCallback)
            midiCallback(juce::MidiMessage::noteOn(1, lastNotePlayed, (juce::uint8)100));
    }
}
