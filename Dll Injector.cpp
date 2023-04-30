#include <iostream>
#include <windows.h>
#include <TlHelp32.h>

const char mydll[MAX_PATH] = "C:\\path\\to\\your.dll";

DWORD getProcId(const char* processName) {
	
	DWORD processId = 0;
	PROCESSENTRY32 processEntry;

	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (hSnapshot == INVALID_HANDLE_VALUE) {
		std::cout << "[-] Error on CreateToolHelp32Snapshot" << std::endl;
		return false;
	}

	processEntry.dwSize = sizeof(PROCESSENTRY32);

	if (!Process32First(hSnapshot, &processEntry)) {
		std::cout << "[-] Error on Process32First" << std::endl;
		return false;
	}

	do {
		if (!_stricmp((const char*)processEntry.szExeFile, processName)) {
			processId = processEntry.th32ProcessID;
			break;
		}
	} while (Process32Next(hSnapshot, &processEntry));

	CloseHandle(hSnapshot);
	return processId;
}

bool Inject(DWORD processId, const char dllPath[MAX_PATH]) {

	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, processId);

	if (hProcess == INVALID_HANDLE_VALUE) {
		std::cout << "[-] Error on OpenProcess" << std::endl;
		return false;
	}

	LPVOID memAlloc = VirtualAllocEx(hProcess, nullptr, strlen(dllPath) + 1, MEM_COMMIT, PAGE_READWRITE);
	WriteProcessMemory(hProcess, memAlloc, dllPath, strlen(dllPath) + 1, nullptr);
	LPTHREAD_START_ROUTINE hLoadLibrary = (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA");
	CreateRemoteThread(hProcess, nullptr, 0, hLoadLibrary, memAlloc, 0, nullptr);
	CloseHandle(hProcess);
}

int main()
{
	Inject(getProcId("process.exe"), mydll);
	std::cout << "Dll Injection successful!" << std::endl;
}
