#include "lpr_alg.h"
#include <iostream>
#include <fstream>
#include <string>
#include <time.h>
using namespace std;

int main(int argc, char ** argv)
{
    if(argc < 2)
    {
        cout << argv[0] << " filename" << endl;
        return 0;
    }
    if(vlpr_init())
    {
        cout<<"LPR_ALG init OK"<<endl;
        std::ifstream file(argv[1], std::ios::in | std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            fprintf(stderr, "open file fail: %s\n", argv[1]);
            return -1;
        }
        
        std::streampos size = file.tellg();
        file.seekg(0, std::ios::beg);
        std::string buffer(size, ' ');
        file.read(&buffer[0], size);
        file.close();
        VPR vpr;
        for(int i=0;i<100;i++)
        {
            clock_t t=clock();
            if(vlpr_analyze((const unsigned char *)buffer.c_str(), (int)size, &vpr))
            {
                cout << "Time\t" << (clock()-t)/1000 << " ms" << endl;
                cout << "OK\t" << vpr.license << "\t" << vpr.color << "\t" << vpr.nColor << "\t" << vpr.nConfidence << endl;
                cout << "Coor\t"  << vpr.left << "\t" << vpr.top << "\t" << vpr.right << "\t" << vpr.bottom << endl;
            }else{
                cout << "Time\t" << (clock()-t)/1000 << " ms" << endl;
                cout << "Fail" << endl;
            }
        }
        
    }else{
        cout<<"LPR_ALG init Fail, Exit"<<endl;
    }
    
    return 0;
}
