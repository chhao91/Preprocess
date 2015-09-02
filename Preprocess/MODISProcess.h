//#pragma once
#ifndef _MODISPROCESS_H_
#define _MODISPROCESS_H_

#include <string>
#include <vector>
#include "LandsatInfo.h"
#include "Utility.h"
#include "LandsatProcess.h"
#include "LT_Envelope.h"

#include "gdalwarper.h"
#include <gdal_alg.h>
#include <gdal_priv.h>
#include <ogr_spatialref.h>

#include <iostream>
#include <stdio.h>

#define RE_NOFILE -1
#define RE_CREATEFILE -2
#define RE_PARAMERROR -3
#define RE_SUCCESS 0

#define BITNUM  32


class CMODISProcess
{
public:
	CMODISProcess(void);
	~CMODISProcess(void);

	int Initialize(CLandsatinfo* pLandsatInfo,std::vector<int>* pnvHdfBands, std::vector<int>* pnvOutputBands, int nMaskNum, std::string sRootPath, std::string sOutputPath="") ;
	int Processing() ;

private:
	std::string m_sRootPath, m_sOutputPath ;
	CLandsatinfo* m_pLandsatInfo ;
	std::vector<int>* m_pnvHdfBands ;
	std::vector<int>* m_pnvOutputBands ;
	int m_nMaskNum ;

	int ProcessModisBand(std::string sSrcFile, std::string sDstFile, int nBandNum) ;
	int CopyModisBand(std::string sSrcFile, std::string sDstFile, int nBandNum) ;
	int ReprojectMODIS(std::string sSrcFile, std::string sDstFile) ;
	int ResampleMODIS(std::string sSrcFile, std::string sOutFile, double dSizeX , double dSizeY, GDALResampleAlg nResampleMode,
			LT_Envelope* pExtent, int* pBandIndex, int *pBandCount, const char* pszFormat) ;
	int ProcessMODISMask(std::string sFileIn, std::string sFileOut) ;
	int TranslateMaskCode(std::string sFileIn, std::string sFileOut) ;
	//char Code2Mask(unsigned int n) ;
	char Code2Mask(unsigned short n) ; // 改成了16位的数据类型 -20150827
	int GetMODISMaskQuickview(std::string sSrcFile, std::string sOutFile, double dSizeX , double dSizeY, GDALResampleAlg nResampleMode,
		LT_Envelope* pExtent, int* pBandIndex, int *pBandCount, const char* pszFormat) ;

public:
	std::string FormatOutputFileName(std::string sFileName);
};

#endif