#include <windows.h>
#include <tlhelp32.h>
#include <iostream>

// Callback function to find the HWND of a process by its ID
BOOL CALLBACK EnumWindowsCallback(HWND hwnd, LPARAM lParam) {
  DWORD processId = 0;
  GetWindowThreadProcessId(hwnd, &processId);
  if (processId == (DWORD)lParam) {
    // Store the HWND
    SetLastError((LONG_PTR)hwnd);
    return FALSE; // Stop enumeration
  }
  return TRUE; // Continue enumeration
}

// Function to get the HWND from a process ID
HWND GetHwndFromProcessID(DWORD processID) {
  EnumWindows(EnumWindowsCallback, (LPARAM)processID);
  return reinterpret_cast<HWND>(GetLastError());
}

// Function to get the parent process ID of a given process ID
DWORD GetParentProcessId(DWORD processId) {
  // Get process handle
  HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
  if (hProcess == NULL) {
    std::cout << "Could not open process with ID " << processId << ". Error: " << GetLastError() << std::endl;
    throw std::runtime_error("Failed to open process");
  }

  // Get parent process ID using CreateToolhelp32Snapshot
  HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (hSnapshot == INVALID_HANDLE_VALUE) {
    CloseHandle(hProcess);
    std::cout << "Could not create snapshot. Error: " << GetLastError() << std::endl;
    throw std::runtime_error("Failed to create snapshot");
  }

  PROCESSENTRY32W pe32;
  pe32.dwSize = sizeof(pe32);

  if (!Process32FirstW(hSnapshot, &pe32)) {
    CloseHandle(hSnapshot);
    CloseHandle(hProcess);
    std::cout << "Could not get process info. Error: " << GetLastError() << std::endl;
    throw std::runtime_error("Failed to get process info");
  }

  DWORD parentProcessId = 0;
  // Find our process in snapshot
  do {
    if (pe32.th32ProcessID == processId) {
      // Get process path
      wchar_t processPath[MAX_PATH];
      DWORD size = MAX_PATH;
      if (QueryFullProcessImageNameW(hProcess, 0, processPath, &size)) {
        std::wcout << L"Process ID: " << processId << std::endl;
        std::wcout << L"Parent Process ID: " << pe32.th32ParentProcessID << std::endl;
        parentProcessId = pe32.th32ParentProcessID;
        std::wcout << L"Process Path: " << processPath << std::endl;
      }
      break;
    }
  } while (Process32NextW(hSnapshot, &pe32));

  CloseHandle(hSnapshot);
  CloseHandle(hProcess);
  return parentProcessId;
}

// Function to simulate typing a character
void TypeCharacter(char c) {
  INPUT inputs[2] = {};

  // Key Down
  inputs[0].type = INPUT_KEYBOARD;
  inputs[0].ki.wVk = VkKeyScan(c);

  // Key Up
  inputs[1].type = INPUT_KEYBOARD;
  inputs[1].ki.wVk = VkKeyScan(c);
  inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;

  SendInput(2, inputs, sizeof(INPUT));
  Sleep(50); // Small delay between keystrokes
}

// Function to get the process ID by its name
DWORD GetProcessIdByName(const wchar_t* processName) {
  DWORD processId = 0;
  HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

  if (snapshot != INVALID_HANDLE_VALUE) {
    PROCESSENTRY32W processEntry;
    processEntry.dwSize = sizeof(processEntry);

    if (Process32FirstW(snapshot, &processEntry)) {
      do {
        if (_wcsicmp(processEntry.szExeFile, processName) == 0) {
          processId = processEntry.th32ProcessID;
          break;
        }
      } while (Process32NextW(snapshot, &processEntry));
    }
    CloseHandle(snapshot);
  }

  if (processId == 0) {
    std::wcout << L"Process " << processName << L" not found." << std::endl;
    throw std::runtime_error("Process not found");
  }

  return processId;
}

int main() {
  try {
    DWORD ffmpegProcessId = GetProcessIdByName(L"ffmpeg.exe");
    DWORD parentProcessId = GetParentProcessId(ffmpegProcessId);

    HWND parentProcessHwnd = GetHwndFromProcessID(parentProcessId);
    if (parentProcessHwnd != NULL) {
      // Found the window handle
    }


    // Set focus to the parent process window
    if (!SetForegroundWindow(parentProcessHwnd)) {
      std::cerr << "Failed to set focus to window with handle " << parentProcessHwnd << ". Error: " << GetLastError() << std::endl;
      return 1;
    }

    Sleep(500);

    // Prepare input array for Ctrl+C
    INPUT inputs[4] = {};

    // Press Ctrl
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = VK_CONTROL;

    // Press C
    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = 'C';

    // Release C
    inputs[2].type = INPUT_KEYBOARD;
    inputs[2].ki.wVk = 'C';
    inputs[2].ki.dwFlags = KEYEVENTF_KEYUP;

    // Release Ctrl
    inputs[3].type = INPUT_KEYBOARD;
    inputs[3].ki.wVk = VK_CONTROL;
    inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;

    // Send all inputs at once
    UINT result = SendInput(4, inputs, sizeof(INPUT));
    if (result != 4) {
      std::cerr << "Failed to send input. Error: " << GetLastError() << std::endl;
      return 1;
    }

    std::wcout << "Ctrl+C Sent";
    return 0;

  } catch (const std::runtime_error& e) {
    std::cerr << "Runtime error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
