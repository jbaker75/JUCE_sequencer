#include <JuceHeader.h>
#include "SequencerEngine.h"
#include "StepColumn.h"

//==============================================================================
class MainComponent  : public juce::Component
{
public:
    //==============================================================================
    MainComponent()
    {
        // --- 1. Initialize Step Grid ---
        for (int i = 0; i < 16; ++i)
        {
            auto step = std::make_unique<StepColumn>(i);
            step->onValueChange = [this, i]() {
                if (stepColumns[i] != nullptr)
                    sequencer.setStep(i, stepColumns[i]->getNoteOffset(), stepColumns[i]->getGateState());
            };
            stepColumns[i] = std::move(step);
            addAndMakeVisible(stepColumns[i].get());
        }

        // --- 2. Initialize Transport Controls ---
        addAndMakeVisible(startButton);
        startButton.setButtonText("Play");
        startButton.onClick = [this] { sequencer.play(); };

        addAndMakeVisible(stopButton);
        stopButton.setButtonText("Stop");
        stopButton.onClick = [this] { sequencer.stop(); };

        addAndMakeVisible(resetButton);
        resetButton.setButtonText("Reset");
        resetButton.onClick = [this] { sequencer.reset(); };

        // --- 3. Initialize BPM Controls ---
        addAndMakeVisible(bpmSlider);
        bpmSlider.setSliderStyle(juce::Slider::LinearHorizontal);
        bpmSlider.setRange(40.0, 240.0, 1.0);
        bpmSlider.setValue(120.0);
        bpmSlider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 50, 20);
        bpmSlider.onValueChange = [this] { sequencer.setBpm(bpmSlider.getValue()); };

        addAndMakeVisible(bpmLabel);
        bpmLabel.setText("BPM", juce::dontSendNotification);
        bpmLabel.attachToComponent(&bpmSlider, true);

        // --- 4. Initialize MIDI Device Selection ---
        addAndMakeVisible(midiOutputList);
        midiOutputList.setTextWhenNoChoicesAvailable("No MIDI Outputs Found");
        
        refreshMidiDevices();
        midiOutputList.onChange = [this] { selectMidiOutput(midiOutputList.getSelectedItemIndex()); };

        // --- 5. Virtual Port Button (Linux Specific) ---
        addAndMakeVisible(virtualPortButton);
        virtualPortButton.setButtonText("Create Virtual Port");
        virtualPortButton.onClick = [this] { createVirtualPort(); };

        addAndMakeVisible(randomizeButton);
        randomizeButton.setButtonText("Randomize");
        randomizeButton.onClick = [this] {
            for (int i = 0; i < 16; ++i) {
                int offset = juce::Random::getSystemRandom().nextInt(juce::Range<int>(-12, 12));
                bool gate = juce::Random::getSystemRandom().nextBool();
                stepColumns[i]->setNoteOffset(offset);
                stepColumns[i]->setGateState(gate);
                sequencer.setStep(i, offset, gate);
            }
        };

        addAndMakeVisible(midiLabel);
        midiLabel.setText("MIDI Out", juce::dontSendNotification);

        // --- 6. Engine Callbacks ---
        sequencer.setMidiCallback([this](const juce::MidiMessage& m) {
            if (auto* out = midiOutput.get()) {
                out->sendMessageNow(m);
                if (m.isNoteOn())
                    juce::Logger::writeToLog("MIDI SENT: Note On " + juce::String(m.getNoteNumber()));
            }
        });

        sequencer.setStepCallback([this](int activeStep) {
            for (int i = 0; i < 16; ++i)
                if (stepColumns[i] != nullptr)
                    stepColumns[i]->setHighlighted(i == activeStep);
        });

        setSize (900, 450);
    }

    ~MainComponent() override
    {
        sequencer.stop();
        midiOutput = nullptr;
    }

    //==============================================================================
    void paint (juce::Graphics& g) override
    {
        g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
        g.setColour(juce::Colours::grey);
        g.drawLine(0, 80, getWidth(), 80, 2.0f);
    }

    void resized() override
    {
        auto area = getLocalBounds();
        auto header = area.removeFromTop(80).reduced(10);
        
        // Top Row: Transport
        auto transportRow = header.removeFromTop(30);
        startButton.setBounds(transportRow.removeFromLeft(80).reduced(2));
        stopButton.setBounds(transportRow.removeFromLeft(80).reduced(2));
        resetButton.setBounds(transportRow.removeFromLeft(80).reduced(2));
        
        transportRow.removeFromLeft(40);
        bpmSlider.setBounds(transportRow.removeFromRight(200).reduced(2));

        // Bottom Row: MIDI
        auto midiRow = header.removeFromTop(30);
        midiLabel.setBounds(midiRow.removeFromLeft(80).reduced(2));
        midiOutputList.setBounds(midiRow.removeFromLeft(200).reduced(2));
        midiRow.removeFromLeft(10);
        virtualPortButton.setBounds(midiRow.removeFromLeft(150).reduced(2));
        midiRow.removeFromLeft(10);
        randomizeButton.setBounds(midiRow.removeFromLeft(100).reduced(2));

        // Grid Area
        auto gridArea = area.reduced(10);
        int stepWidth = gridArea.getWidth() / 16;
        for (int i = 0; i < 16; ++i)
        {
            if (stepColumns[i] != nullptr)
                stepColumns[i]->setBounds(gridArea.removeFromLeft(stepWidth));
        }
    }

private:
    void refreshMidiDevices()
    {
        midiOutputList.clear();
        auto devices = juce::MidiOutput::getAvailableDevices();
        for (int i = 0; i < devices.size(); ++i)
            midiOutputList.addItem(devices[i].name, i + 1);
    }

    void selectMidiOutput(int index)
    {
        auto devices = juce::MidiOutput::getAvailableDevices();
        if (index >= 0 && index < devices.size())
        {
            midiOutput = juce::MidiOutput::openDevice(devices[index].identifier);
            if (midiOutput)
                midiLabel.setText("Active: " + devices[index].name, juce::dontSendNotification);
        }
    }

    void createVirtualPort()
    {
        // createNewDevice is available on Linux (ALSA/JACK) and macOS
        midiOutput = juce::MidiOutput::createNewDevice ("JUCE Sequencer Out");
        if (midiOutput)
        {
            midiLabel.setText("Active: Virtual Port", juce::dontSendNotification);
            juce::Logger::writeToLog("Created Virtual MIDI Port: JUCE Sequencer Out");
        }
        else
        {
            juce::Logger::writeToLog("Failed to create Virtual MIDI Port");
        }
    }

    SequencerEngine sequencer;
    std::array<std::unique_ptr<StepColumn>, 16> stepColumns;
    
    juce::TextButton startButton, stopButton, resetButton, virtualPortButton, randomizeButton;
    juce::Slider bpmSlider;
    juce::Label bpmLabel;
    juce::ComboBox midiOutputList;
    juce::Label midiLabel;

    std::unique_ptr<juce::MidiOutput> midiOutput;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};

//==============================================================================
class JuceSequencerApplication  : public juce::JUCEApplication
{
public:
    JuceSequencerApplication() {}

    const juce::String getApplicationName() override       { return "JUCE Sequencer"; }
    const juce::String getApplicationVersion() override    { return "1.0.0"; }
    bool moreThanOneInstanceAllowed() override             { return true; }

    void initialise (const juce::String& commandLine) override
    {
        mainWindow.reset (new MainWindow (getApplicationName()));
    }

    void shutdown() override
    {
        mainWindow = nullptr;
    }

    void systemRequestedQuit() override
    {
        quit();
    }

    class MainWindow    : public juce::DocumentWindow
    {
    public:
        MainWindow (juce::String name)
            : DocumentWindow (name,
                              juce::Desktop::getInstance().getDefaultLookAndFeel()
                                                          .findColour (juce::ResizableWindow::backgroundColourId),
                              DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar (true);
            setContentOwned (new MainComponent(), true);
            setResizable (true, true);
            centreWithSize (getWidth(), getHeight());
            setVisible (true);
        }

        void closeButtonPressed() override
        {
            juce::JUCEApplication::getInstance()->systemRequestedQuit();
        }

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };

private:
    std::unique_ptr<MainWindow> mainWindow;
};

START_JUCE_APPLICATION (JuceSequencerApplication)
