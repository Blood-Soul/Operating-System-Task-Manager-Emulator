#include<iostream>
#include<string>
#define DEVICENUM 100

//就绪队列优先级变化速度，以时间片为基本单位
#define WAITINGQUEUE_SPEED 2
#define ENJOINGQUEUE_SPEED 1
using namespace std;

double Clock = 0;

enum State{
    CREATED,
    REDEAY,
    OBSTRUCTED
};

struct Block{
    int kval;
    struct Block * llink;
    struct Block * rlink;
    int address;
};

struct RunInfo{
    double RequireTime; //总共需要的CPU时间
    double OccupyTime;  //已经占用CPU的时间
    double DeviceRunInfo[2][DEVICENUM];
    //0表示设备总共需要时间，1表示设备开始使用时间
};
struct PCB{
    int PID;
    string PName;
    string UserID;
    enum State State;
    int Priority;
    struct RunInfo PRunInfo;  //进程运行信息描述

    int StartAdd;
    struct Block* PBlock;
    struct PCB* prev;
    struct PCB* next;
};

//PCB队列
typedef struct{
    struct PCB* front;
    struct PCB* rear;
}PCB_Queue;

struct interrupt {
    int semaphore;
    int DeviceNo;
    double OccupyTime;
};

class ProcessManager{
private:
    PCB_Queue Created_PCBQueue;   //"新建"队列
    PCB_Queue Ready_PCBQueue[2];  //"就绪"队列  //线性优先级算法; 0表示新创建进程队列，1表示享受服务队列
    struct PCB* Obstruct_PCBList[DEVICENUM]; //"阻塞"表单

public:
    PCB * getCreatedProcess(PCB * HugeProcess = nullptr);       
    //获取新建进程,只读
    
    void popCreatedProcess(PCB * process);     
    //从新建队列中取出来
    
    void putProcessReady(PCB* process,bool New);         
    //放入就绪队列:新创建建进程队列（true）+享受服务队列(false)
    
    void popProcessObstruct(int DeviceNo);     //获取进程
    void putProcessObstruct(PCB* process,int DeviceNo);  //放入阻塞队列
    bool createProcess(int PID,string PName,string UserID,int Priority,struct RunInfo PRunInfo); //创建进程
    struct PCB* dispatchProcess(); //调度进程
    interrupt runProcess(struct PCB* process); //执行0.1个时间片，剥夺处理机(true)+不剥夺处理机(false)
    /* 返回值：
     * 0: 正常运行（未释放CPU）
     * 1: 阻塞（释放）
     * 2: 终止（释放）
     */
	void deleteProcess(PCB* process);  //终止进程;
    ProcessManager();
    ~ProcessManager();
};

PCB *ProcessManager::getCreatedProcess(PCB *HugeProcess) {
    PCB* RetProcess = nullptr;
    if(HugeProcess == nullptr){
        RetProcess = Created_PCBQueue.front->next;
    }
    else{
        RetProcess = HugeProcess->next;
    }
    return RetProcess;
}

void ProcessManager::popCreatedProcess(PCB *process) {
    PCB* temp = Created_PCBQueue.front;
    while(temp->next != process);
    temp->next = process->next;
}

void ProcessManager::putProcessReady(PCB *process, bool New) {
    process->State = REDEAY;
    if(New == false){
        this->Ready_PCBQueue[1].rear->next = process;
        process->next = nullptr;
        this->Ready_PCBQueue[1].rear = process;
    }
    else{
        this->Ready_PCBQueue[0].rear->next = process;
        process->next = nullptr;
        this->Ready_PCBQueue[0].rear = process;
    }
}

void ProcessManager::putProcessObstruct(PCB *process, int DeviceNo) {
    if(Obstruct_PCBList[DeviceNo] == nullptr){
        Obstruct_PCBList[DeviceNo] = process;
    }
    else{
        PCB* temp = Obstruct_PCBList[DeviceNo];
        while(temp->next);
        temp->next = process;
    }
    process->next = nullptr;
}

void ProcessManager::popProcessObstruct(int DeviceNo) {
    PCB* ReadyProcess = Obstruct_PCBList[DeviceNo];
    Obstruct_PCBList[DeviceNo] = ReadyProcess->next;
    putProcessReady(ReadyProcess, false);
}

bool ProcessManager::createProcess(int PID, std::string PName, std::string UserID, int Priority, struct RunInfo PRunInfo) {
    struct PCB* PcbPtr = new PCB;
    if(!PcbPtr) return false;

    PcbPtr->PID = PID;
    PcbPtr->PName.assign(PName);
    PcbPtr->UserID.assign(UserID);
    PcbPtr->Priority = Priority;
    PcbPtr->PRunInfo = PRunInfo;
    PcbPtr->next = NULL;

    PcbPtr->State = CREATED;

    this->Created_PCBQueue.rear->next = PcbPtr;
    this->Created_PCBQueue.rear = PcbPtr;

    return true;
}

PCB *ProcessManager::dispatchProcess() {
    //处理新创建队列队首
    if(this->Ready_PCBQueue[0].front != this->Ready_PCBQueue[0].rear){
        if(this->Ready_PCBQueue[1].front == this->Ready_PCBQueue[1].rear || this->Ready_PCBQueue[0].front->next->Priority >= this->Ready_PCBQueue[1].rear->Priority) {
            //从新创建队列取出进程
            PCB *temp = this->Ready_PCBQueue[0].front->next;
            this->Ready_PCBQueue[0].front->next = temp->next;
            if(temp == Ready_PCBQueue[0].rear) Ready_PCBQueue[0].rear=Ready_PCBQueue[0].front;
            //放入享受队列队尾
            this->Ready_PCBQueue[1].rear->next = temp;
            this->Ready_PCBQueue[1].rear = temp;
            temp->next = nullptr;
        }
    }

    //优先级动态变化
    PCB * front0 = this->Ready_PCBQueue[0].front;
    PCB * front1 = this->Ready_PCBQueue[1].front;
    PCB *temp = front0;
    while(temp->next != nullptr){
        temp->next->Priority += WAITINGQUEUE_SPEED;
        temp = temp->next;
    }
    temp = front1;
    while(temp->next != nullptr){
        temp->next->Priority += ENJOINGQUEUE_SPEED;
        temp = temp->next;
    }

    //调度进程
    PCB* DispatchProcess = NULL;
    if(this->Ready_PCBQueue[1].front == this->Ready_PCBQueue[1].rear) DispatchProcess = NULL;
    else{
        DispatchProcess = this->Ready_PCBQueue[1].front->next;
        this->Ready_PCBQueue[1].front->next = DispatchProcess->next;
        if(DispatchProcess == Ready_PCBQueue[1].rear) Ready_PCBQueue[1].rear = Ready_PCBQueue[1].front;
    }
    return DispatchProcess;
}

interrupt ProcessManager::runProcess(struct PCB *process) {
    process->PRunInfo.OccupyTime+=0.1;
    interrupt RetSemaphore={0};
    //判断该进程是否阻塞
    RunInfo runInfo = process->PRunInfo;
    for(int i=0;i<DEVICENUM;i++){
        if(runInfo.DeviceRunInfo[0][i]!=0 && runInfo.OccupyTime == runInfo.DeviceRunInfo[1][i]){
            RetSemaphore.semaphore=1;
            RetSemaphore.DeviceNo = i;
            RetSemaphore.OccupyTime = runInfo.DeviceRunInfo[1][i];
            break;
        }
    }

    //判断该进程是否终止
    if(runInfo.OccupyTime == runInfo.RequireTime){
        RetSemaphore.semaphore = 2;
    }

    return RetSemaphore;
}

void ProcessManager::deleteProcess(PCB *process) {
    delete process;
}

ProcessManager::ProcessManager() {
    //"新建"队列初始化
    struct PCB* HeadNodePtr = new PCB;
    this->Created_PCBQueue.front = HeadNodePtr;
    this->Created_PCBQueue.rear = HeadNodePtr;
    HeadNodePtr->next = NULL;

    //"就绪"队列数组初始化
    struct PCB* HeadNodePtr0 = new PCB;
    struct PCB* HeadNodePtr1 = new PCB;
    this->Ready_PCBQueue[0].front = HeadNodePtr0;
    this->Ready_PCBQueue[0].rear = HeadNodePtr0;
    this->Ready_PCBQueue[1].front = HeadNodePtr1;
    this->Ready_PCBQueue[1].rear = HeadNodePtr1;
    HeadNodePtr0->next = NULL;
    HeadNodePtr1->next = NULL;

    //"阻塞"初始化
    for(int i=0;i<DEVICENUM;i++){
        this->Obstruct_PCBList[i]= nullptr;
    }
}
ProcessManager::~ProcessManager() {
    //新建队列析构
    struct PCB* front = this->Created_PCBQueue.front;
    struct PCB* rear = this->Created_PCBQueue.rear;
    while(front != rear){
        struct PCB* temp = front->next;
        front->next = temp->next;
        if(rear == temp) rear=front;
        delete temp;
    }
    delete front;

    //"就绪"队列析构
    PCB * front0 = this->Ready_PCBQueue[0].front;
    PCB * rear0 = this->Ready_PCBQueue[0].rear;
    PCB * front1 = this->Ready_PCBQueue[1].front;
    PCB * rear1 = this->Ready_PCBQueue[1].rear;
    while(front0!=rear0){
        struct PCB* temp = front0;
        front0 = front0->next;
        delete temp->PBlock;
        delete temp;
    }
    delete front0->PBlock;
    delete front0;
    while(front1!=rear1){
        struct PCB* temp = front1;
        front1 = front1->next;
        delete temp->PBlock;
        delete temp;
    }
    delete front1->PBlock;
    delete front1;

    //"阻塞"队列析构
    for(int i=0;i<DEVICENUM;i++){
        if(this->Obstruct_PCBList[i] != nullptr){
            PCB* temp = this->Obstruct_PCBList[i];
            while(temp){
                PCB* TempCopy = temp;
                temp = temp->next;
                delete TempCopy->PBlock;
                delete TempCopy;
            }

        }

    }
}
