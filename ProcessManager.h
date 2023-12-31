#pragma once

#include"DataStructure.h"

class ProcessManager {
private:
    PCB_Queue Created_PCBQueue;   //"新建"队列
    PCB_Queue Ready_PCBQueue[2];  //"就绪"队列  //线性优先级算法; 0表示新创建进程队列，1表示享受服务队列
    struct PCB* Obstruct_PCBList[DEVICENUM]; //"阻塞"表单

public:
    PCB* getCreatedProcess(PCB* HugeProcess = nullptr);
    //获取新建进程,只读

    void popCreatedProcess(PCB* process);
    //从新建队列中取出来

    void putProcessReady(PCB* process, bool New);
    //放入就绪队列:新创建建进程队列（true）+享受服务队列(false)

    void popProcessObstruct(int DeviceNo);     //获取进程
    void putProcessObstruct(PCB* process, int DeviceNo);  //放入阻塞队列
    bool createProcess(int PID, string PName, string UserID, int Priority, struct RunInfo PRunInfo); //创建进程
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