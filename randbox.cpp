#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

#define UNICODE
#define boolToString(x) ((x)?(L"true"):(L"false"))

#define COPYRIGHT                                                              L"\
                                                                               \n\
            ██████   █████  ███    ██ ██████  ██████   █████  ██   ██          \n\
            ██   ██ ██   ██ ████   ██ ██   ██ ██   ██ ██   ██  ██ ██           \n\
            ██████  ███████ ██ ██  ██ ██   ██ ██████  ██ █ ██   ███            \n\
            ██   ██ ██   ██ ██  ██ ██ ██   ██ ██   ██ ██   ██  ██ ██           \n\
            ██   ██ ██   ██ ██   ████ ██████  ██████   █████  ██   ██          \n\
                                                                               \n\
                Copyright (c) Karen Hambardzumyan (Mahnerak) 2014              \n\
 ______________________________________________________________________________\n\
\n"


#define minTimeLimit ((LONG)(20))
#define defaultTimeLimit ((LONG)(1000))
#define maxTimeLimit ((LONG)(20000))

#define minMemoryLimit ((SIZE_T)(20))
#define defaultMemoryLimit ((SIZE_T)(64))
#define maxMemoryLimit ((SIZE_T)(2048))

#define username L"randb0x"
#define password L"randb0x"



INT wmain(INT argc, LPWSTR argv[]){
	// Command line usage:
	// randbox <executable> <inputFilePath> <outputFilePath> [timeLimit] [memoryLimit]
	if (argc < 4){
		setlocale(0, LC_ALL);
		wprintf(
			COPYRIGHT
			L"  RandB0X for Windows, v0.1\n"
			L"  Usage: RandB0X executable inputFile outputFile [timeLimit] [memoryLimit]\n\n"
			L"  executable\tpath to Win32 executable\n"
			L"  inputFile\tpath to file for reading stdin\n"
			L"  outputFile\tpath to file for writing stdout\n"
			L"  timeLimit\ttime limit in miliseconds\n\t\t(min: %ld ms, default: %ld ms, max: %ld ms)\n"
			L"  memoryLimit\tmemory limit in MegaBytes\n\t\t(min: %Iu MB, default: %Iu MB, max: %Iu MB)\n\n",
			minTimeLimit,
			defaultTimeLimit,
			maxTimeLimit,
			minMemoryLimit,
			defaultMemoryLimit,
			maxMemoryLimit
		);
		return (101);
	}

	LPWSTR executablePath = argv[1];
	LPWSTR inputFilePath = argv[2];
	LPWSTR outputFilePath = argv[3];
	LONG timeLimit;

	if (argc > 4){
		swscanf_s(argv[4], L"%ld", &timeLimit);
		if (timeLimit < minTimeLimit || timeLimit > maxTimeLimit){
			wprintf(L"{"
				L"err: \"RIE\","
				L"message: \"Invalid time limit value\""
			L"}");
			return(0);
		}
	}
	else{
		timeLimit = defaultTimeLimit;
	}

	SIZE_T memoryLimit;
	if (argc > 5){
		swscanf_s(argv[5], L"%Id", &memoryLimit);
		if (memoryLimit < minMemoryLimit || memoryLimit > maxMemoryLimit){
			wprintf(L"{"
				L"err: \"RIE\","
				L"message: \"Invalid memory limit value\""
			L"}");
			return(0);
		}
	}
	else{
		memoryLimit = defaultMemoryLimit;
	}
	memoryLimit *= 1 << 20;

	SECURITY_ATTRIBUTES sa = {
		sizeof(SECURITY_ATTRIBUTES),
		NULL,
		TRUE
	};

	HANDLE job = CreateJobObject(NULL, NULL);

	if (!job){
		INT errorCode = GetLastError();
		wprintf(L"{"
			L"err: \"RIE\","
			L"message: \"Can't CreateJobObject (Error code: %d)\""
		L"}", errorCode);
		return (0);
	}


	JOBOBJECT_EXTENDED_LIMIT_INFORMATION extLimit = {0};
	extLimit.BasicLimitInformation.LimitFlags =
		JOB_OBJECT_LIMIT_JOB_TIME	|
		JOB_OBJECT_LIMIT_PROCESS_TIME |
		//JOB_OBJECT_LIMIT_JOB_MEMORY | 
		//JOB_OBJECT_LIMIT_PROCESS_MEMORY |
		JOB_OBJECT_LIMIT_ACTIVE_PROCESS	  |
		JOB_OBJECT_LIMIT_DIE_ON_UNHANDLED_EXCEPTION;

	extLimit.BasicLimitInformation.ActiveProcessLimit = 1;

	extLimit.BasicLimitInformation.PerJobUserTimeLimit.QuadPart = timeLimit * 10000;
	extLimit.BasicLimitInformation.PerProcessUserTimeLimit.QuadPart = timeLimit * 10000;

	extLimit.JobMemoryLimit = memoryLimit;
	extLimit.ProcessMemoryLimit = memoryLimit;

	if (!SetInformationJobObject(job, JobObjectExtendedLimitInformation, &extLimit, sizeof(extLimit))){
		INT errorCode = GetLastError();
		wprintf(L"{"
			L"err: \"RIE\","
			L"message: \"Can't SetInformationJobObject (Error code: %d)\""
		L"}", errorCode);
		return (0);
	}

	JOBOBJECT_BASIC_UI_RESTRICTIONS uiLimit = { 0 };
	uiLimit.UIRestrictionsClass =
		JOB_OBJECT_UILIMIT_DESKTOP   |
		JOB_OBJECT_UILIMIT_HANDLES    |
		JOB_OBJECT_UILIMIT_GLOBALATOMS |
		JOB_OBJECT_UILIMIT_EXITWINDOWS  |
		JOB_OBJECT_UILIMIT_READCLIPBOARD |
		JOB_OBJECT_UILIMIT_WRITECLIPBOARD |
		JOB_OBJECT_UILIMIT_DISPLAYSETTINGS |
		JOB_OBJECT_UILIMIT_SYSTEMPARAMETERS;
	
	if (!SetInformationJobObject(job, JobObjectBasicUIRestrictions, &uiLimit, sizeof(uiLimit))){
		INT errorCode = GetLastError();
		wprintf(L"{"
			L"err: \"RIE\","
			L"message: \"Can't SetInformationJobObject (Error code: %d)\""
		L"}", errorCode);
		return (0);
	}

	HANDLE hToken;
	if (!LogonUserW(username, NULL, password, LOGON32_LOGON_INTERACTIVE, LOGON32_PROVIDER_DEFAULT, &hToken)){
		INT errorCode = GetLastError();
		wprintf(L"{"
			L"err: \"RIE\","
			L"message: \"Can't LogonUserW (Error code: %d)\""
		L"}", errorCode);
		return (0);
	}

	PROCESS_INFORMATION processInfo;
	STARTUPINFOW pStaus = { sizeof(STARTUPINFOW) };
	pStaus.dwFlags =
		STARTF_USESTDHANDLES|
		STARTF_USESHOWWINDOW;
	pStaus.hStdInput = CreateFileW(inputFilePath, GENERIC_READ, 0, &sa, OPEN_ALWAYS, 0, NULL);
	pStaus.hStdOutput = CreateFileW(outputFilePath, GENERIC_WRITE, 0, &sa, CREATE_ALWAYS, 0, NULL);
	pStaus.wShowWindow = SW_HIDE;

	if (!CreateProcessAsUserW(hToken, executablePath, NULL, NULL, NULL, TRUE, CREATE_SUSPENDED, NULL, NULL, &pStaus, &processInfo)){
		INT errorCode = GetLastError();
		wprintf(L"{"
			L"err: \"RIE\","
			L"message: \"Can't CreateProcessAsUserW (Error code: %d)\""
		L"}", errorCode);
		return (0);
	}

	if (!AssignProcessToJobObject(job, processInfo.hProcess)){
		TerminateProcess(processInfo.hProcess, 0);
		INT errorCode = GetLastError();
		wprintf(L"{"
			L"err: \"RIE\","
			L"message: \"Can't AssignProcessToJobObject (Error code: %d)\""
			L"}", errorCode);
		return (0);
	}

	LONGLONG startTime = GetTickCount64();

	//SetPriorityClass(processInfo.hProcess, HIGH_PRIORITY_CLASS);
	ResumeThread(processInfo.hThread);
	JOBOBJECT_BASIC_ACCOUNTING_INFORMATION basicAcctInfo;
	JOBOBJECT_EXTENDED_LIMIT_INFORMATION extLimitInfo;


	INT waitCode = WaitForSingleObject(processInfo.hProcess, 4 * timeLimit);

	if (waitCode == WAIT_FAILED) {
		INT errorCode = GetLastError();
		wprintf(L"{"
			L"err: \"RIE\","
			L"message: \"Can't WaitForSingleObject (Error code: %d)\""
		L"}", errorCode);
		return (0);
	}

	if (waitCode == WAIT_TIMEOUT) {
		if (job != INVALID_HANDLE_VALUE) {
			if (!TerminateJobObject(job, 255)) {
			}
		}
		else {
			if (!TerminateProcess(processInfo.hProcess, 255)) {
			}
		}
	}
		

	LONGLONG finishTime = GetTickCount64();
		
	INT exitCode;
	if (!GetExitCodeProcess(processInfo.hProcess, (DWORD*)&exitCode)){
		INT errorCode = GetLastError();
		wprintf(L"{"
			L"err: \"RIE\","
			L"message: \"Can't GetExitCodeProcess (Error code: %d)\""
			L"}", errorCode);
		return (0);
	}
		
	if (job == INVALID_HANDLE_VALUE) {
		INT errorCode = GetLastError();
		wprintf(L"{"
			L"err: \"RIE\","
			L"message: \"Invalid Handle Value (Error code: %d)\""
			L"}", errorCode);
		return (0);
	}
	if (!QueryInformationJobObject(job, JobObjectBasicAccountingInformation, &basicAcctInfo, sizeof(basicAcctInfo), NULL)) {
		INT errorCode = GetLastError();
		wprintf(L"{"
			L"err: \"RIE\","
			L"message: \"Can't QueryInformationJobObject (Error code: %d)\""
			L"}", errorCode);
		return (0);
	}
	if (!QueryInformationJobObject(job, JobObjectExtendedLimitInformation, &extLimitInfo, sizeof(extLimitInfo), NULL)) {
		INT errorCode = GetLastError();
		wprintf(L"{"
			L"err: \"RIE\","
			L"message: \"Can't QueryInformationJobObject (Error code: %d)\""
			L"}", errorCode);
		return (0);
	}

	INT errorCode = GetLastError();
	LONGLONG userTime = (basicAcctInfo.TotalKernelTime.QuadPart + basicAcctInfo.TotalUserTime.QuadPart) / 10000;
	LONGLONG realTime = finishTime - startTime;
	INT activeProcesses = basicAcctInfo.ActiveProcesses;
	LONGLONG usedMemory = extLimitInfo.PeakJobMemoryUsed;
	BOOL cpuTimeLimitExceeded = userTime > timeLimit;
	BOOL realTimeLimitExceeded = waitCode == WAIT_TIMEOUT;
	BOOL timeLimitExceeded = cpuTimeLimitExceeded || realTimeLimitExceeded;
	BOOL memoryLimitExceeded = usedMemory > memoryLimit;
	BOOL signaled = (exitCode & 0xC0000000) == 0xC0000000;

	wprintf(L"{\n"
		L"err: null,\n"
		L"message: \"Successful run\",\n"
		L"waitCode: %d,\n"
		L"exitCode: %d,\n"
		L"errorCode: %d,\n"
		L"userTime: %I64d,\n"
		L"realTime: %I64d,\n"
		L"activeProcesses: %d,\n"
		L"usedMemory: %I64d,\n"
		L"cpuTimeLimitExceeded: %s,\n"
		L"realTimeLimitExceeded: %s,\n"
		L"timeLimitExceeded: %s,\n"
		L"memoryLimitExceeded: %s,\n"
		L"signaled: %s\n"
	L"}\n", 
		waitCode, exitCode, errorCode, userTime, realTime, activeProcesses, usedMemory, 
		boolToString(cpuTimeLimitExceeded),
		boolToString(realTimeLimitExceeded),
		boolToString(timeLimitExceeded),
		boolToString(memoryLimitExceeded),
		boolToString(signaled)
	);
		
	CloseHandle(pStaus.hStdInput);
	CloseHandle(pStaus.hStdOutput);
	CloseHandle(pStaus.hStdError);
	CloseHandle(processInfo.hThread);
	CloseHandle(processInfo.hProcess);
	TerminateJobObject(job, 0);
	CloseHandle(job);
	return 0;
}
