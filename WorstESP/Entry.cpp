#include "Memory.hpp"

/*
I discovered with this idea a while ago when experimenting with Team Fortress 2.
I'm quite surprised it's still effective on Source 2. This concept might have been implemented before,
but here is my version of it.

TL;DR: I just wanted to create a PoC.
*/


ProcessMemoryHandler* MemoryHandler;

void MemorySetup() {
    std::cout << "[IMPORTANT]\n Attention, probably a Russian, press the RIGHT Arrow to toggle between teams!\n"
        << "A '.' indicates that 'm_iTeamNum' has not yet been changed, so we're continuously sending WPM until it is.\n"
        << "Remember, the game will behave as if you are on the opposite team when 'spoofed'\nmeaning all VGUI elements will react accordingly\n\n\nOutput:";

    MemoryHandler = new ProcessMemoryHandler("cs2.exe", "client.dll");
}


void ToggleLoop() {
    int iSpoofNum = 3;

    while (1) {
        DWORD_PTR pLocalPlayer = MemoryHandler->readMemory<DWORD_PTR>(MemoryHandler->clientBaseAddress + dwLocalPlayerController);
       
        if ((GetAsyncKeyState(VK_RIGHT) & 0x8000)) {  // You could get the original value by reading then caching it,
            // but if you're interested in improving this, go ahead
            switch (iSpoofNum) {
            case 2:
                iSpoofNum = 3;
                break;

            case 3:
                iSpoofNum = 2;
                break;
            }
            std::cout << "[!] changing m_iTeamNum -> "
                << MemoryHandler->readMemory<int>(pLocalPlayer + m_iTeamNum)
                << " -> " << iSpoofNum << "\n";
        }


        // Even though I am noob, this level of API spam makes me feel uncomfortable 
        // But here is my reasoning, for some reason the game likes to randomly reset the value or only accept the change at certain points
        // after a few write attempts it seems to stay set, I don't know the reason for this.
        // as a safety precaution we will sleep and have a 'timeout'.

        int iTimeOut = 0;
        while (MemoryHandler->readMemory<int>(pLocalPlayer + m_iTeamNum) != iSpoofNum && iTimeOut < 64) {
            MemoryHandler->writeMemory<int>(
                (pLocalPlayer + m_iTeamNum),
                iSpoofNum);

            std::cout << ".";
            iTimeOut += 1;
            Sleep(50);
        }

        std::cout << std::endl;
        
        Sleep(100); // CPU pain relief
    }
}

int main() {
    MemorySetup();
    ToggleLoop();

    return 0;
}