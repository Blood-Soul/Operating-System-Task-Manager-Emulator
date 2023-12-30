#pragma once

#include"DataStructure.h"
using std::cout;
using std::endl;

class BlockManager {//����block�ڿ��ñ��ϵ�push��pop
public:
	void pushBlock(Block* block, Block* availList[]);
	void popBlock(Block* block, Block* availList[]);
};

class MemoryManager {
private:
	struct Memory memory;
	struct Block* availList[MAXKVAL + 1];//�ڴ���ñ�
	BlockManager BM;
public:
	MemoryManager();
	struct Block* alloc(int size);
	void release(struct Block* block);
	~MemoryManager();
};