//#pragma once
#ifndef _LANDSATPROCESS_H_
#define _LANDSATPROCESS_H_

#include <string>
#include <vector>
#include <iostream>
#include <io.h>
#include "LandsatInfo.h"
#include "Utility.h"

#include "gdal_priv.h"
#include "opencv2/core/core.hpp"

#define FILLEDVALUE  -9999

class CLandsatProcess
{
public:
	CLandsatProcess(void);
	~CLandsatProcess(void);

	// 
	int Initialize(CLandsatinfo* pLandsatInfo,std::string sRootPath, std::string sOutputPath="") ;
	int Processing() ;

	std::string m_sRootPath, m_sOutputPath, m_sFileSuffix ;
	std::vector<std::string> *m_psvBandNames ;

	double daRect[4] ;

	CLandsatinfo* m_pLandsatInfo ;

private:
	int MatchFileSuffix(std::string sFile, std::vector<std::string> &svBandNames) ;
	int CutImage(std::string sSrcFile, std::string sDstFile, double *pdVect) ;



public:
	std::string FormatOutputFileName(std::string sOutputFileName);
};

#endif


