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

//����������ṹ��
struct Physical_server {
	int cpu_core;
	int memory_size;
	int disk_size;
};

//����������Ϣ�ṹ��
struct Flavor {
	string flavor_name;
	unsigned int cpu_core;
	unsigned int memory_size;
	vector<string> time;	//����������¼�ľ���ʱ��
	unsigned int *flavor_number_of_day;		//��¼�����ÿ���������Ŀ
	unsigned int predict_number = 0;		//��¼Ԥ������������
};

//ʵ�ʰ�װ�����������
struct Allocated_Physical_server {
	int index;	//���
	unsigned int left_cpu_core;	//ʣ�µ�cpu
	unsigned int left_memory_size;	//ʣ�µ��ڴ�
	vector<Flavor> flavors;			//װ�е������
};

void predict_server(char * info[MAX_INFO_NUM], char * data[MAX_DATA_NUM], int data_num, char * filename);
void get_input(char *info[MAX_INFO_NUM]);
time_t string_to_time(const char *s);
void get_data(char * data[MAX_DATA_NUM], int data_num);
void tackle_train_record(string flavor_name, string time);
void predict(void);	
void allocate_vm(void);
vector<Allocated_Physical_server> allocate_one_time(vector<Flavor> allocate_flavors);
#endif
