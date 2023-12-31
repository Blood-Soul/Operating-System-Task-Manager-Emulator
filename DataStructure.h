#pragma once

#include<iostream>
#include<math.h>
using std::string;

#define MAXKVAL 16		  //内存块最大级数
#define DEVICENUM 100
#define WAITINGQUEUE_SPEED 2
#define ENJOINGQUEUE_SPEED 1  //就绪队列优先级变化速度，以时间片为基本单位

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

enum State {
    CREATED,
    REDEAY,
    OBSTRUCTED
};

struct RunInfo {
    double RequireTime; //总共需要的时间
    double OccupyTime;  //占用CPU的时间
    double DeviceRunInfo[2][DEVICENUM];
    //0表示设备总共需要时间，1表示设备开始使用时间
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


//PCB队列
typedef struct {
	struct PCB* front;
	struct PCB* rear;
}PCB_Queue;

struct interrupt {
	int semaphore;
	int DeviceNo;
	double OccupyTime;
};