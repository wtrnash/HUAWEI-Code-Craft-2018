#include "predict.h"

//���������
Physical_server physical_server;

vector<Flavor> flavors;	//�������ҪԤ���������Ĺ�񼰶�Ӧѵ������¼
int number_of_flavor;	//ҪԤ����������������
bool is_cpu;			//Ҫ�Ż�����Դ��cpu�����ڴ�
string predict_start_time, predict_end_time;		//Ԥ��Ŀ�ʼ�ͽ���ʱ��
time_t predict_start_time_t, predict_end_time_t;	//Ԥ��Ŀ�ʼ�ͽ���ʱ��תΪtime_t��ʽ
int predict_day;					//��Ԥ�������
string train_start_time, train_end_time;		//ѵ���Ŀ�ʼ�ͽ���ʱ��
time_t train_start_time_t, train_end_time_t;	//ѵ���Ŀ�ʼ�ͽ���ʱ��תΪtime_t��ʽ
int train_day;					//��Ԥ�������
int sum_of_flavor = 0;	//ҪԤ������������
vector<Allocated_physical_server> physical_servers;	//�������������

//��Ҫ��ɵĹ��������
void predict_server(char * info[MAX_INFO_NUM], char * data[MAX_DATA_NUM], int data_num, char * filename)
{
	// ��Ҫ���������
	char * result_file = (char *)"17\n\n0 8 0 20";
	//���������ַ���
	get_input(info);	//���������ļ�
	get_data(data, data_num);	//����Ҫѵ��������

	//ѵ��ģ��, Ԥ��
	predict();

	//װ��
	allocate_vm();

	//�������
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
	// ֱ�ӵ�������ļ��ķ��������ָ���ļ���(ps��ע���ʽ����ȷ�ԣ�����н⣬��һ��ֻ��һ�����ݣ��ڶ���Ϊ�գ������п�ʼ���Ǿ�������ݣ�����֮����һ���ո�ָ���)
	write_result(result_file, filename);
}

void get_input(char *info[MAX_INFO_NUM])
{
	//����input��һ�����������
	string physical_info = info[0];
	int first_space = physical_info.find_first_of(' ');
	int second_space = physical_info.find_last_of(' ');
	physical_server.cpu_core = atoi(physical_info.substr(0, first_space).c_str());  
	physical_server.memory_size = atoi(physical_info.substr(first_space + 1,  second_space - first_space - 1).c_str());
	physical_server.disk_size = atoi(physical_info.substr(second_space + 1).c_str());

	//����input������
	number_of_flavor = atoi(info[2]);

	//����flavor
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

	//��¼���Ż�����Դ
	string resource = info[++index];
	if (resource.substr(0, 3).compare("CPU"))
		is_cpu = false;
	else
		is_cpu = true;

	//����ʼ������ʱ��
	index+=2;
	predict_start_time = info[index++];
	predict_end_time = info[index];
	predict_start_time_t = string_to_time(predict_start_time.c_str());
	predict_end_time_t = string_to_time(predict_end_time.c_str());
	predict_day = ceil(((double)(predict_end_time_t - predict_start_time_t)) / (60 * 60 * 24));

	/*
	//�����Ϣ������
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

//�ַ���תtime_t
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
	//��¼ѵ����ʼʱ�䡢����ʱ�估������
	string first_line = data[0];
	int first_tab = first_line.find_first_of('\t');
	int second_tab = first_line.find_last_of('\t');
	//�ҳ����һ���ո�ʹ�ÿ��Խ���ʼʱ���ʱ�������00:00:00
	int last_space = first_line.find_last_of(' ');
	train_start_time = first_line.substr(second_tab + 1, last_space - second_tab - 1) + "00:00:00";
	train_start_time_t = string_to_time(train_start_time.c_str());
	string last_line = data[data_num - 1];
	first_tab = last_line.find_first_of('\t');
	second_tab = last_line.find_last_of('\t');
	train_end_time = last_line.substr(second_tab + 1);
	train_end_time_t = string_to_time(train_end_time.c_str());
	train_day = ceil(((double)(train_end_time_t - train_start_time_t)) / (60 * 60 * 24));

	//����ѵ��������ʼ��ÿ��flavor��ÿ��������Ϊ0
	for (int i = 0; i < number_of_flavor; i++)
	{
		flavors[i].flavor_number_of_day = new unsigned int[train_day + 1];
		fill(flavors[i].flavor_number_of_day, flavors[i].flavor_number_of_day + train_day + 1, 0);	//ȫ����ʼ��Ϊ0
	}
	//����ÿ����¼
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
	//�����Ϣ������
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

//����ѵ����¼
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
			//��һ����¼�����0��0��0����Ҳ�ڵ�һ��
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
	//ȡ��Ԥ���������һ��������
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

//��flavor�������Ż���Դ��������
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
//ģ���˻���������
void allocate_vm()
{
	vector<Flavor> min_allocate_flavors;
	vector<Flavor> allocate_flavors;	//����װ������������
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
	//��������
	sort(allocate_flavors.begin(), allocate_flavors.end(), compare);

	vector<int> indices;	//��¼flavor�����±�
	for (unsigned int i = 0; i < allocate_flavors.size(); i++)
	{
		indices.push_back(i);
	}

	min_allocate_flavors = allocate_flavors;
	vector<Allocated_physical_server> current_physical_server;	//ÿ�η�������������
	double min_utilization_rate = allocate_flavors.size();	//��¼���ٵķ����������ʣ���n-1������������ �����һ������������Դ�����ʱ�ʾ����ʼ��Ϊһ���������һ�����������
	double current_utilization_rate;
	double t = 10.0;	//��ʼ�¶�
	double t_min = 1.0;	//�����¶�
	double k = 0.9999;	//�¶��½�ϵ��
	while(t > t_min)
	{
		random_shuffle(indices.begin(), indices.end());	//�����±�����
		allocate_flavors = min_allocate_flavors;
		//��indicesǰ����Ԫ����Ϊallocate_flavorsҪ�������±�
		swap(allocate_flavors[indices[0]], allocate_flavors[indices[1]]);
		current_physical_server = allocate_one_time(allocate_flavors);
		current_utilization_rate = get_current_utilization_rate(current_physical_server);
		//�����ǰ�Ƚ�С����ֱ��ȡ��ǰΪ���Ž�
		if (current_utilization_rate < min_utilization_rate)
		{
			min_utilization_rate = current_utilization_rate;
			physical_servers = current_physical_server;
			min_allocate_flavors = allocate_flavors;
		}
		else   //����һ�����ʸ��µ�ǰΪ���Ž�
		{
			//exp(delta t / T) ����һ�����ʣ�������0��1֮�䣩������ӽ�
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

//һ��װ��
vector<Allocated_physical_server> allocate_one_time(vector<Flavor> allocate_flavors)
{
	vector<Allocated_physical_server>  allocated_Physical_server;
	//��һ�����������
	int index = 1;
	Allocated_physical_server temp;
	temp.index = index++;
	temp.left_cpu_core = physical_server.cpu_core;
	temp.left_memory_size = physical_server.memory_size;
	allocated_Physical_server.push_back(temp);
	//�������������flavors
	for (unsigned int i = 0; i < allocate_flavors.size(); i++)
	{
		unsigned int m;
		for (m = 0; m < allocated_Physical_server.size(); m++)
		{
			//������Է�
			if (allocate_flavors[i].cpu_core <= allocated_Physical_server[m].left_cpu_core && allocate_flavors[i].memory_size <= allocated_Physical_server[m].left_memory_size)
			{
				//�ж���û���ڸ���������ù�ͬ�����ģ����ù��Ļ�Ԥ��������һ
				unsigned int k;
				for (k = 0; k < allocated_Physical_server[m].flavors.size(); k++)
				{
					if (allocated_Physical_server[m].flavors[k].flavor_name == allocate_flavors[i].flavor_name)
					{
						allocated_Physical_server[m].flavors[k].predict_number++;
						break;
					}
				}

				//û�з��ù���push_backһ���µ�
				if (k == allocated_Physical_server[m].flavors.size())
				{
					Flavor flavor = allocate_flavors[i];
					flavor.predict_number = 1;
					allocated_Physical_server[m].flavors.push_back(flavor);
				}

				//��ȥ���õ�����
				allocated_Physical_server[m].left_cpu_core -= allocate_flavors[i].cpu_core;
				allocated_Physical_server[m].left_memory_size -= allocate_flavors[i].memory_size;
				break;
			}
		}

		if (m == allocated_Physical_server.size())   //������ܷ�����ȫ�µ������
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

//��õ�ǰ������
double get_current_utilization_rate(vector<Allocated_physical_server> allocated_physical_server)
{
	double current_utilization_rate;
	if (is_cpu)
		current_utilization_rate = allocated_physical_server.size() - 1 + 1.0 * (physical_server.cpu_core - allocated_physical_server[allocated_physical_server.size() - 1].left_cpu_core) / physical_server.cpu_core;
	else
		current_utilization_rate = allocated_physical_server.size() - 1 + 1.0 * (physical_server.memory_size - allocated_physical_server[allocated_physical_server.size() - 1].left_memory_size) / physical_server.memory_size;

	return current_utilization_rate;
}