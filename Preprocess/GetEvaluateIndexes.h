// #pragma once
#ifndef _GETEVALUATEINDEXES_H_
#define _GETEVALUATEINDEXES_H_

#include "LandsatInfo.h"
#include <string>
#include <stdio.h>
#include <math.h>
#include "gdal_priv.h"
#include "opencv2/core/core.hpp"

#define MAX_TIME_SERIES_LENGTH 1000 // 时间序列的最大长度
#define MAX_STR_LEN 1000
#define CLASSNUM 7  // 设定土地覆盖类型为7
#define MIMIMUMPIXELS 100  // 设定每个类别中像素最少是100

class CGetEvaluateIndexes
{
// 评价指标包括R2, RMSE, MAE, Slope, Intercept
// 评价包括landsat6个波段（1,2,3,4,5,7）


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