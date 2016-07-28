#include <windows.h>

int main(void)
{

	//50%
	int busyTime = 10;
	int idleTime = busyTime;
	_int64 startTime;
	SetThreadAffinityMask(GetCurrentProcess(), 0x00000001);
	while (true)
	{
		startTime = GetTickCount();
		while ((GetTickCount() - startTime) <= busyTime)
		{
			;
		}
		Sleep(idleTime);
	}
	return 0;
}