#include<math.h>
#include<iostream>
#include<queue>
#include<iomanip>
using namespace std;

#define MAXKVAL 16		  //内存块最大级数
#define DEVICENUM 100

//总内存
struct Memory {
	int Size;
	int SystemAreaSize;
	int UserAreaSize;
};

//内存块
struct Block {//初始状态的Block左右指针指向自己
	int kval;//级数
	struct Block* rlink;
	struct Block* llink;
	int address;
};

struct PCB {
	int PID;
	string PName;
	string UserID;
	enum State State;
	int Priority;
	int StartAdd;
	struct RunInfo PRunInfo;  //进程运行信息描述
	int size;
	struct Block* PBlock;
	struct PCB* next;
};

struct RunInfo {
	double RequireTime; //总共需要的时间
	double OccupyTime;  //占用CPU的时间
	double DeviceRunInfo[2][DEVICENUM];
	//0表示设备总共需要时间，1表示设备开始使用时间 
};

enum State {
	CREATED,
	REDEAY,
	OBSTRUCTED
};

typedef struct {
	PCB* front;
	PCB* rear;
}PCB_Queue;

struct interrupt {
	int semaphore;
	int DeviceNo;
	double OccupyTime;
};

class BlockManager {//控制block在可用表上的push和pop
public:
	void pushBlock(Block* block, Block* availList[]);
	void popBlock(Block* block, Block* availList[]);
};

class MemoryManager {
private:
	struct Memory memory;
	struct Block* availList[MAXKVAL + 1];//内存可用表
	BlockManager BM;
public:
	MemoryManager();
	struct Block* alloc(int size);
	void release(struct Block* block);
	~MemoryManager();
};

class ProcessManager {
private:
	PCB_Queue Created_PCBQueue;   //"新建"队列
	PCB_Queue Ready_PCBQueue[2];  //"就绪"队列
	//线性优先级算法;0表示新创建进程队列，1表示享受服务队列
	struct PCB* Obstruct_PCBList[DEVICENUM]; //"阻塞"表单

public:

	PCB* getCreatedProcess(PCB* process = nullptr);                //获取新建进程,只读

	void popCreatedProcess(PCB* process);     //从新建队列中取出来

	void putProcessReady(PCB* process, bool New);         //放入就绪队列:新创建建进程队列（true）+享受服务队列(false)

	PCB* popProcessObstruct(int DeviceNo);     //获取进程 

	void putProcessObstruct(PCB* process, int DeviceNo);  //放入阻塞队列

	bool createProcess(int PID, string PName, string UserID, int Priority, struct RunInfo PRunInfo); //创建进程

	struct PCB* dispatchProcess(); //调度进程


	struct interrupt runProcess(struct PCB* process); //执行0.1个时间片，剥夺处理机(true)+不剥夺处理机(false)
	/* 返回值：
	 * 0: 正常运行（未释放CPU）
	 * 1: 阻塞（释放）
	 * 2: 终止（释放）
	 */

	void deleteProcess(PCB* process);

	ProcessManager();
	~ProcessManager();
};

class DeviceManager {
private:
	queue<double> DeviceState[DEVICENUM];  // 设备状态

public:
	// 构造函数
	DeviceManager();

	void occupyDevice(int DeviceNo, double Time);
	//使用设备：再对应设备号的queue队尾加上一个Time 

	vector<int> runDevice(); //所有设备时间减少0.1;
	//运行设备：把使用中设备的queue队首元素减去0.1，并返回在该操作后队首元素为0的设备号，最后删除该队首元素 
	//设备号用vector传输，记录所有出现上述情况的设备号 

};

class TaskManager {
private:
	MemoryManager MM;
	ProcessManager PM;
	DeviceManager DM;
	struct PCB* CPU;
	int timeslice;
public:
	TaskManager() {
		CPU = NULL;
		timeslice = 0;
	}
	void input(){
		//输入进程并创建
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
		struct interrupt intSemaphore = PM.runProcess(CPU);
		vector<int>doneDeviceNo = DM.runDevice();
		timeslice++;
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
			if (timeslice == 10) {
				PM.putProcessReady(CPU, false);
				CPU = NULL;
			}
		}
		for (int i = 0; i < doneDeviceNo.size(); i++) {
			PM.putProcessReady(PM.popProcessObstruct(doneDeviceNo[i]), false);
		}
	}
	~TaskManager() {
		CPU = NULL;
	}
};

int main() {
	return 0;
}