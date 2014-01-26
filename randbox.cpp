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
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <boost\filesystem.hpp>

const LONG minTimeLimit	= 20;
const LONG defaultTimeLimit = 1000;
const LONG maxTimeLimit = 20000;

const SIZE_T minMemoryLimit = 20;
const SIZE_T defaultMemoryLimit	= 64;
const SIZE_T maxMemoryLimit	= 2048;

const LPCWSTR username = L"randb0x";
const LPCWSTR password = L"randb0x";

int wmain(int argc, wchar_t* argv[]){
	// Command line usage:
	// randbox <executable> <inputFilePath> <outputFilePath> [timeLimit] [memoryLimit]
	if(argc < 4){
		char* l = setlocale(LC_ALL, "");
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
	//input
	//executable
	LPWSTR executablePath = argv[1];
	if(!boost::filesystem::is_regular_file(executablePath)){
		printf("Invalid executable file.\n");
		return (102);
	}
	
	//input file
	LPWSTR inputFilePath = argv[2];
	if(!boost::filesystem::is_regular_file(inputFilePath)){
		printf("Invalid input file.\n");
		return (103);
	}

	//output file
	LPWSTR outputFilePath = argv[3];
	if(!boost::filesystem::is_regular_file(outputFilePath)){
		printf("Invalid output file.\n");
		return (104);
	}

	//time limit
	LONG timeLimit;
	if(argc > 4){
		swscanf(argv[4], L"%ld", &timeLimit);
		if(timeLimit < minTimeLimit || timeLimit > maxTimeLimit){
			printf("Invalid time limit.\n");
			return(105);
		}
	}
	else{
		timeLimit = defaultTimeLimit;
	}
	timeLimit *= 10000;

	//memory limit
	SIZE_T memoryLimit;
	if(argc > 5){
		swscanf(argv[5], L"%Id", &memoryLimit);
		if(memoryLimit < minMemoryLimit || memoryLimit > maxMemoryLimit){
			printf("Invalid memory limit.\n");
			return(106);
		}
	}
	else{
		memoryLimit = defaultMemoryLimit;
	}
	memoryLimit *= 1024 * 1024;

	SECURITY_ATTRIBUTES sa = {
		sizeof(SECURITY_ATTRIBUTES),
		NULL,
		TRUE
	};

	HANDLE job = CreateJobObject(NULL,NULL);

	if(!job){
		int errorCode = GetLastError();
		printf("RandB0X internal error. Can't CreateJobObject (Error code: %d)\n", errorCode);
		return (107);
	}
	
	// Time and Memory Limits

	JOBOBJECT_EXTENDED_LIMIT_INFORMATION extLimit = {0};

	extLimit.BasicLimitInformation.LimitFlags = 
		JOB_OBJECT_LIMIT_PROCESS_MEMORY | 
		JOB_OBJECT_LIMIT_PROCESS_TIME | 
		JOB_OBJECT_LIMIT_JOB_TIME | 
		JOB_OBJECT_LIMIT_ACTIVE_PROCESS | 
		JOB_OBJECT_LIMIT_DIE_ON_UNHANDLED_EXCEPTION;

	extLimit.BasicLimitInformation.ActiveProcessLimit = 1;

	extLimit.BasicLimitInformation.PerJobUserTimeLimit.QuadPart = timeLimit;
	extLimit.BasicLimitInformation.PerProcessUserTimeLimit.QuadPart = timeLimit;

	extLimit.JobMemoryLimit = memoryLimit;
	extLimit.ProcessMemoryLimit = memoryLimit;

	if(!SetInformationJobObject(job, JobObjectExtendedLimitInformation, &extLimit, sizeof(extLimit))){
		int errorCode = GetLastError();
		printf("RandB0X internal error. Can't SetInformationJobObject (Error L1 code: %d)\n", errorCode);
		return errorCode;
	}

	JOBOBJECT_BASIC_UI_RESTRICTIONS uiLimit = {0};
	uiLimit.UIRestrictionsClass = 
		JOB_OBJECT_UILIMIT_DESKTOP   |
		JOB_OBJECT_UILIMIT_HANDLES    |
		JOB_OBJECT_UILIMIT_GLOBALATOMS |
		JOB_OBJECT_UILIMIT_EXITWINDOWS  | 
		JOB_OBJECT_UILIMIT_READCLIPBOARD | 
		JOB_OBJECT_UILIMIT_WRITECLIPBOARD | 
		JOB_OBJECT_UILIMIT_DISPLAYSETTINGS | 
		JOB_OBJECT_UILIMIT_SYSTEMPARAMETERS ;

	if(!SetInformationJobObject(job, JobObjectBasicUIRestrictions, &uiLimit, sizeof(uiLimit))){
		int errorCode = GetLastError();
		printf("RandB0X internal error. Can't SetInformationJobObject (Error L2 code: %d)\n", errorCode);
		return errorCode;
	}

	
	//Security Limit
	
	HANDLE hToken;
	if(!LogonUser("randb0x", NULL, "randb0x", LOGON32_LOGON_INTERACTIVE, LOGON32_PROVIDER_DEFAULT, &hToken)){
		int errorCode = GetLastError();
		printf("RandB0X internal error. Can't LogonUser (Error code: %d)\n", errorCode);
		return errorCode;
	}
	
	PROCESS_INFORMATION pInfo;
	STARTUPINFOW pStaus = {sizeof(STARTUPINFOW)};
	pStaus.dwFlags = 

		STARTF_USESTDHANDLES | 
		STARTF_USESHOWWINDOW ;

	pStaus.hStdInput = CreateFileW(inputFilePath, GENERIC_READ, 0, &sa, OPEN_ALWAYS, 0, NULL);
	pStaus.hStdOutput = CreateFileW(outputFilePath, GENERIC_WRITE, 0, &sa, CREATE_ALWAYS, 0, NULL);
	pStaus.wShowWindow = SW_HIDE;
	
	if(!CreateProcessAsUserW(hToken, executablePath, NULL, NULL, NULL, TRUE, CREATE_SUSPENDED, NULL, NULL, &pStaus , &pInfo)){
		int errorCode = GetLastError();
		printf("RandB0X internal error. Can't CreateProcessWithLogonW (Error code: %d)\n", errorCode);
		return errorCode;
	}

	if(!AssignProcessToJobObject(job, pInfo.hProcess)){
		TerminateProcess(pInfo.hProcess,0);
		int errorCode = GetLastError();
		printf("RandB0X internal error. Can't AssignProcessToJobObject (Error code: %d)\n", errorCode);
		return errorCode;
	}
	
	ResumeThread(pInfo.hThread);

	DWORD waitStatus = WaitForSingleObject(pInfo.hProcess, INFINITE);
	if(waitStatus == WAIT_FAILED){
		int errorCode = GetLastError();
		printf("RandB0X internal error. Can't WaitForSingleObject (Error code: %d)\n", errorCode);
		return errorCode;
	}else{
		printf("waitStatus: %d\n", waitStatus);
	}


	CloseHandle(pStaus.hStdInput);
	CloseHandle(pStaus.hStdOutput);
	CloseHandle(pStaus.hStdError);
	CloseHandle(pInfo.hThread);
	CloseHandle(pInfo.hProcess);
	TerminateJobObject(job,0);
	CloseHandle(job);
    return 0;  
}
