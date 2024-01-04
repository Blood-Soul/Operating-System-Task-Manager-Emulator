#include"DeviceManager.h"

DeviceManager::DeviceManager() {
    for (int i = 0; i < DEVICENUM; i++) {
        DeviceState[i] = queue<double>(); // 创建空队列
    }
}

void DeviceManager::deleteDevice(int DeviveNo, int No) {
    queue<double> destQueue;
    int count = -1;
    //删除对应数据
    while(!DeviceState[DeviveNo].empty()){
        count++;
        if(count == No){
            DeviceState[DeviveNo].pop();
        }
        else{
            destQueue.push(DeviceState[DeviveNo].front());
            DeviceState[DeviveNo].pop();
        }
    }
    //复制回原队列
    while(!destQueue.empty()){
        DeviceState[DeviveNo].push(destQueue.front());
        destQueue.pop();
    }
}

void DeviceManager::occupyDevice(int DeviceNo, double Time) {
    if (DeviceNo >= 0 && DeviceNo < DEVICENUM) {
        DeviceState[DeviceNo].push(Time);//将占用时间加入状态队列
    }
    else {
        // 处理设备编号超出范围的情况
        cout << "设备编号超出范围" << endl;
    }
}

vector<int> DeviceManager::runDevice() {
    vector<int> finishedDevices;

    for (int i = 0; i < DEVICENUM; i++) {
        if (!DeviceState[i].empty()) {
            double remainingTime = DeviceState[i].front();
            remainingTime -= 0.1;//若设备状态队列不为空，表示设备正在运行，取出队首，剩余时间减一

            if (remainingTime <= 0) {
                finishedDevices.push_back(i);
                DeviceState[i].pop();//设备任务完成
            }
            else {
                DeviceState[i].front() = remainingTime;
            }
        }
    }

    return finishedDevices;
}
