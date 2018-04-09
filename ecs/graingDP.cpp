#include "graingDP.h"

vector<VM> vm;//����������Լ���Ҫ����
vector<VM> temp;//��dp��������
vector<VM> tempput;//һ��dp��������������
vector<PhysicServer> Servers;//�������������
vector<vector<vector<int>>> vc;//dp��
int focus;//0Ϊcpu,>0Ϊ�ڴ棬�����ȿ����ĸ�ά�ȵ�װ����
int cpusize;
int memorysize;

bool getwaitline() {//rateΪ��cpu/��mem
	/*����������Ӵ�С�����dp���У��ҹ�������ʣ������ԽС������ĸ���
	�����ԽС����ĳһά����Դ���Ľϴ�ʱ������ط�����һά�����ĸ���������*/
	//cout << "start " << endl;
	int restcpu = Servers[Servers.size() - 1].restcpu;
	int restmem = Servers[Servers.size() - 1].restmemory;

	if (restcpu == 0 || restmem == 0)return false;//���Ϊ0���Ѿ�����
	double rate = (double)Servers[Servers.size() - 1].cpu/ Servers[Servers.size() - 1].memory;
	double temprate = (double)restcpu / restmem;

	int morecpu, moremem;
	if (temprate >= rate) { morecpu = 5; moremem = 8; }
	else { morecpu = 8; moremem = 5; }
	int max = vm.size() - 1;

	for (; max >= 0; max--)if (vm[max].cpu <= restcpu&&vm[max].mem <= restmem&&vm[max].num>0)break;
	for (int i = max; i >= 0; i --) {
		double temprate2 = (double)vm[i].cpu/ vm[i].mem;
		//cout << "temprate2: " << temprate2 << endl;
		if (vm[i].num > 0 && ((temprate2 >= rate&&morecpu > 0) || (temprate2 < rate && moremem > 0))) {
			if (temprate >= rate)morecpu--;
			else moremem--;
			int n = ceil(pow(vm[i].cpu*vm[i].mem, 0.1));
			int n2 = floor(restcpu/vm[i].cpu);
			int n3 = floor(restmem / vm[i].mem);
			int n4 = vm[i].num;
			if (n > n2)n = n2;
			if (n > n3)n = n3;
			if (n > n4)n = n4;
			for (int j = 0; j < n; j++) {//�������ٷ�
				VM v = vm[i];
				v.num = 1;
				temp.push_back(v);
			}
		}
	}
	/*for (vector<VM>::iterator pr = vm.end(); pr >= vm.begin(); pr--) {//�Ƴ��Ѿ�ȫ���������������
		if ((*pr).num == 0)vm.erase(pr);
	}*/
	//cout << "temp.size: " << temp.size() << endl;
	if (temp.size() == 0)return false;//���޷�����������������������Ϊ������������Ѿ�װ��
	else return true;
}

vector<VM> putaVMin(vector<VM> putin, VM v) {//�������v����Ŀ�����������
	bool isexist = false;
	for (unsigned int i = 0; i < putin.size(); i++) {
		if (putin[i].index == v.index) {
			isexist = true;
			putin[i].num += v.num;
		}
	}
	if (!isexist) {
		if (putin.size()>0) {
			bool isfind = false;
			for (unsigned int i = 0; i < putin.size(); i++) {
				if (putin[i].index > v.index) {
					isfind = true;
					vector<VM>::iterator pr = putin.begin();
					pr += i;
					putin.insert(pr, v);
					break;
				}
			}
			if (!isfind)putin.push_back(v);
		}
		else putin.push_back(v);
	}
	return putin;
}

void putinPhysicServer(vector<VM> putin) {//��һ�η������е������װ����������������������������ʣ������
	for (unsigned int i = 0; i < putin.size(); i++) {
		Servers[Servers.size() - 1].VMList = putaVMin(Servers[Servers.size() - 1].VMList, putin[i]);
		Servers[Servers.size() - 1].restcpu -= putin[i].cpu*putin[i].num;
		Servers[Servers.size() - 1].restmemory -= putin[i].mem*putin[i].num;
		vm[putin[i].index].num -= putin[i].num;//�����ܴ��������������vm���������Ҫ����
	}
}

vector<VM> allocation(int n, int j, int k) {//f[i][j][k] = max{��f[i-1][j][k],f[i-1][j-cpu[i]][k-mem[i]]+cpu[i](��mem[i])��}
	if (n < 0 || j <= 0 || k <= 0) {//δ�Ż��ܷ�
		vector<VM> newone;
		return newone;
	}
	vector<VM> p = allocation(n - 1, j, k);
	vector<VM> q = allocation(n - 1, j - temp[n].cpu, k - temp[n].mem);
	int a = 0;
	if (n >= 1)a = vc[n - 1][j][k];
	int b = 0;
	if (j - temp[n].cpu >= 0 && k - temp[n].mem >= 0) {
		if (n >= 1)b = vc[n - 1][j - temp[n].cpu][k - temp[n].mem];
		if (focus == 0)
			b = b + temp[n].cpu + floor((double)temp[n].mem/ 2 / memorysize*cpusize);
		else
			b = b + temp[n].mem + floor((double)temp[n].cpu/ 2 / cpusize*memorysize);
	}
	if (a >= b) {
		vc[n][j][k] = a;
		return p;
	}
	else {//���ѡ�񽫵�n����������
		vector<VM> r = putaVMin(q, temp[n]);
		vc[n][j][k] = b;
		return r;
	}
}


vector<PhysicServer> allocate_vm(int n, vector<Flavor> flavors, Physical_server physical_server, bool is_cpu) {					//nΪ�ܹ����������, flavorsΪԤ����ÿ����������physical_serverΪ��������is_cpu��ʾ�Ż�����Դ�Ƿ���cpu
	if (is_cpu)
		focus = 0;//0Ϊcpu,>0Ϊ�ڴ棬�����ȿ����ĸ�ά�ȵ�װ����
	else
		focus = 1;
	cpusize = physical_server.cpu_core;
	memorysize = physical_server.memory_size;
	for (unsigned int i = 0; i < flavors.size(); i++)
	{
		VM temp_vm = {flavors[i].flavor_name, atoi(flavors[i].flavor_name.substr(6).c_str()) - 1,flavors[i].cpu_core, (int)ceil(1.0 * flavors[i].memory_size / 1024), (int)flavors[i].predict_number};
		vm.push_back(temp_vm);
	}

	while (n > 0) {
		PhysicServer ps = { cpusize,memorysize,cpusize,memorysize};
		Servers.push_back(ps);
		while (getwaitline()) {
			vector<VM> a = temp;
			int restcpu = Servers[Servers.size() - 1].restcpu;
			int restmem = Servers[Servers.size() - 1].restmemory;
			
			for (unsigned int i = 0; i < temp.size(); i++)
			{
				vc.push_back(vector<vector<int>>(restcpu + 1, vector<int>(restmem + 1, 0)));
			}
				
			tempput = allocation(temp.size()-1,restcpu,restmem);
			putinPhysicServer(tempput);
		
			vc.clear();
			temp.clear();
			tempput.clear();
		}
		n = 0;
		for (unsigned int i = 0; i < vm.size(); i++) {//����ʣ����������������
			n += vm[i].num;
		}
	}

	return Servers;
}