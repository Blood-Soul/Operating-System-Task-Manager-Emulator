#include<iostream>
#include<string>
#define DEVICENUM 100

//就绪队列优先级变化速度，以时间片为基本单位
#define WAITINGQUEUE_SPEED 2
#define ENJOINGQUEUE_SPEED 1
using namespace std;

double Clock = 0;

//进程状态
enum State{
    CREATED,
    REDEAY,
    OBSTRUCTED
};

//内存块
struct Block{
    int kval;
    struct Block * llink;
    struct Block * rlink;
    int address;
};

//进程控制块
//内部所有描述都是相对信息
struct RunInfo{
    double RequireTime; //总共需要的CPU时间
    double OccupyTime;  //已经占用CPU的时间
    double DeviceRunInfo[3][DEVICENUM];
    //0表示设备总共需要时间，1表示设备开始使用时间（相对时间），2表示开始使用时间（绝对时间）
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

//进程管理器+调度器
class ProcessManager{
private:
    PCB_Queue Created_PCBQueue;   //"新建"队列
    PCB_Queue Ready_PCBQueue[2];  //"就绪"队列
    //线性优先级算法;0表示新创建进程队列，1表示享受服务队列
    struct PCB* Obstruct_PCBList[DEVICENUM]; //"阻塞"表单

public:

    PCB * getCreatedProcess();                //获取新建进程

    void putCreatedProcess(PCB* process);     //放回新建进程

    void putProcessReady(PCB* process,bool New);         //放入就绪队列:新创建建进程队列（true）+享受服务队列(false)

    void putProcessObstruct(PCB* process,int DeviceNo);  //放入阻塞队列

    bool createProcess(int PID,string PName,string UserID,int Priority,struct RunInfo PRunInfo); //创建进程

    struct PCB* dispatchProcess(); //调度进程

    void checkObstruct_PCBList(); //检查阻塞队列

    int runProcess(struct PCB* process,bool DepriveSemaphore= false); //执行0.1个时间片，剥夺处理机(true)+不剥夺处理机(false)
    /* 返回值：
     * 0: 正常运行（未释放CPU）
     * 1: 阻塞（释放）
     * 2: 终止（释放）
     */

    ProcessManager();
    ~ProcessManager();
};

void ProcessManager::putProcessReady(PCB *process,bool New) {
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

 void ProcessManager::putProcessObstruct(PCB *process,int DeviceNo) {
    process->State = OBSTRUCTED;
    if(this->Obstruct_PCBList[DeviceNo] == nullptr){
        this->Obstruct_PCBList[DeviceNo] = process;
        process->PRunInfo.DeviceRunInfo[2][DeviceNo] = Clock;
        process->next = nullptr;
    }
    else{
        PCB *temp = this->Obstruct_PCBList[DeviceNo];
        while(temp->next != nullptr) temp = temp->next;
        temp->next = process;
        process->next = nullptr;
    }
}

PCB *ProcessManager::getCreatedProcess() {
    if(Created_PCBQueue.front == Created_PCBQueue.rear) return nullptr;
    PCB * ret = Created_PCBQueue.front->next;
    Created_PCBQueue.front->next = ret->next;
    if(Created_PCBQueue.rear == ret) Created_PCBQueue.rear = Created_PCBQueue.front;
    return ret;
}

void ProcessManager::putCreatedProcess(PCB* process) {
    process->State = CREATED;
    if(Created_PCBQueue.front == Created_PCBQueue.rear){
        Created_PCBQueue.front->next = process;
        process->next = nullptr;
        Created_PCBQueue.rear = process;
    }
    else{
        process->next = Created_PCBQueue.front->next;
        Created_PCBQueue.front->next = process;
    }
}

ProcessManager::ProcessManager(){
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

ProcessManager::~ProcessManager(){
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

bool ProcessManager::createProcess(int PID,string PName,string UserID,int Priority,struct RunInfo PRunInfo){
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

struct PCB *ProcessManager::dispatchProcess() {
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


void ProcessManager::checkObstruct_PCBList(){
    for(int i=0;i<DEVICENUM;i++){
        if(this->Obstruct_PCBList[i] != nullptr && abs(this->Obstruct_PCBList[i]->PRunInfo.DeviceRunInfo[2][i]+this->Obstruct_PCBList[i]->PRunInfo.DeviceRunInfo[0][i]-Clock)<0.00001) {
            PCB * temp = this->Obstruct_PCBList[i];
            this->Obstruct_PCBList[i] = temp->next;
            putProcessReady(temp, false);
        }
    }
}

int ProcessManager::runProcess(struct PCB *process,bool DepriveSemaphore) {
    //执行0.1时间片
    process->PRunInfo.OccupyTime+=0.1;

    //判断该进程是否阻塞
    RunInfo runInfo = process->PRunInfo;
    for(int i=0;i<DEVICENUM;i++){
        if(runInfo.DeviceRunInfo[0][i]!=0 && runInfo.OccupyTime == runInfo.DeviceRunInfo[1][i]){
            putProcessObstruct(process,i);
            cout<<"该进程阻塞"<<endl;
            return 1;
        }
    }

    //判断该进程是否终止
    if(runInfo.OccupyTime == runInfo.RequireTime){
        cout<<"该进程终止"<<endl;
        return 2;
    }

    //判断是否被剥夺处理机
    if(DepriveSemaphore){
        putProcessReady(process, false);
    }
    return 0;
}
