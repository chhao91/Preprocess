//#pragma once
#ifndef _GENERATEQUICKVIEW_H_
#define _GENERATEQUICKVIEW_H_

#include <string>
#include <stdio.h>
#include "Utility.h"
#include "LandsatInfo.h"
#include "gdal_priv.h"
#include "opencv2/core/core.hpp"

#define MAX_TIME_SERIES_LENGTH 1000 // 时间序列的最大长度
#define MAX_STR_LEN 1000
#define FILLEDVALUE  -9999

class CGenerateQuickView
{
public:
	CGenerateQuickView(void);
	~CGenerateQuickView(void);

	int Processing(std::string sRoot, std::string sParameterFile, CLandsatinfo* pLandsatInfo,std::string sOutputPath="");

private:
	CLandsatinfo* m_pLandsatInfo ;
	std::string m_sOutputPath ;
	int m_naTimeSeries[MAX_TIME_SERIES_LENGTH][4] ;

public:
	int CutImages(std::string sSrcFolder, std::string sFileNamePrefix, int* nBands, std::string sDstFile, double *pdVect) ;
};

#endif