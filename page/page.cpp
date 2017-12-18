#include <iostream>
#include <cmath>
#include <iomanip>

using namespace std;

const int sizes = 8;//页表的大小
const int length = 64;//块的大小64byte
const int moveBit = 6;
const int sizeL = 4;//分配给作业的空闲块个数

class node {
public:
	int Page;//主存页号
	bool flags;//标志位,1表示在主存中，0表示不在
	int ramId;//主存块号，-1表示未分配
	int diskId;//外存地址，-1表示未分配
	bool amendFlags;//修改位11表示已修改，0表示未修改

	node(int Page, bool flags, int ramId, int diskId, bool amendFlags) {
		this->Page = Page;
		this->flags = flags;
		this->diskId = diskId;
		this->ramId = ramId;
		this->amendFlags = amendFlags;
	}

	node() {};
};

class stackItem
{
public:
	int page;
	int ramadd;
};

class stack
{
public:
	stackItem p[sizeL];
	int head;
	int length;

	stack() {
		length = 0;
		head = 0;
		for (int i = 0; i < sizeL; i++)
		{
			cout << "input the "<< i <<"ramId which was distributed to yhe job by OS." << endl;
			cin >> p[i].ramadd;
		}
	}

	bool full() {
		return length == sizeL;
	}

	int insert(int pageid)
	{
		p[head].page = pageid;
		head = (head + 1) % sizeL;
		if(!full())
			length++;
		return head;
	}

	bool lookup(int pageId)
	{
		for (int i = 0; i < sizeL; i++)
		{
			if (p[i].page == pageId)
			{
				if (i == head)
				{
					head = (head + 1) % sizeL;
				}
				else
				{
					stackItem temp = p[i];
					for (int k = i; (k + 1) % sizeL != head; k = (k + 1) % sizeL)
					{
						p[k] = p[(k + 1) % sizeL];
					}
					p[(head - 1) % sizeL] = temp;
				}
				return true;
			}
		}
		return false;
	}

	void print()
	{
		cout << setw(7) << "ramId" << setw(7) << "pageId" << endl;
		for (int i = 0; i < sizeL; i++)
		{
			cout << setw(6) << p[i].ramadd << setw(6) << p[i].page;
			if (i == head)
				cout << "  <------head";
			cout << endl;
		}
	}
};

class table {
public:
	node nodes[sizes];
	stack ss;

	table() {
		//int Page;//主存页号
		//bool flags;//标志位,1表示在主存中，0表示不在
		int ramId;//主存块号，-1表示未分配
		int diskId;//外存地址，-1表示未分配
		bool amendFlags;//修改位11表示已修改，0表示未修改

		bool judge = 0;

		cout << "The length of the table is " << sizes << " ." << endl;
		cout << "Please input the initial information" << endl;
		for (int i = 0; i < sizes; i++) {
			cout << "The " << i << " item's is in Ram?(1/0)" << endl;
			cin >> judge;
			if (judge) {
				cout << "The " << i << " item's and ramId,diskId,amedFlags." << endl;
				cin >> ramId >> diskId >> amendFlags;
				nodes[i] = node(i, 1, ramId, diskId, amendFlags);
				ss.insert(i);
			}
			else {
				cout << "The " << i + 1 << " item's diskId." << endl;
				cin >> diskId;
				nodes[i] = node(i, 0, -1, diskId, 0);
			}
		}
		printTable();
	}

	bool find(int target) {
		if (nodes[target].flags)
		{
			ss.lookup(target);
			return true;
		}
		else
			return false;
	}

	void printTable()
	{
		cout << setw(6) << "Page" << setw(6) << "flags" << setw(6) << "ramId" << setw(6) << "diskId" << setw(6) << "amendFlags" << endl;
		for (int i = 0; i < sizes; i++)
		{
			cout << setw(6) << i << setw(6) << nodes[i].flags << setw(6) << nodes[i].ramId << setw(6) << nodes[i].diskId << setw(6) << nodes[i].amendFlags << endl;
		}
		ss.print();
	}

	int generateAddress(int target, int drift) {
		return (nodes[target].ramId << moveBit) + drift;
	}

	void lackPage(int target) {
		if (ss.full())
		{
			nodes[ss.p[ss.head].page].flags = 0;
			nodes[ss.p[ss.head].page].ramId = -1;
			if (nodes[ss.p[ss.head].page].amendFlags)
			{
				cout << "change the page in the disk." << endl;
			}
			nodes[ss.p[ss.head].page].amendFlags = 0;
		}
		ss.insert(target);
		cout << "put the new page into the ram" << endl;
		nodes[target].ramId = ss.p[(ss.head - 1) % sizeL].ramadd;
		nodes[target].flags = 1;
	}

	void changeBlock(int target)
	{
		nodes[target].amendFlags = 1;
		cout << "stored" << endl;
	}
};

class System {
public:

	table table;
	System() {};
	int page, drift;

	void run() {

		while (true) {
			int logicAddress;
			cout << " Tell us the instruct's logic address." << endl;//设定逻辑地址大小默认多于10位
			cin >> logicAddress;
			page = getPage(logicAddress);
			drift = getDrift(logicAddress, page);
			if (page > sizes) //若产生越界，输出错误信息
			{
				cout << "Error: Transborder Interrupt." << endl;
				continue;
			}
			if (!table.find(page)) {
				cout << "This page is not exist in the ram." << endl;
				table.lackPage(page);
				table.printTable();
				continue;
			}
			table.printTable();
			int absoluteAddress = table.generateAddress(page, drift);
			cout << "absoluteAddress is " << absoluteAddress << endl;
			cout << "store instruct ?(0/1)" << endl;
			bool flagss = 0;
			cin >> flagss;
			if (flagss)
			{
				table.changeBlock(page);
				table.printTable();
			}
			cout << "has next instruct?(0/1)" << endl;
			bool flagsend = 0;
			cin >> flagsend;
			if (!flagsend)
				break;
		}
	}

	int getPage(int logic) {
		return (logic >> moveBit);
	}
	int getDrift(int logic, int Page) {
		return (logic - (Page << moveBit));
	}

	/*int getmoveBit() {//获得移动位数，此处不需要，已设定默认块大小为64KB
	int bit = 0;
	int lengths = length;
	while (true) {
	if (lengths != 1) {
	bit++;
	lengths = lengths >> 1;
	}
	else
	return bit;
	}
	}*/
};

int main()
{
	System s = System();
	s.run();
}

/*测试数据：5 8 9 10
1 5 11 1 1 8 12 1 1 9 13 0 1 10 15 0 0 17 0 25 0 212 0 213*/