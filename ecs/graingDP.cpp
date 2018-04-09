#include "graingDP.h"

vector<VM> vm;//虚拟机种类以及需要数量
vector<VM> temp;//待dp放入序列
vector<VM> tempput;//一次dp放入的虚拟机序列
vector<PhysicServer> Servers;//物理服务器序列
vector<vector<vector<int>>> vc;//dp表
int focus;//0为cpu,>0为内存，最优先考虑哪个维度的装载量
int cpusize;
int memorysize;

bool getwaitline() {//rate为总cpu/总mem
	/*将虚拟机规格从大到小放入待dp序列，且规格相对于剩余容量越小，放入的该种
	虚拟机越小，当某一维度资源消耗较大时，更多地放入另一维度消耗更大的虚拟机*/
	//cout << "start " << endl;
	int restcpu = Servers[Servers.size() - 1].restcpu;
	int restmem = Servers[Servers.size() - 1].restmemory;

	if (restcpu == 0 || restmem == 0)return false;//如果为0，已经放满
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
			for (int j = 0; j < n; j++) {//尽可能少放
				VM v = vm[i];
				v.num = 1;
				temp.push_back(v);
			}
		}
	}
	/*for (vector<VM>::iterator pr = vm.end(); pr >= vm.begin(); pr--) {//移除已经全部放入的虚拟机规格
		if ((*pr).num == 0)vm.erase(pr);
	}*/
	//cout << "temp.size: " << temp.size() << endl;
	if (temp.size() == 0)return false;//当无法放入符合条件的虚拟机，视为该物理服务器已经装满
	else return true;
}

vector<VM> putaVMin(vector<VM> putin, VM v) {//将虚拟机v放入目标虚拟机序列
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

void putinPhysicServer(vector<VM> putin) {//将一次放入序列的虚拟机装入物理服务器，并更新物理服务器剩余容量
	for (unsigned int i = 0; i < putin.size(); i++) {
		Servers[Servers.size() - 1].VMList = putaVMin(Servers[Servers.size() - 1].VMList, putin[i]);
		Servers[Servers.size() - 1].restcpu -= putin[i].cpu*putin[i].num;
		Servers[Servers.size() - 1].restmemory -= putin[i].mem*putin[i].num;
		vm[putin[i].index].num -= putin[i].num;//更新总待放入虚拟机序列vm各个规格需要数量
	}
}

vector<VM> allocation(int n, int j, int k) {//f[i][j][k] = max{　f[i-1][j][k],f[i-1][j-cpu[i]][k-mem[i]]+cpu[i](或mem[i])　}
	if (n < 0 || j <= 0 || k <= 0) {//未放或不能放
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
	else {//如果选择将第n个放入其中
		vector<VM> r = putaVMin(q, temp[n]);
		vc[n][j][k] = b;
		return r;
	}
}


vector<PhysicServer> allocate_vm(int n, vector<Flavor> flavors, Physical_server physical_server, bool is_cpu) {					//n为总共的虚拟机数, flavors为预测后的每个虚拟机规格，physical_server为物理机规格，is_cpu表示优化的资源是否是cpu
	if (is_cpu)
		focus = 0;//0为cpu,>0为内存，最优先考虑哪个维度的装载量
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
		for (unsigned int i = 0; i < vm.size(); i++) {//更新剩余待放入虚拟机数量
			n += vm[i].num;
		}
	}

	return Servers;
}