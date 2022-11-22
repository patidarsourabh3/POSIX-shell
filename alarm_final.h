#include<unistd.h>
#include<signal.h>
#include <time.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
using namespace std;
map<long long,string> alrm;
void saveAlarms()
{
    ofstream ofst("alarms.txt");
    if(ofst.is_open())
    {
        for(auto it=alrm.begin();it!=alrm.end();it++)
        {
            ofst<<it->first<<":"<<it->second<<"\n";
        }
    }
    ofst.close();
}
void readAlarms()
{
    ifstream ifst("alarms.txt");

    string line;
    if(ifst.is_open())
    {
        while (getline(ifst,line,'\n'))
        {
            stringstream ss(line);
            string txt;
            getline(ss,txt,':');
            long long k=stoll(txt);
            time_t current_time;
            time(&current_time);
            getline(ss,txt,':');
            if(current_time>k)
            {
                cout<<"Missed Alarm:"<<txt<<endl;
            }
            else
            {
                alrm[k]=txt;
            }
        }
    }
    ifst.close();
    if(alrm.size()>0)
    {
        auto it=alrm.begin();
        time_t current_time;
        time(&current_time);
        alarm(it->first - current_time);
    }
    ofstream ofs;
    ofs.open("alarms.txt", ofstream::out | ofstream::trunc);
    ofs.close();
}
void sig_handler(int signum)
{
    auto it=alrm.begin();
    cout<<"Alarm:"<<it->second<<endl;
    alrm.erase(it);
    it=alrm.begin();
    time_t current_time;
    time(&current_time);
    
    alarm(it->first - current_time);
}
void riseAlarm(string d)
{
    // string d="27/11/2021::19:12:36:::Message";
    int idx=d.find_last_of(":::");
    string message=d.substr(idx+1);
    d=d.substr(0,idx);
    idx=d.find_first_of("/");
    int day=stoi(d.substr(0,idx));
    d=d.substr(idx+1);
    idx=d.find_first_of("/");
    int month=stoi(d.substr(0,idx))-1;
    d=d.substr(idx+1);
    idx=d.find_first_of("::");
    int year=stoi(d.substr(0,idx))-1900;
    d=d.substr(idx+2);
    
    idx=d.find_first_of(":");
    int hours=stoi(d.substr(0,idx));
    d=d.substr(idx+1);
    idx=d.find_first_of(":");
    int minutes=stoi(d.substr(0,idx));
    d=d.substr(idx+1);
    int seconds=stoi(d);
    // cout<<d<<endl;
    // cout<<"Message:"<<message<<endl;
    // cout<<"Day:"<<day<<endl;
    // cout<<"month:"<<month<<endl;
    // cout<<"year:"<<year<<endl;
    // cout<<"hours:"<<hours<<endl;
    // cout<<"minutes:"<<minutes<<endl;
    // cout<<"seconds:"<<seconds<<endl;

    time_t current_time1;
    time(&current_time1);
    struct tm* tt=localtime(&current_time1);
    tt->tm_year=year;
    tt->tm_mday=day;
    tt->tm_mon=month;
    tt->tm_hour=hours;
    tt->tm_min=minutes;
    tt->tm_sec=seconds;
    time_t next = mktime(tt);
    // cout<<next<<endl;

    time_t current_time;
    time(&current_time);
    if(current_time>next)
    {
        cout<<"Alarm passed:"<<message<<endl;
    }
    else
    {
        alrm[next]=message;
    }
    if(alrm.size()>0)
    {
        auto it=alrm.begin();
        alarm(it->first - current_time);
    }

}
