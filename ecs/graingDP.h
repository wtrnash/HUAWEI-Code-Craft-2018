#ifndef __GRAINGDP_H__
#define __GRAINGDP_H__

#include "predict.h"
#include<iostream>
#include<vector>
#include<time.h>
#include<string>
using namespace std;
struct VM
{
	string name;//名称
	int index;//规格类型，即为虚拟机序列vm下标
	int cpu;//cpu核数，2位数内
	int mem;//内存（GB），2位数内
	int num;//需要的数量
};
struct PhysicServer
{
	int cpu;
	int memory;
	int restcpu;//剩余cpu
	int restmemory;//剩余内存
	vector<VM> VMList;//已经装入的虚拟机
};

bool getwaitline();
vector<VM> putaVMin(vector<VM> putin, VM v);
void putinPhysicServer(vector<VM> putin);
vector<VM> allocation(int n, int j, int k);
vector<PhysicServer> allocate_vm(int n, vector<Flavor> flavors, Physical_server physical_server, bool is_cpu);

extern int focus;//0为cpu,>0为内存，最优先考虑哪个维度的装载量
extern int cpusize;
extern int memorysize;
#endif
