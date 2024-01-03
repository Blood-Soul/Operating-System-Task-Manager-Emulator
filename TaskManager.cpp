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
	int timeslice;//1个timeslice表示0.1个时间片
public:
	TaskManager() {
		CPU = NULL;
		timeslice = 0;
	}

    PCB_Queue getCreated_PCBQueue(){
        return PM.getCreated_PCBQueue();
    };

    PCB_Queue* getReady_PCBQueue(){
        return PM.getReady_PCBQueue();
    };

    PCB ** getObstruct_PCBList(){
        return PM.getObstruct_PCBList();
    };

    PCB * getExecuting_PCB(){
        return CPU;
    }

    void createNewTask(string TaskName){

    }

    void terminateprocess(string processName){

    }

	void input(int PID,std::string PName, std::string UserID, int Priority, struct RunInfo PRunInfo, int size) {
		//输入进程并创建
		PM.createProcess(PID, PName, UserID, Priority, PRunInfo, size);
	}
	void allocateMemory() {
		//尝试为新建进程中的每一个进程分配一次内存
		struct PCB* tryingAllocPCB = NULL;
		struct PCB* triedAllocPCB = NULL;
		while ((tryingAllocPCB = PM.getCreatedProcess(triedAllocPCB)) != NULL) {
			if ((tryingAllocPCB->PBlock = MM.alloc(tryingAllocPCB->size)) != NULL) {
				PM.popCreatedProcess(tryingAllocPCB);
				PM.putProcessReady(tryingAllocPCB, true);
				cout << tryingAllocPCB->PID << "已分配内存" << endl;//仅做测试使用
			}
			else {
				triedAllocPCB = tryingAllocPCB;
			}
		}
	}
	void run() {
		//运行进程和设备
		if (CPU == NULL) {
			CPU = PM.dispatchProcess();
			if(CPU != nullptr) cout << CPU->PID << "被调度" << endl;
			timeslice = 0;
		}
		if (CPU != NULL) {
			//cout << CPU->PRunInfo.OccupyTime << endl;
			//cout << timeslice << endl;
			struct interrupt intSemaphore = PM.runProcess(CPU);
			vector<int>doneDeviceNo = DM.runDevice();
			timeslice++;
			//cout << intSemaphore.semaphore << endl;
			if (intSemaphore.semaphore == 1) {
				cout << CPU->PID << "进程中断" << endl;//仅做测试使用
				PM.putProcessObstruct(CPU, intSemaphore.DeviceNo);
				DM.occupyDevice(intSemaphore.DeviceNo, intSemaphore.OccupyTime);
				CPU = NULL;
			}
			else if (intSemaphore.semaphore == 2) {
				cout << CPU->PID << "进程结束" << endl;//仅做测试使用
				MM.release(CPU->PBlock);
				PM.deleteProcess(CPU);
				CPU = NULL;
			}
			else {
				if (timeslice == 10) {
					cout << CPU->PID << "时间片满" << endl;//仅做测试使用
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
	ri.RequireTime = 7;
	ri.OccupyTime = 0;
	for (int i = 0; i < DEVICENUM; i++) {
		ri.DeviceRunInfo[0][i] = ri.DeviceRunInfo[1][i] = 0;
	}
	for (int i = 0; i < 5; i++) {
		ri.DeviceRunInfo[0][i] = i;
		ri.DeviceRunInfo[1][i] = 2;
		TM.input(i, "a" + i, "xbj", 3, ri, 1000);
		//ri.DeviceRunInfo[0][i] = 0;
		ri.DeviceRunInfo[1][i] = 0;
	}
	for (int j = 0; j < 600; j++) {
		cout << j << endl;
		TM.allocateMemory();
		TM.run();
	}
	cout << "没死！" << endl;
	system("pause");
	return 0;
}
