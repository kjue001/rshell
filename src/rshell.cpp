#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <vector>
#include <string.h>
#include <errno.h>
#include <boost/foreach.hpp>
#include <boost/tokenizer.hpp>
using namespace std;
using namespace boost;

class CommandLine
{
    public:
        //takes user input and puts it into a vector
        void split(vector <string> &vctr, char seper, string cmd)
        {
            //cout << "split 1" << endl;
            if(cmd.size()!=0)
            {
                //cout << "split 2" << endl;
                int state = 0;
                if((cmd.at(0) == '(' && seper == '(')||(cmd.at(0) == '#' && seper == '#'))
                     state = 1;
                if(cmd.at(cmd.size()-1) == ')' && seper == ')')
                    state = 2;
                int a = 0,b = 0;
                char* sepa;
                sepa = &seper;
                string s1(1,seper);
                string s2(2,seper);
                char_separator<char> sep(sepa);
                tokenizer< char_separator<char> > tokens(cmd, sep);
                BOOST_FOREACH (const string& t, tokens) 
                {
                    if(t != "/0")
                    ++a;
                }
                BOOST_FOREACH (const string& t, tokens) 
                {
                    ++b;
                    
                    //if(seper )
                    if (state == 1)
                    {
                        vctr.push_back(s1);
                    }
                    if(t != " ")
                    vctr.push_back(t);
                    if(state == 2)
                    {
                        vctr.push_back(s1);
                    }
                    if(b != a)
                    {
                        if(seper == ';' || seper == '#'|| seper == '(' || seper == ')')
                        vctr.push_back(s1);
                        else if(seper == '|' || seper == '&')
                        vctr.push_back(s2);
                    }
                    else if(seper == ')' && b == a && a != 1 && t != " " )
                        vctr.push_back(s1);
                    
                    
                    //cout << t << "." << endl;
                }
            }
            return;
        }
        
        void start(vector <string> &vctr)
        {
            string cmd;
            vector <string> temp;
            
            cout << "$ ";
            getline(cin, cmd);
            //cout << "start 1" << endl;
            split(vctr,';',cmd);
            //cout << "start 1.1" << endl;
            //cout << vctr.size() << endl;
            if(vctr.size() != 0)
            {
                for(int i = 0; i < vctr.size(); ++i)
                    split(temp,'|',vctr.at(i));
                vctr.clear();
                for(int i = 0; i < temp.size(); ++i)
                    split(vctr, '&', temp.at(i));
                temp.clear();
                for(int i = 0; i < vctr.size(); ++i)
                    split(temp, '(', vctr.at(i));
                vctr.clear();
                // for(int i = 0; i < temp.size(); ++i)
                //     split(vctr, '#', temp.at(i));
                // temp.clear();
                for(int i = 0; i < temp.size(); ++i)
                    split(vctr, ')', temp.at(i));
                //temp.swap(vctr);
            }
            
            return;
        }
};

class Connector
{
    public:
        // logic for or connector
        bool ors(bool one)
        {
            if(one)
            {
               return true;
            }
           return false; 
        }
        //logic for and connector
        bool ands(bool one)
        {
            if(one)
            {
                return true;
            }
            return false;
        }
        //logic for semicolon 
        bool semi(bool one)
        {
            return false;
        }
        // logic for hash function
        bool hash()
        {
            return true;
        }
};

class Execute : public Connector
{
    public:
        // executes commands if there is a command and a flag also records the state
        void go(string cmd, string flag, bool &state)
        {
            int status;
            pid_t pid;
            pid = fork();  
            const char* args[3] = { cmd.c_str(),flag.c_str(), '\0'} ;
            
            if(pid < 0)
            {                                                                                                                                  
                perror("fork failed");
                exit(1);                                                                                                                                        
            }
            else if( pid == 0 )
            {
                execvp( args[0], (char**)args);
                //cout << "before" << state << endl;
                //state = true;
                int err = errno;
                perror("execve failed");
                //cout << "after" << state << endl;
                exit(err);
                
                //printf("Child: I'm the child: %d\n", pid);
            }
            else if (pid > 0)
            {
                wait(&status);
                //cout << "status" << status << endl;
                
                if(WIFEXITED(status))
                {
                    // if execution does not succeed then the state is true
                    if(status != 0)
                    {
                        state= true;
                    }
                }
            return;
            }
        }
        //executes a command without a flag and records state
        void go(string cmd, bool &state)
        {
            pid_t pid;
            int status;
            pid = fork();  
            const char* args[2] = { cmd.c_str(), '\0'} ;
            
             if(pid < 0)
            {                                                                                                                                  
                perror("fork failed");
                exit(1);                                                                                                                                        
            }
            else if( pid == 0 )
            {
                execvp( args[0], (char**)args);
                //cout << "before" << state << endl;
                //state = true;
                int err = errno;
                perror("execve failed");
                //cout << "after" << state << endl;
                exit(err);
                
           }
            else if (pid > 0)
            {
                wait(&status);
                if(WIFEXITED(status))
                {
                    if(status != 0)
                    {
                        state= true;
                    }
                }
            
            }
        }
        
        void execute(int &loc, string cmd, vector <string> &cmds, vector <bool> &states)
        {
            //cout << "execute" << endl;
            if(cmds.size() != 0)
            {
                vector <string> single;
                Connector connect;
                bool st = false;
                char_separator<char> sep(" ");
                tokenizer< char_separator<char> > tokens(cmd, sep);
                BOOST_FOREACH (const string& t, tokens) 
                {
                    single.push_back(t);
                }
                if(cmd == "exit" || cmd == " exit" || cmd == "exit ")
                {
                    loc++;
                    return;
                }
                else if(loc > cmds.size())
                    return;
                else if(cmd == "(")
                {
                    int i = loc;
                    vector <string> temp;
                    while(cmds.at(i) != ")")
                    {
                        temp.push_back(cmds.at(i));
                        ++i;
                    }
                    loc = i;
                    int p = 1;
                    while(p < temp.size())
                    {
                        string word = temp.at(p);
                        loc++;
                        ++p;
                        //cout << "main" << endl;
                        execute(loc, word ,cmds,states);
                        //cout << i << endl;
                    }
                    // string word = cmds.at(loc);
                    // while(word != ")")
                    // {
                    //     cout << cmd << 40 << endl;
                    //     if(loc != cmds.size()-1)
                    //     {
                    //         loc++;
                    //     }
                    //     word = cmds.at(loc);
                    //     execute(loc, word, cmds, states);
                        
                    // }
                
                }
                else if(cmd == ")")
                {
                    loc++;
                    return;
                }
                else if(cmd == "#")
                {
                    
                    cout << 1 << endl;
                    loc = cmds.size();
                    return;
                }
                else if(cmd != "||" && cmd != "&&" && cmd != ";")
                {
                    cout << 2 << endl;
                    if(loc <= 1)
                    {
                        cout << 2.2 << endl;
                        if(single.size() == 1)
                        go(single.at(0),st);
                        else 
                        go(single.at(0), single.at(1),st);
                        ++loc;
                        bool stat = connect.ors(st);
                        states.push_back(stat);
                        if(single.at(single.size()-1) == "#")
                            loc = cmds.size();
                    }
                    else if (loc > 1)
                    {
                        cout << 2.3 << endl;
                        if(cmds.at(loc - 1) == "||" || (cmds.at(loc - 1) == "(" && cmds.at(loc - 2) == "||"))
                        {
                            cout << 2.1 << endl;
                            //cout << states.at(states.size()-1) << endl;
                            //cout << single.size() << endl;
                            if(states.size()!= 0)
                            {
                                if(states.at(states.size()-1))
                                {
                                    cout << "inside" << endl;
                                    if(single.size() == 1)
                                    go(single.at(0),st);
                                    else 
                                    go(single.at(0), single.at(1),st);
                                    states.push_back(st);
                                    if(single.at(single.size()-1) == "#")
                                        loc = cmds.size();
                                }
                            }
                            cout << "above loc" << endl;
                            ++loc;
                            cout << "below loc" << endl;
                            //return;
                        }
                        else if(cmds.at(loc - 1) == "&&")
                        {
                            cout << 300 <<endl;
                            if(states.size() != 0)
                            {
                                cout << 400 << endl;
                                if(!states.at(states.size()-1))
                                {
                                    if(single.size() == 1)
                                    go(single.at(0),st);
                                    else 
                                    go(single.at(0), single.at(1),st);
                                    states.push_back(st);
                                    if(single.at(single.size()-1) == "#")
                                        loc = cmds.size();
                                }
                            }
                            cout << 5000 << endl;
                            ++loc;
                        }
                        else if(cmds.at(loc -1) == ";")
                        {
                            cout << 400 << endl;
                            if(single.size() == 1)
                            go(single.at(0),st);
                            else 
                            go(single.at(0), single.at(1),st);
                            ++loc;
                            states.push_back(st);
                            if(single.at(single.size()-1) == "#")
                                loc = cmds.size();
                        }
                    }
                
                }
                else if(cmd == "||" || cmd == "&&" || cmd == ";")
                {
                    cout << 3 << endl;
                    loc++;
                }
            }
            return;
        }
        

};

// class Execute
// {
//     public:
//         // executes commands if there is a command and a flag also records the state
//         void go(string cmd, string flag, bool &state)
//         {
//             int status;
//             pid_t pid;
//             pid = fork();  
//             const char* args[3] = { cmd.c_str(), flag.c_str(), '\0'} ;
            
//             if(pid < 0)
//             {                                                                                                                                  
//                 perror("fork failed");
//                 exit(1);                                                                                                                                        
//             }
//             else if( pid == 0 )
//             {
//                 execvp( args[0], (char**)args);
//                 //cout << "before" << state << endl;
//                 //state = true;
//                 int err = errno;
//                 perror("execve failed");
//                 //cout << "after" << state << endl;
//                 exit(err);
                
//                 //printf("Child: I'm the child: %d\n", pid);
//             }
//             else if (pid > 0)
//             {
//                 wait(&status);
//                 //cout << "status" << status << endl;
                
//                 if(WIFEXITED(status))
//                 {
//                     // if execution does not succeed then the state is true
//                     if(status != 0)
//                     {
//                         state= true;
//                     }
//                 }
//             return;
//             }
//         }
//         // executes a command without a flag and records state
//         void go(string cmd, bool &state)
//         {
//             pid_t pid;
//             int status;
//             pid = fork();  
//             const char* args[2] = { cmd.c_str(), '\0'} ;
            
//              if(pid < 0)
//             {                                                                                                                                  
//                 perror("fork failed");
//                 exit(1);                                                                                                                                        
//             }
//             else if( pid == 0 )
//             {
//                 execvp( args[0], (char**)args);
//                 //cout << "before" << state << endl;
//                 //state = true;
//                 int err = errno;
//                 perror("execve failed");
//                 //cout << "after" << state << endl;
//                 exit(err);
                
//             }
//             else if (pid > 0)
//             {
//                 wait(&status);
//                 if(WIFEXITED(status))
//                 {
//                     if(status != 0)
//                     {
//                         state= true;
//                     }
//                 }
            
//             }
//         }
        

// };



int main(int argc, char *argv[])
{
    vector <string> lst;
    vector <bool> executed;
    CommandLine cmd;
    Execute ex;
    //Connector connect;
    cmd.start(lst);
    for(int i = 0; i < lst.size(); ++i)
    {
        cout << lst.at(i) << endl;
    }
    // lst.clear()
    // cmd.start(lst);
    // for(int i = 0; i < lst.size(); ++i)
    // {
    //     cout << lst.at(i) << endl;
    // }
    //int i = 0;
    // while(i < lst.size())
    // {
    //     //cout << "main" << endl;
    //     ex.execute(i,lst.at(i),lst,executed);
    //     cout << i << endl;
    //     if(lst.at(i) == "exit" || lst.at(i) == "exit " || lst.at(i) == " exit")
    //     return 0;
    //     //cout << i << endl;
    // }
    //cout << lst.at(0) << endl;
    while(lst.size() == 0 || (lst.at(0) != "exit" && lst.at(0) != "exit " && lst.at(0) != " exit"))
    {
        int i = 0;
        cout << lst.size() << endl;
        while(i < lst.size())
        {
            //cout << "main" << endl;
            if((lst.at(i) == "exit" || lst.at(i) == "exit " || lst.at(i) == " exit") && (lst.at(i - 1) == "&&" || lst.at(i - 1) == ";"))
            return 0;
            if(lst.at(i) == "(")
            {
                cout << "parentesis" << endl;
                vector <bool> temp;
                ++i;
                while(lst.at(i) !=")")
                ex.execute(i,lst.at(i),lst,temp);
                executed.push_back(temp.at(temp.size()-1));
            }
            ex.execute(i,lst.at(i),lst,executed);
        }
        lst.clear();
        executed.clear();
        cmd.start(lst);
    }
    // //ex.go("mkdir", "kjue");
    // // executes first command line
    // if(lst.size() != 0)
    // {
    //     //  if the command line has less than two arguements
    //     if(lst.size() <= 6)
    //     {
    //         if(lst.at(0) == "exit")
    //             return 0;
    //         if(lst.at(0) == "#")
    //         {
                
    //         }
    //         else if(lst.size() == 2)
    //         {
    //             ex.go(lst.at(0), states = false);
    //         }
    //         else if(lst.at(1) == "#")
    //         {
    //             ex.go(lst.at(0), states = false);
    //         }
    //         else
    //         ex.go(lst.at(0),lst.at(1),states = false);
    //     }
    //     // if the command line has at least 2 commands
    //     if(lst.size() > 6)
    //     {
    //         if(lst.at(1) == "#")
    //         {
    //             ex.go(lst.at(0), states = false);
    //         }
    //         else if(lst.at(4) == "#")
    //         {
    //             ex.go(lst.at(3), states = false);
    //         }
    //         else if(lst.at(2) == "||")
    //         {
    //             //state = false;
    //             ex.go(lst.at(0),lst.at(1), states);
    //             //cout << states << endl;
    //             bool stat = connect.ors(states);
    //             //cout << stat << endl;
    //             executed.push_back(stat);
    //             if(stat)
    //             {
    //                 ex.go(lst.at(3),lst.at(4),states = false);
    //                 executed.push_back(states);
    //             }
    //         }
    //         else if(lst.at(2) == "&&")
    //         {
    //             ex.go(lst.at(0), lst.at(1), states);
    //             //cout << states << endl;
    //             bool stat = connect.ands(states);
    //             //cout << stat << endl;
    //             executed.push_back(stat);
    //             if(!stat)
    //             {
    //                 ex.go(lst.at(3),lst.at(4),states = false);
    //                 executed.push_back(states);
    //             }
    //         }
    //         else if(lst.at(2) == ";")
    //         {
    //             ex.go(lst.at(0), lst.at(1), states = false);
    //             executed.push_back(states);
    //             ex.go(lst.at(3), lst.at(4), states = false);
    //             executed.push_back(states);
    //         }
    //     }
    // }
    //     //if the command line has more than 2 commands
    //     if(lst.size() > 7)
    //     {
    //         //cout << lst.at(6) << endl;
    //         for (unsigned int i = 5; i < lst.size()-2; i = i + 3)
    //         {
    //             if(lst.at(i+ 2) == "#")
    //             {
    //                 ex.go(lst.at(i + 1), states = false);
    //             }
    //             else if(lst.at(i) == "||")
    //             {
    //                 if(executed.at(executed.size()-1))
    //                 {
    //                     ex.go(lst.at(i + 1), lst.at(i + 2), states = false);
    //                     executed.push_back(states);
    //                 }
    //             }
                
    //             else if(lst.at(i) == "&&")
    //             {
    //                 if(!executed.at(executed.size()-1))
    //                 {
    //                     ex.go(lst.at(i + 1), lst.at(i + 2), states = false);
    //                     executed.push_back(states);
    //                 }
    //             }
                 
    //             else if(lst.at(i) == ";")
    //             {
    //                 ex.go(lst.at(i + 1), lst.at(i + 2), states = false);
    //                 executed.push_back(states);
    //             }
                
    //         }
    //     }
        
    // //executes subsequent command lines
    // while(lst.size() == 0 || lst.at(0) != "exit")
    // {
    //     lst.clear();
    //     cmd.start(lst);
    //     if(lst.size() != 0 && lst.at(0) != "exit")
    //     {
    //         if(lst.size() != 0)
    //         {
    //             if(lst.size() <= 6)
    //             {
    //                 if(lst.at(0) == "#")
    //                 {
                        
    //                 }
    //                 else if(lst.at(1) == "#")
    //                 {
    //                     ex.go(lst.at(0), states = false);
    //                 }
    //                 else if(lst.size() == 2)
    //                 {
    //                     ex.go(lst.at(0), states = false);
    //                 }
    //                 else
    //                 ex.go(lst.at(0),lst.at(1),states = false);
    //             }
    //             if(lst.size() > 6)
    //             {
    //                 if(lst.at(1) == "#")
    //                 {
    //                     ex.go(lst.at(0), states = false);
    //                 }
    //                 else if(lst.at(4) == "#")
    //                 {
    //                     ex.go(lst.at(3), states = false);
    //                 }
    //                 else if(lst.at(2) == "||")
    //                 {
    //                     //state = false;
    //                     ex.go(lst.at(0),lst.at(1), states);
    //                     //cout << states << endl;
    //                     bool stat = connect.ors(states);
    //                     //cout << stat << endl;
    //                     executed.push_back(stat);
    //                     if(stat)
    //                     {
    //                         ex.go(lst.at(3),lst.at(4),states = false);
    //                         executed.push_back(states);
    //                     }
    //                 }
    //                 else if(lst.at(2) == "&&")
    //                 {
    //                     ex.go(lst.at(0), lst.at(1), states);
    //                     //cout << states << endl;
    //                     bool stat = connect.ands(states);
    //                     //cout << stat << endl;
    //                     executed.push_back(stat);
    //                     if(!stat)
    //                     {
    //                         ex.go(lst.at(3),lst.at(4),states = false);
    //                         executed.push_back(states);
    //                     }
    //                 }
    //                 else if(lst.at(2) == ";")
    //                 {
    //                     ex.go(lst.at(0), lst.at(1), states = false);
    //                     executed.push_back(states);
    //                     ex.go(lst.at(3), lst.at(4), states = false);
    //                     executed.push_back(states);
    //                 }
    //             }
    //         }
    //         if(lst.size() > 7)
    //         {
    //             //cout << lst.at(6) << endl;
    //             for (unsigned int i = 5; i < lst.size()-2; i = i + 3)
    //             {
    //                 if(lst.at(i+ 2) == "#")
    //                 {
    //                     ex.go(lst.at(i + 1), states = false);
    //                     return 0;
    //                 }
    //                 else if(lst.at(i) == "||")
    //                 {
    //                     if(executed.at(executed.size()-1))
    //                     {
    //                         ex.go(lst.at(i + 1), lst.at(i + 2), states = false);
    //                         executed.push_back(states);
    //                     }
    //                 }
                    
    //                 else if(lst.at(i) == "&&")
    //                 {
    //                     if(!executed.at(executed.size()-1))
    //                     {
    //                         ex.go(lst.at(i + 1), lst.at(i + 2), states = false);
    //                         executed.push_back(states);
    //                     }
    //                 }
                     
    //                 else if(lst.at(i) == ";")
    //                 {
    //                     ex.go(lst.at(i + 1), lst.at(i + 2), states = false);
    //                     executed.push_back(states);
    //                 }
                    
    //             }
    //         }
    //         }
            
    //     }
   
    return 0;
}