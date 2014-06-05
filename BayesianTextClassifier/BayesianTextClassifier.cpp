// BayesianTextCategorization.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "TextClassify.hpp"
#include "PreprocessClassTxts.hpp"

int _tmain(int argc, _TCHAR* argv[])
{
	const char CLASSIFY = 1;
	const char REEXTRACT = 2;
	const char PREPROCESS = 3;
    while (true)
    {
        printf("\n\n 1: 分类测试样本文件\n 2: 重新抽取测试文本\n 3: 对分词文章预处理\n\n");
        int model;
        do
        {
            printf(" 输入功能序号: ");
            scanf("%d", &model);
        }
		while ( model != CLASSIFY && model != REEXTRACT && model != PREPROCESS);

		if ( model == REEXTRACT )   {
            ExtractSample extract(".\\test\\");

            printf("正在恢复测试文本...\n");
            extract.RestoreSample();
            float percent;

            do
            {
                printf(" 输入抽取样本的百分比: ");
                scanf("%f", &percent);
            }
            while ( percent > 100 || percent < 0 );

            printf(" 正在计算词频(TF)与文档频率(DF) \n");
            extract.ExtractInPercent( percent );
            Df_Tf_Counter counter;
            counter.ComputeAllClasses(".\\test\\");

            printf(" 正在计算卡方统计量... \n");
            CHICompute chicompute;
            chicompute.ComputeAllClassChi(".\\test\\");
        } else if (model == CLASSIFY )  {
            TextClassify textClassify;

            if ( textClassify.loadVsmDataHost(".\\test\\") &&
                    textClassify.loadStopWords(".\\test\\") )
            {
                textClassify.ClassifyAllFiles(".\\test\\");
            }

            system("notepad ClassifyResult.txt");
		} else if (model == PREPROCESS) {
			Preprocess processman;
			processman.ProcessAllClasses(".\\test\\");
		} else {
        system("cls");

		}
    }
	return 0;
}

