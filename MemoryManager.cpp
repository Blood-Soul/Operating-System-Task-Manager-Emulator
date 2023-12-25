#include<math.h>
#include<iostream>
#include<iomanip>
using namespace std;

#define MAXKVAL 16		  //内存块最大级数

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

class BlockManager {//控制block在可用表上的push和pop
public:
	void pushBlock(Block* block, Block* availList[]) {//把block插入可用表对应大小链表的队首
		block->rlink = availList[block->kval]->rlink;
		block->llink = availList[block->kval];
		block->rlink->llink = block;
		block->llink->rlink = block;
	}
	void popBlock(Block* block, Block* availList[]) {//在可用表中删除结点,但没delete block，此时block左右指针指向自己
		block->llink->rlink = block->rlink;
		block->rlink->llink = block->llink;
		block->llink = block->rlink = block;
	}
};

class MemoryManager {
private:
    struct Memory memory;
	struct Block* availList[MAXKVAL + 1];//内存可用表
	BlockManager BM;
public:
	MemoryManager() {
		//初始化内存可用表
		for (int i = 0; i < MAXKVAL + 1; i++) {
			availList[i] = new Block{ i,NULL,NULL ,-1 };//地址无用记为-1
			availList[i]->llink = availList[i]->rlink = availList[i];
		}
		//创建一个初始内存块
		struct Block* initBlock = new Block{ MAXKVAL,availList[MAXKVAL],availList[MAXKVAL],0 };
		availList[MAXKVAL]->llink = availList[MAXKVAL]->rlink = initBlock;
		//初始化分区大小
		memory.Size = (int)pow(2, MAXKVAL);
		memory.SystemAreaSize = (int)pow(2, MAXKVAL - 2);//目前设定为系统区大小比用户区为1：3
		memory.UserAreaSize = memory.Size - memory.SystemAreaSize;
		//从可用表中申请一个系统区
		if (alloc(memory.SystemAreaSize) == NULL) {
			cout << "系统区启动失败！" << endl;
		}
	}
	struct Block* alloc(int size) {//申请大小为size的内存block，若分配失败，返回NULL，若成功则返回对应大小block指针
		//内存分配时优先分配高地址
		int i;
		for (i = 0; i < MAXKVAL + 1 && ((int)pow(2, i) < size || availList[i]->rlink == availList[i]); i++);//找到第一个符合大小且不空的表头
		if (i > MAXKVAL) {//内存不足分配失败！
			return NULL;
		}
		struct Block* allocBlock = availList[i]->rlink;//指向被分配的Block
		BM.popBlock(allocBlock, availList);
		//结点从可用表中删除，开始添加新的小结点,同时修改allocBlock参数
		struct Block* newBlock;//指向新创建的Block
		for (int j = 1; i >= j && (int)pow(2, availList[i - j]->kval) >= size; j++) {
			newBlock = new Block{ i - j,NULL,NULL,allocBlock->address };//创建新的小结点
			newBlock->llink = newBlock->rlink = newBlock;
			BM.pushBlock(newBlock, availList);//把新的小结点插入表头
			// 修改allocBlock参数
			allocBlock->address += (int)pow(2, newBlock->kval);
			allocBlock->kval--;
		}
		return allocBlock;
	};
	void release(struct Block* block) {
		//释放完毕后block会指向NULL
		while (block != NULL) {
			if (availList[block->kval]->rlink != availList[block->kval]) {
				//寻找伙伴，先获得伙伴地址
				int buddyAddress = block->address;
				if (block->address % (int)pow(2, block->kval + 1)) {
					buddyAddress -= (int)pow(2, block->kval);
				}
				else {
					buddyAddress += (int)pow(2, block->kval);
				}
				//根据伙伴地址遍历可用表找伙伴
				struct Block* buddy;//指向伙伴
				for (buddy = availList[block->kval]->rlink; buddy->address != buddyAddress && buddy != availList[block->kval]; buddy = buddy->rlink);//寻找伙伴
				if (buddy == availList[block->kval]) {//若已经全部遍历完，说明无伙伴
					BM.pushBlock(block, availList);
					block = NULL;
				}
				else {//找到伙伴,将其结点从可用表中删除并合并至block中，并重复循环寻找新合并后block的伙伴
					BM.popBlock(buddy, availList);
					delete buddy;
					block->kval++;
					if (block->address > buddyAddress) {
						block->address = buddyAddress;
					}
				}
			}
			else {//若为空，直接链接至表头即可，并且没有伙伴
				BM.pushBlock(block, availList);
				block = NULL;
			}
		}
	}
	void show() {//仅做测试使用
		for (int i = 0; i < MAXKVAL + 1; i++) {
			cout << "第" << setw(2) << i << "层:" << ' ';
			for (Block* j = availList[i]->rlink; j != availList[i]; j = j->rlink) {
				cout << setw(2) << availList[i]->kval << ' ' << "地址" << j->address << "  ";
			}
			cout << endl;
		}
	}
	~MemoryManager() {
		//遍历可用表delete所有结点
		struct Block* deleteBlock;//指向即将被delete的block
		for (int i = 0; i < MAXKVAL + 1; i++) {
			while (availList[i]->rlink != availList[i]) {
				deleteBlock = availList[i]->rlink;
				BM.popBlock(deleteBlock, availList);
				delete deleteBlock;
			}
			delete availList[i];//删除头结点
		}
	}
};

void MemoryManager_test() {//仅做测试使用
	MemoryManager MM;
	int sel=3;
	int size;
	while (sel != 0) {
		cout << "单元测试" << endl;
		MM.show();
		cout << "请输入您的选择——1.申请内存 2.释放内存 0.退出" << endl;
		cin >> sel;
		switch (sel)
		{
		case 1:
			cout << "请输入申请内存大小" << endl;
			cin >> size;
			MM.alloc(size);
			break;
		case 2: {
			cout << "请输入释放内存大小和地址,中间用空格隔开" << endl;
			int address;
			cin >> size >> address;
			int kval;
			for (kval = 0; (int)pow(2, kval) < size; kval++);
			struct Block* temp = new Block{ kval, NULL, NULL, address };
			temp->llink = temp->rlink = temp;
			MM.release(temp);
			break;
		}
		case 0:
			cout << "退出成功！" << endl;
		default:
			cout << "输入错误" << endl;
			break;
		}
		system("cls");
	}
}

int main() {
	//MemoryManager_test();
	system("pause");
	return 0;
}