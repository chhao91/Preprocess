#include "GenerateQuickView.h"

CGenerateQuickView::CGenerateQuickView(void)
{

}

CGenerateQuickView::~CGenerateQuickView(void)
{
}

/*****************************************************************************************************************
* 从处理好的landsat和modis影像集中提取子空间范围的快视图；
* 从根目录中landsat_p和modis_p文件夹中分别读取待处理文件对其处理，在不指定输入路径sOutputPath的情况下，默认保存在
*     与输入参数文件相同路径下（sParameterFile）；
*****************************************************************************************************************/
int CGenerateQuickView::Processing(std::string sRoot, std::string sParameterFile, CLandsatinfo* pLandsatInfo,std::string sOutputPath)
{
	// 第一步：读取matchlist文件提取landsat和modis的“年”和“DOY”
	m_pLandsatInfo = pLandsatInfo ;
	if (sOutputPath.empty())
	{
		char* pszRootPath = CUtility::GetFilePath(sParameterFile.c_str()) ;
		m_sOutputPath = pszRootPath ;
		free(pszRootPath) ;
	}
	else
	{
		m_sOutputPath = sOutputPath ;
	}

	FILE* pFile = fopen(sParameterFile.c_str(), "r") ;
	if (NULL==pFile)
	{
		return false ;
	}

	char buffer[MAX_STR_LEN] = "\0" ;
	char* tokenptr = NULL ;
	char* pszSeparator = " ," ;

	int nTimeLength = 0 ;
	while( fgets(buffer, MAX_STR_LEN, pFile) != NULL )
	{
		/* get string token */
		tokenptr = strtok(buffer, pszSeparator) ;
		// 一定是4个
		int i;
		for (i=0; i< 4;i++)
		{
			if (NULL==tokenptr)
			{
				return false ;
			}
			m_naTimeSeries[nTimeLength][i] = atoi(tokenptr) ;
			tokenptr = strtok(NULL, pszSeparator) ;
		} // for		
		nTimeLength++ ;
	}// while
	fclose(pFile) ;

	// 第二步：拼接成完整的文件名，然后利用GDAL去读该文件，然后从中提取指定范围的影像保存成JPG格式

	// 获取目标区域影像的范围(左上和右下角xy坐标)
	double daRect[4] ;
	daRect[0] = pLandsatInfo->m_daCornerUL[0] ;
	daRect[1] = pLandsatInfo->m_daCornerLR[0] ;
	daRect[2] = pLandsatInfo->m_daCornerUL[1] ;
	daRect[3] = pLandsatInfo->m_daCornerLR[1] ;
	
	// 
	int i;
	std::string sLandsatSrcFolder = sRoot + "landsat_p\\" ;
	int naLandsatBandsJpg[] = {4,3,2} ;
	std::string sModisSrcFolder = sRoot + "modis_p\\" ;
	int naModisBandsJpg[] = {2,1,4} ;
	std::string sDstFile = m_sOutputPath ;
	std::string sLandsatFileNamePrefix, sModisFileNamePrefix ;
	std::string sLandsatDstFile, sModisDstFile ;
	for(i=0; i<nTimeLength; i++)
	{ 
		// landsat
		char buffer[200] ;
		sprintf(buffer, "L%d%03d", m_naTimeSeries[i][0], m_naTimeSeries[i][1]) ;
		sLandsatFileNamePrefix = buffer ;
		sLandsatDstFile = m_sOutputPath + sLandsatFileNamePrefix + ".jpg" ;

		printf("%s ...", sLandsatDstFile.c_str()) ;
		CutImages(sLandsatSrcFolder, sLandsatFileNamePrefix, naLandsatBandsJpg,sLandsatDstFile, daRect) ;
		printf("\n") ;

		// modis
		sprintf(buffer, "M%d%03d", m_naTimeSeries[i][2], m_naTimeSeries[i][3]) ;
		sModisFileNamePrefix = buffer ;
		sModisDstFile = m_sOutputPath + sModisFileNamePrefix + ".jpg" ;

		printf("%s ...", sModisDstFile.c_str()) ;
		CutImages(sModisSrcFolder,sModisFileNamePrefix, naModisBandsJpg, sModisDstFile, daRect) ;
		printf("\n") ;
	}

	return 0 ;
}

/***************************************************************************************************************************************
* 利用源数据文件所在文件夹(sSrcFolder)、文件名前缀(sFileNamePrefix)，以及波段列表(nBands)来拼接出源数据文件完整文件名，
*     对三个波段进行裁剪，然后拼接保存为jpg格式的文件输出；
****************************************************************************************************************************************/
int CGenerateQuickView::CutImages(std::string sSrcFolder, std::string sFileNamePrefix, int* nBands, std::string sDstFile, double *pdVect)
{
	GDALAllRegister() ;
	GDALDataset *poSrcDs = NULL ;
	GDALDataset *poDstDs = NULL ;
	GDALDataset *pMemDs = NULL ;
	std::string sSrcFile ;
	cv::Rect srcRect, dstRect ;  // 输入影像感兴趣的区域，输出影像感兴趣区域
	int cols, rows ;  // 输出影像行列数
	GDALDataType dT ;   // 打开影像的数据类型
	double dDstGeoTrans[6] ;  // 输出影像放射变幻参数
	std::string sWKT ;     // 输入影像投影参数

	// 从Landsat或者modis的三个波段文件中读取波段，获取相关范围的影像，然后放入输出文件中
	int i ;
	for (i=0; i<3; i++)
	{
		char buffer[200] ;
		sprintf(buffer, "%s%s_band%d.img", sSrcFolder.c_str(), sFileNamePrefix.c_str(), nBands[i]) ;
		sSrcFile = buffer ;
		
		poSrcDs = (GDALDataset *)GDALOpen(sSrcFile.c_str(), GA_ReadOnly) ;
		if (poSrcDs == NULL)
		{
			return -1 ;
		}

		if ( i==0 )
		{
			// 获取原图像的基本信息
			int iCols = poSrcDs->GetRasterXSize() ;
			int iRows = poSrcDs->GetRasterYSize() ;
			int iBands = poSrcDs->GetRasterCount() ;
			dT = poSrcDs->GetRasterBand(1)->GetRasterDataType() ;
			double dGeoTrans[6] ;
			poSrcDs->GetGeoTransform(dGeoTrans) ;
			double dPixelX = abs(dGeoTrans[1]) ;
			double dPixelY = abs(dGeoTrans[5]) ;

			sWKT = poSrcDs->GetProjectionRef() ;

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
			CUtility::Projection2ImageRowCol(dGeoTrans, dValidRect[0], dValidRect[2], colrow[0], colrow[2]) ;
			srcRect.x = colrow[0] ;
			srcRect.y = colrow[2] ;
			srcRect.width  = static_cast<int>( ((dValidRect[1]-dValidRect[0]) + dPixelX/2)/dPixelX ) ;
			srcRect.height = static_cast<int>( ((dValidRect[2]-dValidRect[3]) + dPixelY/2)/dPixelY ) ;

			dstRect.width  = srcRect.width ;
			dstRect.height = srcRect.height ;
			// 然后计算目标图像所在矩阵的起始位置(x, y)
			memcpy(dDstGeoTrans, dGeoTrans, 6*sizeof(double)) ;
			dDstGeoTrans[0] = pdVect[0] ;
			dDstGeoTrans[3] = pdVect[2] ;
			CUtility::Projection2ImageRowCol(dDstGeoTrans, dValidRect[0], dValidRect[2], colrow[1], colrow[3]) ;
			dstRect.x = colrow[1] ;
			dstRect.y = colrow[3] ;

			// 计算输出影像行列号
			cols = static_cast<int>( (pdVect[1]-pdVect[0]+dPixelX/2)/dPixelX ) ;
			rows = static_cast<int>( (pdVect[2]-pdVect[3]+dPixelY/2)/dPixelY ) ;

			// 打开输出影像的GDALDataset: jpeg 只支持8bits或者12bit数据类型
			pMemDs = GetGDALDriverManager()->GetDriverByName("MEM")->Create("", cols, rows, 3, GDT_Byte, NULL) ;  // 将dT修改为GDT_Byte
			if(NULL==pMemDs)
			{
				return -1 ;
			}
		} // if(i==0)

		// 将目标GDALDataset的对应波段填充为FILLEDVALUE
		cv::Mat filledValueMat(rows, cols, CUtility::OpencvDataType(dT)) ;
		filledValueMat = FILLEDVALUE ;
		CUtility::RasterDataIO( pMemDs, i+1, GF_Write, cv::Rect(0,0,cols, rows), filledValueMat) ;
		filledValueMat.release() ;

		// 开始取值，复制至指定位置
		cv::Mat validMat(srcRect.height, srcRect.width ,CUtility::OpencvDataType(dT)) ;
		cv::Mat validMat_Byte(srcRect.height, srcRect.width ,CV_8U) ;
		double dMin, dMax ;

		CUtility::RasterDataIO( poSrcDs, 1, GF_Read, srcRect, validMat ) ;
		cv::minMaxLoc(validMat, &dMin, &dMax) ;
		if (dMax==dMin)
		{
			validMat.release() ;
			validMat_Byte = 0 ;
			CUtility::RasterDataIO( pMemDs, i+1, GF_Write, dstRect, validMat_Byte) ;
			continue;
		}
		// 数据拉伸至0-255
		validMat = (validMat-dMin)/(dMax-dMin)*255 ;
		validMat.convertTo(validMat_Byte, CV_8U) ;
		validMat.release() ;
		CUtility::RasterDataIO( pMemDs, i+1, GF_Write, dstRect, validMat_Byte) ;
		validMat_Byte.release() ;

		// 关闭输入影像
		GDALClose((GDALDatasetH)poSrcDs) ;

	} // for i

	poDstDs = GetGDALDriverManager()->GetDriverByName("JPEG")->CreateCopy(sDstFile.c_str(), pMemDs, 0, NULL, NULL, NULL) ;
	if (NULL==poDstDs)
	{
		return -1 ;
	}
	poDstDs->SetGeoTransform(dDstGeoTrans) ;
	poDstDs->SetProjection(sWKT.c_str()) ;

	GDALClose((GDALDatasetH)pMemDs) ;
	GDALClose((GDALDatasetH)poDstDs) ;

	return 0;
}
