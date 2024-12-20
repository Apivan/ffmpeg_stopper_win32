# FFmpeg Stopper

## Description
FFmpeg Stopper is a Windows application designed to stop the `ffmpeg` process by simulating a `Ctrl+C` command in the parent process window. This tool is useful for scenarios where `ffmpeg` is running in a command prompt and needs to be stopped programmatically.

## Usage
   Execute the compiled `ffmpegstopper.exe` file. The application will automatically find the `ffmpeg` process, set focus to its parent window, and send a `Ctrl+C` command to stop the process.

## Build
   Open the Visual Studio Command Prompt and run the following command:
   ```bash
   cl /EHsc /W4 /O2 /std:c++17 ffmpegstopper.cpp user32.lib
   ```
