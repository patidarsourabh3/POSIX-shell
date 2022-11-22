#include <bits/stdc++.h>
#include <iostream>
#include <cstring>
#include <cstdio>
#include <fstream>
#include <unistd.h>
#include <termios.h>

#define clr() cout << "\033[H\033[J";
using namespace std;

char HOME[100];
char USER[100];
char HOSTNAME[100];
char PATH[200];
char PS1[100];
char VIDEO[30];
char MUSIC[30];
char PDF[30];
char TEXT[30];
char HISTSIZE[10];

string getEnv(string name)
{
	ifstream file;
	file.open("myrc.txt");
	if (!file)
	{
		file.close();
		return "Error in Opening myrc file";
	}
	string temp;
	string ans;
	while (getline(file, temp))
	{
		int index = temp.find("=");
		if (index >= 0)
		{
			string envname = temp.substr(0, index);
			if (envname == name)
			{
				ans = temp.substr(index + 2);
				file.close();
				ans.erase(ans.end() - 1);
				return ans;
			}
		}
	}
	file.close();
	return ans;
}

void setValues()
{
	strcpy(HOME, getEnv("HOME").c_str());
	strcpy(USER, getEnv("USER").c_str());
	strcpy(HOSTNAME, getEnv("HOSTNAME").c_str());
	strcpy(PATH, getEnv("PATH").c_str());
	strcpy(PS1, getEnv("PS1").c_str());
	strcpy(VIDEO, getEnv("VIDEO").c_str());
	strcpy(MUSIC, getEnv("MUSIC").c_str());
	strcpy(PDF, getEnv("PDF").c_str());
	strcpy(TEXT, getEnv("TEXT").c_str());
	strcpy(HISTSIZE, getEnv("HISTSIZE").c_str());
	return;
}

string execute_echo(vector<string> str)
{
	if (str.size() == 1)
	{
		return "\n";
	}
	string temp = str[1];
	if (temp[0] == '$')
	{
		temp.erase(temp.begin());
		if (temp == "PATH")
			return PATH;
		else if (temp == "HOME")
			return HOME;
		else if (temp == "HOSTNAME")
			return HOSTNAME;
		else if (temp == "PS1")
			return PS1;
		else if (temp == "USER")
			return USER;
		else if (temp == "HISTSIZE")
			return HISTSIZE;
		else if (temp == "$")
		{
			pid_t pid = getpid();
			return to_string(pid);
		}
		if (temp == "?")
		{
			return to_string(0);
		}

		else
			return "\n";
	}
	else
	{
		string ans = "";
		for (int i = 1; i < str.size(); i++)
		{
			ans = ans + str[i] + " ";
		}
		return ans;
	}
	return "\n";
}

string getPS1()
{
	string ans = "";
	char currdir[100];
	getcwd(currdir, sizeof(currdir));
	string s = "~";
	if (strcmp(USER, "root") == 0)
	{
		ans = string(USER) + "@" + string(HOSTNAME) + ":" + string(currdir) + "# ";
	}
	else
	{
		int i = 0;
		for (i = 0; i < strlen(currdir); i++)
		{
			if (currdir[i] != HOME[i])
				break;
		}
		for (int j = i; j < strlen(currdir); j++)
		{
			s += currdir[j];
		}
		ans = string(USER) + "@" + string(HOSTNAME) + ":" + s;
		ans = ans + "$ ";
	}
	return ans;
}

int execute_open(string path, char *ftype, bool waitflag)
{
	pid_t pid;
	pid = fork();
	if (pid < 0)
	{
		cout << "ERROR NO CHILD" << endl;
		return -1;
	}
	else if (pid == 0)
	{
		close(0);
		close(1);
		close(2);
		int st = execl(ftype, "xdg-open", path.c_str(), (char *)0);
		if (st < 0)
		{
			cout << "ERROR IN OPENING FILE" << endl;
			return -1;
		}
	}
	else
	{
		if(waitflag){
			wait(NULL);
			return 0;
		}
		else 
			return pid;

	}
	return 0;
}
int open_parse(string s, bool waitflag)
{
	string binary;
	int i = 0;
	cout << s << endl;
	while (s[i] != 46)
		i++;
	i++;
	string ftype = "";
	for (int j = i; j < s.length(); j++)
		ftype += s[j];
	if (ftype == "mp4")
		binary = VIDEO;
	if (ftype == "txt" || ftype == "cpp" || ftype == "c" || ftype == "html")
		binary = TEXT;
	if (ftype == "ogx" || ftype == "mp3")
		binary = MUSIC;
	if (ftype == "pdf")
		binary = PDF;
	char ch[50];
	for (int i = 0; i < binary.length(); i++)
		ch[i] = binary[i];
	char currdir[100];
	/* getcwd(currdir,sizeof(currdir));
	s=string(currdir) + "/" + s; */
	return execute_open(s, ch, waitflag);
}
