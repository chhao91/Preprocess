#include "GenerateQuickView.h"

CGenerateQuickView::CGenerateQuickView(void)
{

}

CGenerateQuickView::~CGenerateQuickView(void)
{
}

/*****************************************************************************************************************
* �Ӵ���õ�landsat��modisӰ������ȡ�ӿռ䷶Χ�Ŀ���ͼ��
* �Ӹ�Ŀ¼��landsat_p��modis_p�ļ����зֱ��ȡ�������ļ����䴦���ڲ�ָ������·��sOutputPath������£�Ĭ�ϱ�����
*     ����������ļ���ͬ·���£�sParameterFile����
*****************************************************************************************************************/
int CGenerateQuickView::Processing(std::string sRoot, std::string sParameterFile, CLandsatinfo* pLandsatInfo,std::string sOutputPath)
{
	// ��һ������ȡmatchlist�ļ���ȡlandsat��modis�ġ��ꡱ�͡�DOY��
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
		// һ����4��
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

	// �ڶ�����ƴ�ӳ��������ļ�����Ȼ������GDALȥ�����ļ���Ȼ�������ȡָ����Χ��Ӱ�񱣴��JPG��ʽ

	// ��ȡĿ������Ӱ��ķ�Χ(���Ϻ����½�xy����)
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
* ����Դ�����ļ������ļ���(sSrcFolder)���ļ���ǰ׺(sFileNamePrefix)���Լ������б�(nBands)��ƴ�ӳ�Դ�����ļ������ļ�����
*     ���������ν��вü���Ȼ��ƴ�ӱ���Ϊjpg��ʽ���ļ������
****************************************************************************************************************************************/
int CGenerateQuickView::CutImages(std::string sSrcFolder, std::string sFileNamePrefix, int* nBands, std::string sDstFile, double *pdVect)
{
	GDALAllRegister() ;
	GDALDataset *poSrcDs = NULL ;
	GDALDataset *poDstDs = NULL ;
	GDALDataset *pMemDs = NULL ;
	std::string sSrcFile ;
	cv::Rect srcRect, dstRect ;  // ����Ӱ�����Ȥ���������Ӱ�����Ȥ����
	int cols, rows ;  // ���Ӱ��������
	GDALDataType dT ;   // ��Ӱ�����������
	double dDstGeoTrans[6] ;  // ���Ӱ������ò���
	std::string sWKT ;     // ����Ӱ��ͶӰ����

	// ��Landsat����modis�����������ļ��ж�ȡ���Σ���ȡ��ط�Χ��Ӱ��Ȼ���������ļ���
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
			// ��ȡԭͼ��Ļ�����Ϣ
			int iCols = poSrcDs->GetRasterXSize() ;
			int iRows = poSrcDs->GetRasterYSize() ;
			int iBands = poSrcDs->GetRasterCount() ;
			dT = poSrcDs->GetRasterBand(1)->GetRasterDataType() ;
			double dGeoTrans[6] ;
			poSrcDs->GetGeoTransform(dGeoTrans) ;
			double dPixelX = abs(dGeoTrans[1]) ;
			double dPixelY = abs(dGeoTrans[5]) ;

			sWKT = poSrcDs->GetProjectionRef() ;

			// ��ȡԭͼ�񸲸Ƿ�Χ
			double dSrcRect[4] ;
			dSrcRect[0] = dGeoTrans[0] ;
			dSrcRect[2] = dGeoTrans[3] ;
			CUtility::ImageRowCol2Projection(dGeoTrans, iCols, iRows, dSrcRect[1], dSrcRect[3]) ;

			double dValidRect[4] ;
			dValidRect[0] = pdVect[0] > dSrcRect[0] ? pdVect[0] : dSrcRect[0] ;
			dValidRect[1] = pdVect[1] < dSrcRect[1] ? pdVect[1] : dSrcRect[1] ;
			dValidRect[2] = pdVect[2] < dSrcRect[2] ? pdVect[2] : dSrcRect[2] ;
			dValidRect[3] = pdVect[3] > dSrcRect[3] ? pdVect[3] : dSrcRect[3] ;
			// �ж��Ƿ�����Ч����
			if (dValidRect[0]>dValidRect[1] || dValidRect[2]<dValidRect[3])
			{
				return -1 ;
			}

			int colrow[4] ; // (0,2)��Ч�������Ͻǵ��Ӧԭͼ���кţ�(1,3)��Ч������������ͼ�����к�
			CUtility::Projection2ImageRowCol(dGeoTrans, dValidRect[0], dValidRect[2], colrow[0], colrow[2]) ;
			srcRect.x = colrow[0] ;
			srcRect.y = colrow[2] ;
			srcRect.width  = static_cast<int>( ((dValidRect[1]-dValidRect[0]) + dPixelX/2)/dPixelX ) ;
			srcRect.height = static_cast<int>( ((dValidRect[2]-dValidRect[3]) + dPixelY/2)/dPixelY ) ;

			dstRect.width  = srcRect.width ;
			dstRect.height = srcRect.height ;
			// Ȼ�����Ŀ��ͼ�����ھ������ʼλ��(x, y)
			memcpy(dDstGeoTrans, dGeoTrans, 6*sizeof(double)) ;
			dDstGeoTrans[0] = pdVect[0] ;
			dDstGeoTrans[3] = pdVect[2] ;
			CUtility::Projection2ImageRowCol(dDstGeoTrans, dValidRect[0], dValidRect[2], colrow[1], colrow[3]) ;
			dstRect.x = colrow[1] ;
			dstRect.y = colrow[3] ;

			// �������Ӱ�����к�
			cols = static_cast<int>( (pdVect[1]-pdVect[0]+dPixelX/2)/dPixelX ) ;
			rows = static_cast<int>( (pdVect[2]-pdVect[3]+dPixelY/2)/dPixelY ) ;

			// �����Ӱ���GDALDataset: jpeg ֻ֧��8bits����12bit��������
			pMemDs = GetGDALDriverManager()->GetDriverByName("MEM")->Create("", cols, rows, 3, GDT_Byte, NULL) ;  // ��dT�޸�ΪGDT_Byte
			if(NULL==pMemDs)
			{
				return -1 ;
			}
		} // if(i==0)

		// ��Ŀ��GDALDataset�Ķ�Ӧ�������ΪFILLEDVALUE
		cv::Mat filledValueMat(rows, cols, CUtility::OpencvDataType(dT)) ;
		filledValueMat = FILLEDVALUE ;
		CUtility::RasterDataIO( pMemDs, i+1, GF_Write, cv::Rect(0,0,cols, rows), filledValueMat) ;
		filledValueMat.release() ;

		// ��ʼȡֵ��������ָ��λ��
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
		// ����������0-255
		validMat = (validMat-dMin)/(dMax-dMin)*255 ;
		validMat.convertTo(validMat_Byte, CV_8U) ;
		validMat.release() ;
		CUtility::RasterDataIO( pMemDs, i+1, GF_Write, dstRect, validMat_Byte) ;
		validMat_Byte.release() ;

		// �ر�����Ӱ��
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
