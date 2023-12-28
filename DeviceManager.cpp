#define DEVICENUM 100

class DeviceManager {
private:
    bool DeviceState[DEVICENUM];  // 设备状态

public:
    // 构造函数
    DeviceManager() {
        // 初始化设备状态，默认为未使用
        for (int i = 0; i < DEVICENUM; i++) {
            DeviceState[i] = false;
        }
    }
    
    // 检查设备状态
    bool checkState(int DeviceNo) {
        if (DeviceNo >= 0 && DeviceNo < DEVICENUM) {
            return DeviceState[DeviceNo];
        } else {
            // 处理设备编号超出范围的情况
            return false;
        }
    }
    
    // 更新设备状态
    void updateState(int DeviceNo, bool State) {
        if (DeviceNo >= 0 && DeviceNo < DEVICENUM) {
            DeviceState[DeviceNo] = State;
        } else {
            // 处理设备编号超出范围的情况
            cout << "设备编号超出范围。" << endl;
        }
    }

};
