// BayesianTextCategorization.cpp : �������̨Ӧ�ó������ڵ㡣
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
        printf("\n\n 1: ������������ļ�\n 2: ���³�ȡ�����ı�\n 3: �Էִ�����Ԥ����\n\n");
        int model;
        do
        {
            printf(" ���빦�����: ");
            scanf("%d", &model);
        }
		while ( model != CLASSIFY && model != REEXTRACT && model != PREPROCESS);

		if ( model == REEXTRACT )   {
            ExtractSample extract(".\\test\\");

            printf("���ڻָ������ı�...\n");
            extract.RestoreSample();
            float percent;

            do
            {
                printf(" �����ȡ�����İٷֱ�: ");
                scanf("%f", &percent);
            }
            while ( percent > 100 || percent < 0 );

            printf(" ���ڼ����Ƶ(TF)���ĵ�Ƶ��(DF) \n");
            extract.ExtractInPercent( percent );
            Df_Tf_Counter counter;
            counter.ComputeAllClasses(".\\test\\");

            printf(" ���ڼ��㿨��ͳ����... \n");
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

