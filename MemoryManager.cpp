#include<math.h>
#include<iostream>
#include<iomanip>
using namespace std;

#define MAXKVAL 16		  //�ڴ�������

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

class BlockManager {//����block�ڿ��ñ��ϵ�push��pop
public:
	void pushBlock(Block* block, Block* availList[]) {//��block������ñ��Ӧ��С����Ķ���
		block->rlink = availList[block->kval]->rlink;
		block->llink = availList[block->kval];
		block->rlink->llink = block;
		block->llink->rlink = block;
	}
	void popBlock(Block* block, Block* availList[]) {//�ڿ��ñ���ɾ�����,��ûdelete block����ʱblock����ָ��ָ���Լ�
		block->llink->rlink = block->rlink;
		block->rlink->llink = block->llink;
		block->llink = block->rlink = block;
	}
};

class MemoryManager {
private:
    struct Memory memory;
	struct Block* availList[MAXKVAL + 1];//�ڴ���ñ�
	BlockManager BM;
public:
	MemoryManager() {
		//��ʼ���ڴ���ñ�
		for (int i = 0; i < MAXKVAL + 1; i++) {
			availList[i] = new Block{ i,NULL,NULL ,-1 };//��ַ���ü�Ϊ-1
			availList[i]->llink = availList[i]->rlink = availList[i];
		}
		//����һ����ʼ�ڴ��
		struct Block* initBlock = new Block{ MAXKVAL,availList[MAXKVAL],availList[MAXKVAL],0 };
		availList[MAXKVAL]->llink = availList[MAXKVAL]->rlink = initBlock;
		//��ʼ��������С
		memory.Size = (int)pow(2, MAXKVAL);
		memory.SystemAreaSize = (int)pow(2, MAXKVAL - 2);//Ŀǰ�趨Ϊϵͳ����С���û���Ϊ1��3
		memory.UserAreaSize = memory.Size - memory.SystemAreaSize;
		//�ӿ��ñ�������һ��ϵͳ��
		if (alloc(memory.SystemAreaSize) == NULL) {
			cout << "ϵͳ������ʧ�ܣ�" << endl;
		}
	}
	struct Block* alloc(int size) {//�����СΪsize���ڴ�block��������ʧ�ܣ�����NULL�����ɹ��򷵻ض�Ӧ��Сblockָ��
		//�ڴ����ʱ���ȷ���ߵ�ַ
		int i;
		for (i = 0; i < MAXKVAL + 1 && ((int)pow(2, i) < size || availList[i]->rlink == availList[i]); i++);//�ҵ���һ�����ϴ�С�Ҳ��յı�ͷ
		if (i > MAXKVAL) {//�ڴ治�����ʧ�ܣ�
			return NULL;
		}
		struct Block* allocBlock = availList[i]->rlink;//ָ�򱻷����Block
		BM.popBlock(allocBlock, availList);
		//���ӿ��ñ���ɾ������ʼ����µ�С���,ͬʱ�޸�allocBlock����
		struct Block* newBlock;//ָ���´�����Block
		for (int j = 1; i >= j && (int)pow(2, availList[i - j]->kval) >= size; j++) {
			newBlock = new Block{ i - j,NULL,NULL,allocBlock->address };//�����µ�С���
			newBlock->llink = newBlock->rlink = newBlock;
			BM.pushBlock(newBlock, availList);//���µ�С�������ͷ
			// �޸�allocBlock����
			allocBlock->address += (int)pow(2, newBlock->kval);
			allocBlock->kval--;
		}
		return allocBlock;
	};
	void release(struct Block* block) {
		//�ͷ���Ϻ�block��ָ��NULL
		while (block != NULL) {
			if (availList[block->kval]->rlink != availList[block->kval]) {
				//Ѱ�һ�飬�Ȼ�û���ַ
				int buddyAddress = block->address;
				if (block->address % (int)pow(2, block->kval + 1)) {
					buddyAddress -= (int)pow(2, block->kval);
				}
				else {
					buddyAddress += (int)pow(2, block->kval);
				}
				//���ݻ���ַ�������ñ��һ��
				struct Block* buddy;//ָ����
				for (buddy = availList[block->kval]->rlink; buddy->address != buddyAddress && buddy != availList[block->kval]; buddy = buddy->rlink);//Ѱ�һ��
				if (buddy == availList[block->kval]) {//���Ѿ�ȫ�������꣬˵���޻��
					BM.pushBlock(block, availList);
					block = NULL;
				}
				else {//�ҵ����,������ӿ��ñ���ɾ�����ϲ���block�У����ظ�ѭ��Ѱ���ºϲ���block�Ļ��
					BM.popBlock(buddy, availList);
					delete buddy;
					block->kval++;
					if (block->address > buddyAddress) {
						block->address = buddyAddress;
					}
				}
			}
			else {//��Ϊ�գ�ֱ����������ͷ���ɣ�����û�л��
				BM.pushBlock(block, availList);
				block = NULL;
			}
		}
	}
	void show() {//��������ʹ��
		for (int i = 0; i < MAXKVAL + 1; i++) {
			cout << "��" << setw(2) << i << "��:" << ' ';
			for (Block* j = availList[i]->rlink; j != availList[i]; j = j->rlink) {
				cout << setw(2) << availList[i]->kval << ' ' << "��ַ" << j->address << "  ";
			}
			cout << endl;
		}
	}
	~MemoryManager() {
		//�������ñ�delete���н��
		struct Block* deleteBlock;//ָ�򼴽���delete��block
		for (int i = 0; i < MAXKVAL + 1; i++) {
			while (availList[i]->rlink != availList[i]) {
				deleteBlock = availList[i]->rlink;
				BM.popBlock(deleteBlock, availList);
				delete deleteBlock;
			}
			delete availList[i];//ɾ��ͷ���
		}
	}
};

void MemoryManager_test() {//��������ʹ��
	MemoryManager MM;
	int sel=3;
	int size;
	while (sel != 0) {
		cout << "��Ԫ����" << endl;
		MM.show();
		cout << "����������ѡ�񡪡�1.�����ڴ� 2.�ͷ��ڴ� 0.�˳�" << endl;
		cin >> sel;
		switch (sel)
		{
		case 1:
			cout << "�����������ڴ��С" << endl;
			cin >> size;
			MM.alloc(size);
			break;
		case 2: {
			cout << "�������ͷ��ڴ��С�͵�ַ,�м��ÿո����" << endl;
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
			cout << "�˳��ɹ���" << endl;
		default:
			cout << "�������" << endl;
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