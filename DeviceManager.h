#pragma once

#include<queue>
#include<vector>
#include"DataStructure.h"
using std::queue;
using std::vector;
using std::cout;
using std::endl;

class DeviceManager {
private:
	queue<double> DeviceState[DEVICENUM];  // �����豸״̬

public:
	// ���캯��
	DeviceManager();

	void occupyDevice(int DeviceNo, double Time);
	//ʹ���豸���ٶ�Ӧ�豸�ŵ�queue��β����һ��Time 

	vector<int> runDevice(); //�����豸ʱ�����0.1;
	//�����豸����ʹ�����豸��queue����Ԫ�ؼ�ȥ0.1���������ڸò��������Ԫ��Ϊ0���豸�ţ����ɾ���ö���Ԫ�� 
	//�豸����vector���䣬��¼���г�������������豸�� 

};