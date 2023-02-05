#include "types.h"
#include "riscv.h"
#include "param.h"
#include "spinlock.h"
#include "defs.h"
#include "sysinfo.h"
#include "proc.h"

int getSystemInfo(uint64 infoAddress)
{
	struct proc *p = myproc();
	struct sysinfo systemInfo;

	systemInfo.freemem = freeMemory();
	systemInfo.nproc = numProcesses();

	int didCopySucceed = copyout(p->pagetable, infoAddress, (char *)&systemInfo, sizeof(systemInfo));

	if (didCopySucceed == -1) {
		return -1;
	}

	return 0;
}