#ifndef __DLL_H__
#define __DLL_H__
#include <iostream>

#define DLL_API __declspec(dllexport)

extern "C"
{
    DLL_API  void  dllopenfile(const char *);
}

#endif