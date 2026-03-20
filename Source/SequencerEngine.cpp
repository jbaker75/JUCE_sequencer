#include "SequencerEngine.h"

SequencerEngine::SequencerEngine()
{
    // Initialize pattern with defaults
    for (auto& step : pattern)
    {
        step.noteOffset = 0;
        step.gate = false;
    }
}

SequencerEngine::~SequencerEngine()
{
    stopTimer();
    sendNoteOff(); // Ensure silence on destruction
}

// --- Transport ---

void SequencerEngine::play()
{
    if (playing) return;
    
    playing = true;
    updateTimerInterval();
}

void SequencerEngine::stop()
{
    stopTimer();
    sendNoteOff();
    playing = false;
    // We don't necessarily reset the step on stop, usually pause behavior.
    // Use reset() for that.
}

void SequencerEngine::reset()
{
    stop();
    currentStep = 0;
    // Notify UI of reset if needed
    if (stepCallback) stepCallback(0);
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
    // 16th notes: 4 steps per beat.
    // MS per beat = 60000 / BPM
    // MS per step = (60000 / BPM) / 4
    double msPerStep = (60000.0 / bpm) / 4.0;
    startTimer(static_cast<int>(msPerStep));
}

void SequencerEngine::sendNoteOff()
{
    if (lastNotePlayed != -1)
    {
        if (midiCallback)
        {
            // Send NoteOff on channel 1
            midiCallback(juce::MidiMessage::noteOff(1, lastNotePlayed));
        }
        lastNotePlayed = -1;
    }
}

void SequencerEngine::hiResTimerCallback()
{
    // 1. Silence previous note
    sendNoteOff();

    // 2. Advance Step
    currentStep = (currentStep + 1) % 16;

    // 3. Notify UI (on message thread usually, but callbacks here are direct)
    // Note: The UI update should be async to not block the timer, 
    // but for simple visual feedback, a direct atomic/flag check or 
    // using juce::MessageManager::callAsync is safer if the callback touches UI.
    if (stepCallback)
    {
        // Execute on message thread for UI safety
        juce::MessageManager::callAsync([this, step = currentStep]() {
             if (stepCallback) stepCallback(step); 
        });
    }

    // 4. Play New Note
    const auto& step = pattern[currentStep];
    if (step.gate)
    {
        int noteToPlay = baseNote + step.noteOffset;
        noteToPlay = juce::jlimit(0, 127, noteToPlay);
        
        lastNotePlayed = noteToPlay;
        if (midiCallback)
        {
            // Note On, Channel 1, Velocity 100
            midiCallback(juce::MidiMessage::noteOn(1, lastNotePlayed, (juce::uint8)100));
        }
    }
}
