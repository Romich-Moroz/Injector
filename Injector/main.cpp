#include <iostream>
#include <windows.h>


//arg list: argv[0] - injector executable path, argv[1] = process path, argv[2] = dll path
int wmain(int argc, wchar_t* argv[]) {
	const wchar_t* errNoArgs = L"Error: Application and dll paths were not specified";
	const wchar_t* errNoDllPath = L"Error: Dll path was not specified";
	const wchar_t* errWrongFilePath = L"Error: Failed to run application with specified path";
	const wchar_t* errMemAlloc = L"Error: Failed to allocate memory inside target process";
	const wchar_t* errMemWrite = L"Error: Failed to write to memory of target process";
	const wchar_t* errKrnlNotFound = L"Error: kernel32 not found";
	const wchar_t* errLdLibNotFound = L"Error: LoadLibraryW not found";
	const wchar_t* errRemThread = L"Error: Cannot create remote thread in target process";

	const wchar_t* warTooManyArgs = L"Warning: Too many arguments were passed, processing only first and second argument";

	const wchar_t* msgProcessStartSuccess = L"Message: Suspended process was created successfully!";
	const wchar_t* msgThreadStartWait = L"Message: Waiting for thread to start";
	const wchar_t* msgInjectionDone = L"Injection successfull";

	STARTUPINFO startupInfo = { 0 };
	PROCESS_INFORMATION processInformation = { 0 };

	if (argc == 1) {
		std::wcout << errNoArgs << std::endl;
	}
	else if (argc == 2) {
		std::wcout << errNoDllPath << std::endl;
	}
	else if (argc > 3) {
		std::wcout << warTooManyArgs << std::endl;
	}
	else {
		if (!CreateProcess(argv[1], NULL, NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL, &startupInfo, &processInformation)) {
			std::wcout << errWrongFilePath << std::endl;
			
		}
		else {
			std::wcout << msgProcessStartSuccess << std::endl;
			size_t dllPathLength = wcslen(argv[2]) * sizeof(wchar_t);
			LPVOID lpDllPath = VirtualAllocEx(processInformation.hProcess, NULL, dllPathLength + 1, MEM_COMMIT, PAGE_READWRITE);
			if (!lpDllPath) {
				std::wcout << errMemAlloc << std::endl;
			}
			else {
				if(!WriteProcessMemory(processInformation.hProcess, lpDllPath, argv[2], dllPathLength, NULL)) {
					std::wcout << errMemWrite << std::endl;
					VirtualFreeEx(processInformation.hProcess, lpDllPath, 0, MEM_RELEASE);
				}
				else {
					HMODULE hModule = GetModuleHandle(L"KERNEL32.DLL");
					if (!hModule) {
						std::wcout << errKrnlNotFound << std::endl;
					}
					else {
						LPVOID lpLoadLibraryW = GetProcAddress(hModule, "LoadLibraryW");
						if (!lpLoadLibraryW) {
							std::wcout << errLdLibNotFound << std::endl;
						}
						else {
							HANDLE hThread = CreateRemoteThread(processInformation.hProcess, NULL, NULL, (LPTHREAD_START_ROUTINE)lpLoadLibraryW, lpDllPath, NULL, NULL);
							if (!hThread) {
								std::wcout << errRemThread << std::endl;							
							}
							else {
								std::wcout << msgThreadStartWait << std::endl;
								WaitForSingleObject(hThread, INFINITE);
								ResumeThread(processInformation.hThread);
								std::wcout << msgInjectionDone << std::endl;
								system("pause");
								return 0;
							}
						}
					}
					
				}
			}
		}
		CloseHandle(processInformation.hThread);
		CloseHandle(processInformation.hProcess);
	}	
	system("pause");
}