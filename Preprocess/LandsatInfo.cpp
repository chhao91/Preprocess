#include "LandsatInfo.h"

CLandsatinfo::CLandsatinfo(void)
{
}

CLandsatinfo::~CLandsatinfo(void)
{
}

/***********************************************************************************
* 现在函数返回统一采用false, true, 以后需要更改成返回预定义宏来指示返回错误类型
***********************************************************************************/
int CLandsatinfo::InitializeXML(std::string sPath)
{
	// sPath是否为空
	if (sPath.empty())
	{
		return false ;
	}

	//const char* filepath = "phonebookdata.xml";  
	//TiXmlDocument doc(filepath);  
	//bool loadOkay = doc.LoadFile();  


	// 创建Xml文档对象
	const char* pszPath = sPath.c_str() ;
	TiXmlDocument mDocument(pszPath);
	if (!mDocument.LoadFile())
	{ 
		return false;
	}

	// 获取根元素(即SystemSetting要素)
	TiXmlElement *pSystemSettingEle = mDocument.RootElement();
	if (!pSystemSettingEle)
	{
		return false;
	}

	// 获取HIPASLib要素，初始化HIPASLib行为
	TiXmlElement *pGlobalEle = pSystemSettingEle->FirstChildElement("global_metadata");
	if (!pGlobalEle)
	{
		return -1;
	}

	// 获取projection_information这个要素
	TiXmlElement *pProjctionEle = pGlobalEle->FirstChildElement("projection_information") ;
	m_sProjection = pProjctionEle->Attribute("projection") ;
	m_sDatum      = pProjctionEle->Attribute("datum") ;
	m_sUnit       = pProjctionEle->Attribute("units") ;

	// 获取子要素：UL LR corner_point, grid_origin, utm_proj_params
	TiXmlElement *pULCornerPointEle = pProjctionEle->FirstChildElement("corner_point") ;
	TiXmlElement *pLRCornerPointEle = pULCornerPointEle->NextSiblingElement("corner_point") ;
	TiXmlElement *pGrid_OriginEle = pULCornerPointEle->NextSiblingElement("grid_origin") ;
	TiXmlElement *pUtm_Proj_ParamsEle = pULCornerPointEle->NextSiblingElement("utm_proj_params") ;

	const char* pszULX = pULCornerPointEle->Attribute("x") ;
	const char* pszULY = pULCornerPointEle->Attribute("y") ;

	const char* pszLRX = pLRCornerPointEle->Attribute("x") ;
	const char* pszLRY = pLRCornerPointEle->Attribute("y") ;

	const char* pszGrid_Origin = pGrid_OriginEle->FirstChild()->Value() ;
	const char* pszZone_CodeEle = pUtm_Proj_ParamsEle->FirstChildElement("zone_code")->FirstChild()->Value() ;

	TiXmlElement *pBand_1 = pGlobalEle->NextSiblingElement("bands")->FirstChildElement("band") ;
	const char* pszLines = pBand_1->Attribute("nlines") ;
	const char* pszSamples = pBand_1->Attribute("nsamps") ;
	const char* pszFilledValue = pBand_1->Attribute("fill_value") ;
	const char* pszPixelXSize = pBand_1->FirstChildElement("pixel_size")->Attribute("x") ;
	const char* pszPixelYSize = pBand_1->FirstChildElement("pixel_size")->Attribute("y") ;

	// 成员变量初始化
	m_nCols = atoi(pszSamples) ;
	m_nRows = atoi(pszLines) ;
	m_daCornerUL[0] = atof(pszULX) ;
	m_daCornerUL[1] = atof(pszULY) ;
	m_daCornerLR[0] = atof(pszLRX) ;
	m_daCornerLR[1] = atof(pszLRY) ;
	m_sGridOrigin = pszGrid_Origin ;
	m_nZone = atoi(pszZone_CodeEle) ;
	m_nFilledValue = atoi(pszFilledValue) ;
	m_dPixelXSize = atof(pszPixelXSize) ;
	m_dPixelYSize = atof(pszPixelYSize) ;

	// 处理成真正的影像范围
	if ( "CENTER" == m_sGridOrigin)
	{
		m_daCornerUL[0] -= m_dPixelXSize/2 ;
		m_daCornerUL[1] += m_dPixelYSize/2 ;
		m_daCornerLR[0] += m_dPixelXSize/2 ;
		m_daCornerLR[1] -= m_dPixelYSize/2 ;
	}
	else
	{
		m_daCornerLR[0] += m_dPixelXSize ;
		m_daCornerLR[1] -= m_dPixelYSize ;
	}

	return 0;
}


int CLandsatinfo::InitializeENVI(std::string sPath)
{
	GDALAllRegister() ;
	GDALDataset* poSrcDS = NULL ;
	poSrcDS = 	(GDALDataset *)GDALOpen(sPath.c_str(), GA_ReadOnly) ;
	if(NULL==poSrcDS)
	{
		return -1 ;
	}

	// 获取图像信息
	double dGeoTrans[6] ;
	m_nCols = poSrcDS->GetRasterXSize() ;
	m_nRows = poSrcDS->GetRasterYSize() ;
	poSrcDS->GetGeoTransform(dGeoTrans) ;
	m_dPixelXSize = dGeoTrans[1] ;
	m_dPixelYSize = ABS(dGeoTrans[5]) ;
	m_daCornerUL[0] = dGeoTrans[0] ;
	m_daCornerUL[1] = dGeoTrans[3] ;	
	CUtility::ImageRowCol2Projection(dGeoTrans, m_nCols, m_nRows, m_daCornerLR[0], m_daCornerLR[1]) ;
	m_nFilledValue = -9999 ;
	std::string sSrcWKT = poSrcDS->GetProjectionRef() ;
	char* pszSrcWKT = const_cast<char*>(sSrcWKT.c_str()) ;

	// 设置输出图像的坐标系统 
	OGRSpatialReference oSRS;  
	oSRS.importFromWkt(&pszSrcWKT) ;
	if (oSRS.IsProjected())
	{
		m_nZone = oSRS.GetUTMZone(NULL) ;
		
		m_sProjection = "UTM" ;   // 强制赋值为UTM
		m_sGridOrigin = "UPPERLEFTER" ;  // 强制赋值为UPPERLEFTER
	}
	m_sUnit = oSRS.GetAttrValue("UNIT") ;
	std::string sDatum = oSRS.GetAttrValue("DATUM") ;
	if (sDatum=="WGS_1984")
	{
		m_sDatum = "WGS84" ;
	}

	GDALClose((GDALDatasetH)poSrcDS) ;
	return 0 ;
}
