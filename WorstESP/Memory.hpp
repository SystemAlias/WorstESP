
#include <Windows.h>
#include <TlHelp32.h>
#include <iostream>
#include <string>
#include <memory>
#include <chrono>
#include <thread>

class ProcessMemoryHandler {
public:

    ProcessMemoryHandler(const std::string& processName, const std::string& moduleName)
        : processName(processName), moduleName(moduleName), hProcess(nullptr) {
        attachToProcess();
        clientBaseAddress = getModuleBaseAddress();
    }

    ~ProcessMemoryHandler() {
        if (hProcess) {
            CloseHandle(hProcess);
        }
    }
    template <typename T>
    T readMemory(DWORD_PTR address) {
        T buffer;
        ReadProcessMemory(hProcess, reinterpret_cast<LPCVOID>(address), &buffer, sizeof(T), nullptr);
        return buffer;
    }
    template <typename T>
    void writeMemory(DWORD_PTR address, T value) {
        WriteProcessMemory(hProcess, reinterpret_cast<LPVOID>(address), &value, sizeof(T), nullptr);
    }

    DWORD_PTR clientBaseAddress;
private:
    std::string processName;
    std::string moduleName;
    HANDLE hProcess;

    void attachToProcess() {
        DWORD pid = getProcessIdByName(processName);
        if (pid == 0) {
            std::cerr << "[ERROR] Process '" << processName << "' not found." << std::endl;
            throw std::runtime_error("Process not found.");
        }

        hProcess = OpenProcess(PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, pid);
        if (hProcess == nullptr) {
            std::cerr << "[ERROR] Failed to open process. Error Code: " << GetLastError() << std::endl;
            throw std::runtime_error("Failed to open process.");
        }
    }


    DWORD getProcessIdByName(const std::string& name) {
        PROCESSENTRY32 entry;
        entry.dwSize = sizeof(PROCESSENTRY32);

        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

        if (Process32First(snapshot, &entry)) {
            do {
                if (std::string(entry.szExeFile) == name) {
                    CloseHandle(snapshot);
                    return entry.th32ProcessID;
                }
            } while (Process32Next(snapshot, &entry));
        }

        CloseHandle(snapshot);
        return 0;
    }

    DWORD_PTR getModuleBaseAddress() {
        MODULEENTRY32 entry;
        entry.dwSize = sizeof(MODULEENTRY32);

        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, getProcessIdByName(processName));

        if (Module32First(snapshot, &entry)) {
            do {
                if (std::string(entry.szModule) == moduleName) {
                    CloseHandle(snapshot);
                   
                    return reinterpret_cast<DWORD_PTR>(entry.modBaseAddr);
                }
            } while (Module32Next(snapshot, &entry));
        }

        CloseHandle(snapshot);
        return 0;
    }
};
