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
double original_rate;

//��Ҫ��ɵĹ��������
void predict_server(char * info[MAX_INFO_NUM], char * data[MAX_DATA_NUM], int data_num, char * filename)
{
	// ��Ҫ���������
	char * result_file = (char *)"17\n\n0 8 0 20";
	//���������ַ���
	get_input(info);	//���������ļ�
	get_data(data, data_num);	//����Ҫѵ��������

	//����ȥ��
	denoise();
	//ѵ��ģ��, Ԥ��
	predict();

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

	//װ��
	allocate_vm();

	//�������
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

	original_rate = 1.0 * physical_server.cpu_core / physical_server.memory_size;
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
	index += 2;
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
//һ��ָ��ƽ��Ԥ�ⷨ
double* single_exponential_smoothing(double a, unsigned int* s0)
{
	double *s1 = new double[train_day + predict_day + 1];
	//���ѵ����ƽ��ֵ
	s1[1] = s0[1];	//��һ��ƽ��ֵ���ڵ�һ��ԭʼֵ
	for (int i = 2; i <= train_day; i++)
	{
		s1[i] = a * s0[i] + (1 - a) * s1[i - 1];
	}
	//Ԥ��
	for (int i = train_day + 1; i < train_day + predict_day + 1; i++)
	{
		s1[i] = a * s0[i - 1] + (1 - a) * s1[i - 1];
	}

	return s1;
}

//����ָ��ƽ��Ԥ�ⷨ
double* second_exponential_smoothing(double a, double* s1)
{
	double *s2 = new double[train_day + predict_day + 1];
	//��ȡѵ����ƽ��ֵ
	s2[1] = s1[1];
	for (int i = 2; i <= train_day; i++)
	{
		s2[i] = a * s1[i] + (1 - a) * s2[i - 1];
	}
	//Ԥ��
	double at = 2.0 * s1[train_day] - s2[train_day];
	double bt = a / (1 - a) * (s1[train_day] - s2[train_day]);
	for (int i = train_day + 1; i < train_day + predict_day + 1; i++)
	{
		s2[i] = at + bt * (i - train_day);
	}
	return s2;
}
//���������
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
{//��cpu��mem������ֵ��С����������ȵİ���ģ��С�������򣬼�����ʱmem����������
	for (unsigned int i = 0; i < flavors.size(); i++)
		flavors[i].gap = flavors[i].cpu_core - flavors[i].memory_size * original_rate;
	vector<Flavor> new_flavors;
	new_flavors.push_back(flavors[0]);
	for (unsigned int i = 1; i < flavors.size(); i++) {
		bool is_insert = false;
		for (unsigned int j = 0; j < new_flavors.size(); j++) {
			if (abs(flavors[i].gap) < abs(new_flavors[j].gap)) {//������С��ֱ�Ӳ���jλ��
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
{//�����ǰ������ʣ����Դ������ֵռĿǰʣ��cpu��Դ��%����ʱ��Ϊ����Դ��ȣ���ȵ�ʱ��ѡȡһ��������ʣ����Դ%
						//��һ�������������룬Ȼ����Ѱ������ֵ��ӽ���������ͬ�����������ʹ����Դ��ȣ��ظ����Ϲ���
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
			if (server_gap <= is_equal) {//��ǰ��Сʱ��Ϊ���Խ�Ϊ����ķ�������������ؿ��Ǽ�С���
				for (int i = (int)flavors.size() - 1; i >= 0; i--) {
					if (abs(flavors[i].gap - server_gap) < 1)limit = 1;//limitֵ�����С�йأ������С������Ϊ������ȫ������limitΪ1
					else limit = 0.5;//������ϴ������ռ�װ����һ���Լ�С���
					if (flavors[i].predict_number > 0 && 1.0 * flavors[i].cpu_core / physical_servers[index].left_cpu_core <= limit &&
						1.0 * flavors[i].cpu_core / physical_servers[index].left_cpu_core <= limit) {
						k = i;
						is_find = true;
						break;
					}
				}//endfor
			}//endif
			else {//������ϴ���ѡ������ӽ��������װ���С���
				for (unsigned int i = 0; i < flavors.size(); i++) {//�Ҳ�����ֵ��ӽ�������ͬ�Դ��
					if (flavors[i].predict_number > 0 && abs(flavors[i].gap) >= abs(server_gap) && flavors[i].gap*server_gap > 0 &&
						flavors[i].cpu_core <= physical_servers[index].left_cpu_core &&
						flavors[i].memory_size <= physical_servers[index].left_memory_size) {
						k = i;
						is_find = true;
						break;
					}
				}
				if (!is_find) {//���δ�ҵ����Ҳ����СһЩ��
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
			if (!is_find) {//������϶�δ�ҵ�����������ǰ��������Է���Ľ�cpuƫ���ƫ�٣������������ɷ���װ��
				for (int i = (int)flavors.size() - 1; i >= 0; i--) {
					if (flavors[i].predict_number > 0 && flavors[i].cpu_core <= physical_servers[index].left_cpu_core &&
						flavors[i].memory_size <= physical_servers[index].left_memory_size) {
						k = i;
						is_find = true;
						break;
					}
				}
			}//endelse
			if (is_find) {//���뵱ǰ�����������
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
			else is_finish = true;//�����δ�ҵ�����ǰ����������Ѿ�װ��
		}//endwhile
		index++;
	}//endwhile
}

//����ȥ��
void denoise()
{
	int sum;
	double mean;
	double variance;
	double stddev;
	for (unsigned int i = 0; i < flavors.size(); i++)
	{
		sum = 0;
		//���ֵ
		for (int j = 1; j <= train_day; j++)
		{
			sum += flavors[i].flavor_number_of_day[j];
		}

		mean = 1.0 * sum / train_day;
		//���׼��
		variance = 0.0;
		for (int j = 1; j <= train_day; j++)
		{
			variance += pow(flavors[i].flavor_number_of_day[j] - mean, 2);
		}
		variance /= train_day;
		stddev = sqrt(variance);

		//ȥ���쳣����
		for (int j = 1; j <= train_day; j++)
		{
			if (flavors[i].flavor_number_of_day[j] > 7 * mean)
			{
				flavors[i].flavor_number_of_day[j] = (int)floor(mean);
			}
		}

	}
}