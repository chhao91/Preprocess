// #pragma once
#ifndef _GETEVALUATEINDEXES_H_
#define _GETEVALUATEINDEXES_H_

#include "LandsatInfo.h"
#include <string>
#include <stdio.h>
#include <math.h>
#include "gdal_priv.h"
#include "opencv2/core/core.hpp"

#define MAX_TIME_SERIES_LENGTH 1000 // ʱ�����е���󳤶�
#define MAX_STR_LEN 1000
#define CLASSNUM 7  // �趨���ظ�������Ϊ7
#define MIMIMUMPIXELS 100  // �趨ÿ�����������������100

class CGetEvaluateIndexes
{
// ����ָ�����R2, RMSE, MAE, Slope, Intercept
// ���۰���landsat6�����Σ�1,2,3,4,5,7��


public:
	CGetEvaluateIndexes(void);
	~CGetEvaluateIndexes(void);

	int Precessing(std::string sRoot, std::string sLandsatPreFolder, std::string sParameterFile, CLandsatinfo* pLandsatInfo, int *pnStartNums, int num, std::string sOutputPath="");

private:
	CLandsatinfo *m_pLandsatInfo ;
	std::string m_sOutputPath ;
	int m_naTimeSeries[MAX_TIME_SERIES_LENGTH][4] ;

	int CalcEvaluateIndexes(std::string sRealPath, std::string sPrePath, std::string sMask, std::string sLandcover, int *pRowsCols, double dmEvaluateIndexes[][5]) ;
	int GetNearestLCYear(int nCurrentYear) ;
};


#endif