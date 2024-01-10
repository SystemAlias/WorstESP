#include "Memory.hpp"

class CLogic {
public:
    CLogic()
        : MemoryHandler(std::make_unique<ProcessMemoryHandler>("cs2.exe", "client.dll")) {
        std::cout << "[IMPORTANT] Attention, press the RIGHT Arrow to toggle between teams!\n"
            << "A '.' indicates that 'm_iTeamNum' has not yet been changed, so we're continuously sending WPM until it is.\n"
            << "Remember, the game will behave as if you are on the opposite team when 'spoofed'\n"
            << "meaning all VGUI elements will react accordingly\nIf not already enabled, I recommend putting '+cl_show_team_equipment' in your console.\n\n\nOutput:";
    }

    void ToggleLoop() {
        constexpr int TEAM_2 = 2;
        constexpr int TEAM_3 = 3;
        constexpr int TIMEOUT_LIMIT = 64;
        constexpr auto SLEEP_DURATION = std::chrono::milliseconds(50);

        constexpr DWORD dwLocalPlayerController = 0x1810F48;
        constexpr DWORD m_iTeamNum = 0x3BF;

        int iSpoofNum = TEAM_3;

        while (true) {
            DWORD_PTR pLocalPlayer = MemoryHandler->readMemory<DWORD_PTR>(MemoryHandler->clientBaseAddress + dwLocalPlayerController);

            if ((GetAsyncKeyState(VK_RIGHT) & 0x8000)) {
                iSpoofNum = (iSpoofNum == TEAM_2) ? TEAM_3 : TEAM_2;
                std::cout << "\n[!] changing m_iTeamNum -> "
                    << MemoryHandler->readMemory<int>(pLocalPlayer + m_iTeamNum)
                    << " -> " << iSpoofNum << "\n";
                Beep(1000, 100);
            }

            int iTimeOut = 0;
            while (MemoryHandler->readMemory<int>(pLocalPlayer + m_iTeamNum) != iSpoofNum && iTimeOut < TIMEOUT_LIMIT) {
                MemoryHandler->writeMemory<int>((pLocalPlayer + m_iTeamNum), iSpoofNum);
                std::cout << ".";
                iTimeOut++;
                std::this_thread::sleep_for(SLEEP_DURATION);
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Reduce CPU usage
        }
    }

private:
    std::unique_ptr<ProcessMemoryHandler> MemoryHandler;
};

int main() {
    CLogic logic;
    logic.ToggleLoop();

    return 0;
}
