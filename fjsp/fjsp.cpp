
// fjsp.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include <iostream>
#include<fstream>
#include<vector>
#include<random>
#include<ctime>
#include<string>
#include"us_time_count.h"
#include<algorithm>
#include<list>
#include<unordered_map>
#include<array>
#include<queue>
#include<cstring>
using namespace std;

std::random_device rd;
mt19937 gen(rd());

struct ARR {
    int a[2];
	/*ARR(){
		this->a[0] = -3;
		this->a[1] = -3;
	}*/
	/*void operator =(const ARR&b){
		this->a[0] = b.a[0];
		this->a[1] = b.a[1];
	}*/
};

class Solution {
public:
	vector<vector<ARR>> ope_in_machine;//操作在机器序列中的位置
	vector<vector<ARR>> machine_ass;//机器任务分配
	//vector<vector<ARR>> MP;//机器前序
	//vector<vector<ARR>> MS;//机器后继
	vector<int> ope_count_per_machine;//每个机器安排的任务数
	//vector<ARR> cri_path;//关键路径
	vector<vector<int>> R;//记录R值
	vector<vector<int>> Q;//记录Q值
    int makespan = 0;//记录makespan
    vector<vector<ARR>> cri_block;//记录关键块
    vector<vector<int>> where_machine;
};

vector<vector<vector<ARR>>> job;
vector<vector<vector<ARR>>> best_solution;
vector<vector<int>> empty_QorR;
int machine_count = 0;
int job_count = 0;
int ope_sum = 0;
int cri_path_num = 0;
int most_machine = 0;
int Tabu_tenure = 5;
int iter1 = 0;
float get_best_time = 0;
stop_watch record_time;
int global_best_obj = INT_MAX;
int same_pos_count = 0;
int update_count = 0;
int update_count1 = 0;

queue<ARR> store, store1;
vector<vector<int>> temp_m_ass(machine_count);
vector<ARR> cur_block;


struct Hashfunc {
    size_t operator() (const vector<ARR>& key) const {
        size_t ans = hash<int>()(key[0].a[0])^ hash<int>()(key[0].a[1]);
        for (int i = 1; i < key.size();++i) {
            ans ^= (hash<int>()(key[i].a[0]) ^ hash<int>()(key[i].a[1]));
        }
        return ans;
    }
};
struct Equalfunc {
    bool operator() (const vector<ARR>& a, const vector<ARR>& b) const {
        bool ans = true;
        if (a.size() != b.size()) {
            return false;
        }
        for (int i = 0; i < a.size(); ++i) {
            ans = (a[i].a[0] == b[i].a[0] && a[i].a[1] == b[i].a[1])&&ans;
        }
        return ans;
    }
};

bool my_cmp(vector<int>& a, vector<int>& b) {
    return (a[0] + a[1]) < (b[0] + b[1]);
}

void get_file(char* input_file) {//读取算例文件
    ifstream fin;
    fin.open(input_file, ios::in);
    if (!fin.is_open()) {
        cout << "Can not find target  file." << endl;
        system("pause");
    }
    int i = 0;
    string s;
	while (!fin.eof()) {
		getline(fin, s);
		int size = s.size();
		int j = 0;
		if (i == 0) {
			string x;
			while (s[j] != ' ') {
				x += s[j];
				++j;
			}
			job_count = atoi(x.c_str());
            job.resize(job_count);
			x.clear();
			++j;
			while (s[j] != ' ') {
				x += s[j];
				++j;
			}
			machine_count = atoi(x.c_str());
			x.clear();
			++j;
			while (j < size) {
				x += s[j];
				++j;
			}
			most_machine = atoi(x.c_str());
		}
		else {

            int space_count = 0;
            int k = -1;
            while (j < size) { 
                while (s[j] == ' ') {
                    ++space_count;
                    ++j;
                }
                string x;
                switch (space_count)
                {
                case 0: {
                    while (s[j] != ' ') {
                        x += s[j];
                        ++j;
                    }
                    job[i - 1].resize(atoi(x.c_str()));
                    break;
                }
                case 2: {
                    while (s[j] != ' ') {
                        x += s[j];
                        ++j;
                    }
                    int t = atoi(x.c_str());
                    ++j;
                    x.clear();
                    while (s[j] != ' '&&j<size) {
                        x += s[j];
                        ++j;
                    }
                    job[i - 1][k].push_back({ t,atoi(x.c_str()) });
                    break;
                }
                case 4: {
                    while (s[j] != ' ') {
                        x += s[j];
                        ++j;
                    }
                    job[i - 1][++k].reserve(atoi(x.c_str()));
                    break;
                }
                default:
                    break;
                }
                space_count = 0;
            }
		}
		i++;
	}
    fin.close();
    return;
}

void Initial(Solution &S) {//随机生成初始解
    S.ope_in_machine.resize(job_count);
    S.machine_ass.resize(machine_count);
    //S.MP.resize(job_count);
    //S.MS.resize(job_count);
    S.R.resize(job_count);
    S.Q.resize(job_count);
    S.ope_count_per_machine.reserve(machine_count);
    S.where_machine.resize(job_count);
    //unordered_map<int, int> arrive_job_site;
    vector<ARR> arrive_job_site(job_count);
    for (int i = 0; i < job_count; i++) {
        arrive_job_site[i] = {i,0};
        S.ope_in_machine[i].resize(job[i].size());
        S.R[i].resize(job[i].size(), -1);
        S.Q[i].resize(job[i].size(), -1);
        //S.MP[i].resize(job[i].size(), {-1,-1});
        //S.MS[i].resize(job[i].size(), {-1,-1});
        S.where_machine[i].resize(job[i].size(), -1);
    }
    int count = 0;
    int cur_size = job_count-1;
    while (count < ope_sum) {
        uniform_int_distribution<> dis1(0, cur_size);
        int f1 = dis1(gen);
        int f = arrive_job_site[f1].a[0];
        int s = arrive_job_site[f1].a[1];
        uniform_int_distribution<> dis(0, job[f][s].size() - 1);
        int t = dis(gen);
        /*auto it = job[f][s].begin();
        for (int i = 0; i < t; ++i) {
            ++it;
        }
        t = (*it).first;*/
        S.machine_ass[job[f][s][t].a[0]].push_back({f,s});//是否可行？ 可行！
        S.where_machine[f][s] = t;
        int t1 = S.machine_ass[job[f][s][t].a[0]].size() - 1;
        S.ope_in_machine[f][s] = { job[f][s][t].a[0],t1 };
        /*if (t1 != 0) {
            S.MS[S.machine_ass[job[f][s][t].a[0]][t1 - 1].a[0]][S.machine_ass[job[f][s][t].a[0]][t1 - 1].a[1]] = { f,s };
            S.MP[f][s] = { S.machine_ass[job[f][s][t].a[0]][t1 - 1].a[0] ,S.machine_ass[job[f][s][t].a[0]][t1 - 1].a[1] };
        }*/
        ++count;
        ++arrive_job_site[f1].a[1];
        if (arrive_job_site[f1].a[1]>=job[f].size()) {
            if (f1 != cur_size) {
                ARR temp = arrive_job_site[cur_size];
                arrive_job_site[cur_size] = arrive_job_site[f1];
                arrive_job_site[f1] = temp;
            }
            --cur_size;
        }
    }
    for (const auto& x : S.machine_ass) {
        S.ope_count_per_machine.push_back(x.size());
    }
}

void caculate_makespan_Ini(Solution &S){//计算makespan,并更新R,Q以及cri_block
    S.cri_block.clear();
    //S.cri_path.clear();
    S.makespan = 0;
    //S.Q.assign(job_count, {});
    //S.R.assign(job_count, {});
	S.Q.assign(empty_QorR.begin(), empty_QorR.end());
	S.R.assign(empty_QorR.begin(), empty_QorR.end());
	///*S.Q = empty_QorR;
	//S.R = empty_QorR;*/
	//if (S.Q.size() == 0 && S.R.size() == 0) {
	//	S.Q = empty_QorR;
	//	S.R = empty_QorR;
	//}
	//else {
	//	for (int i = 0; i < job_count; ++i) {
	//		for (int j = 0; j < job[i].size(); ++j) {
	//			S.Q[i][j] = -1;
	//			S.R[i][j] = -1;
	//		}
	//	}
	//}
    S.ope_count_per_machine.assign(machine_count, 0);
    for (int i = 0; i < machine_count; ++i) {
        S.ope_count_per_machine[i] = S.machine_ass[i].size();
    }
    for (int i = 0; i < job_count; ++i) {
        //cri_path_pre[i].resize(job[i].size());
        //S.R[i].assign(job[i].size(), -1);
        //S.Q[i].assign(job[i].size(), -1);
        if (S.ope_in_machine[i][0].a[1] == 0) {
            S.R[i][0] = 0;
            store.push({ i,0 });
            //cri_path_pre[i][0] = { -1,-1 };
        }
		int ss2 = job[i].size() - 1;
		ARR ss1 = S.ope_in_machine[i][ss2];
        if (ss1.a[1] ==S.machine_ass[ss1.a[0]].size() - 1) {
            S.Q[i][ss2] = 0;
            store1.push({ i,ss2 });
        }
    }
    ARR temp;
	stop_watch s;
	s.start();
	int sum = 0;
    while (!store.empty()) {
		//++sum;
        temp = store.front();
        store.pop();
        //cout << "store.size:" << store.size() << endl;
        //cout << temp.a[0] << "," << temp.a[1] << " " << endl;
        int t1 = temp.a[0];
        int t2 = temp.a[1] + 1;
        if (t2 < job[t1].size()&&S.R[t1][t2]==-1) {//求当前JS的R
            int t3 = S.ope_in_machine[t1][t2].a[0];
            int t4 = S.ope_in_machine[t1][t2].a[1];
            //if (t4 == 0 || S.R[S.machine_ass[t3][t4-1].a[0]][S.machine_ass[t3][t4 - 1].a[1]] != -1) {
     //           if (t4 == 0) {
     //               S.R[t1][t2] = S.R[t1][temp.a[1]] + job[t1][temp.a[1]][S.where_machine[t1][temp.a[1]]].a[1];
     //               //cri_path_pre[t1][t2] = temp;
					//store.push({ t1,t2 });
     //           }
     //           else {
					//if (S.R[S.machine_ass[t3][t4 - 1].a[0]][S.machine_ass[t3][t4 - 1].a[1]] != -1) {
					//	int s1 = S.machine_ass[t3][t4 - 1].a[0];
					//	int s2 = S.machine_ass[t3][t4 - 1].a[1];
					//	int s3 = S.R[t1][temp.a[1]] + job[t1][temp.a[1]][S.where_machine[t1][temp.a[1]]].a[1];
					//	int s4 = S.R[s1][s2] + job[s1][s2][S.where_machine[s1][s2]].a[1];
					//	S.R[t1][t2] = s3>s4?s3:s4;
					//	store.push({ t1,t2 });
					//}
     //           }
                //store.push({ t1,t2 });
            //}
			if (t4 == 0 || S.R[S.machine_ass[t3][t4 - 1].a[0]][S.machine_ass[t3][t4 - 1].a[1]] != -1) {
				if (t4 == 0) {
					S.R[t1][t2] = S.R[t1][temp.a[1]] + job[t1][temp.a[1]][S.where_machine[t1][temp.a[1]]].a[1];
					//cri_path_pre[t1][t2] = temp;
				}
				else {
					int s1 = S.machine_ass[t3][t4 - 1].a[0];
					int s2 = S.machine_ass[t3][t4 - 1].a[1];
					int s3 = S.R[t1][temp.a[1]] + job[t1][temp.a[1]][S.where_machine[t1][temp.a[1]]].a[1];
					int s4 = S.R[s1][s2] + job[s1][s2][S.where_machine[s1][s2]].a[1];
					if (s3 > s4) {
						S.R[t1][t2] = s3;
						//cri_path_pre[t1][t2] = temp;
					}
					else {
						S.R[t1][t2] = s4;
						//cri_path_pre[t1][t2] = { s1,s2 };
					}
				}
				store.push({ t1,t2 });
			}
        }
        int t3 = S.ope_in_machine[t1][temp.a[1]].a[0];
        int t4 = S.ope_in_machine[t1][temp.a[1]].a[1];
        if (t4 != S.machine_ass[t3].size() - 1&& S.R[S.machine_ass[t3][t4+1].a[0]][S.machine_ass[t3][t4 + 1].a[1]] == -1) {//求当前MS的R
            int t5 = S.machine_ass[t3][t4 + 1].a[0];
            int t6 = S.machine_ass[t3][t4 + 1].a[1];
            //if (t6 == 0 || S.R[t5][t6 - 1] != -1) {
                /*if (t6 == 0) {
                    S.R[t5][t6] = S.R[t1][temp.a[1]] + job[t1][temp.a[1]][S.where_machine[t1][temp.a[1]]].a[1];
					store.push({ t5,t6 });
                }
                else {
					if (S.R[t5][t6 - 1] != -1) {
						int s3 = S.R[t1][temp.a[1]] + job[t1][temp.a[1]][S.where_machine[t1][temp.a[1]]].a[1];
						int s4 = S.R[t5][t6 - 1] + job[t5][t6 - 1][S.where_machine[t5][t6 - 1]].a[1];
						S.R[t5][t6] = s3>s4?s3:s4;
						store.push({ t5,t6 });
					}
                }*/
                //store.push({ t5,t6 });
            //}
			if (t6 == 0 || S.R[t5][t6 - 1] != -1) {
				if (t6 == 0) {
					S.R[t5][t6] = S.R[t1][temp.a[1]] + job[t1][temp.a[1]][S.where_machine[t1][temp.a[1]]].a[1];
					//cri_path_pre[t5][t6] = temp;
				}
				else {
					int s3 = S.R[t1][temp.a[1]] + job[t1][temp.a[1]][S.where_machine[t1][temp.a[1]]].a[1];
					int s4 = S.R[t5][t6 - 1] + job[t5][t6 - 1][S.where_machine[t5][t6 - 1]].a[1];
					if (s3 > s4) {
						S.R[t5][t6] = s3;
						//cri_path_pre[t5][t6] = temp;
					}
					else {
						S.R[t5][t6] = s4;
						//cri_path_pre[t5][t6] = { t5,t6 - 1 };
					}
				}
				store.push({ t5,t6 });
			}
        }
    }
    while (!store1.empty()) {
		++sum;
        temp = store1.front();
        store1.pop();
        int t1 = temp.a[0];
        int t2 = temp.a[1] - 1;
        if (temp.a[1] != 0) {
            if (S.Q[t1][t2] == -1) {
                int t3 = S.ope_in_machine[t1][t2].a[0];
                int t4 = S.ope_in_machine[t1][t2].a[1];
                //if (t4 == S.ope_count_per_machine[t3] - 1 || S.Q[S.machine_ass[t3][t4+1].a[0]][S.machine_ass[t3][t4+1].a[1]] != -1) {
                    /*if (t4 == S.machine_ass[t3].size() - 1) {
                        S.Q[t1][t2] = S.Q[t1][temp.a[1]] + job[t1][temp.a[1]][S.where_machine[t1][temp.a[1]]].a[1];
						store1.push({ t1,t2 });
                    }
                    else {
						if (S.Q[S.machine_ass[t3][t4 + 1].a[0]][S.machine_ass[t3][t4 + 1].a[1]] != -1) {
							int s1 = S.machine_ass[t3][t4 + 1].a[0];
							int s2 = S.machine_ass[t3][t4 + 1].a[1];
							int s3 = S.Q[t1][temp.a[1]] + job[t1][temp.a[1]][S.where_machine[t1][temp.a[1]]].a[1];
							int s4 = S.Q[s1][s2] + job[s1][s2][S.where_machine[s1][s2]].a[1];
							S.Q[t1][t2] = s3>s4?s3:s4;
							store1.push({ t1,t2 });
						}
                    }*/
                    //store1.push({ t1,t2 });
                //}
				if (t4 == S.ope_count_per_machine[t3] - 1 || S.Q[S.machine_ass[t3][t4 + 1].a[0]][S.machine_ass[t3][t4 + 1].a[1]] != -1) {
					if (t4 == S.ope_count_per_machine[t3] - 1) {
						S.Q[t1][t2] = S.Q[t1][temp.a[1]] + job[t1][temp.a[1]][S.where_machine[t1][temp.a[1]]].a[1];
					}
					else {
						int s1 = S.machine_ass[t3][t4 + 1].a[0];
						int s2 = S.machine_ass[t3][t4 + 1].a[1];
						int s3 = S.Q[t1][temp.a[1]] + job[t1][temp.a[1]][S.where_machine[t1][temp.a[1]]].a[1];
						int s4 = S.Q[s1][s2] + job[s1][s2][S.where_machine[s1][s2]].a[1];
						if (s3 > s4) {
							S.Q[t1][t2] = s3;
						}
						else {
							S.Q[t1][t2] = s4;
						}
					}
					store1.push({ t1,t2 });
				}
            }
        }
        int t3 = S.ope_in_machine[t1][temp.a[1]].a[0];
        int t4 = S.ope_in_machine[t1][temp.a[1]].a[1];
        if (t4 != 0) {
            int t5 = S.machine_ass[t3][t4 -1].a[0];
            int t6 = S.machine_ass[t3][t4 -1].a[1];
            if (S.Q[t5][t6] == -1) {
                //if (t6 == job[t5].size() - 1 || S.Q[t5][t6 + 1] != -1) {
                    /*if (t6 == job[t5].size() - 1) {
                        S.Q[t5][t6] = S.Q[t1][temp.a[1]] + job[t1][temp.a[1]][S.where_machine[t1][temp.a[1]]].a[1];
						store1.push({ t5,t6 });
                    }
                    else {
						if (S.Q[t5][t6 + 1] != -1) {
							int s3 = S.Q[t1][temp.a[1]] + job[t1][temp.a[1]][S.where_machine[t1][temp.a[1]]].a[1];
							int s4 = S.Q[t5][t6 + 1] + job[t5][t6 + 1][S.where_machine[t5][t6 + 1]].a[1];
							S.Q[t5][t6] = s3>s4?s3:s4;
							store1.push({ t5,t6 });
						}
                    }*/
                    //store1.push({ t5,t6 });
               // }
				if (t6 == job[t5].size() - 1 || S.Q[t5][t6 + 1] != -1) {
					if (t6 == job[t5].size() - 1) {
						S.Q[t5][t6] = S.Q[t1][temp.a[1]] + job[t1][temp.a[1]][S.where_machine[t1][temp.a[1]]].a[1];
					}
					else {
						int s3 = S.Q[t1][temp.a[1]] + job[t1][temp.a[1]][S.where_machine[t1][temp.a[1]]].a[1];
						int s4 = S.Q[t5][t6 + 1] + job[t5][t6 + 1][S.where_machine[t5][t6 + 1]].a[1];
						if (s3 > s4) {
							S.Q[t5][t6] = s3;
						}
						else {
							S.Q[t5][t6] = s4;
						}
					}
					store1.push({ t5,t6 });
				}
            }
        }
    }
	/*++iter1;
	if (iter1 % 10000 == 0) {
		cout.clear(ios::goodbit);
		cout << "sum=" << sum << endl;
		cout.setstate(ios::failbit);
	}*/
    //ARR end_ope = { -1,-1 };
    for (int i = 0; i < job_count; ++i) {
        int t = S.R[i][job[i].size() - 1] + job[i][job[i].size() - 1][S.where_machine[i][job[i].size() - 1]].a[1]+S.Q[i][job[i].size()-1];
        if (t > S.makespan) {
            S.makespan = t;
            //end_ope = { i,int(job[i].size() - 1) };
        }
    }
	temp_m_ass.assign(machine_count, {});
    for (int i = 0; i < machine_count;++i) {
        temp_m_ass[i].resize(S.machine_ass[i].size(), 0);
    }
    for (int i = 0; i < job_count; ++i) {
        for (int j = 0; j < job[i].size(); ++j) {
            int t = S.ope_in_machine[i][j].a[0];
            int t1= S.ope_in_machine[i][j].a[1];
            if (S.R[i][j] + job[i][j][S.where_machine[i][j]].a[1] + S.Q[i][j] == S.makespan) {
                temp_m_ass[t][t1] = 1;
            }
        }
    }
    //cur_block.reserve(int(ope_sum / machine_count) + 1);
    for (int i = 0; i < machine_count; ++i) {
        cur_block.clear();
        for (int j = 0; j < S.machine_ass[i].size(); ++j) {
            if (temp_m_ass[i][j] == 0) {
                if (cur_block.size() != 0) {
                    S.cri_block.push_back(cur_block);
                    cur_block.clear();
                }
            }
            else {
                cur_block.push_back({ S.machine_ass[i][j].a[0],S.machine_ass[i][j].a[1] });
                if (j == S.machine_ass[i].size() - 1) {
                    S.cri_block.push_back(cur_block);
                }
                //S.cri_path.push_back({ S.machine_ass[i][j].a[0],S.machine_ass[i][j].a[1] });
            }
        }
    }
}

int caculate_d(Solution &S1, Solution & S2) {
    int d_sum = 0;
	same_pos_count = 0;
    for (int i = 0; i < job_count; i++) {
        for (int j = 0; j < job[i].size(); j++) {
            if (S1.ope_in_machine[i][j].a[0] == S2.ope_in_machine[i][j].a[0]) {
				if (S1.ope_in_machine[i][j].a[1] == S2.ope_in_machine[i][j].a[1])
					++same_pos_count;
                d_sum += abs(S1.ope_in_machine[i][j].a[1] - S2.ope_in_machine[i][j].a[1]);
            }
            else {
                int t1 = S1.ope_in_machine[i][j].a[1] + S2.ope_in_machine[i][j].a[1];
                int t2 = S1.ope_in_machine[i][j].a[0];
                int t3 = S2.ope_in_machine[i][j].a[0];
                int t4 = S1.machine_ass[t2].size() -1 - S1.ope_in_machine[i][j].a[1] + 
                    S2.machine_ass[t3].size()-1 - S1.ope_in_machine[i][j].a[1];
                d_sum += t1 > t4 ? t1 : t4;
            }
        }
    }
    return d_sum;
}

int update_d(Solution& S1,Solution&S2,int machine1,int machine2,int ori_d,int flag) {
    int d_m1 = 0;
    for (const auto& x : S1.machine_ass[machine1]) {
        int ai = x.a[0],aj=x.a[1];
        if (S1.ope_in_machine[ai][aj].a[0] == S2.ope_in_machine[ai][aj].a[0]) {
            d_m1 += abs(S1.ope_in_machine[ai][aj].a[1] - S2.ope_in_machine[ai][aj].a[1]);
        }
        else {
            int t1 = S1.ope_in_machine[ai][aj].a[1] + S2.ope_in_machine[ai][aj].a[1];
            int t2 = S1.ope_in_machine[ai][aj].a[0];
            int t3 = S2.ope_in_machine[ai][aj].a[0];
            int t4 = S1.ope_count_per_machine[t2] - 1 - S1.ope_in_machine[ai][aj].a[1] +
                S2.ope_count_per_machine[t3] - 1 - S1.ope_in_machine[ai][aj].a[1];
            d_m1 += t1 > t4 ? t1 : t4;
        }
    }
    if (flag == 0) {
        ori_d += d_m1;
    }
    else {
        ori_d -= d_m1;
    }
    if (machine2 != -1) {
        int d_m2 = 0;
        for (const auto& x : S1.machine_ass[machine2]) {
            int ai = x.a[0], aj = x.a[1];
            if (S1.ope_in_machine[ai][aj].a[0] == S2.ope_in_machine[ai][aj].a[0]) {
                d_m2 += abs(S1.ope_in_machine[ai][aj].a[1] - S2.ope_in_machine[ai][aj].a[1]);
            }
            else {
                int t1 = S1.ope_in_machine[ai][aj].a[1] + S2.ope_in_machine[ai][aj].a[1];
                int t2 = S1.ope_in_machine[ai][aj].a[0];
                int t3 = S2.ope_in_machine[ai][aj].a[0];
                int t4 = S1.ope_count_per_machine[t2] - 1 - S1.ope_in_machine[ai][aj].a[1] +
                    S2.ope_count_per_machine[t3] - 1 - S1.ope_in_machine[ai][aj].a[1];
                d_m2 += t1 > t4 ? t1 : t4;
            }
        }
        if (flag == 0) {
            ori_d += d_m2;
        }
        else {
            ori_d -= d_m2;
        }
    }
    return ori_d;
}

int caculate_makespan(Solution& S) {//暂定
    //int makespan = 0;
    //vector<vector<int>> start_time(job_count);
    //for (int i = 0; i < start_time.size();++i) {
    //    start_time[i].resize(job[i].size(), 0);
    //}
    //for (int i = 0; i < S[0].size();++i) {
    //    int cur_makespan = 0;
    //    for (const auto& y : S[0][i]) {
    //        start_time[y.a[0]][y.a[1]] = cur_makespan;
    //        cur_makespan += job[y.a[0]][y.a[1]][i];
    //    }
    //    /*cri_path_num = cur_makespan > makespan ? i : cri_path_num;
    //    makespan = cur_makespan > makespan ? cur_makespan : makespan;*/
    //}
    //for (int i = 0; i < job_count; ++i) {
    //    for (int j = 1; j < job[i].size(); ++j) {
    //        if (start_time[i][j] <= start_time[i][j - 1]) {
    //            int t = S[1][i][j].a[0];
    //            int t1= S[1][i][j].a[1];
    //            int t2 = start_time[i][j - 1] + job[i][j - 1][t] - start_time[i][j];
    //            for (int k = t1; k < S[0][t].size(); ++k) {
    //                start_time[S[0][t][k].a[0]][S[0][t][k].a[1]] += t2;
    //            }
    //        }
    //    }
    //    int t = job[i].size() - 1;
    //    if (start_time[i][t] + job[i][t][S[1][i][t].a[0]] > makespan)
    //        makespan = start_time[i][t] + job[i][t][S[1][i][t].a[0]];
    //}
    return 0;
}

void make_move(vector<int>& move, Solution& S) {//进行移动
    int ai = move[0], aj = move[1], bi = move[2], bj = move[3];
    if (move[4] == 0) {
        //cout << "前插" << endl;
        int m1 = S.ope_in_machine[ai][aj].a[0];
        int m2 = -1;
        if(move[2]==-2){
            m2 = move[3];
        }
        else {
            m2 = S.ope_in_machine[bi][bj].a[0];
        }
        S.machine_ass[m1].erase(S.machine_ass[m1].begin() + S.ope_in_machine[ai][aj].a[1]); 
        if(m1==m2)
            S.machine_ass[m2].insert(S.machine_ass[m2].begin() + (S.ope_in_machine[bi][bj].a[1]<0?0:
                S.ope_in_machine[bi][bj].a[1]), {ai,aj});
        else {
            if (m2 == -2) {
                S.machine_ass[m2].insert(S.machine_ass[m2].begin(), { ai,aj });
            }
            else {
                S.machine_ass[m2].insert(S.machine_ass[m2].begin() + S.ope_in_machine[bi][bj].a[1], { ai,aj });
            }
        }
        for (int i = 0; i < S.machine_ass[m1].size(); ++i) {
            int t1 = S.machine_ass[m1][i].a[0], t2 = S.machine_ass[m1][i].a[1];
            S.ope_in_machine[t1][t2] = { m1,i };
            /*if(i!=0)
                S.MS[S.machine_ass[m1][i - 1].a[0]][S.machine_ass[m1][i - 1].a[1]] = { t1,t2 };
            if(i!= S.machine_ass[m1].size()-1)
                S.MP[S.machine_ass[m1][i + 1].a[0]][S.machine_ass[m1][i + 1].a[1]] = { t1,t2 };*/
        }
        //cout << "333" << endl;
        if (m2 != m1) {
            S.ope_count_per_machine[m1] -= 1;
            S.ope_count_per_machine[m2] += 1;
            int t = 0;
            for (int i = 0; i < job[ai][aj].size();++i) {
                if (job[ai][aj][i].a[0] == m2) {
                    t = i;
                    break;
                }
            }
            S.where_machine[ai][aj] = t;
            for (int i = 0; i < S.machine_ass[m2].size(); ++i) {
                int t1 = S.machine_ass[m2][i].a[0], t2 = S.machine_ass[m2][i].a[1];
                S.ope_in_machine[t1][t2] = { m2,i };
                /*if (i != 0)
                    S.MS[S.machine_ass[m2][i - 1].a[0]][S.machine_ass[m2][i - 1].a[1]] = { t1,t2 };
                if (i != S.machine_ass[m2].size() - 1)
                    S.MP[S.machine_ass[m2][i + 1].a[0]][S.machine_ass[m2][i + 1].a[1]] = { t1,t2 };*/
            }
        }
    }
    else {
        //cout << "后插" << endl;
        int m1 = S.ope_in_machine[ai][aj].a[0];
        int m2 = -1;
        if (move[2] == -2) {
            m2 = move[3];
        }
        else {
            m2 = S.ope_in_machine[bi][bj].a[0];
        }
        S.machine_ass[m1].erase(S.machine_ass[m1].begin() + S.ope_in_machine[ai][aj].a[1]);
        if(m1==m2)
            S.machine_ass[m2].insert(S.machine_ass[m2].begin() + S.ope_in_machine[bi][bj].a[1], { ai,aj });
        else {
            if (move[2] == -2) {
                S.machine_ass[m2].insert(S.machine_ass[m2].begin(), { ai,aj });
            }
            else {
                S.machine_ass[m2].insert(S.machine_ass[m2].begin() + (S.ope_in_machine[bi][bj].a[1] + 1 >=
                    S.machine_ass[m2].size() ? S.machine_ass[m2].size() : S.ope_in_machine[bi][bj].a[1] + 1), { ai,aj });
            }
        }
        for (int i = 0; i < S.machine_ass[m1].size(); ++i) {
            int t1 = S.machine_ass[m1][i].a[0], t2 = S.machine_ass[m1][i].a[1];
            S.ope_in_machine[t1][t2] = { m1,i };
            /*if (i != 0)
                S.MS[S.machine_ass[m1][i - 1].a[0]][S.machine_ass[m1][i - 1].a[1]] = { t1,t2 };
            if (i != S.machine_ass[m1].size() - 1)
                S.MP[S.machine_ass[m1][i + 1].a[0]][S.machine_ass[m1][i + 1].a[1]] = { t1,t2 };*/
        }
        if (m2 != m1) {
            S.ope_count_per_machine[m1] -= 1;
            S.ope_count_per_machine[m2] += 1;
            int t = 0;
            for (int i = 0; i < job[ai][aj].size(); ++i) {
                if (job[ai][aj][i].a[0] == m2) {
                    t = i;
                    break;
                }
            }
            S.where_machine[ai][aj] = t;
            for (int i = 0; i < S.machine_ass[m2].size(); ++i) {
                int t1 = S.machine_ass[m2][i].a[0], t2 = S.machine_ass[m2][i].a[1];
                S.ope_in_machine[t1][t2] = { m2,i };
                /*if (i != 0)
                    S.MS[S.machine_ass[m2][i - 1].a[0]][S.machine_ass[m2][i - 1].a[1]] = { t1,t2 };
                if (i != S.machine_ass[m2].size() - 1)
                    S.MP[S.machine_ass[m2][i + 1].a[0]][S.machine_ass[m2][i + 1].a[1]] = { t1,t2 };*/
            }
        }
    }
    caculate_makespan_Ini(S);
}

void Nk_nei(const vector<ARR> &x,Solution& S,int i,int &best_makespan,vector<vector<int>>&best_move,
    vector<unordered_map<vector<ARR>, int, Hashfunc, Equalfunc>> &Tabu_table,vector<vector<ARR>> &best_pattern,
    vector<vector<ARR>>& best_pattern2,vector<int> &best_move_tabu,vector<ARR> &best_pattern_tabu,int& best_makespan_tabu) {
    int machine_num = S.ope_in_machine[x[i].a[0]][x[i].a[1]].a[0];
    int t1 = x[i].a[0];
    int t2 = x[i].a[1];
    vector<ARR> bf_changed;
    vector<ARR> af_changed;
    //vector<ARR> ori_ma_bf_changed(S.machine_ass[machine_num].begin() + S.ope_in_machine[t1][t2].a[1], S.machine_ass[machine_num].end());
	vector<ARR>ori_ma_bf_changed(S.machine_ass[machine_num]);
	vector<ARR> ori_ma_af_changed(ori_ma_bf_changed);
    ori_ma_af_changed.erase(ori_ma_af_changed.begin() + S.ope_in_machine[t1][t2].a[1]);
    if (ori_ma_af_changed.size() == 0) {
        if (S.machine_ass[machine_num].size() == 1)
            ori_ma_af_changed = { {-1,-1} };
        else {
            ori_ma_af_changed = { {-2,-2} };
        }
    }
	if (Tabu_table[machine_num].find(ori_ma_af_changed) != Tabu_table[machine_num].end())
		return;
    for (const auto& y : job[t1][t2]) {
        //cout << "cc:" << y.a[0] << "," << machine_num << endl;
        if (y.a[0] == machine_num)
            continue;
        //cout << "??" << endl;
        int cur_machine = y.a[0];
        if (S.machine_ass[cur_machine].size() == 0) {
            /*vector<ARR> */bf_changed.assign(1, { -1,-1 });
            /*vector<ARR> */af_changed.assign(1, { t1,t2 });
			//vector<ARR> tabu1({ {t1,t2},{-2,cur_machine} });
			//vector<ARR> tabu2({ {s1,s2},{t1,t2} });
            int temp_Ri = t2 == 0 ? 0 : S.R[t1][t2 - 1] + job[t1][t2 - 1][S.where_machine[t1][t2 - 1]].a[1];
            int temp_Qi = t2 == job[t1].size() - 1 ? 0 : S.Q[t1][t2 + 1] + job[t1][t2 + 1][S.where_machine[t1][t2 + 1]].a[1];
            int temp_makespan = temp_Ri + job[t1][t2][S.where_machine[t1][t2]].a[1] + temp_Qi;
            if (S.makespan - job[t1][t2][S.where_machine[t1][t2]].a[1] > temp_makespan)
                temp_makespan = S.makespan - job[t1][t2][S.where_machine[t1][t2]].a[1];
            if (temp_makespan < best_makespan_tabu) {
				best_makespan_tabu = temp_makespan;
                best_move_tabu = { t1,t2,-2,cur_machine,1 };
                best_pattern_tabu = ori_ma_bf_changed;
            }
            if (temp_makespan <= best_makespan && Tabu_table[cur_machine].find(af_changed) ==
                Tabu_table[cur_machine].end() && Tabu_table[machine_num].find(ori_ma_af_changed) ==
                    Tabu_table[machine_num].end()/*&&Tabu_table[machine_num].find(tabu1) ==
				Tabu_table[machine_num].end()*/) {
                if (temp_makespan == best_makespan) {
                    //best_makespan = temp_makespan;
                    best_move.push_back({ t1,t2,-2,cur_machine,1 });
                    best_pattern.push_back(ori_ma_bf_changed);
                    best_pattern2.push_back(bf_changed);
                }
                else {
                    best_makespan = temp_makespan;
                    best_move.clear();
                    best_pattern.clear();
                    best_pattern2.clear();
                    best_move.push_back({ t1,t2,-2,cur_machine,1 });
                    best_pattern.push_back(ori_ma_bf_changed);
                    best_pattern2.push_back(bf_changed);
                }
            }
            continue;
        }
		bf_changed = S.machine_ass[cur_machine];
        for (int j = 0; j < S.machine_ass[cur_machine].size() - 1; ++j) {
            int ai = S.machine_ass[cur_machine][j].a[0];
            int aj = S.machine_ass[cur_machine][j].a[1];
            int bi = S.machine_ass[cur_machine][j + 1].a[0];
            int bj = S.machine_ass[cur_machine][j + 1].a[1];
            //vector<ARR> x_temp = { {ai,aj},{t1,t2},{bi,bj} };
            ///*vector<ARR> */bf_changed.assign(S.machine_ass[cur_machine].begin() + (j + 1), S.machine_ass[cur_machine].end());
            /*vector<ARR>*/ af_changed = bf_changed;
            af_changed.insert(af_changed.begin()+(j+1), { t1,t2 });
            if ((t2 == job[t1].size() - 1 || ((ai!=t1||aj!=t2+1)&&S.Q[ai][aj] >= S.Q[t1][t2 + 1])) &&
                (t2 == 0 || ((bi!=t1||bj!=t2-1)&&S.Q[t1][t2 - 1] >= S.Q[bi][bj]))) {
				//vector<ARR> tabu1({ {t1,t2},{ai,aj} });
				//vector<ARR> tabu2({ {ai,aj},{t1,t2} });
                int temp_Ri = t2 == 0 ? S.R[ai][aj] + job[ai][aj][S.where_machine[ai][aj]].a[1] :
                    max(S.R[t1][t2 - 1] + job[t1][t2 - 1][S.where_machine[t1][t2 - 1]].a[1],
                        S.R[ai][aj] + job[ai][aj][S.where_machine[ai][aj]].a[1]);
                int temp_Qi = t2 == job[t1].size() - 1 ? S.Q[bi][bj] + job[bi][bj][S.where_machine[bi][bj]].a[1] :
                    max(S.Q[t1][t2 + 1] + job[t1][t2 + 1][S.where_machine[t1][t2 + 1]].a[1],
                        S.Q[bi][bj] + job[bi][bj][S.where_machine[bi][bj]].a[1]);//存在疑问
                int temp_makespan = temp_Ri + job[t1][t2][S.where_machine[t1][t2]].a[1] + temp_Qi;
                if (S.makespan - job[t1][t2][S.where_machine[t1][t2]].a[1] > temp_makespan)
                    temp_makespan = S.makespan - job[t1][t2][S.where_machine[t1][t2]].a[1];
                if (temp_makespan < best_makespan_tabu) {
					best_makespan_tabu = temp_makespan;
                    best_move_tabu = { t1,t2,ai,aj,1 };
                    best_pattern_tabu = ori_ma_bf_changed;
                }
                if (temp_makespan <= best_makespan && Tabu_table[cur_machine].find(af_changed) ==
                    Tabu_table[cur_machine].end()&& Tabu_table[machine_num].find(ori_ma_af_changed) ==
                    Tabu_table[machine_num].end()/*&& Tabu_table[machine_num].find(tabu1) ==
					Tabu_table[machine_num].end()&& Tabu_table[cur_machine].find(tabu2) ==
					Tabu_table[cur_machine].end()*/) {
                    if (temp_makespan == best_makespan) {
                        //best_makespan = temp_makespan;
                        best_move.push_back({ t1,t2,ai,aj,1 });
                        best_pattern.push_back(ori_ma_bf_changed);
                        best_pattern2.push_back(bf_changed);
                    }
                    else {
                        best_makespan = temp_makespan;
                        best_move.clear();
                        best_pattern.clear();
                        best_pattern2.clear();
                        best_move.push_back({ t1,t2,ai,aj,1 });
                        best_pattern.push_back(ori_ma_bf_changed);
                        best_pattern2.push_back(bf_changed);
                    }
                }
            }
        }
        int firsti = S.machine_ass[cur_machine][0].a[0];
        int firstj = S.machine_ass[cur_machine][0].a[1];
        int lasti = S.machine_ass[cur_machine][S.machine_ass[cur_machine].size() - 1].a[0];
        int lastj = S.machine_ass[cur_machine][S.machine_ass[cur_machine].size() - 1].a[1];
        int last = S.machine_ass[cur_machine].size() - 1;
        if (t2 == 0 || ((firsti!=t1||firstj!=t2-1)&&S.Q[t1][t2 - 1] >= S.Q[firsti][firstj])) {
			//vector<ARR> tabu1({ {t1,t2},{firsti,firstj} });
			//vector<ARR> tabu2({ {firsti,firstj},{t1,t2} });
            int temp_Ri = t2 == 0 ? 0 : S.R[t1][t2 - 1] + job[t1][t2 - 1][S.where_machine[t1][t2 - 1]].a[1];
            int temp_Qi = t2 == job[t1].size() - 1 ? S.Q[firsti][firstj] + job[firsti][firstj][S.where_machine[firsti][firstj]].a[1] :
                max(S.Q[t1][t2 + 1] + job[t1][t2 + 1][S.where_machine[t1][t2 + 1]].a[1],
                    S.Q[firsti][firstj] + job[firsti][firstj][S.where_machine[firsti][firstj]].a[1]);
            int temp_makespan = temp_Ri + job[t1][t2][S.where_machine[t1][t2]].a[1] + temp_Qi;
            //vector<ARR> x_temp = {{t1,t2},{firsti,firstj}};
            //bf_changed.assign(S.machine_ass[cur_machine].begin(), S.machine_ass[cur_machine].end());
            /*vector<ARR>*/ af_changed = bf_changed;
            af_changed.insert(af_changed.begin(), { t1,t2 });
            if (S.makespan - job[t1][t2][S.where_machine[t1][t2]].a[1] > temp_makespan)
                temp_makespan = S.makespan - job[t1][t2][S.where_machine[t1][t2]].a[1];
            if (temp_makespan < best_makespan_tabu) {
				best_makespan_tabu = temp_makespan;
                best_move_tabu = { t1,t2,firsti,firstj,0 };
                best_pattern_tabu = ori_ma_bf_changed;
            }
            if (temp_makespan <= best_makespan&&Tabu_table[cur_machine].find(af_changed)==Tabu_table[cur_machine].end()&&
                Tabu_table[machine_num].find(ori_ma_af_changed)==Tabu_table[machine_num].end()/*&&
				Tabu_table[machine_num].find(tabu1) == Tabu_table[machine_num].end()&&
				Tabu_table[cur_machine].find(tabu2) == Tabu_table[cur_machine].end()*/) {
                if (temp_makespan == best_makespan) {
                    //best_makespan = temp_makespan;
                    best_move.push_back({ t1,t2,firsti,firstj,0 });
                    best_pattern.push_back(ori_ma_bf_changed);
                    best_pattern2.push_back(bf_changed);
                }
                else {
                    best_makespan = temp_makespan;
                    best_move.clear();
                    best_pattern.clear();
                    best_pattern2.clear();
                    best_move.push_back({ t1,t2,firsti,firstj,0 });
                    best_pattern.push_back(ori_ma_bf_changed);
                    best_pattern2.push_back(bf_changed);
                }
            }
        }
        if (t2 == job[t1].size() - 1 || ((lasti!=t1||lastj!=t2+1)&&S.Q[lasti][lastj] >= S.Q[t1][t2 + 1])) {
			//vector<ARR> tabu1({ {t1,t2},{lasti,lastj} });
			//vector<ARR> tabu2({ {lasti,lastj},{t1,t2} });
            int temp_Ri = t2 == 0 ? S.R[lasti][lastj] + job[lasti][lastj][S.where_machine[lasti][lastj]].a[1] :
                max(S.R[t1][t2 - 1] + job[t1][t2 - 1][S.where_machine[t1][t2 - 1]].a[1],
                    S.R[lasti][lastj] + job[lasti][lastj][S.where_machine[lasti][lastj]].a[1]);
            int temp_Qi = t2 == job[t1].size() - 1 ? 0 : S.Q[t1][t2 + 1] + job[t1][t2 + 1][S.where_machine[t1][t2 + 1]].a[1];
            int temp_makespan = temp_Ri + job[t1][t2][S.where_machine[t1][t2]].a[1] + temp_Qi;
            //vector<ARR> x_temp = { {lasti,lastj},{t1,t2}};
            //bf_changed.assign(S.machine_ass[cur_machine].begin()+last, S.machine_ass[cur_machine].end());
            /*vector<ARR>*/ af_changed = bf_changed;
            af_changed.insert(af_changed.end(), { t1,t2 });
            if (S.makespan - job[t1][t2][S.where_machine[t1][t2]].a[1] > temp_makespan)
                temp_makespan = S.makespan - job[t1][t2][S.where_machine[t1][t2]].a[1];
            if (temp_makespan < best_makespan_tabu) {
				best_makespan_tabu = temp_makespan;
                best_move_tabu = { t1,t2,lasti,lastj,1 };
                best_pattern_tabu = ori_ma_bf_changed;
            }
            if (temp_makespan <= best_makespan&& Tabu_table[cur_machine].find(af_changed) == Tabu_table[cur_machine].end()&&
                Tabu_table[machine_num].find(ori_ma_af_changed) == Tabu_table[machine_num].end() /*&&
				Tabu_table[machine_num].find(tabu1) == Tabu_table[machine_num].end() &&
				Tabu_table[cur_machine].find(tabu2) == Tabu_table[cur_machine].end()*/) {
                if (temp_makespan == best_makespan) {
                    //best_makespan = temp_makespan;
                    best_move.push_back({ t1,t2,lasti,lastj,1 });
                    best_pattern.push_back(ori_ma_bf_changed);
                    best_pattern2.push_back(bf_changed);
                }
                else {
                    best_makespan = temp_makespan;
                    best_move.clear();
                    best_pattern.clear();
                    best_pattern2.clear();
                    best_move.push_back({ t1,t2,lasti,lastj,1 });
                    best_pattern.push_back(ori_ma_bf_changed);
                    best_pattern2.push_back(bf_changed);
                }
            }
        }
    }
}

void PR_make_move(Solution& S_min, vector<int>& move) {
    int i = move[0], j = move[1];
    int t1 = S_min.ope_in_machine[i][j].a[0];
    int t3 = S_min.ope_in_machine[i][j].a[1];
    int t2 = move[2];
    S_min.machine_ass[t1].erase(S_min.machine_ass[t1].begin() + t3);
    S_min.machine_ass[t2].insert(S_min.machine_ass[t2].begin() + move[3], {i,j});
    if (t1 == t2) {
        for (int k = 0; k < S_min.machine_ass[t1].size(); ++k) {
            auto x = S_min.machine_ass[t1][k];
            S_min.ope_in_machine[x.a[0]][x.a[1]] = { t1,k };
        }
    }
    else {
        int t = -1;
        for (int k = 0; k < job[i][j].size(); ++k) {
            if (job[i][j][k].a[0] == t2) { 
                t = k;
                break;
            }
        }
        S_min.where_machine[i][j] = t;
        for (int k = 0; k < S_min.machine_ass[t1].size();++k) {
            auto x = S_min.machine_ass[t1][k];
            S_min.ope_in_machine[x.a[0]][x.a[1]] = { t1,k };
        }
        for (int k = 0; k < S_min.machine_ass[t2].size(); ++k) {
            auto x = S_min.machine_ass[t2][k];
            S_min.ope_in_machine[x.a[0]][x.a[1]] = { t2,k };
        }
    }
}

void PR(Solution &Si, Solution &Sg,
    Solution &S,float alpha,float beta, float gama) {
    Solution S_cur = Si;
    vector<Solution> path_set;
    vector<Solution> N;
	N.reserve(ope_sum);
	path_set.reserve(100);
    vector<int> d_record;
    vector<int> obj_record;
    vector<int> obj_re_pathset;
	obj_re_pathset.reserve(100);
    int d_initial = caculate_d(Si, Sg);
    int cur_d = d_initial;
    //cout << "d_initial:" << d_initial << endl;
    caculate_makespan_Ini(S_cur);
    int iter = 0;
    Solution S_min;
    vector<ARR> temp_ma_ass;
    vector<vector<int>> best_move;
    best_move.reserve(ope_sum);
    while (cur_d > alpha * d_initial) {
        //cout << "cur_d:"<<cur_d << endl;
        if (iter > 100)
            break;
        int temp_d = cur_d;
        int start_size = N.size();
        for (int i = 0; i < job_count; ++i) {
            for (int j = 0; j < job[i].size(); ++j) {
                int min_d = INT_MAX;
                best_move.clear();
                temp_ma_ass = S_cur.machine_ass[S_cur.ope_in_machine[i][j].a[0]];
                temp_ma_ass.erase(temp_ma_ass.begin() + S_cur.ope_in_machine[i][j].a[1]);
                if (S_cur.ope_in_machine[i][j].a[0] != Sg.ope_in_machine[i][j].a[0]) {//改变机器
                    int t1 = S_cur.ope_in_machine[i][j].a[0];//待更新死锁检查
                    int t3 = S_cur.ope_in_machine[i][j].a[1];
                    int t2 = Sg.ope_in_machine[i][j].a[0];
                    //int af_delete_d = update_d(S_cur, Sg, t1, t2, cur_d, 1);
                    //Solution S_temp = S_cur;
                    //S_temp.machine_ass[t1].erase(S_temp.machine_ass[t1].begin() + t3);
                    //S_temp.ope_count_per_machine[t1] -= 1;
                    ///*if(t3!=0)
                    //    S_temp.MS[S_temp.machine_ass[t1][t3 - 1].a[0]][S_temp.machine_ass[t1][t3 - 1].a[1]] = { i,j };*/
                    //for (int h = 0; h < S_temp.machine_ass[t1].size(); ++h) {
                    //    auto x = S_temp.machine_ass[t1][h];
                    //    S_temp.ope_in_machine[x.a[0]][x.a[1]] = { t1,h };

                    //}
                    //Solution S_temp1 = S_temp;
                    //Solution S_min;
                    //int min_d = INT_MAX;
                    int size = S_cur.machine_ass[t2].size();
                    for (int k = 0; k <=size; ++k) {
                        int ai, aj;
                        int bi, bj;
                        bool flag = false;
                        if (k == 0) {
                            if (size > 0) {
                                ai = S_cur.machine_ass[t2][k].a[0];
                                aj = S_cur.machine_ass[t2][k].a[1];
                                flag = j == 0 || (S_cur.Q[i][j - 1] >= S_cur.Q[ai][aj]&&(!(ai==i&&aj==j-1)));
                            }
                            else {
                                flag = true;
                            }
                        }
                        else {
                            if (k == size) {
                                bi = S_cur.machine_ass[t2][k-1].a[0];
                                bj = S_cur.machine_ass[t2][k-1].a[1];
                                flag = j == job[i].size() - 1 || ((!(bi==i&&bj==j+1))&&S_cur.Q[bi][bj] >= S_cur.Q[i][j + 1]);
                            }
                            else {
                                ai = S_cur.machine_ass[t2][k-1].a[0];
                                aj = S_cur.machine_ass[t2][k-1].a[1];
                                bi = S_cur.machine_ass[t2][k].a[0];
                                bj = S_cur.machine_ass[t2][k].a[1];
                                flag = (j == 0 || (S_cur.Q[i][j - 1] >= S_cur.Q[bi][bj] && (!(bi == i && bj == j - 1)))) && 
                                    (j == job[i].size() - 1 || ((!(ai == i && aj == j + 1)) && S_cur.Q[ai][aj] >= S_cur.Q[i][j + 1]));
                            }
                        }
                        if (flag) {
                            /*S_temp1 = S_temp;
                            S_temp1.machine_ass[t2].insert(S_temp1.machine_ass[t2].begin() + k, { i,j });
                            for (int h = 0; h < S_temp1.machine_ass[t2].size(); ++h) {
                                auto x = S_temp1.machine_ass[t2][h];
                                S_temp1.ope_in_machine[x.a[0]][x.a[1]] = { t2,h };
                            }
                            S_temp1.ope_count_per_machine[t2] += 1;*/
                            int temp_d = abs(k - Sg.ope_in_machine[i][j].a[1]);
							if (temp_d == min_d) {
								best_move.push_back({ i,j,t2,k });
							}
							if(temp_d<min_d) {
								min_d = temp_d;
								best_move.clear();
								best_move.push_back({ i,j,t2,k });
							}
                                
                        }
                    }
                    
                }
                else {//改变位置
                    if (S_cur.ope_in_machine[i][j].a[1] != Sg.ope_in_machine[i][j].a[1]) {
                        int t1 = S_cur.ope_in_machine[i][j].a[0];
                        int t2 = S_cur.ope_in_machine[i][j].a[1];
                        //int af_delete_d = update_d(S_cur, Sg, t1, -1, cur_d, 1);
                        /*Solution S_temp = S_cur;
                        S_temp.machine_ass[t1].erase(S_temp.machine_ass[t1].begin() + t2);
                        Solution S_temp1 = S_temp;*/
                        //Solution S_min;
                        //int min_d = INT_MAX;
                        int size = temp_ma_ass.size();
                        for (int k = 0; k <=size; ++k) {
                            if (k == t2)
                                continue;
                            int ai, aj;
                            int bi, bj;
                            bool flag = false;
                            if (k == 0) {
                                if (size > 0) {
                                    ai = temp_ma_ass[k].a[0];
                                    aj = temp_ma_ass[k].a[1];
                                    flag = j == 0 || (S_cur.Q[i][j - 1] >= S_cur.Q[ai][aj] && (!(ai == i && aj == j - 1)));
                                }
                                else {
                                    flag = true;
                                }
                            }
                            else {
                                if (k == size) {
                                    bi = temp_ma_ass[k - 1].a[0];
                                    bj = temp_ma_ass[k - 1].a[1];
                                    flag = j == job[i].size() - 1 || ((!(bi == i && bj == j + 1)) && S_cur.Q[bi][bj] >= S_cur.Q[i][j + 1]);
                                }
                                else {
                                    ai = temp_ma_ass[k - 1].a[0];
                                    aj = temp_ma_ass[k - 1].a[1];
                                    bi = temp_ma_ass[k].a[0];
                                    bj = temp_ma_ass[k].a[1];
                                    flag = (j == 0 || (S_cur.Q[i][j - 1] >= S_cur.Q[bi][bj] && (!(bi == i && bj == j - 1)))) &&
                                        (j == job[i].size() - 1 || ((!(ai == i && aj == j + 1)) && S_cur.Q[ai][aj] >= S_cur.Q[i][j + 1]));
                                }
                            }
                            if (flag) {
                                //cout << "333" << endl;
                                //S_temp1 = S_temp;
                                //S_temp1.machine_ass[t1].insert(S_temp1.machine_ass[t1].begin() + k, { i,j });
                                ///*if (k != 0)
                                //    S_temp1.MS[S_temp1.machine_ass[t1][k - 1].a[0]][S_temp1.machine_ass[t1][k - 1].a[1]] = { i,j };*/
                                //for (int h = 0; h < S_temp1.machine_ass[t1].size(); ++h) {
                                //    auto x = S_temp1.machine_ass[t1][h];
                                //    S_temp1.ope_in_machine[x.a[0]][x.a[1]] = { t1, h };
                                //    /*if (h != 0)
                                //        S_temp1.MP[x.a[0]][x.a[1]] = { S_temp1.machine_ass[t1][h - 1].a[0], S_temp1.machine_ass[t1][h - 1].a[1] };
                                //    if (h != S_temp1.machine_ass[t1].size() - 1)
                                //        S_temp1.MS[x.a[0]][x.a[1]] = { S_temp1.machine_ass[t1][h + 1].a[0], S_temp1.machine_ass[t1][h + 1].a[1] };*/
                                //}
                                //int temp_d = update_d(S_temp1, Sg, t1, -1, af_delete_d, 0);
                                int temp_d = abs(k - Sg.ope_in_machine[i][j].a[1]);
								if (temp_d == min_d) {
									best_move.push_back({ i,j,t1,k });
								}
								if(temp_d<min_d){
									min_d = temp_d;
									best_move.clear();
									best_move.push_back({ i,j,t1,k });
								}

                            }
                        }
                    }
                }
                //cout << "min_d=" << min_d << endl;
                if (best_move.size() != 0) {
                    S_min.machine_ass = S_cur.machine_ass;
                    S_min.ope_in_machine = S_cur.ope_in_machine;
                    S_min.where_machine = S_cur.where_machine;
                    //S_min = S_cur;
                    uniform_int_distribution<> dis(0, best_move.size() - 1);
                    int choose = dis(gen);
                    PR_make_move(S_min, best_move[choose]);
                    N.push_back(S_min);
                }
            }
        }
        int N_size = N.size();
        for (auto it = N.begin(); it != N.end();) {
            int t = caculate_d((*it), Sg);
            if (t > cur_d) {
                it = N.erase(it);
            }
            else {
                d_record.push_back(t);
                caculate_makespan_Ini((*it));
                obj_record.push_back((*it).makespan);
                ++it;
            }
        }
        N_size = N.size();
        vector<vector<int>> index_Dis_Obj(N.size(),vector<int>(2,0));
        for (int i = 0; i < N_size; ++i) {//还可优化，比如说，怎样能记录一个有序的d_record,obj_record;
            for (int j = 0; j < N_size; ++j) {
                if (j != i) {
                    if (d_record[j] < d_record[i])
                        ++index_Dis_Obj[i][0];
                    if (obj_record[j] < obj_record[i])
                        ++index_Dis_Obj[i][1];
                }
            }
        }
        sort(index_Dis_Obj.begin(), index_Dis_Obj.end(), my_cmp);
        uniform_int_distribution<> dis(0, gama<(N_size-1)?gama:N_size-1);
        int k = dis(gen);
        S_cur = N[k];
        cur_d = d_record[k];
        if (cur_d == temp_d)
            ++iter;
        else {
            iter = 0;
        }
        if (d_record[k] < beta * d_initial) {
            path_set.push_back(S_cur);
            obj_re_pathset.push_back(obj_record[k]);
        }
        N.clear();
        obj_record.clear();
        d_record.clear();
    }
    int min_i = 0;
    int min_obj = INT_MAX;
    for (int i = 0; i < path_set.size(); ++i) {
        if (obj_re_pathset[i] < min_obj) {
            min_i = i;
            min_obj = obj_re_pathset[i];
        }
    }
    if (path_set.size() == 0) {
        S = S_cur;
    }
    else {
        S = path_set[min_i];
    }
    return ;
}

void TS(Solution &S, Solution &cur) {
    vector<unordered_map<vector<ARR>,int,Hashfunc,Equalfunc>> Tabu_table(machine_count);
    int obj_best=S.makespan;
    int iter = 0;
    vector<int> R_temp;
    vector<int> Q_temp;
    vector<ARR> temp;
    vector<ARR> ori_ma_bf_changed;
    vector<ARR> ori_ma_af_changed;
    vector<vector<int>> best_move;//得记录多个相等makespan的动作
    best_move.reserve(ope_sum);
    vector<vector<ARR>> best_pattern;
    best_pattern.reserve(ope_sum);
    vector<vector<ARR>> best_pattern2;
    best_pattern2.reserve(ope_sum);
    vector<int> best_move_tabu(5, -1);
    vector<ARR> best_pattern_tabu;
	int global_iter = 0;
    while (iter < 10000) {
		++global_iter;
        int best_makespan = INT_MAX;
		int best_makespan_tabu = INT_MAX;
		/*cout.clear(ios::goodbit);
		cout << "iter=" << iter << endl;
		cout << "best_obj=" << obj_best << endl;
		cout.setstate(ios::failbit);*/
        best_move.clear();
        best_pattern.clear();
        best_pattern2.clear();
        best_move_tabu.assign(5, -1);
        best_pattern_tabu.clear();
        for (const auto& x : S.cri_block) {
            if (x.size() > 1) {
                //cout << "size=" << x.size() << endl;
                for (int i = 0; i < x.size(); ++i) {
                    int machine_num = S.ope_in_machine[x[i].a[0]][x[i].a[1]].a[0];
					ori_ma_bf_changed = S.machine_ass[machine_num];
                    if (i == 0) {
                        //cout << "case1" << endl;
                        int t1 = x[i].a[0];
                        int t2 = x[i].a[1];
                        for (int j = i + 1; j < x.size(); ++j) {
                            if (t2 == job[t1].size() - 1 || ((x[j].a[0]!=t1||x[j].a[1]!=t2+1)&&S.Q[x[j].a[0]][x[j].a[1]] >= S.Q[t1][t2 + 1])) {
                                int cur_makespan = 0;
								//vector<ARR> tabu1({ {t1,t2},{x[j].a[0],x[j].a[1]} });
								//vector<ARR> tabu2({{x[j].a[0],x[j].a[1]},{t1,t2} });
                                /*vector<int> */R_temp.assign(j - i + 1, -1);
                                /*vector<int> */Q_temp.assign(j - i + 1, -1);
                                /*vector<ARR> */temp.assign(x.begin() + (i + 1), x.begin()+(j+1));
                                temp.push_back(x[i]);
                                /*vector<ARR> *///ori_ma_bf_changed.assign(S.machine_ass[machine_num].begin() + S.ope_in_machine[t1][t2].a[1], S.machine_ass[machine_num].end());
                                //vector<ARR> ori_ma_af_changed = ori_ma_bf_changed;
                                ori_ma_af_changed = ori_ma_bf_changed;
                                ori_ma_af_changed.erase(ori_ma_af_changed.begin() + S.ope_in_machine[t1][t2].a[1]);
                                ori_ma_af_changed.insert(ori_ma_af_changed.begin() + (S.ope_in_machine[t1][t2].a[1]+j - i), {t1,t2});
                                int t3 = x[i + 1].a[0];
                                int t4 = x[i + 1].a[1];
                                if (S.ope_in_machine[x[i].a[0]][x[i].a[1]].a[1] == 0) {//R[L1]
                                    R_temp[0] = t4 == 0 ? 0 : S.R[t3][t4 - 1] +
                                        job[t3][t4 - 1][S.where_machine[t3][t4 - 1]].a[1];
                                }
                                else {
                                    int s2 = S.ope_in_machine[t1][t2].a[1];
                                    int t5 = S.machine_ass[machine_num][s2 - 1].a[0];
                                    int t6 = S.machine_ass[machine_num][s2 - 1].a[1];
                                    R_temp[0] = t4 == 0 ? S.R[t5][t6] + job[t5][t6][S.where_machine[t5][t6]].a[1] :
                                        max(S.R[t3][t4 - 1] + job[t3][t4 - 1][S.where_machine[t3][t4 - 1]].a[1],
                                            S.R[t5][t6] + job[t5][t6][S.where_machine[t5][t6]].a[1]);
                                }
                                for (int k = i + 2; k < j; ++k) {//R L2到v
                                    t3 = x[k].a[0];
                                    t4 = x[k].a[1];
                                    int t5 = x[k - 1].a[0], t6 = x[k - 1].a[1];
                                    R_temp[k - 1-i] = t4 == 0 ? R_temp[k - 2 - i]+job[t5][t6][S.where_machine[t5][t6]].a[1] : 
                                        max(S.R[t3][t4 - 1]+job[t3][t4-1][S.where_machine[t3][t4-1]].a[1],
                                            R_temp[k - 2 - i] + job[t5][t6][S.where_machine[t5][t6]].a[1]);
                                }
                                R_temp[j - i] = t2 == 0 ? R_temp[j - i - 1]+job[x[j].a[0]][x[j].a[1]][S.where_machine[x[j].a[0]][x[j].a[1]]].a[1] : 
                                    max(S.R[t1][t2 - 1]+ job[t1][t2 - 1][S.where_machine[t1][t2 - 1]].a[1]
                                        , R_temp[j - i - 1] + job[x[j].a[0]][x[j].a[1]][S.where_machine[x[j].a[0]][x[j].a[1]]].a[1]);//R[u]
                                t3 = x[j].a[0];
                                t4 = x[j].a[1];
                                if (S.ope_in_machine[t3][t4].a[1] == S.machine_ass[machine_num].size() - 1) {//Q[u]
                                    Q_temp[j - i] = t2 == job[t1].size() - 1 ? 0 : 
                                        S.Q[t1][t2 + 1]+job[t1][t2+1][S.where_machine[t1][t2+ 1]].a[1];
                                }
                                else {
                                    int s2 = S.ope_in_machine[t3][t4].a[1];
                                    int t5 = S.machine_ass[machine_num][s2+1].a[0];
                                    int t6 = S.machine_ass[machine_num][s2 + 1].a[1];
                                    Q_temp[j - i] = t2 == job[t1].size() - 1 ? S.Q[t5][t6]+job[t5][t6][S.where_machine[t5][t6]].a[1] : 
                                        max(S.Q[t1][t2 + 1] + job[t1][t2+1][S.where_machine[t1][t2 + 1]].a[1],
                                            S.Q[t5][t6] + job[t5][t6][S.where_machine[t5][t6]].a[1]);
                                }
                                //Q[v]
                                Q_temp[j - i - 1] = t4 == job[t3].size() - 1 ? Q_temp[j - i]+job[t1][t2][S.where_machine[t1][t2]].a[1] :
                                    max(S.Q[t3][t4 + 1]+job[t3][t4+1][S.where_machine[t3][t4+1]].a[1],
                                        Q_temp[j - i] + job[t1][t2][S.where_machine[t1][t2]].a[1]);
                                for (int k = j - 1; k > i; --k) {//Q[L1至Lk]
                                    int t7 = x[k].a[0];
                                    int t8 = x[k].a[1];
                                    int t9 = x[k + 1].a[0], t10 = x[k + 1].a[1];
                                    Q_temp[k - i - 1] = t8 == job[t7].size() - 1 ? Q_temp[k - i]+job[t9][t10][S.where_machine[t9][t10]].a[1] : 
                                        max(S.Q[t7][t8 + 1]+job[t7][t8+1][S.where_machine[t7][t8+1]].a[1],
                                            Q_temp[k - i] + job[t9][t10][S.where_machine[t9][t10]].a[1]);
                                }
                                for (int k = 0; k < j - i + 1; ++k) {
                                    int s1 = R_temp[k] + job[temp[k].a[0]][temp[k].a[1]][S.where_machine[temp[k].a[0]][temp[k].a[1]]].a[1] + Q_temp[k];
                                    cur_makespan = s1 > cur_makespan ? s1 : cur_makespan;
                                }
                                //cout << "cur_makespan=" << cur_makespan << endl;
                                if (cur_makespan < best_makespan_tabu) {
									best_makespan_tabu = cur_makespan;
                                    best_move_tabu = { { t1,t2,x[j].a[0],x[j].a[1],1 } };
									best_pattern_tabu = ori_ma_bf_changed;
                                }
                                if (cur_makespan <= best_makespan&&Tabu_table[machine_num].find(ori_ma_af_changed)== 
                                    Tabu_table[machine_num].end()/*&&Tabu_table[machine_num].find(tabu1)== Tabu_table[machine_num].end()&&
									Tabu_table[machine_num].find(tabu2) == Tabu_table[machine_num].end()*/) {
                                    if (cur_makespan == best_makespan) {
                                        best_move.push_back({ t1,t2,x[j].a[0],x[j].a[1],1 });
                                        best_pattern.push_back(ori_ma_bf_changed);
                                        best_pattern2.push_back({});
                                    }
                                    else {
                                        best_makespan = cur_makespan;
                                        best_move.clear();
                                        best_pattern.clear();
                                        best_pattern2.clear();
                                        best_move.push_back({ t1,t2,x[j].a[0],x[j].a[1],1 });
                                        best_pattern.push_back(ori_ma_bf_changed);
                                        best_pattern2.push_back({});
                                    }
                                }
                            }
                        }
                        Nk_nei(x, S, i, best_makespan, best_move, Tabu_table, best_pattern,best_pattern2,best_move_tabu,best_pattern_tabu,best_makespan_tabu);
                        continue;
                    }
                    if (i == x.size() - 1) {
                        //cout << "case2" << endl;
                        int t1 = x[i].a[0];
                        int t2 = x[i].a[1];          
                        for (int j = i - 1; j > 0; --j) {
                            temp.clear();
                            temp.reserve(i);
                            temp.push_back(x[i]);
                            for (int k = j; k < i; ++k) {
                                temp.push_back(x[k]);
                            }
                            int t3 = x[j].a[0];
                            int t4 = x[j].a[1];
                            if (t2 == 0 || ((t3!=t1||t4!=t2-1)&&S.Q[t1][t2 - 1] >= S.Q[t3][t4])) {
								//vector<ARR> tabu1({ {t1,t2},{x[j].a[0],x[j].a[1]} });
								//vector<ARR> tabu2({ {x[j].a[0],x[j].a[1]},{t1,t2} });
                                /*vector<int> */R_temp.assign(i - j + 1, -1);
                                Q_temp.assign(i - j + 1, -1);
                                //ori_ma_bf_changed.assign(S.machine_ass[machine_num].begin() + S.ope_in_machine[t3][t4].a[1], S.machine_ass[machine_num].end());
								//ori_ma_bf_changed = x;
								ori_ma_af_changed = ori_ma_bf_changed;
                                ori_ma_af_changed.erase(ori_ma_af_changed.begin()+ S.ope_in_machine[t1][t2].a[1]);
                                ori_ma_af_changed.insert(ori_ma_af_changed.begin()+ S.ope_in_machine[t3][t4].a[1], { t1,t2 });
                                if (t2 == 0 && S.ope_in_machine[t3][t4].a[1] == 0) {
                                    R_temp[0] = 0;
                                }
                                else {
                                    if (t2 == 0) {
                                        int s2 = S.ope_in_machine[t3][t4].a[1];
                                        int t5 = S.machine_ass[machine_num][s2- 1].a[0];
                                        int t6 = S.machine_ass[machine_num][s2 - 1].a[1];
                                        R_temp[0] = S.R[t5][t6] + job[t5][t6][S.where_machine[t5][t6]].a[1];
                                    }
                                    else {
                                        if (S.ope_in_machine[t3][t4].a[1] == 0) {
                                            R_temp[0] = S.R[t1][t2 - 1] + job[t1][t2 - 1][S.where_machine[t1][t2 - 1]].a[1];
                                        }
                                        else {
                                            int s2 = S.ope_in_machine[t3][t4].a[1];
                                            int t5 = S.machine_ass[machine_num][s2 - 1].a[0];
                                            int t6 = S.machine_ass[machine_num][s2 - 1].a[1];
                                            R_temp[0] =max(S.R[t5][t6] + job[t5][t6][S.where_machine[t5][t6]].a[1],
                                                S.R[t1][t2 - 1] + job[t1][t2 - 1][S.where_machine[t1][t2 - 1]].a[1]);
                                        }
                                    }
                                }
                                R_temp[1] = t4 == 0 ? R_temp[0] + job[t1][t2][S.where_machine[t1][t2]].a[1] :
                                    max(S.Q[t3][t4 - 1] + job[t3][t4 - 1][S.where_machine[t3][t4 - 1]].a[1],
                                        R_temp[0] + job[t1][t2][S.where_machine[t1][t2]].a[1]);
                                for (int k = j + 1; k < i; ++k) {
                                    int t5 = x[k].a[0];
                                    int t6 = x[k].a[1];
                                    int t7 = x[k - 1].a[0];
                                    int t8 = x[k - 1].a[1];
                                    R_temp[k-j+1] = t6 == 0 ? R_temp[k-j] + job[t7][t8][S.where_machine[t7][t8]].a[1] :
                                        max(S.R[t5][t6 - 1] + job[t5][t6-1][S.where_machine[t5][t6 - 1]].a[1],
                                            R_temp[k-j] + job[t7][t8][S.where_machine[t7][t8]].a[1]);
                                }
                                t3 = x[i - 1].a[0];
                                t4 = x[i - 1].a[1];
                                if (t4 == job[t3].size() - 1 && S.ope_in_machine[t1][t2].a[1] == S.ope_count_per_machine[machine_num] - 1) {
                                    Q_temp[i - j] = 0;
                                }
                                else {
                                    if (t4 == job[t3].size() - 1) {
                                        int s2 = S.ope_in_machine[t1][t2].a[1];
                                        int t5 = S.machine_ass[machine_num][s2+1].a[0];
                                        int t6 = S.machine_ass[machine_num][s2 + 1].a[1];
                                        Q_temp[i - j] = S.Q[t5][t6] + job[t5][t6][S.where_machine[t5][t6]].a[1];
                                    }
                                    else {
                                        if(S.ope_in_machine[t1][t2].a[1] == S.ope_count_per_machine[machine_num] - 1)
                                            Q_temp[i - j] = S.Q[t3][t4 + 1] + job[t3][t4 + 1][S.where_machine[t3][t4+1]].a[1];
                                        else {
                                            int s2 = S.ope_in_machine[t1][t2].a[1];
                                            int t5 = S.machine_ass[machine_num][s2+1].a[0];
                                            int t6 = S.machine_ass[machine_num][s2 + 1].a[1];
                                            Q_temp[i - j] = max(S.Q[t5][t6] + job[t5][t6][S.where_machine[t5][t6]].a[1],
                                                Q_temp[i - j] = S.Q[t3][t4 + 1] + job[t3][t4 + 1][S.where_machine[t3][t4 + 1]].a[1]);
                                        }
                                    }
                                }
                                for (int k = i - 2; k >=j; --k) {
                                    int t5 = x[k].a[0], t6 = x[k].a[1];
                                    int t7 = x[k + 1].a[0], t8 = x[k + 1].a[1];
                                    Q_temp[k - j + 1] = t6 == job[t5].size() - 1 ? Q_temp[k - j + 2] + job[t7][t8][S.where_machine[t7][t8]].a[1] :
                                        max(S.Q[t5][t6 + 1] + job[t5][t6 + 1][S.where_machine[t5][t6 + 1]].a[1],
                                            Q_temp[k - j + 2] + job[t7][t8][S.where_machine[t7][t8]].a[1]);
                                }
                                Q_temp[0] = t2 == job[t1].size() - 1 ? Q_temp[1] + job[x[j].a[0]][x[j].a[1]][S.where_machine[x[j].a[0]][x[j].a[1]]].a[1] :
                                    max(S.Q[t1][t2 + 1] + job[t1][t2+1][S.where_machine[t1][t2 + 1]].a[1],
                                        Q_temp[1] + job[x[j].a[0]][x[j].a[1]][S.where_machine[x[j].a[0]][x[j].a[1]]].a[1]);
                                int temp_makespan = 0;
                                for (int k = 0; k < i - j + 1; ++k) {
                                    int s = R_temp[k] + job[temp[k].a[0]][temp[k].a[1]][S.where_machine[temp[k].a[0]][temp[k].a[1]]].a[1] + Q_temp[k];
                                    temp_makespan = s > temp_makespan ? s : temp_makespan;
                                }
                                if (temp_makespan < best_makespan_tabu) {
									best_makespan_tabu = temp_makespan;
                                    best_move_tabu = { { t1,t2,x[j].a[0],x[j].a[1],0 } };
									best_pattern_tabu = ori_ma_bf_changed;
                                }
                                if (temp_makespan < best_makespan && Tabu_table[machine_num].find(ori_ma_af_changed) ==
                                    Tabu_table[machine_num].end()/*&& Tabu_table[machine_num].find(tabu1) == Tabu_table[machine_num].end() &&
									Tabu_table[machine_num].find(tabu2) == Tabu_table[machine_num].end()*/) {
                                    if (temp_makespan == best_makespan) {
                                        best_move.push_back({ t1,t2,x[j].a[0],x[j].a[1],0 });
                                        best_pattern.push_back(ori_ma_bf_changed);
                                        best_pattern2.push_back({});
                                    }
                                    else {
                                        best_makespan = temp_makespan;
                                        best_move.clear();
                                        best_pattern.clear();
                                        best_pattern2.clear();
                                        best_move.push_back({ t1,t2,x[j].a[0],x[j].a[1],0 });
                                        best_pattern.push_back(ori_ma_bf_changed);
                                        best_pattern2.push_back({});
                                    }
                                }
                            }
                        }
                        Nk_nei(x, S, i, best_makespan, best_move, Tabu_table, best_pattern,best_pattern2,best_move_tabu,best_pattern_tabu,best_makespan_tabu);
                        continue;
                    }
                    if (i != 0 && i != x.size() - 1) {
                        //cout << "case3" << endl;
                        //cout << "i=" << i << endl;
                        int t1 = x[i].a[0];
                        int t2 = x[i].a[1];
                        int last = x.size() - 1;
                        int s3 = x[x.size() - 1].a[0];
                        int s4 = x[x.size() - 1].a[1];
                        int s1 = x[0].a[0];
                        int s2 = x[0].a[1];
                        if (t2 == job[t1].size() - 1 || ((s3!=t1||s4!=t2+1)&&S.Q[s3][s4] >= S.Q[t1][t2 + 1])) {
                            if (i + 1 != x.size() - 1) {
								//vector<ARR> tabu1({ {t1,t2},{s3,s4} });
								//vector<ARR> tabu2({ {s3,s4},{t1,t2} });
                                int cur_makespan = 0;
                                temp.assign(x.begin() + (i + 1), x.end());
                                temp.push_back(x[i]);
                                R_temp.assign(last - i + 1, -1);
                                Q_temp.assign(last - i + 1, -1);
                                //ori_ma_bf_changed.assign(S.machine_ass[machine_num].begin() + S.ope_in_machine[t1][t2].a[1], S.machine_ass[machine_num].end());
								//ori_ma_bf_changed = x;
								ori_ma_af_changed = ori_ma_bf_changed;
                                ori_ma_af_changed.erase(ori_ma_af_changed.begin()+ S.ope_in_machine[t1][t2].a[1]);
                                ori_ma_af_changed.insert(ori_ma_af_changed.begin() + (S.ope_in_machine[t1][t2].a[1]+last - i), { t1,t2 });
                                int t3 = x[i + 1].a[0];
                                int t4 = x[i + 1].a[1];
                                if (S.ope_in_machine[x[i].a[0]][x[i].a[1]].a[1] == 0) {//R[L1]
                                    R_temp[0] = t4 == 0 ? 0 : S.R[t3][t4 - 1] +
                                        job[t3][t4 - 1][S.where_machine[t3][t4 - 1]].a[1];
                                }
                                else {
                                    int s8 = S.ope_in_machine[x[i].a[0]][x[i].a[1]].a[1];
                                    int t5 = S.machine_ass[machine_num][s8-1].a[0];
                                    int t6 = S.machine_ass[machine_num][s8-1].a[1];
                                    R_temp[0] = t4 == 0 ? S.R[t5][t6] + job[t5][t6][S.where_machine[t5][t6]].a[1] :
                                        max(S.R[t3][t4 - 1] + job[t3][t4 - 1][S.where_machine[t3][t4 - 1]].a[1],
                                            S.R[t5][t6] + job[t5][t6][S.where_machine[t5][t6]].a[1]);
                                }
                                for (int k = i + 2; k < last; ++k) {//R L2到v
                                    t3 = x[k].a[0];
                                    t4 = x[k].a[1];
                                    int t5 = x[k - 1].a[0], t6 = x[k - 1].a[1];
                                    R_temp[k - 1 - i] = t4 == 0 ? R_temp[k - 2 - i]+ job[t5][t6][S.where_machine[t5][t6]].a[1] :
                                        max(S.R[t3][t4 - 1]+ job[t3][t4 - 1][S.where_machine[t3][t4 - 1]].a[1],
                                            R_temp[k - 2 - i] + job[t5][t6][S.where_machine[t5][t6]].a[1]);
                                }
                                R_temp[last - i] = t2 == 0 ? R_temp[last - i - 1]+job[s3][s4][S.where_machine[s3][s4]].a[1] : 
                                    max(S.R[t1][t2 - 1]+job[t1][t2-1][S.where_machine[t1][t2-1]].a[1],
                                        R_temp[last - i - 1] + job[s3][s4][S.where_machine[s3][s4]].a[1]);//R[u]
                                t3 = x[last].a[0];
                                t4 = x[last].a[1];
                                if (S.ope_in_machine[s3][s4].a[1] == S.ope_count_per_machine[machine_num] - 1) {//Q[u]
                                    Q_temp[last - i] = t2 == job[t1].size() - 1 ? 0 : 
                                        S.Q[t1][t2 + 1]+job[t1][t2+1][S.where_machine[t1][t2+1]].a[1];
                                }
                                else {
                                    int s8 = S.ope_in_machine[s3][s4].a[1];
                                    int t5 = S.machine_ass[machine_num][s8+1].a[0];
                                    int t6 = S.machine_ass[machine_num][s8 + 1].a[1];
                                    Q_temp[last - i] = t2 == job[t1].size() - 1 ? S.Q[t5][t6]+job[t5][t6][S.where_machine[t5][t6]].a[1]
                                        : max(S.Q[t1][t2 + 1] + job[t1][t2 + 1][S.where_machine[t1][t2 + 1]].a[1],
                                            S.Q[t5][t6] + job[t5][t6][S.where_machine[t5][t6]].a[1]);
                                }
                                //Q[v]
                                Q_temp[last - i - 1] = t4 == job[t3].size() - 1 ? Q_temp[last - i]+job[t1][t2][S.where_machine[t1][t2]].a[1] :
                                    max(S.Q[t3][t4 + 1]+job[t3][t4+1][S.where_machine[t3][t4+1]].a[1],
                                        Q_temp[last - i] + job[t1][t2][S.where_machine[t1][t2]].a[1]);
                                for (int k = last - 1; k > i; --k) {//Q[L1至Lk]
                                    int t7 = x[k].a[0];
                                    int t8 = x[k].a[1];
                                    int t9 = x[k + 1].a[0], t10 = x[k + 1].a[1];
                                    Q_temp[k - i - 1] = t8 == job[t7].size() - 1 ? Q_temp[k - i]+job[t9][t10][S.where_machine[t9][t10]].a[1] :
                                        max(S.Q[t7][t8 + 1]+job[t7][t8+1][S.where_machine[t7][t8+1]].a[1],
                                            Q_temp[k - i] + job[t9][t10][S.where_machine[t9][t10]].a[1]);
                                }
                                for (int k = 0; k < last - i + 1; ++k) {
                                    int s1 = R_temp[k] + job[temp[k].a[0]][temp[k].a[1]][S.where_machine[temp[k].a[0]][temp[k].a[1]]].a[1] + Q_temp[k];
                                    cur_makespan = s1 > cur_makespan ? s1 : cur_makespan;
                                }
                                if (cur_makespan < best_makespan_tabu) {
									best_makespan_tabu = cur_makespan;
                                    best_move_tabu = { { t1,t2,x[last].a[0],x[last].a[1],1 } };
									best_pattern_tabu = ori_ma_bf_changed;
                                }
                                if (cur_makespan < best_makespan&&Tabu_table[machine_num].find(ori_ma_af_changed)==
                                    Tabu_table[machine_num].end()/*&& Tabu_table[machine_num].find(tabu1) == Tabu_table[machine_num].end() &&
									Tabu_table[machine_num].find(tabu2) == Tabu_table[machine_num].end()*/) {
                                    if (cur_makespan == best_makespan) {
                                        best_move.push_back({ t1,t2,x[last].a[0],x[last].a[1],1 });
                                        best_pattern.push_back(ori_ma_bf_changed);
                                        best_pattern2.push_back({});
                                    }
                                    else {
                                        best_makespan = cur_makespan;
                                        best_move.clear();
                                        best_pattern.clear();
                                        best_pattern2.clear();
                                        best_move.push_back({ t1,t2,x[last].a[0],x[last].a[1],1 });
                                        best_pattern.push_back(ori_ma_bf_changed);
                                        best_pattern2.push_back({});
                                    }
                                }
                            }
                        }
                        if (t2 == 0 || ((s1!=t1||s2!=t2-1)&&S.Q[t1][t2 - 1] >= S.Q[s1][s2])) {
                            if (i - 1 != 0) {
                                temp.clear();
                                temp.reserve(i);
                                temp.push_back(x[i]);
                                for (int k = 0; k < i; ++k) {
                                    temp.push_back(x[k]);
                                }
                                int t3 = x[0].a[0];
                                int t4 = x[0].a[1];
                                if (t2 == 0 || S.Q[t1][t2 - 1] >= S.Q[t3][t4]) {
									//vector<ARR> tabu1({ {t1,t2},{s1,s2} });
									//vector<ARR> tabu2({ {s1,s2},{t1,t2} });
                                    R_temp.assign(i - 0 + 1, 0);
                                    Q_temp.assign(i - 0 + 1, 0);
                                    //ori_ma_bf_changed.assign(S.machine_ass[machine_num].begin() + S.ope_in_machine[t3][t4].a[1], S.machine_ass[machine_num].end());
									//ori_ma_bf_changed = x;
									ori_ma_af_changed = ori_ma_bf_changed;
                                    ori_ma_af_changed.erase(ori_ma_af_changed.begin() + S.ope_in_machine[t1][t2].a[1]);
                                    ori_ma_af_changed.insert(ori_ma_af_changed.begin()+ S.ope_in_machine[s1][s2].a[1], { t1,t2 });
                                    //cout << "R_temp.size=" << R_temp.size() << endl;
                                    if (t2 == 0 && S.ope_in_machine[t3][t4].a[1] == 0) {
                                        R_temp[0] = 0;
                                    }
                                    else {
                                        if (t2 == 0) {
                                            int s8 = S.ope_in_machine[t3][t4].a[1];
                                            int t5 = S.machine_ass[machine_num][s8-1].a[0];
                                            int t6 = S.machine_ass[machine_num][s8 - 1].a[1];
                                            R_temp[0] = S.R[t5][t6] + job[t5][t6][S.where_machine[t5][t6]].a[1];
                                        }
                                        else {
                                            if (S.ope_in_machine[t3][t4].a[1] == 0) {
                                                R_temp[0] = S.R[t1][t2 - 1] + job[t1][t2 - 1][S.where_machine[t1][t2 - 1]].a[1];
                                            }
                                            else {
                                                int s8 = S.ope_in_machine[t3][t4].a[1];
                                                int t5 = S.machine_ass[machine_num][s8-1].a[0];
                                                int t6 = S.machine_ass[machine_num][s8 - 1].a[1];
                                                R_temp[0] = max(S.R[t5][t6] + job[t5][t6][S.where_machine[t5][t6]].a[1],
                                                    S.R[t1][t2 - 1] + job[t1][t2 - 1][S.where_machine[t1][t2 - 1]].a[1]);
                                            }
                                        }
                                    }
                                    R_temp[1] = t4 == 0 ? R_temp[0] + job[t1][t2][S.where_machine[t1][t2]].a[1] :
                                        max(S.Q[t3][t4 - 1] + job[t3][t4 - 1][S.where_machine[t3][t4 - 1]].a[1],
                                            R_temp[0] + job[t1][t2][S.where_machine[t1][t2]].a[1]);
                                    for (int k = 0 + 1; k < i; ++k) {
                                        int t5 = x[k].a[0];
                                        int t6 = x[k].a[1];
                                        int t7 = x[k - 1].a[0];
                                        int t8 = x[k - 1].a[1];
                                        R_temp[k + 1] = t6 == 0 ? R_temp[k] + job[t7][t8][S.where_machine[t7][t8]].a[1] :
                                            max(S.R[t5][t6 - 1] + job[t5][t6-1][S.where_machine[t5][t6 - 1]].a[1],
                                                R_temp[k] + job[t7][t8][S.where_machine[t7][t8]].a[1]);
                                    }
                                    t3 = x[i - 1].a[0];
                                    t4 = x[i - 1].a[1];
                                    if (t4 == job[t3].size() - 1 && S.ope_in_machine[t1][t2].a[1] == S.ope_count_per_machine[machine_num] - 1) {
                                        Q_temp[i - 0] = 0;
                                    }
                                    else {
                                        if (t4 == job[t3].size() - 1) {
                                            int s8 = S.ope_in_machine[t1][t2].a[1];
                                            int t5 = S.machine_ass[machine_num][s8+1].a[0];
                                            int t6 = S.machine_ass[machine_num][s8 + 1].a[1];
                                            Q_temp[i - 0] = S.Q[t5][t6] + job[t5][t6][S.where_machine[t5][t6]].a[1];
                                        }
                                        else {
                                            if(S.ope_in_machine[t1][t2].a[1] == S.ope_count_per_machine[machine_num] - 1)
                                                Q_temp[i - 0] = S.Q[t3][t4 + 1] + job[t3][t4 + 1][S.where_machine[t3][t4]].a[1];
                                            else {
                                                int s8 = S.ope_in_machine[t1][t2].a[1];
                                                int t5 = S.machine_ass[machine_num][s8 + 1].a[0];
                                                int t6 = S.machine_ass[machine_num][s8 + 1].a[1];
                                                Q_temp[i - 0] = max(S.Q[t5][t6] + job[t5][t6][S.where_machine[t5][t6]].a[1],
                                                    S.Q[t3][t4 + 1] + job[t3][t4 + 1][S.where_machine[t3][t4+1]].a[1]);
                                            }
                                        }
                                    }
                                    for (int k = i - 2; k > 0; --k) {
                                        int t5 = x[k].a[0], t6 = x[k].a[1];
                                        int t7 = x[k + 1].a[0], t8 = x[k + 1].a[1];
                                        Q_temp[k - 0 + 1] = t6 == job[t5].size() - 1 ? Q_temp[k - 0 + 2] + job[t7][t8][S.where_machine[t7][t8]].a[1] :
                                            max(S.Q[t5][t6 + 1] + job[t5][t6 + 1][S.where_machine[t5][t6 + 1]].a[1],
                                                Q_temp[k - 0 + 2] + job[t7][t8][S.where_machine[t7][t8]].a[1]);
                                    }
                                    Q_temp[0] = t2 == job[t1].size() - 1 ? Q_temp[1] + job[x[0].a[0]][x[0].a[1]][S.where_machine[x[0].a[0]][x[0].a[1]]].a[1]:
                                        max(S.Q[t1][t2 + 1] + job[t1][t2+1][S.where_machine[t1][t2 + 1]].a[1],
                                            Q_temp[1] + job[x[0].a[0]][x[0].a[1]][S.where_machine[x[0].a[0]][x[0].a[1]]].a[1]);
                                    int temp_makespan = 0;
                                    for (int k = 0; k < i - 0 + 1; ++k) {
                                        int s = R_temp[k] + job[temp[k].a[0]][temp[k].a[1]][S.where_machine[temp[k].a[0]][temp[k].a[1]]].a[1] + Q_temp[k];
                                        temp_makespan = s > temp_makespan ? s : temp_makespan;
                                    }
                                    if (temp_makespan < best_makespan_tabu) {
										best_makespan_tabu = temp_makespan;
                                        best_move_tabu = { { t1,t2,x[0].a[0],x[0].a[1],0 } };
                                        best_pattern_tabu = ori_ma_bf_changed;
                                    }
                                    if (temp_makespan < best_makespan&& Tabu_table[machine_num].find(ori_ma_af_changed) ==
                                        Tabu_table[machine_num].end()/*&& Tabu_table[machine_num].find(tabu1) == Tabu_table[machine_num].end() &&
										Tabu_table[machine_num].find(tabu2) == Tabu_table[machine_num].end()*/) {
                                        if (temp_makespan == best_makespan) {
                                            best_move.push_back({ t1,t2,x[0].a[0],x[0].a[1],0 });
                                            best_pattern.push_back(ori_ma_bf_changed);
                                            best_pattern2.push_back({});
                                        }
                                        else {
                                            best_makespan = temp_makespan;
                                            best_move.clear();
                                            best_pattern.clear();
                                            best_pattern.clear();
                                            best_move.push_back({ t1,t2,x[0].a[0],x[0].a[1],0 });
                                            best_pattern.push_back(ori_ma_bf_changed);
                                            best_pattern2.push_back({});
                                        }
                                    }
                                }
                            }
                        }
                        Nk_nei(x, S, i, best_makespan, best_move, Tabu_table, best_pattern,best_pattern2,best_move_tabu,best_pattern_tabu,best_makespan_tabu);
                        continue;
                    }
                }
            }
            else {
                Nk_nei(x, S, 0, best_makespan, best_move, Tabu_table, best_pattern,best_pattern2,best_move_tabu,best_pattern_tabu,best_makespan_tabu);
            }
        }
        for (auto& x : Tabu_table) {
            for (auto it = x.begin(); it != x.end();) {
                --(*it).second;
                if ((*it).second <= 0) {
                    it=x.erase(it);
                }
                else {
                    ++it;
                }
            }
        }
        vector<int> choosed_move;
        vector<ARR> choosed_pattern;
        vector<ARR> choosed_pattern2;
        if (best_move.size() > 0) {
            uniform_int_distribution<> dis1(0, best_move.size() - 1);
            int choose = dis1(gen);
            choosed_move = best_move[choose];
            choosed_pattern = best_pattern[choose];
            choosed_pattern2 = best_pattern2[choose];
        }
        else {
			//++iter;
			//continue;
			/*cout.clear(ios::goodbit);
			cout << "全被tabu1" << endl;
			cout.setstate(ios::failbit);*/
            choosed_move = best_move_tabu;
            choosed_pattern = best_pattern_tabu;
        }
        vector<int>  b(5, -1);
		vector<ARR> tabu1, tabu2;
        int m_num = 0,m_num1=0;
        if (choosed_move != b) {
            m_num = S.ope_in_machine[choosed_move[0]][choosed_move[1]].a[0];
			//tabu1 = { {choosed_move[0],choosed_move[1]},{choosed_move[2],choosed_move[3]} };
			//tabu2 = { {choosed_move[2],choosed_move[3]},{choosed_move[0],choosed_move[1]} };
            if (choosed_move[2] != -2) {
                m_num1 = S.ope_in_machine[choosed_move[2]][choosed_move[3]].a[0];
            }
            else {
                m_num1 = choosed_move[3];
            }
			//Tabu_table[m_num][tabu1] = Tabu_tenure;
			//Tabu_table[m_num1][tabu2] = Tabu_tenure;
        }
		else {
			/*cout.clear(ios::goodbit);
			cout << "全被tabu" << endl;
			cout.setstate(ios::failbit);*/
		}
		int new_tabu_tenure = S.machine_ass[m_num].size() > 2 ? (S.machine_ass[m_num].size() - 2) * 3+1 : 5;
        if (choosed_pattern.size() != 0) {
            Tabu_table[m_num][choosed_pattern] = max(Tabu_tenure,new_tabu_tenure);
            if(choosed_pattern2.size()!=0)
                Tabu_table[m_num1][choosed_pattern2] = Tabu_tenure;
        }
		/*cout << "cri_block:" << endl;
		for (const auto& x : S.cri_block) {
			for (const auto& y : x)
				cout << y.a[0] << "," << y.a[1] << " ";
			cout << endl;
		}
		cout << endl;
		for (const auto& x : S.machine_ass[m_num])
			cout << x.a[0] << "," << x.a[1] << " ";
		cout << endl;
		for (const auto& x : choosed_move)
			cout << x << " ";
		cout << endl;
		for (const auto& x : Tabu_table) {
			for (const auto& y : x) {
				for (const auto& z : y.first)
					cout << z.a[0] << "," << z.a[1] << " ";
				cout << endl;
			}
		}
		cout << endl;*/
        if (choosed_move != b) {
            make_move(choosed_move, S);
        }
        if (S.makespan < obj_best) {
			++update_count;
            obj_best = S.makespan;
            cur = S;
            iter = 0;
        }
		if (obj_best < global_best_obj) {
			update_count1 = update_count;
			global_best_obj = obj_best;
			record_time.stop();
			get_best_time = record_time.elapsed_ms();
			record_time.start();
		}
        ++iter;
    }
    //cur = S;
	cout.clear(ios::goodbit);
	cout << "iter:" << global_iter << endl;
	cout.setstate(ios::failbit);
    cout << "best_obj=" << obj_best << endl;
    return ;
}

void MAE(int p,Solution &S_star) {
    Solution S1, S2, Sc, Sp;
    Solution empty;
    int iter = 0;
    Initial(S1);
    Initial(S2);
    Initial(Sc);
    caculate_makespan_Ini(Sc);
    Initial(Sp);
    caculate_makespan_Ini(Sp);
    //Initial(S_star);
    caculate_makespan_Ini(S_star);
    Solution S1_temp;
    Solution S2_temp;
    int gen = 0;
    int best_obj = INT_MAX;
	cout.clear(ios::goodbit);
	cout<<"11:" << S_star.makespan << endl;
	cout.setstate(ios::failbit);
    while (S_star.makespan>=26) {
        stop_watch s;
        s.start();
        /*if (gen > 100)
            break;*/
        PR(S1, S2, S1_temp,0.4,0.6,5);
        PR(S2, S1, S2_temp,0.4,0.6,5);
        TS(S1_temp,S1);
        TS(S2_temp,S2);
        int t1, t2, t3;
        t1 = S1.makespan;
        t2 = S2.makespan;
        t3 = Sc.makespan;
        Sc = (t1<t2?t1:t2) < t3 ?(t1<t2?S1:S2): Sc;
        t3 = (t1 < t2 ? t1 : t2) < t3 ? (t1 < t2 ? t1 : t2) : t3;
        t1 = S_star.makespan;
        S_star = t1 < t3 ? S_star : Sc;
        if (S_star.makespan < best_obj) {
            best_obj = S_star.makespan;
            gen = 0;
        }
        ++gen;
		caculate_d(S1, S2);
        if (same_pos_count>= 0.8*ope_sum) {//需修改
			cout.clear(ios::goodbit);
			cout << "S2 ini" << endl;
			cout.setstate(ios::failbit);
            S2 = empty;
            Initial(S2);
            caculate_makespan_Ini(S2);
        }
        if (iter == p) {
            S1 = Sp;
            Sp = Sc;
            Sc = empty;
            Initial(Sc);
            caculate_makespan_Ini(Sc);
            iter = 0;
        }
        ++iter;
        s.stop();
        cout.clear(ios::goodbit);
        cout << "用时:" << s.elapsed_ms() << endl;
        cout.setstate(ios::failbit);
    }
}

int main(char argc,char*argv[])
{   
    char input_file[] = "data\\fjsp.brandimarte.Mk02.m6j10c6.txt";
    get_file(input_file);
    for (const auto& x : job) {
        ope_sum += x.size();
    }
	empty_QorR.resize(job_count);
	for (int i = 0; i < job_count; ++i)
		empty_QorR[i].resize(job[i].size(), -1);
    Solution test_s;
    Solution test_s1;
    Solution test_s2;
    Initial(test_s);
    Initial(test_s1);
    Initial(test_s2);
    caculate_makespan_Ini(test_s);
	//caculate_makespan_Ini(test_s1);
    cout << "ope_sum:" << ope_sum << endl;
    //caculate_makespan_Ini(test_s1);
    
    //int t = caculate_d(test_s, test_s1);
    cout << "machine_count:" << machine_count << endl;
    cout << "ope_in_machine:" << endl;
    for (const auto& x : test_s.ope_in_machine) {
        for (const auto& y : x) {
            cout << y.a[0] << ","<<y.a[1]<<" ";
        }
        cout << endl;
    }
    cout << "machine_ass:" << endl;
    for (const auto& x : test_s.machine_ass) {
        for (const auto& y : x)
            cout << y.a[0] << "," << y.a[1] << " ";
        cout << endl;
    }
    for (const auto& x : test_s.R) {
        for (const auto& y : x) {
            //cout << y.a[0] << "," << y.a[1] << " ";
            cout << y << " ";
        }
        cout << endl;
    }
    cout << "makespan=" << test_s.makespan << endl;
    cout << "cri_block:" << endl;
    for (const auto& x : test_s.cri_block) {
        for (const auto& y : x) {
            cout <<test_s.ope_in_machine[y.a[0]][y.a[1]].a[0]<<","<<y.a[0] << "," << y.a[1] << " ";
        }
        cout << endl;
    }
    stop_watch s1;
    s1.start();
    cout.setstate(ios::failbit);
    //PR(test_s, test_s1, test_s2, 0.4, 0.6, 5);
	record_time.start();
    MAE(5,test_s1);
    cout.clear(ios::goodbit);
	//TS(test_s, test_s1);
    s1.stop();
    cout << "用时:" << s1.elapsed_ms() << endl;
    cout << "best_makespan:" << global_best_obj << endl;
	cout << "首次获得最优值时间:" << get_best_time << endl;
	cout << "update_count:" << update_count1 << endl;
    
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
