#include<windows.h>
#include<iostream>
#include<string>
#include <io.h>
#include<fcntl.h> 
#include "dll.h"
using namespace std;

int main()
{

#if 1
    char buffer[100];
    _setmaxstdio(64);
    cout << sizeof(FILE) << endl;
    for (int i = 1; i <= 6000; i++)
    {
        cout << i << endl;
        if (i == 509)
            cout << "please focus" << endl;
        FILE *fp = fopen("file.txt", "rb");
        //setbuf(fp, NULL);
        setvbuf(fp, buffer, _IONBF, 100);
        int fd = _fileno(fp);                  //文件描述符
        _lseek(fd, 0, SEEK_SET);
        FILE *f = _fdopen(fd, "rb");
        if (NULL == f)
        {
            int e = errno;
            cout << i << endl;
            cout << "the error code is " << e << endl;
        }

        //fprintf(f, "%d\n", i);
        //fflush(f);
        int closeFlag = fclose(f);
        //if (closeFlag == 0)
        //{
        //    cout << "close successfully" << endl;
        //}
        if (closeFlag == EOF)
        {
            cout << "close failed" << endl;
        }

        //fclose(fp);
        Sleep(50);
    }
#endif

#if 0
        _setmaxstdio(64);
        for (int i = 1; i <= 600; i++)
        {
            cout << i << endl;
            if (i == 509)
                cout << "please focus" << endl;
            int fileFd = _open("file.txt", O_RDWR);
            FILE *fp = _fdopen(fileFd, "rb");

            int fd = _fileno(fp);                  //文件描述符
            FILE *f = _fdopen(fd, "rb");

            if (NULL == f)
            {
                int e = errno;
                cout << i << endl;
                cout << "the error code is " << e << endl;
            }

            fprintf(f, "%d\n", i);
            //fflush(f);
            int closeFlag = fclose(f);
            //if (closeFlag == 0)
            //{
            //    cout << "close successfully" << endl;
            //}
            if (closeFlag == EOF)
            {
                cout << "close failed" << endl;
            }

            fclose(fp);
            Sleep(50);
    }
#endif
}
