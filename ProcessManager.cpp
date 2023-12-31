#include"ProcessManager.h"

PCB* ProcessManager::getCreatedProcess(PCB* HugeProcess) {
    PCB* RetProcess = nullptr;
    if (HugeProcess == nullptr) {
        RetProcess = Created_PCBQueue.front->next;
    }
    else {
        RetProcess = HugeProcess->next;
    }
    return RetProcess;
}

void ProcessManager::popCreatedProcess(PCB* process) {
    PCB* temp = Created_PCBQueue.front;
    while (temp->next != process);
    temp->next = process->next;
    if(temp->next == nullptr){
        Created_PCBQueue.rear = Created_PCBQueue.front;
    }
}

void ProcessManager::putProcessReady(PCB* process, bool New) {
    process->State = REDEAY;
    if (!New) {
        this->Ready_PCBQueue[1].rear->next = process;
        process->next = nullptr;
        this->Ready_PCBQueue[1].rear = process;
    }
    else {
        this->Ready_PCBQueue[0].rear->next = process;
        process->next = nullptr;
        this->Ready_PCBQueue[0].rear = process;
    }
}

void ProcessManager::putProcessObstruct(PCB* process, int DeviceNo) {
    if (Obstruct_PCBList[DeviceNo] == nullptr) {
        Obstruct_PCBList[DeviceNo] = process;
    }
    else {
        PCB* temp = Obstruct_PCBList[DeviceNo];
        while (temp->next);
        temp->next = process;
    }
    process->next = nullptr;
}

void ProcessManager::popProcessObstruct(int DeviceNo) {
    PCB* ReadyProcess = Obstruct_PCBList[DeviceNo];
    Obstruct_PCBList[DeviceNo] = ReadyProcess->next;
    putProcessReady(ReadyProcess, false);
}

bool ProcessManager::createProcess(int PID, std::string PName, std::string UserID, int Priority, struct RunInfo PRunInfo, int size) {
    PCB* PcbPtr = new PCB;
    if (!PcbPtr) return false;
    PcbPtr->PID = PID;
    PcbPtr->PName.assign(PName);
    PcbPtr->UserID.assign(UserID);
    PcbPtr->Priority = Priority;
    PcbPtr->PRunInfo = PRunInfo;
    PcbPtr->size = size;
    PcbPtr->next = nullptr;

    PcbPtr->State = CREATED;

    this->Created_PCBQueue.rear->next = PcbPtr;
    this->Created_PCBQueue.rear = PcbPtr;

    cout << PID << "The process has been created successfully!" << endl;//仅做测试使用

    return true;
}

PCB* ProcessManager::dispatchProcess() {
    //处理新创建队列队首
    if (this->Ready_PCBQueue[0].front != this->Ready_PCBQueue[0].rear) {
        if (this->Ready_PCBQueue[1].front == this->Ready_PCBQueue[1].rear || this->Ready_PCBQueue[0].front->next->Priority >= this->Ready_PCBQueue[1].rear->Priority) {
            //从新创建队列取出进程
            PCB* temp = this->Ready_PCBQueue[0].front->next;
            this->Ready_PCBQueue[0].front->next = temp->next;
            if (temp == Ready_PCBQueue[0].rear) Ready_PCBQueue[0].rear = Ready_PCBQueue[0].front;
            //放入享受队列队尾
            this->Ready_PCBQueue[1].rear->next = temp;
            this->Ready_PCBQueue[1].rear = temp;
            temp->next = nullptr;
        }
    }

    //优先级动态变化
    PCB* front0 = this->Ready_PCBQueue[0].front;
    PCB* front1 = this->Ready_PCBQueue[1].front;
    PCB* temp = front0;
    while (temp->next != nullptr) {
        temp->next->Priority += WAITINGQUEUE_SPEED;
        temp = temp->next;
    }
    temp = front1;
    while (temp->next != nullptr) {
        temp->next->Priority += ENJOINGQUEUE_SPEED;
        temp = temp->next;
    }

    //调度进程
    PCB* DispatchProcess = nullptr;
    if (this->Ready_PCBQueue[1].front == this->Ready_PCBQueue[1].rear) DispatchProcess = nullptr;
    else {
        DispatchProcess = this->Ready_PCBQueue[1].front->next;
        this->Ready_PCBQueue[1].front->next = DispatchProcess->next;
        if (DispatchProcess == Ready_PCBQueue[1].rear) Ready_PCBQueue[1].rear = Ready_PCBQueue[1].front;
    }
    return DispatchProcess;
}

interrupt ProcessManager::runProcess(struct PCB* process) {
    process->PRunInfo.OccupyTime += 0.1;
    interrupt RetSemaphore = { 0 };
    //判断该进程是否阻塞
    RunInfo runInfo = process->PRunInfo;
    for (int i = 0; i < DEVICENUM; i++) {
        if (runInfo.DeviceRunInfo[0][i] != 0 && runInfo.OccupyTime == runInfo.DeviceRunInfo[1][i]) {
            RetSemaphore.semaphore = 1;
            RetSemaphore.DeviceNo = i;
            RetSemaphore.OccupyTime = runInfo.DeviceRunInfo[1][i];
            break;
        }
    }

    //判断该进程是否终止
    if (runInfo.OccupyTime == runInfo.RequireTime) {
        RetSemaphore.semaphore = 2;
    }

    return RetSemaphore;
}

void ProcessManager::deleteProcess(PCB* process) {
    delete process;
}

ProcessManager::ProcessManager() {
    //"新建"队列初始化
    PCB* HeadNodePtr = new PCB;
    this->Created_PCBQueue.front = HeadNodePtr;
    this->Created_PCBQueue.rear = HeadNodePtr;
    HeadNodePtr->next = nullptr;

    //"就绪"队列数组初始化
    PCB* HeadNodePtr0 = new PCB;
    PCB* HeadNodePtr1 = new PCB;
    this->Ready_PCBQueue[0].front = HeadNodePtr0;
    this->Ready_PCBQueue[0].rear = HeadNodePtr0;
    this->Ready_PCBQueue[1].front = HeadNodePtr1;
    this->Ready_PCBQueue[1].rear = HeadNodePtr1;
    HeadNodePtr0->next = nullptr;
    HeadNodePtr1->next = nullptr;

    //"阻塞"初始化
    for (int i = 0; i < DEVICENUM; i++) {
        this->Obstruct_PCBList[i] = nullptr;
    }
}
ProcessManager::~ProcessManager() {
    //新建队列析构
    PCB* front = this->Created_PCBQueue.front;
    PCB* rear = this->Created_PCBQueue.rear;
    while (front != rear) {
        PCB* temp = front->next;
        front->next = temp->next;
        if (rear == temp) rear = front;
        delete temp;
    }
    delete front;

    //"就绪"队列析构
    PCB* front0 = this->Ready_PCBQueue[0].front;
    PCB* rear0 = this->Ready_PCBQueue[0].rear;
    PCB* front1 = this->Ready_PCBQueue[1].front;
    PCB* rear1 = this->Ready_PCBQueue[1].rear;
    while (front0 != rear0) {
        struct PCB* temp = front0->next;
        front0->next = temp->next;
        if(temp == rear0) rear0 = front0;
        delete temp->PBlock;
        delete temp;
    }
    delete front0;
    while (front1 != rear1) {
        struct PCB* temp = front1->next;
        front1->next = temp->next;
        if(temp == rear1) rear1 = front1;
        delete temp->PBlock;
        delete temp;
    }
    delete front1;
    
    //"阻塞"队列析构
    for (int i = 0; i < DEVICENUM; i++) {
        if (this->Obstruct_PCBList[i] != nullptr) {
            PCB* temp = this->Obstruct_PCBList[i];
            while (temp) {
                PCB* TempCopy = temp;
                temp = temp->next;
                delete TempCopy->PBlock;
                delete TempCopy;
            }
        }
    }
}
