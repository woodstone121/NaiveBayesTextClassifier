#ifndef  BAYESIAN_TEXTCLASSIFY_H_
#define  BAYESIAN_TEXTCLASSIFY_H_

#include <map>
#include <set>
#include <stdint.h>
#include <fstream>
#include <string>
#include <algorithm>
#include <vector>
#include <stdio.h>

#include "FileScanner.hpp"

#define INVALID -1
#define VSMLEN 2000

using namespace std;

/*
 *      P( Ci | Doc ) = P( Doc | Ci ) * P( Ci ) / P( Doc )
 *      Using logE Probility
 */

class TextClassify : public DirScanner
{
protected :
	int                classSize;
	map<string, int>   *TfMap;
	set<string>        stopWordsSet;
	uint32_t           totalDocs;
	uint32_t           *classDocs;
	double             *Prob_Ci;         // P( Ci )
	double             *Prob_Doc_Ci;     // P( Doc | Ci )
    double             *sumTf;
	double             maxsumTf;

public :
	TextClassify():  TfMap(nullptr), Prob_Ci(nullptr),classDocs(nullptr) ,
		             sumTf(nullptr), Prob_Doc_Ci(nullptr) { };
	~TextClassify();
private :
	inline bool     loadDict(const char *filePath, int ID);
	inline int      matchOneString(const string &str, int ID);
	bool            ClassifyOneFile(const char *filePath, vector<int> &vClassID);
	bool            InitClassID(const char *dirPath);
	
public :
	bool            ClassifyAllFiles(const char *dirPath);
	bool            loadVsmDataHost(const char *dirPath);
	bool            loadStopWords(const char *vDirPath);

};


TextClassify::~TextClassify()
{
	if (sumTf != nullptr)       delete []sumTf;
	sumTf = nullptr;
	if (Prob_Doc_Ci !=nullptr)  delete []Prob_Doc_Ci;
	Prob_Doc_Ci = nullptr;
	if (Prob_Ci != nullptr)     delete []Prob_Ci;
	Prob_Ci = nullptr;
	if (classDocs != nullptr)   delete []classDocs;
	classDocs = nullptr;
	if (TfMap != nullptr)       delete []TfMap;
	TfMap = nullptr;
}


bool TextClassify::loadStopWords(const char *dirPath)
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


bool TextClassify::ClassifyAllFiles(const char *dirPath)
{
	int ID;
	maxsumTf = 0;
	for (ID=0; ID < classSize; ID++) {
		Prob_Ci[ID] = abs(log( classDocs[ID] / double(totalDocs )));
		sumTf[ID] = 0;
		for (auto elem : TfMap[ID]) {
			maxsumTf = (maxsumTf < elem.second) ? elem.second : maxsumTf; 
			sumTf[ID] += elem.second;
		}
	}
	for (ID =0; ID < classSize; ID++) {
	    auto len = vFilePath.size();
		ScanFilesInDir(vDirPath[ID].c_str(), "*.sample");
		//vFilePath.resize(len + (vFilePath.size()-len)/9); 
	}
	if (vFilePath.size() < 1 ) return false;
	vector<int> vClassID;
        for (auto elem : vFilePath) {
		printf("Processing %s\n" , elem.c_str() );
		ClassifyOneFile(elem.c_str(), vClassID);
	}
	printf("exporting result\n");
	
	ofstream outFilefs(".\\ClassifyResult.txt");
	if (!outFilefs) return false;
	for (int i=0; i < vFilePath.size(); i++) {
		outFilefs << vFilePath[i] << " " << vDirPath[vClassID[i]] << endl;
	}
    outFilefs.close();
	
	return true;
}

bool TextClassify::ClassifyOneFile(const char *filePath, vector<int> &vClassID)
{
	ifstream inFilefs(filePath);
	if (!inFilefs)   return false;
	int ID, tf;
	memset( Prob_Doc_Ci, 0 , sizeof(Prob_Doc_Ci[0]) * classSize);
	string word;
	double temp;
	while ( !inFilefs.eof() ) {
		inFilefs >> word;
		if ( stopWordsSet.find(word) != stopWordsSet.end() )   continue;
		for (ID=0; ID < classSize; ID++) {
		     tf = matchOneString(word, ID);
			 temp = (tf != 0 ) ? abs( log( ( 1 + tf )/( VSMLEN + sumTf[ID] ) ) ) : 
				                 abs( log( 1 / (2 + VSMLEN + maxsumTf) ) );  //Êý¾ÝÆ½»¬
			 Prob_Doc_Ci[ID] += temp;
		}
	}
	inFilefs.close();
	double Min = DBL_MAX;
	int index = 0;
	for (ID=0; ID < classSize; ID++) {
		temp = Prob_Ci[ID] + Prob_Doc_Ci[ID];
		if ( temp < Min ) {
			Min = temp;
			index = ID;
		}
	}
	vClassID.push_back(index);
	return true;
}

bool TextClassify::InitClassID(const char *dirPath)
{
	ScanAllDirs(".\\test\\");
	if (vDirPath.size() < 1) return false;
	classSize = vDirPath.size();
	TfMap = new map<string,int>[classSize];
	classDocs = new uint32_t[classSize];
	Prob_Ci = new double[classSize];
	Prob_Doc_Ci = new double[classSize];
	sumTf = new double[classSize];

	return true;
}

bool TextClassify::loadVsmDataHost(const char *dirPath)
{
	if ( !InitClassID(dirPath) ) return false;
	char dataPath[MAXPATH];
	sprintf(dataPath, "%stotalTxts.txt", dirPath);
	ifstream inFilefs(dataPath);
	if (inFilefs) {
		inFilefs >> totalDocs;
		inFilefs.close();
	}
	memset( dataPath, 0, sizeof( dataPath ) );
	for (int id=0; id < classSize; id++ ) {  // from base class
		sprintf( dataPath, "%sthisClassTxts.txt", vDirPath[id].c_str() );
		ifstream infs(dataPath);
		if (infs) {
			infs >> classDocs[id];
			infs.close();
		}
		memset( dataPath, 0 ,sizeof(dataPath) );
		sprintf(dataPath, "%sDf_Tf_Chi_Data.txt", vDirPath[id].c_str());
		printf("loading %s\n" ,dataPath);
	    loadDict(dataPath, id);
		memset( dataPath, 0 , sizeof( dataPath ) );
	}
	return true;
}


bool TextClassify::loadDict(const char *filePath, int ID)
{
	ifstream ifs(filePath);
	if ( ! ifs ) {
		return false;
	}
	string word;
	int w, tf;
	for (int i=0; i < VSMLEN && !ifs.eof(); i++) {
		ifs >> word >> w >> tf;
		TfMap[ID].insert(make_pair(word, tf));
		ifs.ignore(128, '\n');
	}
	ifs.close();
	return true;
}


int TextClassify::matchOneString(const string &str, int ID)
{
	if (ID >= classSize ) {
	    return 0;
	}
	if (TfMap[ID].find(str) != TfMap[ID].end()) {
		return TfMap[ID][str];
	}
	return 0;
}


#endif
