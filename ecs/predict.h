#ifndef __ROUTE_H__
#define __ROUTE_H__

#include "lib_io.h"
#include <stdio.h>
#include <string>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <time.h>
#include <cmath>
#include <algorithm>
using namespace std;

//物理服务器结构体
struct Physical_server {
	int cpu_core;
	int memory_size;
	int disk_size;
};

//虚拟机规格信息结构体
struct Flavor {
	string flavor_name;
	unsigned int cpu_core;
	unsigned int memory_size;
	vector<string> time;	//虚拟机申请记录的具体时间
	unsigned int *flavor_number_of_day;		//记录虚拟机每天的申请数目
	unsigned int predict_number = 0;		//记录预测的虚拟机总数
	double gap = 0;
};

//实际安装的物理服务器
struct Allocated_physical_server {
	unsigned int left_cpu_core;	//剩下的cpu
	unsigned int left_memory_size;	//剩下的内存
	vector<Flavor> flavors;			//装有的虚拟机
};

void predict_server(char * info[MAX_INFO_NUM], char * data[MAX_DATA_NUM], int data_num, char * filename);
void get_input(char *info[MAX_INFO_NUM]);
time_t string_to_time(const char *s);
void get_data(char * data[MAX_DATA_NUM], int data_num);
void tackle_train_record(string flavor_name, string time);
void predict(void);	
double* single_exponential_smoothing(double a, unsigned int* s0);
double* second_exponential_smoothing(double a, double* s1);
double* third_exponential_smoothing(double a, double* s1, double* s2);
void allocate_vm(void);
vector<Allocated_physical_server> allocate_one_time(vector<Flavor> allocate_flavors);
double get_current_utilization_rate(vector<Allocated_physical_server> allocated_physical_server);
void denoise(void);
void balance_sort(void);
void greedy_allocate(void);
Allocated_physical_server final_process(void);
#endif
