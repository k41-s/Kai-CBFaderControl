# Kai-CBFaderControl

Kai-CBFaderControl is a VST3 audio plugin designed to serve as a high-performance bridge between digital audio workstations (DAWs) and the XPatch software. Built on a slot-based architecture, it provides bidirectional synchronisation and advanced mixing control for 32 channel slots.

---

## Core Architecture

### Slot-Based Design
The plugin is built on a fixed backbone of 32 "Channel Slots". Each slot is a permanent container within the AudioProcessorValueTreeState (APVTS) to ensure DAW stability. 

Individual slot components include:
* **Volume:** Supports a range from +22dB to -96dB.
* **Mute:** Standard toggle functionality.
* **Pan/Balance:** A float range from -1.0 to 1.0 (Left to Right).
* **Solo/Solo-Safe:** Supports SIP (Solo In Place) and standard Solo groups with protective Solo-Safe logic.

### Dual-Mode Interface
The GUI is divided into two distinct operating modes:
* **Setup Mode:** A 32-slot grid used to configure network settings, IP addresses, and port numbers. Users mark faders as "Active" here to determine what appears on the mixing surface.
* **Performance Mode:** The primary mixing surface displaying only active faders. It features adaptive scaling, where channel strips scale in width to fit the screen.

---

## Features

### Advanced Grouping and Linking
The Link Manager enables complex inter-channel relationships through a three-tier hierarchy:
* **VCA Master:** Acts as a mathematical multiplier for a group's total gain and includes expand/collapse logic for member faders.
* **Group Leader:** A standard fader that physically controls all linked members.
* **Member:** Follows the leader while allowing for independent internal balancing.

### Panning and Stereo Logic
When two mono slots are linked as a stereo pair, they are treated as a single logical entity. The plugin internally handles the stereo link logic, merging the slots into one visual stereo fader with a single Pan/Balance control and applying -3dB Pan Law calculations.

### High-Resolution Control
The system supports a "Stationary Click" detection method on fader caps to toggle between two resolution modes:
* **Coarse Mode:** 1dB increments.
* **Fine Mode:** 0.25dB increments.

### Preset and Snapshot Management
The system distinguishes between visual layouts and audio data:
* **Layout Presets:** Save visibility, custom names, and visual sequences.
* **Data Presets:** Save fader values, mutes, and pan positions.
* **Snapshots (Scenes):** Internal quick-saves for toggling between different session states.

---

## Technical Specifications

### Networking and OSC Protocol
All communication is handled via Open Sound Control (OSC) over UDP. 
* **Default Ports:** Incoming Port 8000; Outgoing Port 8001.
* **Address Format:** `/XP4/fader/[SlotNumber]/[Parameter]`.
* **Data Handling:** Uses Lock-Free FIFO buffers and a Message Thread Dispatch to ensure thread-safe updates to the APVTS without blocking the GUI or audio thread.

### Synchronisation Logic
* **Bidirectional Sync:** Real-time updates between the plugin and XPatch for labels and parameters.
* **Anti-Feedback Rules:** Specific rules prevent data loops between GUI changes, DAW automation, and incoming OSC messages.
* **Heartbeat:** A 1000ms keep-alive "Ping" monitors the connection. A timeout of 3000ms triggers a "Connection Lost" state.

### Implementation Details
* **Framework:** Developed using JUCE.
* **Layout Management:** Utilises JUCE FlexBox and Grid for fluid adaptive scaling.
* **Visual Assets:** High-resolution SVG assets are used for fader caps and knobs to maintain a hardware aesthetic.