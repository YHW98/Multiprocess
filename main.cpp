#include<stdio.h>
#include<string.h>
#include<iostream>
#include<fstream>
#include<vector>
#include<chrono>
#include<unistd.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<sys/stat.h>

using namespace std;

void menu();
int  TASK3(string filename);
void BubbleWithProcess( int start, int end);
void Merge3(int start1, int end1, int start2, int end2);

int main() {
	menu();
	return 0;
}

void menu() {
	int cmd;
	string filename;
	ifstream file;
	filename = "1000000" ;
	file.open(filename);

	if (!file.is_open()) {
		cout << "filename error\n" ;
		return ;
	}
	
	file.close();
	TASK3(filename);
}

void BubbleWithProcess( int start, int end) {
	int shm_id = 0;
	int* total3;
	void* shm_addr;
	if ((shm_id = shmget(1234, 2 * sizeof(int) * 1000001, IPC_CREAT | 0666)) < 0) {
		perror("shmget");
		return;
	}
	if ((total3 = (int*)shmat(shm_id, NULL, 0)) == (void*)-1) {
		perror("shmat");
		return;
	}

	int numofdata = end - start + 1;
	int fixednumofdata = numofdata;
	int t[numofdata];
	for (; numofdata > 1; ) {
		numofdata = numofdata - 1;
		for (int i = 0; i < numofdata; i++) {
			if (start + i + 1 > 999999) cout << "Index Out of Range ";

			if (total3[start + i] > total3[start + i + 1]) {
				int n = total3[start + i];
				total3[start + i] = total3[start + i + 1];
				total3[start + i + 1] = n;
			}
		}
	}
}

void Merge3(int start1, int end1, int start2, int end2) {
	int numofdata = end2 - start1 + 1;
	int t[numofdata];
	int shm_id = 0;
	int* total3;
	void* shm_addr;
	if ((shm_id = shmget(1234, 2 * sizeof(int) * 1000001, IPC_CREAT | 0666)) < 0) {
		perror("shmget");
		return;
	}
	if ((total3 = (int*)shmat(shm_id, NULL, 0)) == (void*)-1) {
		perror("shmat");
		return;
	}

	int i = 0;
	int i1 = start1;
	int i2 = start2;
	for (; (i1 <= end1 || i2 <= end2) && i < numofdata; ) {
		if (i1 <= end1 && i2 <= end2) {
			if (total3[i1] <= total3[i2]) {
				t[i] = total3[i1];
				i1 = i1 + 1;
				i++;
			}
			else {
				t[i] = total3[i2];
				i2 = i2 + 1;
				i++;
			}

		}
		else if (i1 <= end1 && i2 > end2) {
			t[i] = total3[i1];
			i1 = i1 + 1;
			i++;
		}
		else if (i1 > end1&& i2 <= end2) {
			t[i] = total3[i2];
			i2 = i2 + 1;
			i++;
		}
		else break;
	}


	for (int i = 0; i < numofdata; i++) {
		total3[start1 + i] = t[i];
	}
}


int TASK3(string filename) {
	pid_t pid;
	int status;
	int n = 0;
	int k = 0;
	int i = 0;
	int count = 0;
	int shm_id = 0;
	int* total3;
	void* shm_addr;
	if ((shm_id = shmget(1234, 2 * sizeof(int) * 1000001, IPC_CREAT | 0666)) < 0) {
		perror("shmget");
		return 0;
	}
	if ((total3 = (int*)shmat(shm_id, NULL, 0)) == (void*)-1) {
		perror("shmat");
		return 1;
	}

	for (int j = 0; j < 1000001; j++) total3[j] = -1;

	FILE* pFile;
	pFile = fopen(filename.c_str(), "r");

	while (fscanf(pFile, "%d", &n) != EOF) {
		total3[i] = n;
		i++;
	}

	fclose(pFile);
	k = 320 ;

	cout << "Program Start...\n";
	chrono::steady_clock::time_point t1 = chrono::steady_clock::now();

	int numofdata = i; 
	for (int i = 0; i < k; i++) {
		pid = fork();

		count++;
		if (pid == 0 || pid == -1) break; 
	}

	if (pid == -1) {

		cout << "Error occur\n";//error
		exit(0);
	}
	else if (pid == 0) {
		// cout << "Process : " << count << endl;
		if (count == k) BubbleWithProcess( (count - 1) * (int)(numofdata / k), (numofdata - 1));
		else if (count < k) BubbleWithProcess( (count - 1) * (int)(numofdata / k), (count) * (int)(numofdata / k) - 1);
		else cout << "Calling function : Bubble With Process error\n";
		exit(0);
	}
	else {
		for (int i = 0; i < k; i++) {
			pid = wait(&status); //parent process
		}
		
		int step = numofdata / k;
		while (k > 1) {
			int prek = k;
			if (k % 2 == 1) k++;
			k = k / 2;

			count = 0;
			for (int i = 0; i < k; i++) {
				pid = fork();
				count++;
				if (pid == 0 || pid == -1) break;
			}

			if (pid == -1) {
				cout << "Error occur\n";//error
			}
			else if (pid == 0) {
				if (((count - 1) * 2 + 1) < prek) {
					int start1 = ((count - 1) * 2) * step;
					int start2 = ((count - 1) * 2 + 1) * step;
					int end1 = ((count - 1) * 2 + 1) * step - 1;
					int end2 = (count * 2) * step - 1;
					if (count == k) end2 = numofdata - 1;
					Merge3(start1, end1, start2, end2);
				}
				else; // wait the next round

				exit(0);
			}
			else {
				//
				for (int i = 0; i < k; i++) {

					pid = wait(&status); //parent process
					if (pid == -1 || status != 0) {
						cout << "In " << k << " children : " << i << "th Child died \n";
						cout << status << endl;
					}
				}


			}

			step = step * 2;

		}




		chrono::steady_clock::time_point t2 = chrono::steady_clock::now();
		cout << "Program End...\n" ;
		cout << "Duration : " << chrono::duration_cast<chrono::milliseconds>(t2 - t1).count() << " (ms)\n";

		ofstream out("10627125_3_" + filename + ".out");
		out << "Method : Bubble and Merge With Process\n";
		out << "number of datas : " << numofdata << endl;
		out << "Duration : " << chrono::duration_cast<chrono::milliseconds>(t2 - t1).count() << " (ms)\n" ;

		out.close();
	}



}
