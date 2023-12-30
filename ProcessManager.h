#pragma once

#include"DataStructure.h"

class ProcessManager {
private:
    PCB_Queue Created_PCBQueue;   //"�½�"����
    PCB_Queue Ready_PCBQueue[2];  //"����"����  //�������ȼ��㷨; 0��ʾ�´������̶��У�1��ʾ���ܷ������
    struct PCB* Obstruct_PCBList[DEVICENUM]; //"����"��

public:
    PCB* getCreatedProcess(PCB* HugeProcess = nullptr);
    //��ȡ�½�����,ֻ��

    void popCreatedProcess(PCB* process);
    //���½�������ȡ����

    void putProcessReady(PCB* process, bool New);
    //�����������:�´��������̶��У�true��+���ܷ������(false)

    void popProcessObstruct(int DeviceNo);     //��ȡ����
    void putProcessObstruct(PCB* process, int DeviceNo);  //������������
    bool createProcess(int PID, string PName, string UserID, int Priority, struct RunInfo PRunInfo); //��������
    struct PCB* dispatchProcess(); //���Ƚ���
    interrupt runProcess(struct PCB* process); //ִ��0.1��ʱ��Ƭ�����ᴦ���(true)+�����ᴦ���(false)
    /* ����ֵ��
     * 0: �������У�δ�ͷ�CPU��
     * 1: �������ͷţ�
     * 2: ��ֹ���ͷţ�
     */
    void deleteProcess(PCB* process);  //��ֹ����;
    ProcessManager();
    ~ProcessManager();
};