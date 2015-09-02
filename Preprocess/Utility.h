//#pragma once
#ifndef _UTILITY_H_
#define _UTILITY_H_

#include "opencv2/core/core.hpp"
#include "gdal_priv.h"


class CUtility
{
public:
	CUtility(void);
	~CUtility(void);

	static char* GetFileName(const char* fullpathname) ;
	static char* GetFilePath(const char* fullpathname) ;
	static int OpencvDataType(GDALDataType gdalDataType) ;
	static GDALDataType GdalDataType(int opencvDataType) ;
	static bool Projection2ImageRowCol(double *adfGeoTransform, double dProjX, double dProjY, int &iCol, int &iRow) ;
	static bool ImageRowCol2Projection(double *adfGeoTransform, int iCol, int iRow, double &dProjX, double &dProjY) ;
	static bool RasterDataIO(GDALDataset *pDataset, int gdalBandId, GDALRWFlag rwFlag, cv::Rect const &imgRect,cv::Mat &matrix) ;
};


#endif