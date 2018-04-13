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
double original_rate;

//你要完成的功能总入口
void predict_server(char * info[MAX_INFO_NUM], char * data[MAX_DATA_NUM], int data_num, char * filename)
{
	// 需要输出的内容
	char * result_file = (char *)"17\n\n0 8 0 20";
	//解析输入字符串
	get_input(info);	//解析输入文件
	get_data(data, data_num);	//解析要训练的样本

	//数据去噪
	denoise();
	//训练模型, 预测
	predict();

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

	//装配
	allocate_vm();

	//处理输出
	result += to_string(physical_servers.size());
	result += "\n";
	for (unsigned int i = 0; i < physical_servers.size(); i++)
	{
		result += to_string(i + 1);
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

	original_rate = 1.0 * physical_server.cpu_core / physical_server.memory_size;
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
	index += 2;
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
	double a = 0.1;
	double *s1, *s2;
	for (unsigned int i = 0; i < flavors.size(); i++)
	{
		s1 = single_exponential_smoothing(a, flavors[i].flavor_number_of_day);
		s2 = second_exponential_smoothing(a, s1);

		for (int j = train_day + 1; j < train_day + 1 + predict_day; j++)
		{
			flavors[i].predict_number += s2[j] >= 0? (int)floor(s2[j]) : 0;
		}

		sum_of_flavor += flavors[i].predict_number;
		delete s1;
		delete s2;
	}
}
//一次指数平滑预测法
double* single_exponential_smoothing(double a, unsigned int* s0)
{
	double *s1 = new double[train_day + predict_day + 1];
	//获得训练的平滑值
	s1[1] = s0[1];	//第一个平滑值等于第一个原始值
	for (int i = 2; i <= train_day; i++)
	{
		s1[i] = a * s0[i] + (1 - a) * s1[i - 1];
	}
	//预测
	for (int i = train_day + 1; i < train_day + predict_day + 1; i++)
	{
		s1[i] = a * s0[i - 1] + (1 - a) * s1[i - 1];
	}

	return s1;
}

//二次指数平滑预测法
double* second_exponential_smoothing(double a, double* s1)
{
	double *s2 = new double[train_day + predict_day + 1];
	//获取训练的平滑值
	s2[1] = s1[1];
	for (int i = 2; i <= train_day; i++)
	{
		s2[i] = a * s1[i] + (1 - a) * s2[i - 1];
	}
	//预测
	double at = 2.0 * s1[train_day] - s2[train_day];
	double bt = a / (1 - a) * (s1[train_day] - s2[train_day]);
	for (int i = train_day + 1; i < train_day + predict_day + 1; i++)
	{
		s2[i] = at + bt * (i - train_day);
	}
	return s2;
}
//分配虚拟机
void allocate_vm()
{
	for (unsigned int i = 0; i < flavors.size(); i++)
	{
		flavors[i].memory_size = (int)ceil(1.0 * flavors[i].memory_size / 1024);
	}

	balance_sort();
	greedy_allocate();
		
}

void balance_sort() 
{//按cpu与mem差额绝对值从小到大排序，相等的按规模从小到大排序，计算差额时mem按比例换算
	for (unsigned int i = 0; i < flavors.size(); i++)
		flavors[i].gap = flavors[i].cpu_core - flavors[i].memory_size * original_rate;
	vector<Flavor> new_flavors;
	new_flavors.push_back(flavors[0]);
	for (unsigned int i = 1; i < flavors.size(); i++) {
		bool is_insert = false;
		for (unsigned int j = 0; j < new_flavors.size(); j++) {
			if (abs(flavors[i].gap) < abs(new_flavors[j].gap)) {//如果差额小于直接插入j位置
				vector<Flavor>::iterator pr = new_flavors.begin();
				pr += j;
				new_flavors.insert(pr, flavors[i]);
				is_insert = true;
				break;
			}//endif
			else if (abs(flavors[i].gap) == abs(new_flavors[j].gap)) {
				if (is_cpu) {
					if (flavors[i].cpu_core < new_flavors[j].cpu_core ||
						(flavors[i].cpu_core == new_flavors[j].cpu_core && flavors[i].memory_size < new_flavors[j].memory_size))
					{
						vector<Flavor>::iterator pr = new_flavors.begin();
						pr += j;
						new_flavors.insert(pr, flavors[i]);
						is_insert = true;
						break;
					}
				}
				else {
					if (flavors[i].memory_size < new_flavors[j].memory_size ||
						(flavors[i].memory_size == new_flavors[j].memory_size && flavors[i].cpu_core < new_flavors[j].cpu_core))
					{
						vector<Flavor>::iterator pr = new_flavors.begin();
						pr += j;
						new_flavors.insert(pr, flavors[i]);
						is_insert = true;
						break;
					}
				}
			}//endelse
		}//endfor
		if (!is_insert) new_flavors.push_back(flavors[i]);
	}//endfor
	flavors = new_flavors;
	new_flavors.clear();
}

void greedy_allocate() 
{//如果当前服务器剩余资源差额绝对值占目前剩余cpu资源在%以下时视为两资源相等，相等的时候选取一个不大于剩余资源%
						//的一个最大虚拟机放入，然后找寻差额绝对值最接近，符号相同的虚拟机放入使两资源相等，重复以上过程
	int n = 0;
	for (unsigned int i = 0; i < flavors.size(); i++) n += flavors[i].predict_number;
	Allocated_physical_server empty_physic_server;
	empty_physic_server.left_cpu_core = physical_server.cpu_core;
	empty_physic_server.left_memory_size = physical_server.memory_size;
	double original_rate = 1.0* physical_server.cpu_core / physical_server.memory_size;
	int index = 0;
	while (n > 0) {
		physical_servers.push_back(empty_physic_server);
		bool is_finish = false;
		double is_equal = 0.02;
		double limit = 0.5;
		while (!is_finish) {
			double server_gap = physical_servers[index].left_cpu_core - physical_servers[index].left_memory_size*original_rate;
			int k = 0;
			bool is_find = false;
			if (server_gap <= is_equal) {//当前差额极小时认为可以较为随意的放置虚拟机而不必考虑减小差额
				for (int i = (int)flavors.size() - 1; i >= 0; i--) {
					if (abs(flavors[i].gap - server_gap) < 1)limit = 1;//limit值与差额大小有关，如果差额极小，则认为可以完全填满，limit为1
					else limit = 0.5;//如果差额较大，留出空间装填另一个以减小差额
					if (flavors[i].predict_number > 0 && 1.0 * flavors[i].cpu_core / physical_servers[index].left_cpu_core <= limit &&
						1.0 * flavors[i].cpu_core / physical_servers[index].left_cpu_core <= limit) {
						k = i;
						is_find = true;
						break;
					}
				}//endfor
			}//endif
			else {//如果差额较大，则选择差额最接近的虚拟机装入减小差额
				for (unsigned int i = 0; i < flavors.size(); i++) {//找差额绝对值最接近符号相同稍大的
					if (flavors[i].predict_number > 0 && abs(flavors[i].gap) >= abs(server_gap) && flavors[i].gap*server_gap > 0 &&
						flavors[i].cpu_core <= physical_servers[index].left_cpu_core &&
						flavors[i].memory_size <= physical_servers[index].left_memory_size) {
						k = i;
						is_find = true;
						break;
					}
				}
				if (!is_find) {//如果未找到，找差额稍小一些的
					for (int i = (int)flavors.size() - 1; i >= 0; i--) {
						if (flavors[i].predict_number > 0 && abs(flavors[i].gap) <= abs(server_gap) && flavors[i].gap*server_gap > 0 &&
							flavors[i].cpu_core <= physical_servers[index].left_cpu_core &&
							flavors[i].memory_size <= physical_servers[index].left_memory_size) {
							k = i;
							is_find = true;
							break;
						}
					}//endfor
				}
			}//enelse
			if (!is_find) {//如果以上都未找到，可视作当前虚拟机可以放入的皆cpu偏多或偏少，接下来找最大可放入装入
				for (int i = (int)flavors.size() - 1; i >= 0; i--) {
					if (flavors[i].predict_number > 0 && flavors[i].cpu_core <= physical_servers[index].left_cpu_core &&
						flavors[i].memory_size <= physical_servers[index].left_memory_size) {
						k = i;
						is_find = true;
						break;
					}
				}
			}//endelse
			if (is_find) {//放入当前物理服务器中
				bool is_exist = false;
				for (unsigned int i = 0; i < physical_servers[index].flavors.size(); i++) {
					if (flavors[k].flavor_name == physical_servers[index].flavors[i].flavor_name) {
						physical_servers[index].flavors[i].predict_number++;
						is_exist = true;
						break;
					}
				}
				if (!is_exist) {
					for (unsigned int i = 0; i < physical_servers[index].flavors.size(); i++) {
						if (flavors[k].index < physical_servers[index].flavors[i].index) {
							vector<Flavor>::iterator pr = physical_servers[index].flavors.begin();
							pr += i;
							Flavor flavor = flavors[k];
							flavor.predict_number = 1;
							physical_servers[index].flavors.insert(pr, flavor);
							is_exist = true;
							break;
						}
					}
				}
				if (!is_exist) {
					Flavor flavor = flavors[k];
					flavor.predict_number = 1;
					physical_servers[index].flavors.push_back(flavor);
				}
				flavors[k].predict_number--;
				physical_servers[index].left_cpu_core -= flavors[k].cpu_core;
				physical_servers[index].left_memory_size -= flavors[k].memory_size;
				n--;
				//cout << n <<",index:"<< k <<endl;
				if (physical_servers[index].left_cpu_core == 0 || physical_servers[index].left_memory_size == 0)is_finish = true;
			}//endif
			else is_finish = true;//如果仍未找到，当前物理服务器已经装满
		}//endwhile
		index++;
	}//endwhile
}

//数据去噪
void denoise()
{
	int sum;
	double mean;
	double variance;
	double stddev;
	for (unsigned int i = 0; i < flavors.size(); i++)
	{
		sum = 0;
		//求均值
		for (int j = 1; j <= train_day; j++)
		{
			sum += flavors[i].flavor_number_of_day[j];
		}

		mean = 1.0 * sum / train_day;
		//求标准差
		variance = 0.0;
		for (int j = 1; j <= train_day; j++)
		{
			variance += pow(flavors[i].flavor_number_of_day[j] - mean, 2);
		}
		variance /= train_day;
		stddev = sqrt(variance);

		//去除异常数据
		for (int j = 1; j <= train_day; j++)
		{
			if (flavors[i].flavor_number_of_day[j] > 7 * mean)
			{
				flavors[i].flavor_number_of_day[j] = (int)floor(mean);
			}
		}

	}
}