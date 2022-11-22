#include <dirent.h>
#include <string.h>
#include <iostream>
#include <sys/stat.h>
#include <vector>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <termios.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <cstring>
#include <stack>
#include <sys/wait.h>
#include <cstdio>
#include <unordered_map>
#include <algorithm>
#include <poll.h>
#include <map>
#include "alarm_final.h"
#include "init.h"
using namespace std;

bool record_flag = 0;
unordered_map<int, int> process_bg;
bool bg_flag = 0;
struct w_conf
{
    struct termios inp;
    int coln;
    int rown;
    int cur_pos;
    string start_directory;
} wc;

class trie_node
{
public:
    int end;
    unordered_map<char, trie_node *> m;
    trie_node()
    {
        end = -1;
    }
};

class trie
{
    vector<string> ans;
    vector<vector<int>> matrix;
    string curr_ans;

public:
    trie_node *head;

    trie()
    {
        head = new trie_node();
    }

    void insert(string a, int list_no)
    {
        trie_node *temp = head;
        for (int i = 0; i < a.length(); i++)
        {
            char cur = a[i];
            if (temp->m[cur] == nullptr)
            {
                temp->m[cur] = new trie_node();
            }
            temp = temp->m[cur];
        }
        temp->end = list_no;
    }
    void autocomplete_helper(string s, trie_node *temp)
    {
        if (temp == nullptr)
            return;
        if (temp->end >= 0)
            ans.push_back(s);
        for (auto i : temp->m)
        {
            s.push_back(i.first);
            autocomplete_helper(s, i.second);
            s.pop_back();
        }
    }
    vector<string> autocomplete(string s)
    {
        ans.clear();
        trie_node *temp = head;
        for (int i = 0; i < s.length(); i++)
        {
            if (temp == nullptr)
                return ans;
            if (temp->m.find(s[i]) == temp->m.end())
                return ans;
            else
                temp = temp->m[s[i]];
        }
        // if(temp -> end >= 0)
        //    ans.push_back(s);
        autocomplete_helper(s, temp);
        sort(ans.begin(), ans.end());
        return ans;
    }
};

void enter_noncanon()
{
    tcgetattr(STDIN_FILENO, &(wc.inp));
    struct termios otp = wc.inp;
    otp.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    //inp.c_oflag &= ~(OPOST);
    otp.c_cflag |= (CS8);
    otp.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    otp.c_cc[VMIN] = 0;
    otp.c_cc[VTIME] = 1;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &otp);
    //return otp;
}

void exit_noncanon()
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &(wc.inp));
}

vector<string> process_string(string buffer)
{
    vector<string> command_input;
    string buf = "";
    for (int i = 0; i < buffer.length(); i++)
    {
        if (buffer[i] == ' ')
        {
            command_input.push_back(buf);
            buf.clear();
        }
        else
        {
            buf.push_back(buffer[i]);
        }
    }
    command_input.push_back(buf);
    for (int i = 0; i < command_input.size(); i++)
    {
        string op = command_input[i] + " ,";
        // write(STDOUT_FILENO, op.c_str(), op.length());
        //cout << command_input[i] << " ,";
    }
    return command_input;
}
int cflag;
char character_input()
{
    char c = 0;
    cflag = 0;
    enter_noncanon();
    if (read(STDIN_FILENO, &c, 1) == 1)
    {
        if (iscntrl(c) && c == 27)
        {
            read(STDIN_FILENO, &c, 1);
            if (c == 91)
            {
                read(STDIN_FILENO, &c, 1);
                switch (c)
                {
                case 'A':
                    cflag = 'A';
                    break;
                case 'B':
                    cflag = 'B';
                    break;
                case 'C':
                    cflag = 'C';
                    break;
                case 'D':
                    cflag = 'D';
                    break;
                }
            }
        }
    }

    exit_noncanon();
    return c;
}

string execute_without_input_pipe(vector<string> command, string inp)
{
    int j = 0;
    /*  while (j < command.size())
    {
        write(STDOUT_FILENO, (command[j] + "\n").c_str(), command[j].length() + 1);
        j++;
    } */
    string result;
    int pip1[2], pip2[2];
    if (pipe(pip1) == -1)
    {
        write(STDOUT_FILENO, "pipe failed", 11);
        return " ";
    }
    if (pipe(pip2) == -1)
    {
        write(STDOUT_FILENO, "pipe failed", 11);
        return " ";
    }
    if (!inp.empty())
        write(pip2[1], inp.c_str(), inp.length());
    close(pip2[1]);
    //write(1,inp.c_str(),inp.length());
    int status;
    string retbuf, r1;
    int cpid;
    if ((cpid = fork()) == 0)
    {

        close(1);     //closing stdout
        dup(pip1[1]); //replacing stdout with pipe write
        close(0);
        dup(pip2[0]);
        close(pip1[0]); //closing pipe read
        close(pip1[1]);
        close(pip2[0]); //closing pipe read
        close(pip2[1]);
        char buff[4096];
        // int ace = read(0,buff,4096);
        //write(1,buff,ace);
        //write(1,"here",4);
        //char cs[] = "cat";
        //char cs1[] = "ls -l";
        char *cmd[command.size() + 1];
        int i;
        for (i = 0; i < command.size(); i++)
        {
            cmd[i] = const_cast<char *>(command[i].c_str());
        }
        cmd[i] = NULL;

        execvp(cmd[0], cmd);
        //exit(0);
    }
    else
    {
        if (bg_flag)
        {
            process_bg[cpid] = 1;
            retbuf = to_string(cpid);
        }
        else
        {
            waitpid(cpid, &status, 0);
            char buff[4096] = {0};
            int ctr = 0;

            //retbuf.assign(buff);
            struct pollfd pfd;
            pfd.fd = pip1[0];
            pfd.events = POLLIN;
            while (poll(&pfd, 1, 0) == 1)
            {

                ctr = read(pip1[0], buff, 4096);
                r1.clear();
                r1 = buff;
                retbuf += r1;
            }
            close(pip1[0]); //closing pipe read
            close(pip1[1]);
            close(pip2[0]); //closing pipe read
            close(pip2[1]);
        } //write(STDOUT_FILENO,buff,ctr);
    }

    return retbuf;
}

void file_write(string word, string fileName)
{
    //fstream f;
    ofstream fout;
    fout.open(fileName, ios::out);
    if (fout.is_open())
        fout << word << "\n";

    fout.close();
}
void file_append(string word, string fileName)
{
    ofstream fout;
    fout.open(fileName, ios::app);
    if (fout.is_open())
        fout << word << "\n";
    fout.close();
}
string execute_cmd(vector<string> inp, vector<string> kcmd, trie hl)
{
    bg_flag = 0;
    //{"ls", "echo", "touch", "mkdir",
    //"grep", "pwd", "cd", "cat", "head", "tail", "chmod", "exit", "history", "clear", "cp"}
    unordered_map<string, int> mp;
    for (int mi = 0; mi < kcmd.size(); mi++)
    {
        mp[kcmd[mi]] = 1;
    }
    mp["|"] == 1;
    mp[">"] == 1;
    mp[">>"] == 1;
    string an = "";
    int i = 0;
    vector<string> command;
    if (inp[0] == "exit")
    {
        saveAlarms();
        return "-1";
    }
    if (inp[0] == "record")
    {
        if (inp.size() == 1)
            return "";
        if (inp[1] == "start")
            record_flag = 1;
        else if (inp[1] == "stop")
            record_flag = 0;
        return "";
    }
    if (inp[0] == "history")
    {
        if (inp.size() == 1)
            inp.push_back("");
        if (inp.size() > 2)
        {
            for (int kl = 2; kl < inp.size(); kl++)
                inp[1] += " " + inp[kl];
        }
        vector<string> va = hl.autocomplete(inp[1]);
        for (int kl = 0; kl < va.size(); kl++)
        {
            an += va[kl] + "\n";
        }
        return an;
    }
    if (inp[inp.size() - 1] == "&")
    {
        bg_flag = 1;
        inp.pop_back();
    }
    if (inp[0] == "open")
    {
        if (!bg_flag)
            open_parse(inp[1], 1);
        else
        {
            string anaa;
            int ana;

            ana = open_parse(inp[1], 0);
            process_bg[ana] = 1;
            anaa = to_string(ana);
            return anaa;
        }
        return " ";
    }

    if (inp[0] == "alarm")
    {
        if (inp.size() < 2)
            return "";
        riseAlarm(inp[1]);
        return "";
    }
    if (inp[0] == "cd")
    {
        if (inp.size() < 2)
            return "";
        chdir(inp[1].c_str());
        return "";
    }
    if (inp[0] == "fg")
    {
        if (inp.size() < 2)
            return "";
        if (process_bg.find(stoi(inp[1])) == process_bg.end())
            return "";
        waitpid(stoi(inp[1]), NULL, 0);
        process_bg.erase(stoi(inp[1]));
        return "";
    }

    while (i < inp.size())
    {
        command.clear();
        command.push_back(inp[i++]);
        while (i < inp.size() && mp.find(inp[i]) == mp.end())
            command.push_back(inp[i++]);
        // command.clear();
        if (command[0] == "echo")
        {
            an = execute_echo(command);
        }
        else
            an = execute_without_input_pipe(command, an);
        //write(STDOUT_FILENO, an.c_str(), an.length());
        if (i >= inp.size())
            break;
        if (inp[i] == ">")
        {
            if ((inp.size() - 1) == i)
                break;
            file_write(an, inp[i + 1]);
            an.clear();
        }
        if (inp[i] == ">>")
        {
            if ((inp.size() - 1) == i)
                break;
            file_append(an, inp[i + 1]);
            an.clear();
        }
        if (inp[i] != "|")
            break;
        i++;
    }
    //  write(STDOUT_FILENO, "hi", 2);
    return an;
}
int detect_commands(trie t, vector<string> vs1, trie &hl)
{
    char c;
    string buffer;
    string pn;
    string green = "\033[32m";
    string white = "\033[00m";
    pn = green + getPS1() + white;
    int ctl = 1;
    /* pn = green + PS1 + white; */
    write(STDOUT_FILENO, pn.c_str(), pn.length());

    while (1)
    {

        char c1;
        c1 = character_input();
        if (cflag)
        {
            switch (cflag)
            {
            case 'A':
                //  cout << "Up" << endl;
                // write(STDOUT_FILENO, "up", 2);
                c = 0;
                break;
            case 'B':
                //  cout << "Down" << endl;
                //write(STDOUT_FILENO, "down", 4);
                c = 0;
                break;
            case 'C':
                //  cout << "Right" << endl;
                // write(STDOUT_FILENO, "right", 5);
                c = 0;
                break;
            case 'D':
                //  cout << "Left" << endl;
                //write(STDOUT_FILENO, "left", 4);
                c = 0;
                break;
            }
        }
        else
        {
            c = c1;
            if (c == 0)
                ;
            else if (c == 9)
            {
                //write(STDOUT_FILENO, "tab", 3);
                vector<string> va = t.autocomplete(buffer);
                buffer.clear();
                if (va.size() == 1)
                {
                    buffer.clear();

                    // write(STDOUT_FILENO, " ", 1);
                    //buffer.pop_back();
                    // write(STDOUT_FILENO, "\b", 1);

                    //write(STDOUT_FILENO, va[0].c_str(), va[0].length());
                    buffer = va[0];
                    write(STDOUT_FILENO, "\n", 1);
                    pn = green + getPS1() + white;
                    write(STDOUT_FILENO, pn.c_str(), pn.length());
                    write(STDOUT_FILENO, va[0].c_str(), va[0].length());
                }
                else
                {
                    write(STDOUT_FILENO, "\n", 2);
                    for (int i = 0; i < va.size(); i++)
                    {
                        write(STDOUT_FILENO, va[i].c_str(), va[i].length());
                        write(STDOUT_FILENO, "\n", 2);
                    }
                    pn = green + getPS1() + white;
                    write(STDOUT_FILENO, pn.c_str(), pn.length());
                }
            }

            else if (c == 13)
            {
                write(STDOUT_FILENO, "\n", 2);
                hl.insert(buffer, ctl++);
                vector<string> vsi = process_string(buffer);

                string ans;
                ans = execute_cmd(vsi, vs1, hl);
                if (ans == "-1")
                    return 1;
                write(1, ans.c_str(), ans.length());

                if (record_flag)
                    file_append(buffer + "\n" + ans, "record.txt");
                buffer.clear();
                write(STDOUT_FILENO, "\n", 1);
                pn = green + getPS1() + white;
                write(STDOUT_FILENO, pn.c_str(), pn.length());
            }
            /* else if (c == 'q')
                return 1; */
            else if (c == 127)
            {
                if (!buffer.empty())
                {
                    buffer.pop_back();
                    //write(STDOUT_FILENO, buffer.c_str(),buffer.length());
                    write(STDOUT_FILENO, "\b", 1);
                    write(STDOUT_FILENO, " ", 1);
                    //buffer.pop_back();
                    write(STDOUT_FILENO, "\b", 1);
                }
            }
            else
            {
                const char *co = &c;
                write(STDOUT_FILENO, co, 1);
                buffer.push_back(c);
            }
        }
    }
}
void initialize_trie(trie &t, vector<string> s)
{

    for (int i = 0; i < s.size(); i++)
        t.insert(s[i], i + 1);
}

int main()
{
    signal(SIGALRM, sig_handler);
    readAlarms();
    setValues();
    vector<string> vs{"open", "record stop", "record start", "alarm", "ls", "echo", "touch", "mkdir", "grep", "pwd", "cd", "cat", "head", "tail", "chmod", "exit", "history", "clear", "cp"};
    trie t;
    trie hl;
    initialize_trie(t, vs);
    detect_commands(t, vs, hl);
    return 0;
}