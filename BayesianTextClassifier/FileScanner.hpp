
#ifndef FILESCANNER_H
#define FILESCANNER_H

#include <vector>
#include <string>
#include <string.h>
#include <io.h>
#include <direct.h>

#define MAXPATH 256

using namespace std;


class DirScanner
{
protected:
    vector<string> vFilePath;
    vector<string> vDirPath;
    bool ScanFilesInDir(const char *Path, const char *dest);
    bool ScanAllDirs(const char *Path);

};


bool DirScanner::ScanAllDirs(const char *Path)
{
   long Handle;
   char _Path[MAXPATH] = {0};
   vDirPath.clear();
   sprintf(_Path, "%s*", Path);
   struct _finddata_t FileInfo;
   if((Handle=_findfirst(_Path,&FileInfo))==-1L)
   {
	   return -1;
   }
   char _Path_new[MAXPATH];
   do
   {
	   // 为目录，且不是 .. .
	   if (!(FileInfo.name[0] == '.') &&
		   (FileInfo.attrib & _A_SUBDIR))
	   {
		   strcpy(_Path_new,Path);
		   strcat(_Path_new,FileInfo.name);
		   strcat(_Path_new,"\\");
		   vDirPath.push_back(_Path_new);
		   memset(_Path_new, 0 , sizeof(_Path_new));
	   }
   }while( _findnext(Handle,&FileInfo)==0);
   _findclose(Handle);
   return true;
}


bool DirScanner::ScanFilesInDir(const char *Path, const char *dest)
{
  // vFilePath.clear();
   long Handle;
   char _Path[MAXPATH] = {0};
   sprintf(_Path, "%s%s", Path, dest);
   struct _finddata_t FileInfo;
   if((Handle=_findfirst(_Path,&FileInfo))==-1L)
   {
	   return -1;
   }
   char _Path_new[MAXPATH];
   do
   {
	   if (!(FileInfo.name[0] == '.') &&
		   !(FileInfo.attrib & _A_SUBDIR))
	   {
		   strcpy(_Path_new,Path);
		   strcat(_Path_new,FileInfo.name);
		   vFilePath.push_back(_Path_new);
		   memset(_Path_new, 0 , sizeof(_Path_new));
	   }
   }while( _findnext(Handle,&FileInfo)==0);
   _findclose(Handle);
   return true;
}

#endif