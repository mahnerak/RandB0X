// Windows API
#include <windows.h>
// IO functional
#include <stdio.h>
// IO functional for Unicode
#include <locale.h>

// RandB0X is using Unicode
#define UNICODE	

// Get LPCWSTR presentation of BOOL value
#define BOOLTOSTRING(x) ((x)?(L"true"):(L"false"))

// Copyright information is visible on start page
#define COPYRIGHT                                                         L"\n\
                                                                            \n\
         ██████   █████  ███    ██ ██████  ██████   █████  ██   ██          \n\
         ██   ██ ██   ██ ████   ██ ██   ██ ██   ██ ██   ██  ██ ██           \n\
         ██████  ███████ ██ ██  ██ ██   ██ ██████  ██ █ ██   ███            \n\
         ██   ██ ██   ██ ██  ██ ██ ██   ██ ██   ██ ██   ██  ██ ██           \n\
         ██   ██ ██   ██ ██   ████ ██████  ██████   █████  ██   ██          \n\
                                                                            \n\
             Copyright (c) Karen Hambardzumyan (Mahnerak) 2014              \n\
    ________________________________________________________________________\n"

// Usage information is visible on start page
#define USAGE												              L"\n\
    RandB0X for Windows, v0.1                                               \n\
    randb0x executable stdin stdout [timeLimit[memoryLimit[processes]]]     \n\
                                                                            \n\
     executable  -  path to Win32 executable                                \n\
     stdin       -  path to file for reading stdin                          \n\
     stdout      -  path to file for writing stdout                         \n\
     timeLimit   -  time limit in miliseconds                               \n\
                    (min: %ld ms, default: %ld ms, max: %ld ms)             \n\
     memoryLimit -  memory limit in MegaBytes                               \n\
                    (min: %Iu MB, default: %Iu MB, max: %Iu MB)             \n\
     processes   -  process count limit                                     \n\
                    (min: %u, default: %u, max: %u)                         \n\
                                                                            \n"
// Time limit (LONG, miliseconds) minimum, default and maximum values
#define MIN_TIME_LIMIT			((LONG)20)
#define DEFAULT_TIME_LIMIT		((LONG)1000)
#define MAX_TIME_LIMIT			((LONG)20000)

// Memory limit (SIZE_T, megabytes) minimum, default and maximum values
#define MIN_MEMORY_LIMIT		((SIZE_T)20)
#define DEFAULT_MEMORY_LIMIT	((SIZE_T)64)
#define MAX_MEMORY_LIMIT		((SIZE_T)2048)

// Active processes limit (DWORD, count) minimum, default and maximum values
#define MIN_ACTIVE_PROCESSES_LIMIT		((DWORD)1)
#define DEFAULT_ACTIVE_PROCESSES_LIMIT	((DWORD)1)
#define MAX_ACTIVE_PROCESSES_LIMIT		((DWORD)4)

// RandB0X dedicated user credentials
#define username	L"randb0x"
#define password	L"randb0x"

// Windows NT exit codes and signales table
struct STATUS {
	// Codename, Defined in WinNT.h
	LPWSTR	codename;
	// Exit code
	DWORD	code;
	// Status name
	LPWSTR	status;
} statuses[] = {
	{ L"STATUS_ABANDONED_WAIT_0", ((DWORD)0x00000080L), L"RE" },
	{ L"STATUS_USER_APC", ((DWORD)0x000000C0L), L"RE" },
	{ L"STATUS_TIMEOUT", ((DWORD)0x00000102L), L"TLE" },
	{ L"STATUS_PENDING", ((DWORD)0x00000103L), L"TLE" },
	{ L"DBG_EXCEPTION_HANDLED", ((DWORD)0x00010001L), L"RE" },
	{ L"DBG_CONTINUE", ((DWORD)0x00010002L), L"RE" },
	{ L"STATUS_SEGMENT_NOTIFICATION", ((DWORD)0x40000005L), L"RE" },
	{ L"STATUS_FATAL_APP_EXIT", ((DWORD)0x40000015L), L"RE" },
	{ L"DBG_TERMINATE_THREAD", ((DWORD)0x40010003L), L"RE" },
	{ L"DBG_TERMINATE_PROCESS", ((DWORD)0x40010004L), L"RE" },
	{ L"DBG_CONTROL_C", ((DWORD)0x40010005L), L"RE" },
	{ L"DBG_PRINTEXCEPTION_C", ((DWORD)0x40010006L), L"RE" },
	{ L"DBG_RIPEXCEPTION", ((DWORD)0x40010007L), L"RE" },
	{ L"DBG_CONTROL_BREAK", ((DWORD)0x40010008L), L"RE" },
	{ L"DBG_COMMAND_EXCEPTION", ((DWORD)0x40010009L), L"RE" },
	{ L"STATUS_GUARD_PAGE_VIOLATION", ((DWORD)0x80000001L), L"RE" },
	{ L"STATUS_DATATYPE_MISALIGNMENT", ((DWORD)0x80000002L), L"RE" },
	{ L"STATUS_BREAKPOINT", ((DWORD)0x80000003L), L"RE" },
	{ L"STATUS_SINGLE_STEP", ((DWORD)0x80000004L), L"RE" },
	{ L"STATUS_LONGJUMP", ((DWORD)0x80000026L), L"RE" },
	{ L"STATUS_UNWIND_CONSOLIDATE", ((DWORD)0x80000029L), L"RE" },
	{ L"DBG_EXCEPTION_NOT_HANDLED", ((DWORD)0x80010001L), L"RE" },
	{ L"STATUS_ACCESS_VIOLATION", ((DWORD)0xC0000005L), L"RE" },
	{ L"STATUS_IN_PAGE_ERROR", ((DWORD)0xC0000006L), L"RE" },
	{ L"STATUS_INVALID_HANDLE", ((DWORD)0xC0000008L), L"RE" },
	{ L"STATUS_INVALID_PARAMETER", ((DWORD)0xC000000DL), L"RE" },
	{ L"STATUS_NO_MEMORY", ((DWORD)0xC0000017L), L"MLE" },
	{ L"STATUS_ILLEGAL_INSTRUCTION", ((DWORD)0xC000001DL), L"RE" },
	{ L"STATUS_NONCONTINUABLE_EXCEPTION", ((DWORD)0xC0000025L), L"RE" },
	{ L"STATUS_INVALID_DISPOSITION", ((DWORD)0xC0000026L), L"RE" },
	{ L"STATUS_ARRAY_BOUNDS_EXCEEDED", ((DWORD)0xC000008CL), L"RE" },
	{ L"STATUS_FLOAT_DENORMAL_OPERAND", ((DWORD)0xC000008DL), L"RE" },
	{ L"STATUS_FLOAT_DIVIDE_BY_ZERO", ((DWORD)0xC000008EL), L"RE" },
	{ L"STATUS_FLOAT_INEXACT_RESULT", ((DWORD)0xC000008FL), L"RE" },
	{ L"STATUS_FLOAT_INVALID_OPERATION", ((DWORD)0xC0000090L), L"RE" },
	{ L"STATUS_FLOAT_OVERFLOW", ((DWORD)0xC0000091L), L"RE" },
	{ L"STATUS_FLOAT_STACK_CHECK", ((DWORD)0xC0000092L), L"RE" },
	{ L"STATUS_FLOAT_UNDERFLOW", ((DWORD)0xC0000093L), L"RE" },
	{ L"STATUS_INTEGER_DIVIDE_BY_ZERO", ((DWORD)0xC0000094L), L"RE" },
	{ L"STATUS_INTEGER_OVERFLOW", ((DWORD)0xC0000095L), L"RE" },
	{ L"STATUS_PRIVILEGED_INSTRUCTION", ((DWORD)0xC0000096L), L"RE" },
	{ L"STATUS_STACK_OVERFLOW", ((DWORD)0xC00000FDL), L"MLE" },
	{ L"STATUS_DLL_NOT_FOUND", ((DWORD)0xC0000135L), L"RE" },
	{ L"STATUS_ORDINAL_NOT_FOUND", ((DWORD)0xC0000138L), L"RE" },
	{ L"STATUS_ENTRYPOINT_NOT_FOUND", ((DWORD)0xC0000139L), L"RE" },
	{ L"STATUS_CONTROL_C_EXIT", ((DWORD)0xC000013AL), L"RE" },
	{ L"STATUS_DLL_INIT_FAILED", ((DWORD)0xC0000142L), L"RE" },
	{ L"STATUS_FLOAT_MULTIPLE_FAULTS", ((DWORD)0xC00002B4L), L"RE" },
	{ L"STATUS_FLOAT_MULTIPLE_TRAPS", ((DWORD)0xC00002B5L), L"RE" },
	{ L"STATUS_REG_NAT_CONSUMPTION", ((DWORD)0xC00002C9L), L"RE" },
	{ L"STATUS_HEAP_CORRUPTION", ((DWORD)0xC0000374L), L"MLE" },
	{ L"STATUS_STACK_BUFFER_OVERRUN", ((DWORD)0xC0000409L), L"MLE" },
	{ L"STATUS_INVALID_CRUNTIME_PARAMETER", ((DWORD)0xC0000417L), L"RE" },
	{ L"STATUS_ASSERTION_FAILURE", ((DWORD)0xC0000420L), L"RE" },
	{ L"STATUS_SXS_EARLY_DEACTIVATION", ((DWORD)0xC015000FL), L"RE" },
	{ L"STATUS_SXS_INVALID_DEACTIVATION", ((DWORD)0xC0150010L), L"RE" },
	{ L"STATUS_WAIT_0", ((DWORD)0x00000000L), L"OK" }
};

// Get information about signal and status by exit code
void lookupSignal(DWORD exitCode, BOOL* signaled, LPWSTR* signal, LPWSTR* status, BOOL* timeLimitExceeded, BOOL* memoryLimitExceeded){
	// Is process signaled
	*signaled = (exitCode & 0xF0000000) == 0xC0000000;
	
	// Pointer to status
	struct STATUS* index = statuses;
	
	// Search until reaching end of table
	do{
		// Checking if codes matching
		if (index->code == exitCode){
			// Signal matches codename
			*signal = index->codename;
			// So status matches status
			*status = index->status;
			// If status is TLE
			if (**status == 'T'){
				// then time limit is exceeded
				*timeLimitExceeded = TRUE;
			}
			// If status is MLE
			if (**status == 'M'){
				// then memory limit is exceeded
				*memoryLimitExceeded = TRUE;
			}
			return;
		}
	} while (index++->code);
	// Last element in table must have code STATUS_WAIT_0

	// If process is signaled then report about signal
	if(*signaled) {
		// Allocating required space for report
		*signal = (WCHAR*) malloc(2 << 6);
		// Printing report to signal with formating
		swprintf(*signal, 1 << 6, L"UNRECOGNIZED SIGNAL (0x%X)", exitCode);
		// If signaled then status is "Runtime Error"
		*status = L"RE";
		return;
	}

	// If exit code is not zero then report about code
	if (exitCode){
		// Allocating required space for report
		*signal = (WCHAR*)malloc(2 << 6);
		// Printing report to signal with formating
		swprintf(*signal, 1 << 6, L"UNRECOGNIZED CODE (0x%X)", exitCode);
		// If not signaled, status is "Non Zero Exit Code"
		*status = L"NZEC";
		return;
	}

	// Everything is OK
	*signal = L"OK";
	*status = L"OK";
	return;
}

// Main function. As said before, programm takes Unicode data
INT wmain(INT argc, LPWSTR argv[]){
	// Get Handle of himself
	HANDLE self = GetCurrentProcess();
	// Set priority to High
	SetPriorityClass(self, HIGH_PRIORITY_CLASS); 
	// Set error mode
	SetErrorMode(SEM_FAILCRITICALERRORS);
	// Setting up stdout for Unicode
	setlocale(LC_ALL, "");
	
	// Check for arguments count
	if (argc < 4){
		// If any of required arguments is not given then print usage page
		wprintf(
			COPYRIGHT USAGE,
			MIN_TIME_LIMIT,
			DEFAULT_TIME_LIMIT,
			MAX_TIME_LIMIT,
			MIN_MEMORY_LIMIT,
			DEFAULT_MEMORY_LIMIT,
			MAX_MEMORY_LIMIT,
			MIN_ACTIVE_PROCESSES_LIMIT,
			DEFAULT_ACTIVE_PROCESSES_LIMIT,
			MAX_ACTIVE_PROCESSES_LIMIT
		);
		// Report that run is not successful
		return (101);
	}

	// Executable path
	LPWSTR executablePath = argv[1];
	// Input (stdin) file path
	LPWSTR inputFilePath = argv[2];
	// Output (stdout) file path
	LPWSTR outputFilePath = argv[3];
	
	// Time limit (LONG, miliseconds)
	LONG timeLimit;
	// Check if time limit argument is given
	if (argc > 4){
		// Reading time limit
		swscanf_s(argv[4], L"%ld", &timeLimit);
		// Checking if given limit is valid
		if (timeLimit < MIN_TIME_LIMIT || timeLimit > MAX_TIME_LIMIT){
			// Reporting error
			wprintf(L"{"
				L"\"err\": \"RIE\","
				L"\"status\": \"RIE\","
				L"\"message\": \"Invalid time limit value\""
			L"}");
			return(0);
		}
	}
	else{
		// If limit is not given then use default value
		timeLimit = DEFAULT_TIME_LIMIT;
	}

	// Memory limit (SIZE_T, megabytes)
	SIZE_T memoryLimit;
	// Check if memory limit argument is given
	if (argc > 5){
		// Reading memory limit
		swscanf_s(argv[5], L"%Id", &memoryLimit);
		// Checking if given limit is valid
		if (memoryLimit < MIN_MEMORY_LIMIT || memoryLimit > MAX_MEMORY_LIMIT){
			// Reporting error
			wprintf(L"{"
				L"\"err\": \"RIE\","
				L"\"status\": \"RIE\","
				L"\"message\": \"Invalid memory limit value\""
			L"}");
			return(0);
		}
	}
	else{
		// If limit is not given then use default value
		memoryLimit = DEFAULT_MEMORY_LIMIT;
	}
	// Converting megabytes to bytes
	memoryLimit <<= 20;

	// Active processes limit (DWORD, count)
	DWORD activeProcessesLimit;
	// Check if active processes limit argument is given
	if (argc > 6){
		// Reading active processes limit
		swscanf_s(argv[6], L"%lu", &activeProcessesLimit);
		// Checking if given limit is valid
		if (activeProcessesLimit < MIN_ACTIVE_PROCESSES_LIMIT || activeProcessesLimit > MAX_ACTIVE_PROCESSES_LIMIT){
			// Reporting error
			wprintf(L"{"
				L"\"err\": \"RIE\","
				L"\"status\": \"RIE\","
				L"\"message\": \"Invalid active processes limit value\""
			L"}");
			return(0);
		}
	}
	else{
		// If limit is not given then use default value
		activeProcessesLimit = DEFAULT_ACTIVE_PROCESSES_LIMIT;
	}

	// Initializing Security Attributes
	SECURITY_ATTRIBUTES sa = {
		sizeof(SECURITY_ATTRIBUTES),
		NULL,
		TRUE
	};

	// Initializing new job
	HANDLE job = CreateJobObject(NULL, NULL);

	// Check if job is valid
	if (!job){
		// Get last error
		INT errorCode = GetLastError();
		// And report
		wprintf(L"{"
			L"\"err\": \"RIE\","
			L"\"status\": \"RIE\","
			L"\"message\": \"Can't CreateJobObject (Error code: %d)\""
		L"}", errorCode);
		return (0);
	}
	
	// Initializing limit information
	JOBOBJECT_EXTENDED_LIMIT_INFORMATION extLimit = {0};
	// Configuring limit flags
	extLimit.BasicLimitInformation.LimitFlags =
		JOB_OBJECT_LIMIT_JOB_TIME |	// Limit job CPU time
		JOB_OBJECT_LIMIT_JOB_MEMORY | // Limit job memory size
		JOB_OBJECT_LIMIT_PROCESS_TIME | // Limit process CPU time
		JOB_OBJECT_LIMIT_PROCESS_MEMORY | // Limit process memory size
		JOB_OBJECT_LIMIT_ACTIVE_PROCESS	  | // Limit child process count
		JOB_OBJECT_LIMIT_DIE_ON_UNHANDLED_EXCEPTION; // Die on unhandled exception

	// Configuring active processes limit
	extLimit.BasicLimitInformation.ActiveProcessLimit = activeProcessesLimit;

	// Configuring Job time limit
	extLimit.BasicLimitInformation.PerJobUserTimeLimit.QuadPart = timeLimit * 10000;
	// Configuring Process time limit
	extLimit.BasicLimitInformation.PerProcessUserTimeLimit.QuadPart = timeLimit * 10000;

	// Configuring Job memory limit
	extLimit.JobMemoryLimit = memoryLimit;
	// Configuring process memory limit
	extLimit.ProcessMemoryLimit = memoryLimit;

	// Trying to set limits 
	if (!SetInformationJobObject(job, JobObjectExtendedLimitInformation, &extLimit, sizeof(extLimit))){
		// Get last error
		INT errorCode = GetLastError();
		// And report
		wprintf(L"{"
			L"\"err\": \"RIE\","
			L"\"status\": \"RIE\","
			L"\"message\": \"Can't SetInformationJobObject (Error code: %d)\""
		L"}", errorCode);
		return (0);
	}

	// Initializing basic UI restrictions
	JOBOBJECT_BASIC_UI_RESTRICTIONS uiLimit = { 0 };
	// Configuring limit flags
	uiLimit.UIRestrictionsClass =
		JOB_OBJECT_UILIMIT_ALL      |
		JOB_OBJECT_UILIMIT_DESKTOP   |
		JOB_OBJECT_UILIMIT_HANDLES    |
		JOB_OBJECT_UILIMIT_GLOBALATOMS |
		JOB_OBJECT_UILIMIT_EXITWINDOWS  |
		JOB_OBJECT_UILIMIT_READCLIPBOARD |
		JOB_OBJECT_UILIMIT_WRITECLIPBOARD |
		JOB_OBJECT_UILIMIT_DISPLAYSETTINGS |
		JOB_OBJECT_UILIMIT_SYSTEMPARAMETERS;
	
	// Trying to set limits
	if (!SetInformationJobObject(job, JobObjectBasicUIRestrictions, &uiLimit, sizeof(uiLimit))){
		// Get last error
		INT errorCode = GetLastError();
		// And report
		wprintf(L"{"
			L"\"err\": \"RIE\","
			L"\"status\": \"RIE\","
			L"\"message\": \"Can't SetInformationJobObject (Error code: %d)\""
		L"}", errorCode);
		return (0);
	}

	// Initializing token
	HANDLE hToken = INVALID_HANDLE_VALUE;
	// Trying to login with randb0x user credentials
	if (!LogonUserW(username, NULL, password, LOGON32_LOGON_INTERACTIVE, LOGON32_PROVIDER_DEFAULT, &hToken)){
		// Get last error
		INT errorCode = GetLastError();
		// And report
		wprintf(L"{"
			L"\"err\": \"RIE\","
			L"\"status\": \"RIE\","
			L"\"message\": \"Can't LogonUserW (Error code: %d)\""
		L"}", errorCode);
		return (0);
	}

	// Initializing process info
	PROCESS_INFORMATION processInfo = {0};
	// Initializing startup info
	STARTUPINFOW pStaus = { sizeof(STARTUPINFOW) };
	// Configuring flags
	pStaus.dwFlags =
		STARTF_USESTDHANDLES|
		STARTF_USESHOWWINDOW;

	// Redirecting stdin and stdout
	pStaus.hStdInput = CreateFileW(inputFilePath, GENERIC_READ, 0, &sa, OPEN_ALWAYS, 0, NULL);
	pStaus.hStdOutput = CreateFileW(outputFilePath, GENERIC_WRITE, 0, &sa, CREATE_ALWAYS, 0, NULL);
	// Configuring windows state
	pStaus.wShowWindow = SW_HIDE;

	// Trying to create a process with user
	if (!CreateProcessAsUserW(hToken, executablePath, NULL, NULL, NULL, TRUE, CREATE_SUSPENDED, NULL, NULL, &pStaus, &processInfo)){
		// Get last error
		INT errorCode = GetLastError();
		// And report
		wprintf(L"{"
			L"\"err\": \"RIE\","
			L"\"status\": \"RIE\","
			L"\"message\": \"Can't CreateProcessAsUserW (Error code: %d)\""
		L"}", errorCode);
		return (0);
	}

	// Trying to assign created process to job
	if (!AssignProcessToJobObject(job, processInfo.hProcess)){
		// Terminate created process
		TerminateProcess(processInfo.hProcess, 0);
		// Get last error
		INT errorCode = GetLastError();
		// And report
		wprintf(L"{"
			L"\"err\": \"RIE\","
			L"\"status\": \"RIE\","
			L"\"message\": \"Can't AssignProcessToJobObject (Error code: %d)\""
		L"}", errorCode);
		return (0);
	}

	// Trying to set priority to High
	if (!SetPriorityClass(processInfo.hProcess, HIGH_PRIORITY_CLASS)){
		// Get last error
		INT errorCode = GetLastError();
		// And report
		wprintf(L"{"
			L"\"err\": \"RIE\","
			L"\"status\": \"RIE\","
			L"\"message\": \"Can't SetPriorityClass (Error code: %d)\""
		L"}", errorCode);
		return (0);
	}

	// Start counting ticks
	LONGLONG startTime = GetTickCount64();

	// Resuming thread
	ResumeThread(processInfo.hThread);

	// Initializing basic accounting and extended limit info
	JOBOBJECT_BASIC_ACCOUNTING_INFORMATION basicAcctInfo = {0};
	JOBOBJECT_EXTENDED_LIMIT_INFORMATION extLimitInfo = {0};

	// Waiting for process
	INT waitCode = WaitForSingleObject(processInfo.hProcess, 4 * timeLimit);

	// It can fail, so
	if (waitCode == WAIT_FAILED) {
		// Get last error
		INT errorCode = GetLastError();
		// And report
		wprintf(L"{"
			L"\"err\": \"RIE\","
			L"\"status\": \"RIE\","
			L"\"message\": \"Can't WaitForSingleObject (Error code: %d)\""
		L"}", errorCode);
		return (0);
	}

	// It can be timed out, so
	if (waitCode == WAIT_TIMEOUT) {
		// If job exists
		if (job != INVALID_HANDLE_VALUE) {
			// Terminate job
			if (!TerminateJobObject(job, 255)) {
			}
		}
		else {
			// Directly terminate process
			if (!TerminateProcess(processInfo.hProcess, 255)) {
			}
		}
	}
		
	// Register finish time
	LONGLONG finishTime = GetTickCount64();
		
	// Initializing exit code
	INT exitCode = 0;
	// Trying to get exit code
	if (!GetExitCodeProcess(processInfo.hProcess, (DWORD*)&exitCode)){
		// Get last error
		INT errorCode = GetLastError();
		// And report
		wprintf(L"{"
			L"\"err\": \"RIE\","
			L"\"status\": \"RIE\","
			L"\"message\": \"Can't GetExitCodeProcess (Error code: %d)\""
		L"}", errorCode);
		return (0);
	}
		
	// If job not exists
	if (job == INVALID_HANDLE_VALUE) {
		// Get lest error
		INT errorCode = GetLastError();
		// And report
		wprintf(L"{"
			L"\"err\": \"RIE\","
			L"\"status\": \"RIE\","
			L"\"message\": \"Invalid Handle Value (Error code: %d)\""
			L"}", errorCode);
		return (0);
	}

	// Try to get info about basic accounting
	if (!QueryInformationJobObject(job, JobObjectBasicAccountingInformation, &basicAcctInfo, sizeof(basicAcctInfo), NULL)) {
		// Get last error
		INT errorCode = GetLastError();
		// And report
		wprintf(L"{"
			L"\"err\": \"RIE\","
			L"\"status\": \"RIE\","
			L"\"message\": \"Can't QueryInformationJobObject (Error code: %d)\""
		L"}", errorCode);
		return (0);
	}

	// Try to get info about extended limit
	if (!QueryInformationJobObject(job, JobObjectExtendedLimitInformation, &extLimitInfo, sizeof(extLimitInfo), NULL)) {
		// Get last error
		INT errorCode = GetLastError();
		// And report
		wprintf(L"{"
			L"\"err\": \"RIE\","
			L"\"status\": \"RIE\","
			L"\"message\": \"Can't QueryInformationJobObject (Error code: %d)\""
		L"}", errorCode);
		return (0);
	}

	// Get last error
	INT errorCode = GetLastError();
	// Calculating user time
	LONGLONG userTime = (basicAcctInfo.TotalKernelTime.QuadPart + basicAcctInfo.TotalUserTime.QuadPart) / 10000;
	// Calculating real time
	LONGLONG realTime = finishTime - startTime;
	// Couting total active processes count
	INT totalProcesses = basicAcctInfo.TotalProcesses;
	// Log used memory
	LONGLONG usedMemory = extLimitInfo.PeakJobMemoryUsed;
	// Checking if user time limit is exceeded by comparing user time with limit
	BOOL cpuTimeLimitExceeded = userTime > timeLimit;
	// Checking if real time limit is exceeded by looking on wait code
	BOOL realTimeLimitExceeded = waitCode == WAIT_TIMEOUT;

	// Initializing limit info
	BOOL timeLimitExceeded = 0;
	BOOL memoryLimitExceeded = 0;
	BOOL activeProcessesLimitExceeded = 0;

	// Initializing signal and status info
	BOOL signaled = 0;
	LPWSTR signal = 0;
	LPWSTR status = 0;
	// Trying to find information
	lookupSignal(exitCode, &signaled, &signal, &status, &timeLimitExceeded, &memoryLimitExceeded);

	// If on of time limit is exceeded then time limit is exceeded
	timeLimitExceeded |= cpuTimeLimitExceeded || realTimeLimitExceeded;
	
	// If time limit is exceeded
	if (timeLimitExceeded){
		// then status is TLE
		status = L"TLE";
	}

	// If memory limit is exceeded
	if (memoryLimitExceeded){
		// then status is MLE
		status = L"MLE";
	}
	
	// Check if memory limit is exceeded
	memoryLimitExceeded |= usedMemory > memoryLimit;

	// Check if active processes limit is exceeded
	activeProcessesLimitExceeded = totalProcesses > activeProcessesLimit;

	// Write info
	wprintf(L"{\n"
		L"\t\"err\": null,\n"
		L"\t\"message\": \"Successful run\",\n"
		L"\t\"waitCode\": %d,\n"
		L"\t\"exitCode\": %d,\n"
		L"\t\"errorCode\": %d,\n"
		L"\t\"userTime\": %I64d,\n"
		L"\t\"realTime\": %I64d,\n"
		L"\t\"totalProcesses\": %d,\n"
		L"\t\"usedMemory\": %I64d,\n"
		L"\t\"cpuTimeLimitExceeded\": %s,\n"
		L"\t\"realTimeLimitExceeded\": %s,\n"
		L"\t\"timeLimitExceeded\": %s,\n"
		L"\t\"memoryLimitExceeded\": %s,\n"
		L"\t\"activeProcessesLimitExceeded\": %s,\n"
		L"\t\"signaled\": %s,\n"
		L"\t\"signal\": \"%s\",\n"
		L"\t\"status\": \"%s\"\n"
		L"}\n",
		waitCode, exitCode, errorCode, userTime, realTime, totalProcesses, usedMemory,
		BOOLTOSTRING(cpuTimeLimitExceeded),
		BOOLTOSTRING(realTimeLimitExceeded),
		BOOLTOSTRING(timeLimitExceeded),
		BOOLTOSTRING(memoryLimitExceeded),
		BOOLTOSTRING(activeProcessesLimitExceeded),
		BOOLTOSTRING(signaled),
		signal,
		status
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
