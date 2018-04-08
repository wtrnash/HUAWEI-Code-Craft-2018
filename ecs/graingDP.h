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
	string name;//����
	int index;//������ͣ���Ϊ���������vm�±�
	int cpu;//cpu������2λ����
	int mem;//�ڴ棨GB����2λ����
	int num;//��Ҫ������
};
struct PhysicServer
{
	int cpu;
	int memory;
	int restcpu;//ʣ��cpu
	int restmemory;//ʣ���ڴ�
	vector<VM> VMList;//�Ѿ�װ��������
};

bool getwaitline();
vector<VM> putaVMin(vector<VM> putin, VM v);
void putinPhysicServer(vector<VM> putin);
vector<VM> allocation(int n, int j, int k);
vector<PhysicServer> allocate_vm(int n, vector<Flavor> flavors, Physical_server physical_server, bool is_cpu);

extern int focus;//0Ϊcpu,>0Ϊ�ڴ棬�����ȿ����ĸ�ά�ȵ�װ����
extern int cpusize;
extern int memorysize;
#endif
