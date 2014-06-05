
#ifndef TEXTPREPROCESS_H
#define TEXTPREPROCESS_H

#include <stdio.h>
#include <fstream>
#include <map>
#include <set>
#include <string.h>
#include <cmath>
#include <string>
#include <vector>
#include <algorithm>  // for sort vector
#include <iomanip> // for setprecision

#include "FileScanner.hpp"


using namespace std;


/*
 *    将已分词的文本转换为： 词 词频 的文本格式
 *
 */

class Preprocess : public DirScanner
{
private:
    map<string, int> TfMap;
    int totalTxtFiles;
    bool ConvertTfFile(const char *filePath);

public:
    bool ProcessTxtsInDir(const char *Path);
    bool ProcessAllClasses(const char *Path);

};

/*
 *    抽取测试样本空间   
 *
 */

class ExtractSample  : public DirScanner
{
private:
	const char *ParentDirPath;
public:
	bool ExtractInPercent(float percent);
	bool RestoreSample();
	ExtractSample(const char *Path) {  ParentDirPath = Path; } ;
	ExtractSample() : ParentDirPath(nullptr) { };
	~ExtractSample() {  ParentDirPath = nullptr; };
	bool SetPath(const char *Path);
};

/*
 *    计算类别中所有词的卡方统计量，保存到Df_Tf_Chi_Data.txt
 *
 */

class CHICompute: public DirScanner
{
private:
	// map< term , pair< DF, Tf> >
    map<string , pair<int, int> > *TfMap;
    int *classDocs;
    int totalDocs;
    bool InitClassID(const char *DirPath);
    inline int QueryItemDf(const string &word, int ID) const;
    double ComputeItemChi(const string &word, int ID) const;

public:
	CHICompute(): TfMap(nullptr), totalDocs(0), classDocs(nullptr) {};
    virtual ~CHICompute();

public:
    bool ComputeOneClassChi(const char *subPath, int ID);
    bool ComputeAllClassChi(const char *DirPath);
    bool LoadDataHost(const char *DirPath);
    inline bool LoadDataHelper(const char *subPath, int ID);
};

/*
 *   计算类别中所有词的文档频率，并保存到Df_Tf_Data.txt
 *
 */

class Df_Tf_Counter : public DirScanner
{
private:
	// map< term, pair< Df, Tf> >
    map<string, pair<int, int> > *TfMap;
	set<string>    stopWordsSet;
    bool InitClassID(const char *DirPath);
	inline bool loadStopWords(const char *dirPath);

public:
    bool ComputeOneClass(const char *DirPath, int ID);
    bool ComputeAllClasses(const char *DirPath);
    void ExportAllDfData();
    virtual ~Df_Tf_Counter();
};

/*
int main()
{
    CHICompute chicompute;
    chicompute.ComputeAllClassChi(".\\");
}
*/


bool ExtractSample::RestoreSample()
{
	if (ParentDirPath == nullptr)    return false;
	ScanAllDirs(ParentDirPath);
	char newname[MAXPATH];
	if (vDirPath.size() < 1)  return false;
	for (auto elem : vDirPath ) {
		printf("Restoring sample to Tf in %s\n" , elem.c_str());
		vFilePath.clear();
		ScanFilesInDir(elem.c_str(), "*.sample");
		if (vFilePath.size() < 1)  continue;
		for (auto elem : vFilePath) {
			rename( elem.c_str() , elem.substr(0, elem.rfind('.')).c_str() );
		}
	}
	return true;
}

bool ExtractSample::ExtractInPercent(float percent)
{
	if (ParentDirPath == nullptr)    return false;
	ScanAllDirs(ParentDirPath);
	char newname[MAXPATH];
	if (vDirPath.size() < 1)  return false;
	for (auto elem : vDirPath ) {
		printf("Extracting sample in %s\n", elem.c_str() );
		vFilePath.clear();
		ScanFilesInDir(elem.c_str(), "*.TF");
		if (vFilePath.size() < 1)  continue;
		for (int i=0; i < int(vFilePath.size()*percent); i++) {
			sprintf(newname, "%s.sample", vFilePath[i].c_str());
			rename(vFilePath[i].c_str(), newname);
		}
	}

	return true;
}


inline bool CmpDouble(const pair<pair<string, pair<int, int> >, double> &x,
        const pair<pair<string, pair<int, int> > ,double> &y)
{
    return x.second > y.second;
}

bool CHICompute::ComputeAllClassChi(const char *DirPath)
{
    if (!LoadDataHost(DirPath)) return false;
    for (int i=0; i < vDirPath.size(); i++) {
        printf("computing %s\n", vDirPath[i].c_str() );
        ComputeOneClassChi(vDirPath[i].c_str(), i);
    }
    return true;
}

bool CHICompute::ComputeOneClassChi(const char *subPath, int ID)
{
	char destPath[MAXPATH];
    sprintf(destPath, "%sDf_Tf_Chi_Data.txt", subPath);

    double chi;
    vector<pair< pair<string, pair<int, int> >, double> > vTemp;
    for (auto elem : TfMap[ID]) {
        chi = ComputeItemChi(elem.first, ID);
        vTemp.push_back( make_pair(elem ,chi) );
    }
    sort(vTemp.begin(), vTemp.end(), CmpDouble);
    ofstream outFilefs(destPath);
    if (!outFilefs) return false;
    for (auto elem : vTemp) {
		outFilefs << elem.first.first << " " << elem.first.second.first
			      << " " << elem.first.second.second << " " 
			      <<setprecision(15) << elem.second << endl;
    }
    outFilefs.close();
    return true;
}

/*
 *  A 属于分类 包含特征
 *  B 不属于分类 包含特征
 *  C 属于分类 不包含特征
 *  D 不属于分类 不包含特征
 *  采用对数log计算
 */

double CHICompute::ComputeItemChi(const string &word, int ID) const
{
    int A = QueryItemDf(word, ID);
    int C = classDocs[ID] - A;
    int B = 0;
    int size = vDirPath.size();
    int index = (ID + 1)%size;
    while (index != ID) {
        B += QueryItemDf(word, index);
        index = (index + 1)%size;
    }
    int D = totalDocs - classDocs[ID] - B;
    double num = log(totalDocs+1) + 2* log(abs(A*D - B*C+1));
    double der = log(A+C+1) + log(A+B+1) + log(B+D+1) + log(C+D+1);
    return num - der;
}

inline int CHICompute::QueryItemDf(const string &word, int ID) const
{
    if (ID >= vDirPath.size()) return 0;
    auto iter = TfMap[ID].find(word);
    if (iter != TfMap[ID].end()) {
		return iter->second.first;
    } else {
        return 0;
    }
}

inline bool CHICompute::LoadDataHelper(const char *subPath, int ID)
{
     char destPath[MAXPATH];
     sprintf(destPath, "%sthisClassTxts.txt", subPath);
     ifstream inFilefs(destPath);
     if (!inFilefs) {
        return false;
     }
     inFilefs >> classDocs[ID];
     inFilefs.close();
     memset(destPath, 0, sizeof(destPath));
     sprintf(destPath, "%sDf_Tf_Data.txt", subPath);
     ifstream inDatafs(destPath);
     if (!inDatafs) {
        return false;
     }
     string word;
     int df, tf;
     while (!inDatafs.eof()) {
        inDatafs >> word >> df >> tf;
        TfMap[ID].insert( make_pair(word, make_pair(df, tf) ) );
     }
     inDatafs.close();
     return true;
}

bool CHICompute::LoadDataHost(const char *DirPath)
{
    if ( !InitClassID(DirPath) )  return false;
    if (DirPath == nullptr)  return false;
    char destPath[MAXPATH];
    sprintf(destPath, "%stotalTxts.txt", DirPath);
    ifstream inFilefs(destPath);
    if (!inFilefs) {
       return false;
    }
    inFilefs >> totalDocs;
    inFilefs.close();
    for (int i=0; i < vDirPath.size(); i++) {
        printf("loading %s\n" , vDirPath[i].c_str() );
        LoadDataHelper(vDirPath[i].c_str(), i);
    }
    return true;
}

bool CHICompute::InitClassID(const char *DirPath)
{
    if (DirPath == nullptr) {
        return false;
    }
    ScanAllDirs(DirPath);
    ofstream outFileFs("classID.txt");
    if (!outFileFs || vDirPath.size() == 0) {
        return false;
    }
    TfMap = new map<string , pair<int, int> >[vDirPath.size()];
    classDocs = new int[vDirPath.size()];
    for (int i=0; i < vDirPath.size(); i++) {
        outFileFs << i << " " << vDirPath[i] << endl;
    }
    outFileFs.close();
    return true;
}


CHICompute::~CHICompute()
{
    if (TfMap != nullptr) {
        delete []TfMap;
    }
    if (classDocs != nullptr) {
        delete []classDocs;
    }
}


inline int cmp(const pair<string, pair<int, int> > &x,
        const pair<string, pair<int, int> > &y)
{
	return x.second.first > y.second.first;
}

void Df_Tf_Counter::ExportAllDfData()
{
    vector<pair<string, pair<int, int> > > vTemp;
    char destPath[MAXPATH];
    for (int i=0; i < vDirPath.size(); i++) {
		printf("exporting %s\n" ,vDirPath[i].c_str() );
        vTemp.clear();
        for (auto elem : TfMap[i]) {
            vTemp.push_back(elem);
        }
        sort(vTemp.begin(), vTemp.end(), cmp);
        sprintf(destPath, "%sDf_Tf_Data.txt", vDirPath[i].c_str());
        ofstream outFileFs(destPath);
        if (!outFileFs) continue;
        for (auto elem : vTemp) {
			outFileFs << elem.first << " " << elem.second.first
				      << " " << elem.second.second << endl;
        }
        outFileFs.close();
        memset(destPath, 0, sizeof(destPath));
    }
}

bool Df_Tf_Counter::loadStopWords(const char *dirPath)
{
	char srcPath[MAXPATH];
	sprintf(srcPath, "%sstopwords.txt", dirPath);
	ifstream infilefs(srcPath);
	if (! infilefs)    return    false;
	string word;
	while (! infilefs.eof() ) {
		infilefs >> word;
		stopWordsSet.insert(word);
		infilefs.ignore(128, '\n');
	}
	return true;
}

bool Df_Tf_Counter::ComputeAllClasses(const char *DirPath)
{
	if ( !InitClassID(DirPath) || !loadStopWords(DirPath) )   
		return false;
    if (vDirPath.size() < 1) return false;
    for (int i=0; i < vDirPath.size(); i++) {
        printf("computing Df and Tf in directionary: %s\n" , vDirPath[i].c_str() );
        ComputeOneClass(vDirPath[i].c_str(), i);
    }
	ExportAllDfData();
	return true;
}

bool Df_Tf_Counter::ComputeOneClass(const char *DirPath, int ID)
{
	vFilePath.clear();
	ScanFilesInDir(DirPath, "*.TF");
    if (vFilePath.size() < 1) return false;
    map<string , pair<int, int> >::iterator iter;
    string word;
	int Tf;
    for (auto elem : vFilePath) {
        ifstream inFileFs(elem.c_str());
        if (!inFileFs) continue;
        while (!inFileFs.eof()) {
            inFileFs >> word >> Tf;
			// skip stop word
			if ( stopWordsSet.find(word) != stopWordsSet.end() ) continue;
            iter = TfMap[ID].find( word );
            if (iter == TfMap[ID].end() ) {
                TfMap[ID].insert( make_pair(word, make_pair(1, Tf) ));
            } else {
				iter->second.first++;
				iter->second.second += Tf;
            }
            inFileFs.ignore(128, '\n');
        }
    }
    return true;
}

Df_Tf_Counter::~Df_Tf_Counter()
{
    if (TfMap != nullptr) {
        delete []TfMap;
    }
}

bool Df_Tf_Counter::InitClassID(const char *DirPath)
{
    if (DirPath == nullptr) {
        return false;
    }
    ScanAllDirs(DirPath);
    ofstream outFileFs("classID.txt");
    if (!outFileFs || vDirPath.size() == 0) {
        return false;
    }
    TfMap = new map<string , pair<int, int> >[vDirPath.size()];
    for (int i=0; i < vDirPath.size(); i++) {
        outFileFs << i << " " << vDirPath[i] << endl;
    }
    outFileFs.close();
    return true;
}


bool Preprocess::ProcessAllClasses(const char *Path)
{
    totalTxtFiles = 0;
    ScanAllDirs(Path);
    for (auto elem : vDirPath) {
		printf("deal with: %s\n", elem.c_str() );
        ProcessTxtsInDir(elem.c_str());
    }
	char outPath[MAXPATH];
	sprintf(outPath, "%s\\totalTxts.txt", Path);
	ofstream outFileFs(outPath);
    if (outFileFs) {
        outFileFs << totalTxtFiles << endl;
        outFileFs.close();
    }
    return true;
}

bool Preprocess::ProcessTxtsInDir(const char *Path)
{
    if (Path == nullptr) {
        return false;
    }
    ScanFilesInDir(Path, "*.txt");
    char destfile[MAXPATH];
    sprintf(destfile, "%s\\thisClassTxts.txt", Path);
    ofstream outFileFs(destfile);
    if (outFileFs) {
        outFileFs << vFilePath.size() << endl;
        outFileFs.close();
    }
    for (auto elem : vFilePath)
    {
        ConvertTfFile(elem.c_str());
        totalTxtFiles++;
    }
}

bool Preprocess::ConvertTfFile(const char *filePath)
{
    if (filePath == nullptr) {
        return false;
    }
    ifstream inFileFs(filePath);
    if (!inFileFs) {
        return false;
    }
    TfMap.clear();
    string words;
    map<string, int>::iterator iter;
    while (!inFileFs.eof()) {
        inFileFs >> words;
        iter = TfMap.find(words);
        if ( iter == TfMap.end() ) {
            TfMap.insert( make_pair(words, 1) );
        } else {
            iter->second++;
        }
    }
    inFileFs.close();
    //remove(filePath);
    char destFile[MAXPATH];
    sprintf(destFile, "%s.TF", filePath);
    ofstream outFileFs(destFile);
    if (! outFileFs) {
        return false;
    }
    for (auto elem : TfMap) {
        outFileFs << elem.first << " " << elem.second << endl;
    }
    outFileFs.close();
    return true;
}


#endif
