#ifndef __MYDLL_H__
#define __MYDLL_H__

#define DLL_API __declspec(dllexport)

extern "C"
{
    DLL_API  int  dllmax(int, int);
    DLL_API  int  dlladd(int, int);
}

#endif