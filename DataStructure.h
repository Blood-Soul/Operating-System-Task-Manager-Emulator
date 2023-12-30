#pragma once

#include<iostream>
using std::string;

#define MAXKVAL 16		  //�ڴ�������
#define DEVICENUM 100
#define WAITINGQUEUE_SPEED 2
#define ENJOINGQUEUE_SPEED 1  //�����������ȼ��仯�ٶȣ���ʱ��ƬΪ������λ

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

//PCB����
typedef struct {
	struct PCB* front;
	struct PCB* rear;
}PCB_Queue;

struct interrupt {
	int semaphore;
	int DeviceNo;
	double OccupyTime;
};