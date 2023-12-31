#pragma once

#include"DataStructure.h"
using std::cout;
using std::endl;

class BlockManager {//控制block在可用表上的push和pop
public:
	void pushBlock(Block* block, Block* availList[]);
	void popBlock(Block* block, Block* availList[]);
};

class MemoryManager {
private:
	struct Memory memory;
	struct Block* availList[MAXKVAL + 1];//内存可用表
	BlockManager BM;
public:
	MemoryManager();
	struct Block* alloc(int size);
	void release(struct Block* block);
	~MemoryManager();
};