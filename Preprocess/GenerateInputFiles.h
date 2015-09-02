//#pragma once
#ifndef _GENERATEINPUTFILES_H_
#define _GENERATEINPUTFILES_H_

#include <stdio.h>
#include <string>
#include "LandsatInfo.h"
#include "Utility.h"
#include "LandsatProcess.h"

#define MAX_STR_LEN 1000
#define MAX_NPAIRS  5    /* maximum number of input pairs (not including output pair)*/
#define MAX_TIME_SERIES_LENGTH 1000 // ʱ�����е���󳤶�

class CGenerateInputFiles
{
public:
	CGenerateInputFiles(void);
	~CGenerateInputFiles(void);
	int Processing(std::string sParameterFile, CLandsatinfo* pLandsatInfo,std::string sOutputPath="");
	int Generating();

	// �����������������Landsat��MODISʱ��ƥ���ļ����н�����Ȼ������input�ļ��ķ�ʽ
	int Processing2(std::string sParameterFile, CLandsatinfo* pLandsatInfo,std::string sOutputPath="");
	int Generating2();


private:
	int m_naPredictPattern[5+MAX_NPAIRS] ;
	int m_naTimeSeries[MAX_TIME_SERIES_LENGTH][4] ;
	CLandsatinfo* m_pLandsatInfo ;
	std::string m_sOutputPath ;

public:
	int GetSeasonMidDay(int naTimeSeries[][4], int nActualLen, int naStartPreTime[], int &nStartPreTimeLen);
};


#endif