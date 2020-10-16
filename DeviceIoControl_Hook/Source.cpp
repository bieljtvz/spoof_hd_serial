#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <stdio.h>
#include <random>
#include <algorithm>
#include <random>
#include <tuple>
#include "detours.h"
#include "Spoof.h"

using namespace std;

string RandomString(int len)
{

	string str = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	string newstr;
	int pos;
	while (newstr.size() != len) {
		pos = ((rand() % (str.size() - 1)));
		newstr += str.substr(pos, 1);
	}
	return newstr;
}

typedef bool(WINAPI* pDeviceIoControl)
(
	HANDLE hDevice,
	DWORD        dwIoControlCode,
	LPVOID       lpInBuffer,
	DWORD        nInBufferSize,
	LPVOID       lpOutBuffer,
	DWORD        nOutBufferSize,
	LPDWORD      lpBytesReturned,
	LPOVERLAPPED lpOverlapped);
pDeviceIoControl oDeviceIoControl;

bool WINAPI hk_DeviceIoControl
(
	HANDLE hDevice,
	DWORD        dwIoControlCode,
	LPVOID       lpInBuffer,
	DWORD        nInBufferSize,
	LPVOID       lpOutBuffer,
	DWORD        nOutBufferSize,
	LPDWORD      lpBytesReturned,
	LPOVERLAPPED lpOverlapped)
{
	if (dwIoControlCode == IOCTL_STORAGE_QUERY_PROPERTY)
	{		
		if (oDeviceIoControl(hDevice, dwIoControlCode, lpInBuffer, nInBufferSize, lpOutBuffer, nOutBufferSize, lpBytesReturned, lpOverlapped))
		{
			int seed = 1;
			printf("Escolha a seed: ");
			scanf("%i",&seed);
			srand(seed);

			string random_str = RandomString(20);

			BYTE tmp = 0x7E; //Offset o inicio do Serial	
			int i = 0;
			while (tmp < 0x92) //0x92 termina o Serial;
			{
				if (*(BYTE*)((BYTE*)lpOutBuffer + tmp) != '\0')
					*(BYTE*)((BYTE*)lpOutBuffer + tmp) = random_str.c_str()[i];
				i++;
				tmp++;
			}
			return true;
		}
		return false;
	}
	else 
		return oDeviceIoControl(hDevice, dwIoControlCode, lpInBuffer, nInBufferSize, lpOutBuffer, nOutBufferSize, lpBytesReturned, lpOverlapped);

	
}


BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{


		DWORD64 DeviceIoControl_Address = (DWORD64)GetProcAddress(GetModuleHandleA("Kernel32.dll"), "DeviceIoControl");
		oDeviceIoControl = (pDeviceIoControl)DeviceIoControl_Address;
		printf("\n[DLL]DeviceIoContro: 0x%X\n", DeviceIoControl_Address);

		(VOID)DetourTransactionBegin();
		(VOID)DetourUpdateThread(GetCurrentThread());		
		(VOID)DetourAttach(&(PVOID&)oDeviceIoControl, hk_DeviceIoControl);
		(VOID)DetourTransactionCommit();

			
			
		
	}
	return TRUE;
}