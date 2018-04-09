#include "predict.h"
#include "graingDP.h"

//����������ṹ��
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
	vector<PhysicServer> answer = allocate_vm(sum_of_flavor, flavors, physical_server, is_cpu);

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
	result += to_string(answer.size());
	result += "\n";
	for (unsigned int i = 0; i < answer.size(); i++)
	{
		result += to_string(i + 1);
		for (unsigned int j = 0; j < answer[i].VMList.size(); j++)
		{
			result += " ";
			result += answer[i].VMList[j].name;
			result += " ";
			result += to_string(answer[i].VMList[j].num);
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
	//���ֵ
	for (unsigned int i = 0; i < flavors.size(); i++)
	{
		sum = 0;
		for (int j = 1; j <= train_day; j++)
		{
			sum += flavors[i].flavor_number_of_day[j];
		}

		flavors[i].predict_number = (double)sum / train_day * predict_day;
		sum_of_flavor += flavors[i].predict_number;
	}

	return;
}
