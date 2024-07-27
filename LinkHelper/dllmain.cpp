// This probably fucking sucks.
// If you can make it not fucking suck, that would be *great*.
#include "pch.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#include <fcntl.h> /* For O_* constants */
#endif // _WIN32
#include <stdio.h>
#include <iostream>
#include "dllmain.h"
#include <fstream>
#include <string>

struct LinkedMem {
#ifdef _WIN32
	UINT32	uiVersion;
	DWORD	uiTick;
#else
	uint32_t uiVersion;
	uint32_t uiTick;
#endif
	float	fAvatarPosition[3];
	float	fAvatarFront[3];
	float	fAvatarTop[3];
	wchar_t	name[256];
	float	fCameraPosition[3];
	float	fCameraFront[3];
	float	fCameraTop[3];
	wchar_t	identity[256];
#ifdef _WIN32
	UINT32	context_len;
#else
	uint32_t context_len;
#endif
	unsigned char context[256];
	wchar_t description[2048];
};
LinkedMem* lm = NULL;

extern "C" __declspec(dllexport) void initMumble() {
#ifdef _WIN32
	HANDLE hMapObject = OpenFileMappingW(FILE_MAP_ALL_ACCESS, FALSE, L"MumbleLink");
	if (hMapObject == NULL)
		return;

	lm = (LinkedMem*)MapViewOfFile(hMapObject, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(LinkedMem));
	if (lm == NULL) {
		CloseHandle(hMapObject);
		hMapObject = NULL;
		return;
	}

#else
	char memname[256];
	snprintf(memname, 256, "/MumbleLink.%d", getuid());
	int shmfd = shm_open(memname, O_RDWR, S_IRUSR | S_IWUSR);
	if (shmfd < 0) {
		return;
	}
	lm = (LinkedMem*)(mmap(NULL, sizeof(struct LinkedMem), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0));
	if (lm == (void*)(-1)) {
		lm = NULL;
		return;
	}

#endif
	printf("Mumble stuff set up");
}

extern "C" __declspec(dllexport) void updateMumble(wchar_t *identity, wchar_t *context, float fAvatarFrontX, float fAvatarFrontY, float fAvatarFrontZ, float fAvatarTopX, float fAvatarTopY, float fAvatarTopZ, float fAvatarPositionX, float fAvatarPositionY, float fAvatarPositionZ, float fCameraPositionX, float fCameraPositionY, float fCameraPositionZ, float fCameraFrontX, float fCameraFrontY, float fCameraFrontZ, float fCameraTopX, float fCameraTopY, float fCameraTopZ) {
	if (!lm)
		return;

	if (lm->uiVersion != 2) {
		wcsncpy(lm->name, L"GTMumbleLink", 256);
		wcsncpy(lm->description, L"Mumble Link for Gorilla Tag.", 2048);
		lm->uiVersion = 2;
	}
	lm->uiTick++;
	// Left handed coordinate system.
	// X positive towards "right".
	// Y positive towards "up".
	// Z positive towards "front".
	//
	// 1 unit = 1 meter

	// Unit vector pointing out of the avatar's eyes aka "At"-vector.
	lm->fAvatarFront[0] = fAvatarFrontX;
	lm->fAvatarFront[1] = fAvatarFrontY;
	lm->fAvatarFront[2] = fAvatarFrontZ;

	// Unit vector pointing out of the top of the avatar's head aka "Up"-vector (here Top points straight up).
	lm->fAvatarTop[0] = fAvatarTopX;
	lm->fAvatarTop[1] = fAvatarTopY;
	lm->fAvatarTop[2] = fAvatarTopZ;

	// Position of the avatar (here standing slightly off the origin)
	lm->fAvatarPosition[0] = fAvatarPositionX;
	lm->fAvatarPosition[1] = fAvatarPositionY;
	lm->fAvatarPosition[2] = fAvatarPositionY;


	// Same as avatar but for the camera.
	lm->fCameraPosition[0] = fCameraPositionX;
	lm->fCameraPosition[1] = fCameraPositionY;
	lm->fCameraPosition[2] = fCameraPositionZ;

	lm->fCameraFront[0] = fCameraFrontX;
	lm->fCameraFront[1] = fCameraFrontY;
	lm->fCameraFront[2] = fCameraFrontZ;

	lm->fCameraTop[0] = fCameraTopX;
	lm->fCameraTop[1] = fCameraTopY;
	lm->fCameraTop[2] = fCameraTopZ;

	// Identifier which uniquely identifies a certain player in a context (e.g. the ingame name).
	wcsncpy(lm->identity, identity, 256);

	// Context should be equal for players which should be able to hear each other positional and
	// differ for those who shouldn't (e.g. it could contain the server+port and team)
	memcpy(lm->context, context, 16);

	lm->context_len = 16;
}

extern "C" __declspec(dllexport) void testInterop(wchar_t str, float num) {
	std::ofstream interopString;
	interopString.open("interop-string.txt");
	interopString << str;
	interopString.close();

	std::ofstream interopFloat;
	interopFloat.open("interop-float.txt");
	interopFloat << num;
	interopFloat.close();
}