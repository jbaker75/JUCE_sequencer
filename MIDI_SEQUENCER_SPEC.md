# Functional Specification: 16-Step MIDI Sequencer

## 1. Project Overview
A standalone desktop application that generates MIDI note data based on a 16-step pattern. Each step defines a pitch offset relative to a base note and includes a "gate" (on/off) toggle.

## 2. Core Requirements (MVP)

### A. Sequencer Engine (The Logic)
1.  **Clocking:** 
    *   Maintain a steady pulse based on a BPM (Beats Per Minute) value.
    *   Resolution: 16th notes (4 steps per quarter note).
    *   Drift-corrected timing for stable performance.
2.  **State Management:**
    *   `steps`: 16 integer values representing semitone offsets (-12 to +12).
    *   `gates`: 16 boolean values (True = play note, False = silent).
    *   `base_note`: A root MIDI note (0-127, e.g., 60 for C3).
3.  **MIDI Handling:**
    *   Send `note_on` and `note_off` messages to a selected MIDI output.
    *   **Cleanup:** Every `note_on` must be preceded or followed by a corresponding `note_off` to prevent hanging notes.
    *   **Transport:** Must support "Play," "Stop," and "Reset" (return to step 0).

### B. User Interface (The Visuals)
1.  **Control Bar (Top):**
    *   **Transport Buttons:** Play, Stop, Reset.
    *   **Tempo:** A slider and numeric display for BPM (40-240).
    *   **MIDI Config:** 
        *   Dropdown to select from available MIDI output devices.
        *   Spinbox/Input for the `base_note`.
2.  **Step Grid (Main):**
    *   16 vertical columns, each containing:
        *   **Value Label:** Shows the current semitone offset (e.g., "+3").
        *   **Vertical Slider:** Controls the semitone offset (-12 to +12).
        *   **Gate Button:** Toggles the step on/off (visual state: Green for On, Gray for Off).
        *   **Step Number:** 1 through 16.
3.  **Playhead Visualization:**
    *   Highlight the entire active column (slider + buttons) as the sequencer moves.

## 3. JUCE-Specific Implementation Notes (C++)

### A. Class Structure
*   `MainComponent`: The main UI container.
*   `SequencerEngine` (or `AudioProcessor`): Handles the `HighResolutionTimer` for timing and `MidiOutput` for messages.
*   `StepColumn`: A custom `juce::Component` to represent a single step (slider + button).

### B. Timing (JUCE vs. Python)
*   **Python:** Used a `QThread` with `time.sleep()`.
*   **JUCE:** Should use `juce::HighResolutionTimer` or a background `juce::Thread` with high-priority timing. For even better precision, this could be implemented as a MIDI plugin using the `processBlock` audio callback.

### C. Visuals
*   Use `juce::LookAndFeel_V4` for modern dark-themed styling.
*   Sliders should use `LinearVertical` style.

## 4. Verification & Testing
*   **Visual Check:** Does the playhead move correctly at high BPM?
*   **MIDI Check:** Use a MIDI monitor (like MIDI-OX or a DAW) to verify note values (`base_note` + `offset`).
*   **Termination:** Closing the application must send an "All Notes Off" or a final `note_off` to prevent hanging notes.
