#include"ProcessManager.h"
#include"MemoryManager.h"
#include"DeviceManager.h"
using namespace std;

class TaskManager {
private:
	MemoryManager MM;
	ProcessManager PM;
	DeviceManager DM;
	struct PCB* CPU;
	double timeslice;
public:
	TaskManager() {
		CPU = NULL;
		timeslice = 0;
	}
	void input(int PID, std::string PName, std::string UserID, int Priority, struct RunInfo PRunInfo) {
		//输入进程并创建
		PM.createProcess(PID, PName, UserID, Priority, PRunInfo);
	}
	void allocateMemory() {
		//尝试为新建进程中的每一个进程分配一次内存
		struct PCB* tryAllocPCB = NULL;
		while ((tryAllocPCB = PM.getCreatedProcess(tryAllocPCB)) != NULL) {
			if ((tryAllocPCB->PBlock = MM.alloc(tryAllocPCB->size)) != NULL) {
				PM.popCreatedProcess(tryAllocPCB);
				PM.putProcessReady(tryAllocPCB, true);
			}
		}
	}
	void run() {
		//运行进程和设备
		if (CPU == NULL) {
			CPU = PM.dispatchProcess();
			timeslice = 0;
		}
		if (CPU != NULL) {
			struct interrupt intSemaphore = PM.runProcess(CPU);
			vector<int>doneDeviceNo = DM.runDevice();
			timeslice += 0.1;
			if (intSemaphore.semaphore == 1) {
				PM.putProcessObstruct(CPU, intSemaphore.DeviceNo);
				DM.occupyDevice(intSemaphore.DeviceNo, intSemaphore.OccupyTime);
				CPU = NULL;
			}
			else if (intSemaphore.semaphore == 2) {
				MM.release(CPU->PBlock);
				PM.deleteProcess(CPU);
				CPU = NULL;
			}
			else {
				if (timeslice == 1) {
					PM.putProcessReady(CPU, false);
					CPU = NULL;
				}
			}
			for (int i = 0; i < doneDeviceNo.size(); i++) {
				PM.popProcessObstruct(doneDeviceNo[i]);
			}
		}
	}
	~TaskManager() {
		CPU = NULL;
	}
};

int main() {
	TaskManager TM;
	RunInfo ri;
	ri.RequireTime = 10;
	ri.OccupyTime = 0;
	for (int i = 0; i < DEVICENUM; i++) {
		ri.DeviceRunInfo[0][i] = ri.DeviceRunInfo[1][i] = 0;
	}
	TM.input(2, "aaa", "xbj", 3, ri);
	TM.allocateMemory();
	system("pause");
	return 0;
}
