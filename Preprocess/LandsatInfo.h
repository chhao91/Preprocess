//#pragma once
#ifndef _LANDSATINFO_H_
#define _LANDSATINFO_H_

#include <string>
#include "tinyxml.h"
#include "tinystr.h"
#include "gdal_priv.h"
#include <ogr_spatialref.h>
#include "Utility.h"

class CLandsatinfo
{
public:
	CLandsatinfo(void);
	~CLandsatinfo(void);


	std::string m_sProjection ;
	std::string m_sDatum ;
	std::string m_sUnit ;
	std::string m_sGridOrigin ;
	double  m_daCornerUL[2] ; // 0:x, 1:y
	double  m_daCornerLR[2] ;
	double m_dPixelXSize, m_dPixelYSize ;
	int    m_nZone ;
	int    m_nCols, m_nRows, m_nFilledValue ;

	int InitializeXML(std::string sPath);
	int InitializeENVI(std::string sPath) ;
};


#endif