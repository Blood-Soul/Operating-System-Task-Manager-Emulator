#include<math.h>
#include<iostream>
#include<queue>
#include<iomanip>
using namespace std;

#define MAXKVAL 16		  //�ڴ�������
#define DEVICENUM 100

//���ڴ�
struct Memory {
	int Size;
	int SystemAreaSize;
	int UserAreaSize;
};

//�ڴ��
struct Block {//��ʼ״̬��Block����ָ��ָ���Լ�
	int kval;//����
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
	struct RunInfo PRunInfo;  //����������Ϣ����
	int size;
	struct Block* PBlock;
	struct PCB* next;
};

struct RunInfo {
	double RequireTime; //�ܹ���Ҫ��ʱ��
	double OccupyTime;  //ռ��CPU��ʱ��
	double DeviceRunInfo[2][DEVICENUM];
	//0��ʾ�豸�ܹ���Ҫʱ�䣬1��ʾ�豸��ʼʹ��ʱ�� 
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

class BlockManager {//����block�ڿ��ñ��ϵ�push��pop
public:
	void pushBlock(Block* block, Block* availList[]);
	void popBlock(Block* block, Block* availList[]);
};

class MemoryManager {
private:
	struct Memory memory;
	struct Block* availList[MAXKVAL + 1];//�ڴ���ñ�
	BlockManager BM;
public:
	MemoryManager();
	struct Block* alloc(int size);
	void release(struct Block* block);
	~MemoryManager();
};

class ProcessManager {
private:
	PCB_Queue Created_PCBQueue;   //"�½�"����
	PCB_Queue Ready_PCBQueue[2];  //"����"����
	//�������ȼ��㷨;0��ʾ�´������̶��У�1��ʾ���ܷ������
	struct PCB* Obstruct_PCBList[DEVICENUM]; //"����"��

public:

	PCB* getCreatedProcess(PCB* process = nullptr);                //��ȡ�½�����,ֻ��

	void popCreatedProcess(PCB* process);     //���½�������ȡ����

	void putProcessReady(PCB* process, bool New);         //�����������:�´��������̶��У�true��+���ܷ������(false)

	PCB* popProcessObstruct(int DeviceNo);     //��ȡ���� 

	void putProcessObstruct(PCB* process, int DeviceNo);  //������������

	bool createProcess(int PID, string PName, string UserID, int Priority, struct RunInfo PRunInfo); //��������

	struct PCB* dispatchProcess(); //���Ƚ���


	struct interrupt runProcess(struct PCB* process); //ִ��0.1��ʱ��Ƭ�����ᴦ���(true)+�����ᴦ���(false)
	/* ����ֵ��
	 * 0: �������У�δ�ͷ�CPU��
	 * 1: �������ͷţ�
	 * 2: ��ֹ���ͷţ�
	 */

	void deleteProcess(PCB* process);

	ProcessManager();
	~ProcessManager();
};

class DeviceManager {
private:
	queue<double> DeviceState[DEVICENUM];  // �豸״̬

public:
	// ���캯��
	DeviceManager();

	void occupyDevice(int DeviceNo, double Time);
	//ʹ���豸���ٶ�Ӧ�豸�ŵ�queue��β����һ��Time 

	vector<int> runDevice(); //�����豸ʱ�����0.1;
	//�����豸����ʹ�����豸��queue����Ԫ�ؼ�ȥ0.1���������ڸò��������Ԫ��Ϊ0���豸�ţ����ɾ���ö���Ԫ�� 
	//�豸����vector���䣬��¼���г�������������豸�� 

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
		//������̲�����
	}
	void allocateMemory() {
		//����Ϊ�½������е�ÿһ�����̷���һ���ڴ�
		struct PCB* tryAllocPCB = NULL;
		while ((tryAllocPCB = PM.getCreatedProcess(tryAllocPCB)) != NULL) {
			if ((tryAllocPCB->PBlock = MM.alloc(tryAllocPCB->size)) != NULL) {
				PM.popCreatedProcess(tryAllocPCB);
				PM.putProcessReady(tryAllocPCB, true);
			}
		}
	}
	void run() {
		//���н��̺��豸
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