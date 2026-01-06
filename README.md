# DrumVisualizer

This is a program to visualize midi input specifically for Clone Hero drums, WIP more to come.

## Setup

### Windows

1. Follow this tutorial for LoopMIDI and MIDIOX setup: <https://www.youtube.com/watch?v=RS9NtVNdHnE>
2. The default ports it looks for are 'loopMIDI CH' and 'loopMIDI Visualizer'
3. Run Clone Hero and set your Input Device to loopMIDI CH, or whatever you named that port
4. Go to your Documents/Clone Hero/MIDI Profiles and edit loopMIDI CH.yaml to match your existing mappings **OR** remap everything in game
5. Download the latest release, run it, and enjoy

### Linux

Currently there is no build for Linux but hopefully soon

### Mac

Currently there is no build for Mac but hopefully soon

## Developing Locally (Building from Source)

If you don't use Visual Studio, you can use **CMake** to build the project which allows development in other IDEs like VS Code. Currently, this method only supports Windows out of the box.

### Prerequisites

1. **CMake**: [Download](https://cmake.org/download/).
2. **C++ Compiler**:
    - **Windows**: [Visual Studio Build Tools](https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2026) (Desktop development with C++).
    - **Linux/Mac**: [GCC](https://gcc.gnu.org/install/) or [Clang](https://clang.llvm.org/get_started.html).
3. **VS Code Extensions** (Recommended):
    - [C/C++](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools) (Microsoft)
    - [CMake](https://marketplace.visualstudio.com/items?itemName=twxs.cmake) (twxs)
    - [CMake Tools](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools) (Microsoft)
    - [clangd](https://marketplace.visualstudio.com/items?itemName=llvm-vs-code-extensions.vscode-clangd) (LLVM)

### Building in VS Code

1. Open the folder in VS Code.
2. Install the recommended extensions.
3. Press `Ctrl+Shift+P` and run `CMake: Configure`.
4. Press `Ctrl+Shift+B` (or click "Build" in the status bar) to build.
5. Press `F5` to debug/run.

### Building from Command Line

```bash
cmake -S . -B build
cmake --build build --config Debug
./build/Debug/DrumVisualizer.exe
```
