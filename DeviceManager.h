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
	queue<double> DeviceState[DEVICENUM];  // 所有设备状态

public:
	// 构造函数
	DeviceManager();

	void occupyDevice(int DeviceNo, double Time);
	//使用设备：再对应设备号的queue队尾加上一个Time 

	vector<int> runDevice(); //所有设备时间减少0.1;
	//运行设备：把使用中设备的queue队首元素减去0.1，并返回在该操作后队首元素为0的设备号，最后删除该队首元素 
	//设备号用vector传输，记录所有出现上述情况的设备号 

};