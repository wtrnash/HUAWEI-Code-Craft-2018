#include "predict.h"

//物理服务器
Physical_server physical_server;

vector<Flavor> flavors;	//存放所有要预测的虚拟机的规格及对应训练集记录
int number_of_flavor;	//要预测的虚拟机规格的数量
bool is_cpu;			//要优化的资源是cpu还是内存
string predict_start_time, predict_end_time;		//预测的开始和结束时间
time_t predict_start_time_t, predict_end_time_t;	//预测的开始和结束时间转为time_t格式
int predict_day;					//需预测的天数
string train_start_time, train_end_time;		//训练的开始和结束时间
time_t train_start_time_t, train_end_time_t;	//训练的开始和结束时间转为time_t格式
int train_day;					//需预测的天数
int sum_of_flavor = 0;	//要预测的虚拟机总数
vector<Allocated_physical_server> physical_servers;	//所有物理服务器

//你要完成的功能总入口
void predict_server(char * info[MAX_INFO_NUM], char * data[MAX_DATA_NUM], int data_num, char * filename)
{
	// 需要输出的内容
	char * result_file = (char *)"17\n\n0 8 0 20";
	//解析输入字符串
	get_input(info);	//解析输入文件
	get_data(data, data_num);	//解析要训练的样本

	//训练模型, 预测
	predict();

	//装配
	allocate_vm();

	//处理输出
	string result = to_string(sum_of_flavor) + "\n";
	for (unsigned int i = 0; i < flavors.size(); i++)
	{
		result += flavors[i].flavor_name;
		result += " ";
		result += to_string(flavors[i].predict_number);
		result += " \n";
	}
	result += "\n";
	result += to_string(physical_servers.size());
	result += "\n";
	for (unsigned int i = 0; i < physical_servers.size(); i++)
	{
		result += to_string(physical_servers[i].index);
		for (unsigned int j = 0; j < physical_servers[i].flavors.size(); j++)
		{
			result += " ";
			result += physical_servers[i].flavors[j].flavor_name;
			result += " ";
			result += to_string(physical_servers[i].flavors[j].predict_number);
		}
		result += "\n";
	}

	result_file = (char *)result.c_str();
	// 直接调用输出文件的方法输出到指定文件中(ps请注意格式的正确性，如果有解，第一行只有一个数据；第二行为空；第三行开始才是具体的数据，数据之间用一个空格分隔开)
	write_result(result_file, filename);
}

void get_input(char *info[MAX_INFO_NUM])
{
	//处理input第一行物理服务器
	string physical_info = info[0];
	int first_space = physical_info.find_first_of(' ');
	int second_space = physical_info.find_last_of(' ');
	physical_server.cpu_core = atoi(physical_info.substr(0, first_space).c_str());  
	physical_server.memory_size = atoi(physical_info.substr(first_space + 1,  second_space - first_space - 1).c_str());
	physical_server.disk_size = atoi(physical_info.substr(second_space + 1).c_str());

	//处理input第三行
	number_of_flavor = atoi(info[2]);

	//处理flavor
	int index;
	Flavor temp;
	string flavor_info;
	for (index = 3; index < 3 + number_of_flavor; index++)
	{
		flavor_info = info[index];
		first_space = flavor_info.find_first_of(' ');
		second_space = flavor_info.find_last_of(' ');
		temp.flavor_name = flavor_info.substr(0, first_space);
		temp.cpu_core = atoi(flavor_info.substr(first_space + 1, second_space - first_space - 1).c_str());
		temp.memory_size = atoi(flavor_info.substr(second_space + 1).c_str());
		flavors.push_back(temp);
	}

	//记录需优化的资源
	string resource = info[++index];
	if (resource.substr(0, 3).compare("CPU"))
		is_cpu = false;
	else
		is_cpu = true;

	//处理开始、结束时间
	index+=2;
	predict_start_time = info[index++];
	predict_end_time = info[index];
	predict_start_time_t = string_to_time(predict_start_time.c_str());
	predict_end_time_t = string_to_time(predict_end_time.c_str());
	predict_day = ceil(((double)(predict_end_time_t - predict_start_time_t)) / (60 * 60 * 24));

	/*
	//输出信息做调试
	cout << "physical:" << endl;
	cout << physical_server.cpu_core << " " << physical_server.memory_size << " " << physical_server.disk_size << endl;
	cout << "flavor:" << endl;
	for (int i = 0; i < number_of_flavor; i++)
	{
		cout << flavors[i].flavor_name << " " << flavors[i].cpu_core << " " << flavors[i].memory_size << endl;
	}

	cout << "is Cpu?" << is_cpu << endl;

	cout << "start time: " << predict_start_time << endl;
	cout << "end time:" << predict_end_time << endl;
	cout << "predict_day:" << predict_day << endl;*/

	return;
}

//字符串转time_t
time_t string_to_time(const char *s)
{
	tm* temp = new tm;
	strptime(s, "%Y-%m-%d %H:%M:%S", temp);
	time_t t = mktime(temp);
	delete temp;
	return t;
}

void get_data(char * data[MAX_DATA_NUM], int data_num)
{
	//记录训练开始时间、结束时间及总天数
	string first_line = data[0];
	int first_tab = first_line.find_first_of('\t');
	int second_tab = first_line.find_last_of('\t');
	//找出最后一个空格，使得可以将开始时间的时分秒设成00:00:00
	int last_space = first_line.find_last_of(' ');
	train_start_time = first_line.substr(second_tab + 1, last_space - second_tab - 1) + "00:00:00";
	train_start_time_t = string_to_time(train_start_time.c_str());
	string last_line = data[data_num - 1];
	first_tab = last_line.find_first_of('\t');
	second_tab = last_line.find_last_of('\t');
	train_end_time = last_line.substr(second_tab + 1);
	train_end_time_t = string_to_time(train_end_time.c_str());
	train_day = ceil(((double)(train_end_time_t - train_start_time_t)) / (60 * 60 * 24));

	//根据训练天数初始化每个flavor的每天申请量为0
	for (int i = 0; i < number_of_flavor; i++)
	{
		flavors[i].flavor_number_of_day = new unsigned int[train_day + 1];
		fill(flavors[i].flavor_number_of_day, flavors[i].flavor_number_of_day + train_day + 1, 0);	//全部初始化为0
	}
	//处理每条记录
	string line, flavor_name, time;
	for (int i = 0;i < data_num; i++)
	{
		line = data[i];
		first_tab = line.find_first_of('\t');
		second_tab = line.find_last_of('\t');
		flavor_name = line.substr(first_tab + 1, second_tab - first_tab - 1);
		time = line.substr(second_tab + 1);
		tackle_train_record(flavor_name, time);
	}

	/*
	//输出信息来调试
	cout << "train_start_time: " << train_start_time << endl;
	cout << "train_end_time:" << train_end_time << endl;
	cout << "train_day:" << train_day << endl;
	for (int i = 0; i < number_of_flavor; i++)
	{
		cout << flavors[i].flavor_name << endl;
		for (unsigned int j = 0; j < flavors[i].time.size(); j++)
		{
			cout << flavors[i].time[j] << endl;
		}

		for (int k = 1; k < train_day + 1; k++)
		{
			cout << flavors[i].flavor_number_of_day[k] << " ";
		}
		cout << endl;
		
	}*/
	return;

}

//处理训练记录
void tackle_train_record(string flavor_name, string time)
{
	int day;
	time_t t;
	for (unsigned int i = 0; i < flavors.size(); i++)
	{
		if (flavors[i].flavor_name.compare(flavor_name) == 0)
		{
			flavors[i].time.push_back(time);
			t = string_to_time(time.c_str());
			day = ceil(((double)(t - train_start_time_t)) / (60 * 60 * 24));
			//第一个记录如果是0点0分0秒则也在第一天
			if (day == 0)	
				day = 1;
			flavors[i].flavor_number_of_day[day]++;
			break;
		}
	}

	return;
}

void predict()
{
	int sum;
	//取和预测天数最后一样的天数
	for (unsigned int i = 0; i < flavors.size(); i++)
	{
		sum = 0;
		for (int j = train_day - predict_day + 1; j <= train_day; j++)
		{
			sum += flavors[i].flavor_number_of_day[j];
		}

		flavors[i].predict_number = sum;
		sum_of_flavor += flavors[i].predict_number;
	}

}

//将flavor根据所优化资源降序排序
bool compare(Flavor f1, Flavor f2)
{
	if (is_cpu) 
	{
		if (f1.cpu_core == f2.cpu_core)
			return f1.memory_size > f2.memory_size;
		else
			return f1.cpu_core > f2.cpu_core;
	}
	else
	{
		if (f1.memory_size == f2.memory_size)
			return f1.cpu_core > f2.cpu_core;
		else
			return f1.memory_size > f2.memory_size;
	}
}
//模拟退火分配虚拟机
void allocate_vm()
{
	vector<Flavor> min_allocate_flavors;
	vector<Flavor> allocate_flavors;	//用来装配的所有虚拟机
	Flavor temp_flavor;
	for (unsigned int i = 0; i < flavors.size(); i++)
	{
		for (unsigned int j = 0; j < flavors[i].predict_number; j++)
		{
			temp_flavor = flavors[i];
			temp_flavor.predict_number = 1;
			temp_flavor.memory_size = (int)ceil(1.0 * temp_flavor.memory_size / 1024);
			allocate_flavors.push_back(temp_flavor);
		}
	}
	//降序排序
	sort(allocate_flavors.begin(), allocate_flavors.end(), compare);

	vector<int> indices;	//记录flavor所有下标
	for (unsigned int i = 0; i < allocate_flavors.size(); i++)
	{
		indices.push_back(i);
	}

	min_allocate_flavors = allocate_flavors;
	vector<Allocated_physical_server> current_physical_server;	//每次分配的物理服务器
	double min_utilization_rate = allocate_flavors.size();	//记录最少的服务器利用率，用n-1个服务器个数 和最后一个服务器的资源利用率表示，初始化为一个虚拟机放一个物理服务器
	double current_utilization_rate;
	double t = 10.0;	//初始温度
	double t_min = 1.0;	//最终温度
	double k = 0.9999;	//温度下降系数
	while(t > t_min)
	{
		random_shuffle(indices.begin(), indices.end());	//打乱下标排序
		allocate_flavors = min_allocate_flavors;
		//以indices前两个元素作为allocate_flavors要交换的下标
		swap(allocate_flavors[indices[0]], allocate_flavors[indices[1]]);
		current_physical_server = allocate_one_time(allocate_flavors);
		current_utilization_rate = get_current_utilization_rate(current_physical_server);
		//如果当前比较小，则直接取当前为最优解
		if (current_utilization_rate < min_utilization_rate)
		{
			min_utilization_rate = current_utilization_rate;
			physical_servers = current_physical_server;
			min_allocate_flavors = allocate_flavors;
		}
		else   //否则一定概率更新当前为最优解
		{
			//exp(delta t / T) 大于一定概率（概率在0到1之间）则接受劣解
			if (exp(1.0 * (min_utilization_rate - current_utilization_rate) / t) > (rand()/RAND_MAX))
			{
				min_utilization_rate = current_utilization_rate;
				physical_servers = current_physical_server;
				min_allocate_flavors = allocate_flavors;
			}
		}

		t *= k;
	}
		
}

//一次装配
vector<Allocated_physical_server> allocate_one_time(vector<Flavor> allocate_flavors)
{
	vector<Allocated_physical_server>  allocated_Physical_server;
	//第一个物理服务器
	int index = 1;
	Allocated_physical_server temp;
	temp.index = index++;
	temp.left_cpu_core = physical_server.cpu_core;
	temp.left_memory_size = physical_server.memory_size;
	allocated_Physical_server.push_back(temp);
	//遍历所有种类的flavors
	for (unsigned int i = 0; i < allocate_flavors.size(); i++)
	{
		unsigned int m;
		for (m = 0; m < allocated_Physical_server.size(); m++)
		{
			//如果可以放
			if (allocate_flavors[i].cpu_core <= allocated_Physical_server[m].left_cpu_core && allocate_flavors[i].memory_size <= allocated_Physical_server[m].left_memory_size)
			{
				//判断有没有在该物理机放置过同样规格的，放置过的话预测数量加一
				unsigned int k;
				for (k = 0; k < allocated_Physical_server[m].flavors.size(); k++)
				{
					if (allocated_Physical_server[m].flavors[k].flavor_name == allocate_flavors[i].flavor_name)
					{
						allocated_Physical_server[m].flavors[k].predict_number++;
						break;
					}
				}

				//没有放置过则push_back一个新的
				if (k == allocated_Physical_server[m].flavors.size())
				{
					Flavor flavor = allocate_flavors[i];
					flavor.predict_number = 1;
					allocated_Physical_server[m].flavors.push_back(flavor);
				}

				//减去放置的容量
				allocated_Physical_server[m].left_cpu_core -= allocate_flavors[i].cpu_core;
				allocated_Physical_server[m].left_memory_size -= allocate_flavors[i].memory_size;
				break;
			}
		}

		if (m == allocated_Physical_server.size())   //如果不能放则用全新的物理机
		{
			temp.index = index++;
			temp.left_cpu_core = physical_server.cpu_core - allocate_flavors[i].cpu_core;
			temp.left_memory_size = physical_server.memory_size - allocate_flavors[i].memory_size;
			temp.flavors.clear();
			Flavor flavor = allocate_flavors[i];
			temp.flavors.push_back(flavor);
			allocated_Physical_server.push_back(temp);
		}
	}

	return allocated_Physical_server;
}

//获得当前利用率
double get_current_utilization_rate(vector<Allocated_physical_server> allocated_physical_server)
{
	double current_utilization_rate;
	if (is_cpu)
		current_utilization_rate = allocated_physical_server.size() - 1 + 1.0 * (physical_server.cpu_core - allocated_physical_server[allocated_physical_server.size() - 1].left_cpu_core) / physical_server.cpu_core;
	else
		current_utilization_rate = allocated_physical_server.size() - 1 + 1.0 * (physical_server.memory_size - allocated_physical_server[allocated_physical_server.size() - 1].left_memory_size) / physical_server.memory_size;

	return current_utilization_rate;
}