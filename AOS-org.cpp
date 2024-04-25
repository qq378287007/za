#include <iostream>
#include <fstream>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <vector>
#include <map>
#include <bitset>
#include <algorithm>
#include <utility>
using namespace std;

#define BUF_SIZE 4096
#define AOS_SIZE 1024
#define OCT 0b010001

typedef struct
{
	uint8_t tag;
	uint32_t num;
	int flag;
	uint8_t framedata[948];
} frame;

const uint8_t AOS_head[4] = {0x1A, 0xCF, 0xFC, 0x1D};
vector<frame> oct_vector;
frame Frame;

bool cmp(frame a, frame b)
{
	if (a.flag == b.flag)
		return a.num < b.num;
	return a.flag < b.flag;
}

pair<int, int> findseries(vector<frame> &a)
{
	int maxn = 0, loc = 0;
	int t = 1;
	for (int i = 0; i < a.size() - 1; i++)
	{
		if ((a[i].num == a[i + 1].num - 1) && (a[i].flag == a[i + 1].flag))
		{
			t++;
		}
		else if (a[i].num == a[i + 1].num)
		{
			a.erase(a.begin() + i);
		}
		else
		{
			if (t > maxn)
			{
				maxn = t;
				loc = i - t + 1;
			}
			t = 1;
		}
	}
	pair<int, int> l(loc, maxn);
	return l;
}

void error_handle(const char *message)
{
	perror(message);
	exit(1);
}

int main()
{
	string path;
	FILE *fp;

	path = "F:\\HY1C_XXXXX_CHN1_20201027_215338_BJ_11202_A_00XX.org";
	fp = fopen(path.c_str(), "rb");
	if (fp == nullptr)
		error_handle("fopen() error");

	char last_buff[AOS_SIZE - 1] = {0};
	char all_buff[BUF_SIZE + AOS_SIZE - 1] = {0};
	char current_buff[BUF_SIZE] = {0};
	size_t last_size = 0;
	size_t current_size = 0;
	int buffLen;
	while (true)
	{
		current_size = fread(current_buff, BUF_SIZE, 1, fp);
		if (current_size <= 0)
			break;

		strncpy(all_buff, last_buff, last_size);
		strncpy(all_buff, current_buff, current_size);

		buffLen = last_size + current_size - AOS_SIZE + 1;
		for (int i = 0; i < buffLen; i++)
		{
			if (!strncmp(all_buff + i, (const char *)AOS_head, 4))
			{
				char tmpFlag = all_buff[i + 5] & 0b00111111;
				if (tmpFlag == OCT)
				{
					Frame.num = uint32_t(all_buff[i + 6]) << 16 + uint32_t(all_buff[i + 7]) << 8 + uint32_t(all_buff[i + 8]);
					Frame.flag = (all_buff[i + 9] & 0b10000000) >> 7;
					for (int j = 0; j < 948; j++)
						Frame.framedata[j] = all_buff[i + 10 + j];
					oct_vector.push_back(Frame);
				}

					i += AOS_SIZE - 1;
			}
		}
		strncpy(last_buff, current_buff + BUF_SIZE - AOS_SIZE + 1, AOS_SIZE - 1);
	}
	fclose(fp);

	if (!oct_vector.empty())
	{
		path = "oct.org";
		fp = fopen(path.c_str(), "wb");
		if (fp == nullptr)
			error_handle("fopen() error");

		sort(oct_vector.begin(), oct_vector.end(), cmp);
		pair<int, int> loc = findseries(oct_vector);
		printf("load data!\n");
		for (int i = 0; i < loc.second; i++)
		{
			int k = i + loc.first;
			fwrite(oct_vector[k].framedata, sizeof(oct_vector[k].framedata), 1, fp);
		}

		cout << loc.first << " " << loc.second << endl;
		fclose(fp);
	}

	cout << "Over!\n";
	return 0;
}