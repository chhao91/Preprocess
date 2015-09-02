#include "LandsatProcess.h"

CLandsatProcess::CLandsatProcess(void)
{
	m_pLandsatInfo = NULL ;
}

CLandsatProcess::~CLandsatProcess(void)
{
	delete m_psvBandNames ;
}

int CLandsatProcess::Initialize(CLandsatinfo* pLandsatInfo,std::string sRootPath, std::string sOutputPath)
{
	// 默认赋值，如果需要更改，则通过set函数修改（在Initialize之后）
	m_sFileSuffix = "*.tif" ;
	std::string saBandNames[] = {"sr_band1","sr_band2","sr_band3","sr_band4","sr_band5","sr_band7","cfmask"} ;
	m_psvBandNames = new std::vector<std::string>(saBandNames, saBandNames+7) ; // 这个数字7待修正

	if (!pLandsatInfo)
	{
		return false ;
	}
	m_pLandsatInfo = pLandsatInfo ;

	m_sRootPath = sRootPath ;
	if (!sOutputPath.empty())
	{
		m_sOutputPath = sOutputPath ;
	}

	daRect[0] = pLandsatInfo->m_daCornerUL[0] ;
	daRect[1] = pLandsatInfo->m_daCornerLR[0] ;
	daRect[2] = pLandsatInfo->m_daCornerUL[1] ;
	daRect[3] = pLandsatInfo->m_daCornerLR[1] ;

	return 0 ;
}

int CLandsatProcess::Processing()
{
	std::vector<std::string> folderVect, fileVect ; 
	std::string sSuffixFolder = "*.*" ;
	std::string sAddPath ;

	struct _finddata_t folder, file;
	long hFolder, hFile ;
	std::string sPathFolder = m_sRootPath + sSuffixFolder; // sPathLast = "c:\test\*.*"
	hFolder = _findfirst(sPathFolder.c_str(), &folder);
	if(hFolder == -1)
	{
		std::cout<<"目录不存在.\n"<<std::endl;
		return -1 ;
	}
	_findnext(hFolder, &folder) ;
	while(_findnext(hFolder, &folder) == 0)
	{
		sAddPath = m_sRootPath + folder.name + "\\" ;
		folderVect.push_back(sAddPath) ;
	}
	unsigned int i, j ;
	for (i=0; i<folderVect.size(); i++)
	{
		std::cout<< std::endl << "Folder_"<< i << ": " << folderVect.at(i)  ;
		std::string sPathFolder = folderVect.at(i) + m_sFileSuffix ;
		hFile = _findfirst(sPathFolder.c_str(), &file);
		fileVect.clear() ;
		if(hFile == -1)
		{
			std::cout<<"文件不存在.\n"<<std::endl;
			return -1 ;
		}
		else
		{
			sAddPath = folderVect.at(i) + file.name;
			fileVect.push_back(sAddPath) ;
		}
		while(_findnext(hFile, &file) == 0)
		{
			sAddPath = folderVect.at(i) + file.name;
			fileVect.push_back(sAddPath) ;
		}
		for (j=0; j<fileVect.size(); j++)
		{			
			if ( ! MatchFileSuffix(fileVect.at(j), *m_psvBandNames) )
				continue ;
			std::cout<< std::endl << "   File_"<< j << ": " << fileVect.at(j) << "..." ;
			char* pszFileName = CUtility::GetFileName(fileVect.at(j).c_str()) ;
			std::string sFileName = FormatOutputFileName(std::string(pszFileName)) ;
			// 如果用户没有指定路径就输出至根目录的平级文件夹landsat_p下
			std::string sFileOutput ;
			if(m_sOutputPath.empty())
				sFileOutput = m_sRootPath + "landsat_p\\" + sFileName + ".img" ;
			else
				sFileOutput = m_sOutputPath + sFileName + ".img" ;

			CutImage(fileVect.at(j), sFileOutput, daRect) ;

			free(pszFileName) ;
		}

		std::cout<<std::endl ;
	}

	return 0 ;
}

int CLandsatProcess::MatchFileSuffix(std::string sFile, std::vector<std::string> &svBandNames)
{
	char *pszFileName = CUtility::GetFileName(sFile.c_str()) ;
	int i ;
	for (i=0; i<svBandNames.size(); i++)
	{
		const char *pszObj = svBandNames.at(i).c_str() ;
		char *pszBeginPos = strstr(pszFileName, pszObj) ;
		if (pszBeginPos!=NULL)
		{
			free(pszFileName) ;
			return 1 ;
		}
	}

	free(pszFileName) ;
	return 0 ;
}


int CLandsatProcess::CutImage(std::string sSrcFile, std::string sDstFile, double *pdVect) 
{
	GDALAllRegister() ;
	GDALDataset *poSrcDs = NULL ;
	poSrcDs = (GDALDataset *)GDALOpen(sSrcFile.c_str(), GA_ReadOnly) ;
	if (poSrcDs == NULL)
	{
		return -1 ;
	}

	// 获取原图像的基本信息
	int iCols = poSrcDs->GetRasterXSize() ;
	int iRows = poSrcDs->GetRasterYSize() ;
	int iBands = poSrcDs->GetRasterCount() ;
	GDALDataType dT = poSrcDs->GetRasterBand(1)->GetRasterDataType() ;
	double dGeoTrans[6] ;
	poSrcDs->GetGeoTransform(dGeoTrans) ;
	double dPixelX = abs(dGeoTrans[1]) ;
	double dPixelY = abs(dGeoTrans[5]) ;

	// 获取原图像覆盖范围
	double dSrcRect[4] ;
	dSrcRect[0] = dGeoTrans[0] ;
	dSrcRect[2] = dGeoTrans[3] ;
	CUtility::ImageRowCol2Projection(dGeoTrans, iCols, iRows, dSrcRect[1], dSrcRect[3]) ;

	double dValidRect[4] ;
	dValidRect[0] = pdVect[0] > dSrcRect[0] ? pdVect[0] : dSrcRect[0] ;
	dValidRect[1] = pdVect[1] < dSrcRect[1] ? pdVect[1] : dSrcRect[1] ;
	dValidRect[2] = pdVect[2] < dSrcRect[2] ? pdVect[2] : dSrcRect[2] ;
	dValidRect[3] = pdVect[3] > dSrcRect[3] ? pdVect[3] : dSrcRect[3] ;
	// 判断是否是有效区域
	if (dValidRect[0]>dValidRect[1] || dValidRect[2]<dValidRect[3])
	{
		return -1 ;
	}

	int colrow[4] ; // (0,2)有效区域左上角点对应原图行列号，(1,3)有效区域相对于输出图像行列号
	cv::Rect srcRect, dstRect ;
	CUtility::Projection2ImageRowCol(dGeoTrans, dValidRect[0], dValidRect[2], colrow[0], colrow[2]) ;
	srcRect.x = colrow[0] ;
	srcRect.y = colrow[2] ;
	srcRect.width  = static_cast<int>( ((dValidRect[1]-dValidRect[0]) + dPixelX/2)/dPixelX ) ;
	srcRect.height = static_cast<int>( ((dValidRect[2]-dValidRect[3]) + dPixelY/2)/dPixelY ) ;

	dstRect.width  = srcRect.width ;
	dstRect.height = srcRect.height ;
	// 然后计算目标图像所在矩阵的起始位置(x, y)
	double dDstGeoTrans[6] ;
	memcpy(dDstGeoTrans, dGeoTrans, 6*sizeof(double)) ;
	dDstGeoTrans[0] = pdVect[0] ;
	dDstGeoTrans[3] = pdVect[2] ;
	CUtility::Projection2ImageRowCol(dDstGeoTrans, dValidRect[0], dValidRect[2], colrow[1], colrow[3]) ;
	dstRect.x = colrow[1] ;
	dstRect.y = colrow[3] ;

	// 打开输出图像数据集，并且进行初始化为filledValue
	int cols, rows ;
	int i ;
	cols = static_cast<int>( (pdVect[1]-pdVect[0]+dPixelX/2)/dPixelX ) ;
	rows = static_cast<int>( (pdVect[2]-pdVect[3]+dPixelY/2)/dPixelY ) ;
	GDALDataset *poDstDs = GetGDALDriverManager()->GetDriverByName("ENVI")->Create(sDstFile.c_str(), cols, rows, iBands, dT, NULL) ;
	cv::Mat filledValueMat(rows, cols, CUtility::OpencvDataType(dT)) ;
	filledValueMat = FILLEDVALUE ;
	for (i=0; i<iBands; i++)
	{
		CUtility::RasterDataIO( poDstDs, i+1, GF_Write, cv::Rect(0,0,cols, rows), filledValueMat) ;
	}
	filledValueMat.release() ;

	// 开始取值，复制至指定位置
	cv::Mat validMat(srcRect.height, srcRect.width ,CUtility::OpencvDataType(dT)) ;
	for (i=0; i<iBands; i++)
	{
		CUtility::RasterDataIO( poSrcDs, i+1, GF_Read, srcRect, validMat ) ;
		// 判断是否是掩模图层
		if (! CUtility::OpencvDataType(dT))
		{
			validMat = (validMat==0) ;
			validMat /= 255 ;
		}
		CUtility::RasterDataIO( poDstDs, i+1, GF_Write, dstRect, validMat) ;
	}
	validMat.release() ;

	poDstDs->SetGeoTransform(dDstGeoTrans) ;

	const char *pszWkt = poSrcDs->GetProjectionRef() ;
	poDstDs->SetProjection(pszWkt) ;

	GDALClose((GDALDatasetH)poSrcDs) ;
	GDALClose((GDALDatasetH)poDstDs) ;

	return 0 ;
}


std::string CLandsatProcess::FormatOutputFileName(std::string sOutputFileName)
{
//LC81230322013308LGN00_B1
//LT50370372001168XXX02_sr_band1
//LE70370372012351EDC00_sr_band2
//	std::string sSensor = sOutputFileName.substr(2,1) ;
	std::string sTime = sOutputFileName.substr(9,7) ;
	std::string sFlag = sOutputFileName.substr(22,6) ;
	std::string sBand ;
	if (sFlag=="cfmask")
		sBand = sFlag ;
	else
		sBand = sOutputFileName.substr(25,5) ;

	std::string sFileName = "L" + sTime + "_" + sBand ;
	
	if( sTime.empty() || sBand.empty())
		return "" ;
	return sFileName ;
}
