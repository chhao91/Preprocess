#include "MODISProcess.h"

CMODISProcess::CMODISProcess(void)
{
}

CMODISProcess::~CMODISProcess(void)
{

}

int CMODISProcess::Initialize(CLandsatinfo* pLandsatInfo, std::vector<int>* pnvHdfBands, std::vector<int>* pnvOutputBands, int nMaskNum, std::string sRootPath, std::string sOutputPath)
{
	if ( !pLandsatInfo || !pnvHdfBands ||!pnvOutputBands || sRootPath.empty() )
	{
		return 0 ;
	}
	m_pLandsatInfo = pLandsatInfo ;
	m_pnvHdfBands = pnvHdfBands ;
	m_pnvOutputBands = pnvOutputBands ;
	m_nMaskNum = nMaskNum ;
	m_sRootPath = sRootPath ;
	if (sOutputPath.empty())
	{
		m_sOutputPath = sRootPath+"modis_p\\" ;
	}
	else
	{
		m_sOutputPath = sOutputPath ;
	}

	return 0 ;
}

int CMODISProcess::Processing()
{
	std::vector<std::string> fileVect ; 
	std::string sAddPath ;
	struct _finddata_t file;
	long hFile ;
	std::string strPath = m_sRootPath + "*.hdf"; // sPathLast = "c:\test\*.*"
	hFile = _findfirst(strPath.c_str(), &file);
	if(hFile == -1)
	{
		std::cout<<"�ļ�������.\n"<<std::endl;
		return NULL ;
	}
	else
	{
		sAddPath = m_sRootPath + file.name;
		fileVect.push_back(sAddPath) ;
	}
	while(_findnext(hFile, &file) == 0)
	{
		sAddPath = m_sRootPath + file.name;
		fileVect.push_back(sAddPath) ;
	}

	char *pszFileName ;
	std::string sFileName ;
	char cBandNum[5] ;
	std::string sFileIn, sFileOut ;
	for (unsigned i=0; i<fileVect.size(); i++)
	{
// ����һ�����⣬��;���ݳ��������⣬��������֮�󣬹ʴ����￪ʼ����������� -20150405
//if (i<209)
//{
//	continue; 
//}

		sFileIn = fileVect.at(i) ;
		std::cout <<"InputFile_"<<i<<": "<<sFileIn<< std::endl;

		pszFileName = CUtility::GetFileName(sFileIn.c_str()) ;
		sFileName = FormatOutputFileName(std::string(pszFileName)) ;
		// ��ʼ����HDF�еĲ���    // ��ʱע�͵� -20150826
		//int iband =-1 ;
		//int nBandNum = -1 ;
		//for (iband=0; iband < m_pnvHdfBands->size(); iband++ )
		//{
		//	nBandNum = m_pnvOutputBands->at(iband) ;
		//	sprintf(cBandNum, "%d", nBandNum) ; // MOD09A1 MOD09GA
		//	sFileOut = m_sOutputPath + sFileName + "_band" + cBandNum + ".img" ;
		//	std::cout <<"  OutputFile_"<<nBandNum<<": "<<sFileOut << std::endl ;
		//	ProcessModisBand(sFileIn, sFileOut, m_pnvHdfBands->at(iband)) ;
		//}// for

		// ��ʼ����MODIS����Ĥ
		if (m_nMaskNum<0)
		{
			continue ;
		}
		sFileOut = m_sOutputPath + sFileName + "_mask.img" ;
		std::cout <<"  OutputFile_mask: "<<sFileOut<< std::endl;
		ProcessMODISMask(sFileIn, sFileOut) ;

		free(pszFileName) ;
	}// for

	return RE_SUCCESS ;
}

int CMODISProcess::ProcessModisBand(std::string sSrcFile, std::string sDstFile, int nBandNum)
{
	char *pFilePath = CUtility::GetFilePath(sDstFile.c_str()) ;
	std::string strFileOutTmp = std::string(pFilePath) + "FileOutTmp.img" ;
	std::string strFileInTmp = std::string(pFilePath) + "FileInTmp.img" ;
	// Translate Codes
	int status = CopyModisBand(sSrcFile, strFileOutTmp.c_str(), nBandNum) ;
	if (status<0)
	{
		std::cout<< "Translating code failed." << std::endl ;
		return RE_PARAMERROR ;
	}

	// Reproject image
	status = ReprojectMODIS(strFileOutTmp, strFileInTmp) ;
	if (status<0)
	{
		std::cout<< "Reprojection failed."<< std::endl ;
		return RE_PARAMERROR ;
	}

	// Resample image
	LT_Envelope enExtent;
	enExtent.setToNull();
	enExtent.init(m_pLandsatInfo->m_daCornerUL[0], m_pLandsatInfo->m_daCornerLR[0], 
					m_pLandsatInfo->m_daCornerUL[1], m_pLandsatInfo->m_daCornerLR[1]) ;
	status = ResampleMODIS(strFileInTmp, sDstFile,
				m_pLandsatInfo->m_dPixelXSize, m_pLandsatInfo->m_dPixelYSize, 
				GRA_NearestNeighbour, &enExtent, NULL, NULL, "ENVI") ;
	if (status<0)
	{
		std::cout<< "Resampling failed."<< std::endl ;
		return RE_PARAMERROR ;
	}

	free(pFilePath) ;
	return RE_SUCCESS ;

}

int CMODISProcess::CopyModisBand(std::string sSrcFile, std::string sDstFile, int nBandNum)
{
	GDALDatasetH hSrcDS, hDstDS ;
	GDALDriverH hDriver ;
	int i ;

	//
	nBandNum = 2 * (nBandNum-1) ;

	GDALAllRegister(); 
	hSrcDS = GDALOpen( sSrcFile.c_str(), GA_ReadOnly );
	CPLAssert( hSrcDS != NULL );

	char ** papszSUBDATASETS = GDALGetMetadata( hSrcDS, "SUBDATASETS");

	std::string tmpstr = std::string(papszSUBDATASETS[nBandNum]);
	tmpstr = tmpstr.substr(tmpstr.find_first_of("=") + 1);
	const char *tmpc_str = tmpstr.c_str();	

	GDALClose(hSrcDS) ;

	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8","NO");     
	hSrcDS = GDALOpen(tmpc_str, GA_ReadOnly);    
	if (hSrcDS == NULL)    
	{    
		return RE_NOFILE ;    
	}    
	int nWidth= GDALGetRasterXSize(hSrcDS) ;
	int nHeight=GDALGetRasterXSize(hSrcDS) ;  
	int nBandCount = GDALGetRasterCount(hSrcDS) ;     
	GDALDataType dataType = GDALGetRasterDataType(GDALGetRasterBand(hSrcDS,1)) ;
	char *pszSrcWKT = NULL ;  
	pszSrcWKT = const_cast<char *>( GDALGetProjectionRef(hSrcDS) );  
	double dGeoTrans[6] ;
	GDALGetGeoTransform(hSrcDS, dGeoTrans) ;

	hDriver = GetGDALDriverManager()->GetDriverByName("ENVI") ;    
	if (hDriver == NULL)    
	{    
		GDALClose(hSrcDS);    
		return RE_PARAMERROR ;    
	} 
	hDstDS = GDALCreate(hDriver, sDstFile.c_str(), nWidth, nHeight, nBandCount, dataType, NULL) ;
	if (hDstDS == NULL)    
	{   
		GDALClose(hSrcDS);    
		return RE_PARAMERROR ;    
	} 
	GDALSetGeoTransform(hDstDS, dGeoTrans) ;
	GDALSetProjection(hDstDS, pszSrcWKT) ;

	GDALRasterBandH hSrcRasterBand = GDALGetRasterBand(hSrcDS, 1) ;
	GDALRasterBandH hDstRasterBand = GDALGetRasterBand(hDstDS, 1) ;

	cv::Mat rowMat ;
	rowMat.create(1,nWidth, CUtility::OpencvDataType(dataType)) ;
	void *pValue = (void *)rowMat.data ;

	for (i=0; i<nHeight; i++)
	{
		GDALRasterIO(hSrcRasterBand, GF_Read, 0, i, nWidth, 1, pValue, nWidth, 1, dataType, 0, 0) ;
		GDALRasterIO(hDstRasterBand, GF_Write, 0, i, nWidth, 1, pValue, nWidth, 1, dataType, 0, 0) ;
	}

	GDALClose(hSrcDS) ;
	GDALClose(hDstDS) ;
	return RE_SUCCESS ;

}

/************************************************************************
* ��ͶӰ                                                              
************************************************************************/
int CMODISProcess::ReprojectMODIS(std::string sSrcFile, std::string sDstFile) 
{
	GDALDatasetH hSrcDS, hDstDS ;
	GDALDriverH hDriver ;
	GDALAllRegister() ;

	hSrcDS = GDALOpen(sSrcFile.c_str(), GA_ReadOnly);    
	if (hSrcDS == NULL)    
	{    
		return RE_NOFILE ;    
	}    

	hDriver = GetGDALDriverManager()->GetDriverByName("ENVI");    
	if (hDriver == NULL)    
	{    
		GDALClose(hSrcDS);    
		return RE_PARAMERROR ;    
	} 
	int nWidth= GDALGetRasterXSize(hSrcDS) ;
	int nHeight=GDALGetRasterXSize(hSrcDS) ;  
	int nBandCount = GDALGetRasterCount(hSrcDS) ;     
	GDALDataType dataType = GDALGetRasterDataType(GDALGetRasterBand(hSrcDS,1)) ;    

	char *pszSrcWKT = NULL, *pszDstWKT = NULL ;  
	pszSrcWKT = const_cast<char *>( GDALGetProjectionRef(hSrcDS) );  

	// �������ͼ�������ϵͳ 
	OGRSpatialReference oSRS;  
	oSRS.SetUTM( m_pLandsatInfo->m_nZone, TRUE );  
	oSRS.SetWellKnownGeogCS( m_pLandsatInfo->m_sDatum.c_str() );  
	oSRS.exportToWkt( &pszDstWKT ); 

	void *hTransformArg;  
	hTransformArg = GDALCreateGenImgProjTransformer(hSrcDS,pszSrcWKT,NULL,pszDstWKT,FALSE,0.0,1);  

	//(û��ͶӰ��Ӱ��������߲�ͨ��)  
	if (hTransformArg == NULL)    
	{    
		GDALClose(hSrcDS);    
		return RE_PARAMERROR ;    
	}  

	double dGeoTrans[6] = {0};    
	int nNewWidth=0,nNewHeight=0;
	if(GDALSuggestedWarpOutput(hSrcDS,GDALGenImgProjTransform,hTransformArg,dGeoTrans,&nNewWidth,&nNewHeight)!=CE_None)  
	{  
		GDALClose(hSrcDS);    
		return RE_PARAMERROR ;   
	}  

	GDALDestroyGenImgProjTransformer(hTransformArg);  

	// Create the output file. 
	hDstDS = GDALCreate( hDriver, sDstFile.c_str(), nNewWidth, nNewHeight, nBandCount, dataType, NULL );

	CPLAssert( hDstDS != NULL );

	// Write out the projection definition. 
	GDALSetProjection( hDstDS, pszDstWKT );
	GDALSetGeoTransform( hDstDS, dGeoTrans );

	// �����任ѡ��  
	GDALWarpOptions *psWarpOptions = GDALCreateWarpOptions();  

	psWarpOptions->hSrcDS =hSrcDS;  
	psWarpOptions->hDstDS =hDstDS;  

	psWarpOptions->nBandCount = 1;  
	psWarpOptions->panSrcBands = (int *) CPLMalloc(sizeof(int) * psWarpOptions->nBandCount );  
	psWarpOptions->panSrcBands[0] = 1;  
	psWarpOptions->panDstBands = (int *) CPLMalloc(sizeof(int) * psWarpOptions->nBandCount );  
	psWarpOptions->panDstBands[0] = 1;  

	psWarpOptions->pfnProgress = GDALTermProgress;    

	// ������ͶӰ�任����  
	psWarpOptions->pTransformerArg =  GDALCreateGenImgProjTransformer( hSrcDS,  
		GDALGetProjectionRef(hSrcDS),  
		hDstDS,  
		GDALGetProjectionRef(hDstDS),  
		FALSE,0.0, 1 );  
	psWarpOptions->pfnTransformer = GDALGenImgProjTransform;  

	// ��ʼ������ִ�б任����  
	GDALWarpOperation oOperation;
	std::cout<< "   Reprojecting: "  ;
	oOperation.Initialize(psWarpOptions );  
	oOperation.ChunkAndWarpImage( 0, 0,  
		GDALGetRasterXSize( hDstDS),  
		GDALGetRasterYSize( hDstDS) );  

	GDALDestroyGenImgProjTransformer(psWarpOptions->pTransformerArg );  
	GDALDestroyWarpOptions( psWarpOptions );  

	GDALClose(hSrcDS) ;
	GDALClose(hDstDS) ;

	return RE_SUCCESS ;
}

/**********************************************************************************************
* �ز�������(GDAL)
* @param pszSrcFile        �����ļ���·��
* @param pszOutFile        д��Ľ��ͼ���·��
* @param dSizeX             X�ز������С
* @param dSizeY             Y�ز������С
* @param nResampleMode     ����ģʽ�������֣�����μ�GDALResampleAlg���壬Ĭ��Ϊ˫�����ڲ�
* @param pExtent           ������Χ��ΪNULL��ʾ����ȫͼ
* @param pBandIndex        ָ���Ĳ���������ţ�ΪNULL��ʾ����ȫ������
* @param pBandCount        �����Ĳ��θ�����ͬpBandIndexһͬʹ�ã���ʾ�������εĸ���
* @param pszFormat         д��Ľ��ͼ��ĸ�ʽ
* @return �ɹ�����0������Ϊ����ֵ
***********************************************************************************************/
int CMODISProcess::ResampleMODIS(std::string sSrcFile, std::string sOutFile, double dSizeX , double dSizeY, GDALResampleAlg nResampleMode,
							LT_Envelope* pExtent, int* pBandIndex, int *pBandCount, const char* pszFormat)
{
	GDALAllRegister();
	GDALDataset *pDSrc = (GDALDataset *)GDALOpen(sSrcFile.c_str(), GA_ReadOnly);
	if (pDSrc == NULL)
	{
		return RE_NOFILE;
	}

	GDALDriver *pDriver = GetGDALDriverManager()->GetDriverByName(pszFormat);
	if (pDriver == NULL)
	{
		GDALClose((GDALDatasetH) pDSrc);
		return RE_CREATEFILE;
	}

	int iBandCount = pDSrc->GetRasterCount();
	const char *pszWkt = pDSrc->GetProjectionRef();   // Changed by chenhao 20141216

	GDALDataType dataType = pDSrc->GetRasterBand(1)->GetRasterDataType();

	double dGeoTrans[6] = {0};
	pDSrc->GetGeoTransform(dGeoTrans);

	int iNewBandCount = iBandCount;
	if (pBandIndex != NULL && pBandCount != NULL)
	{
		int iMaxBandIndex = pBandIndex[0];    //�ҳ����Ĳ����������
		for (int i=1; i<*pBandCount; i++)
		{
			if (iMaxBandIndex < pBandIndex[i])
				iMaxBandIndex = pBandIndex[i];
		}

		if(iMaxBandIndex > iBandCount)
		{
			GDALClose((GDALDatasetH) pDSrc);
			return RE_PARAMERROR;
		}

		iNewBandCount = *pBandCount;
	}

	LT_Envelope enExtent;
	enExtent.setToNull();

	if (pExtent == NULL)    //ȫͼ����
	{
		double dPrj[4] = {0};    //x1,x2,y1,y2
		CUtility::ImageRowCol2Projection(dGeoTrans, 0, 0, dPrj[0], dPrj[2]);
		CUtility::ImageRowCol2Projection(dGeoTrans, pDSrc->GetRasterXSize(), pDSrc->GetRasterYSize(), dPrj[1], dPrj[3]);
		enExtent.init(dPrj[0], dPrj[1], dPrj[2], dPrj[3]);

		pExtent = &enExtent;
	}

	dGeoTrans[0] = pExtent->getMinX();
	dGeoTrans[3] = pExtent->getMaxY();
	dGeoTrans[1] = dSizeX ;
	dGeoTrans[5] = -dSizeY ;

	int iNewWidth  = static_cast<int>( (pExtent->getMaxX() - pExtent->getMinX() )/ ABS(dGeoTrans[1]) + 0.5 );
	int iNewHeight = static_cast<int>( (pExtent->getMaxY() - pExtent->getMinY() )/ ABS(dGeoTrans[5]) + 0.5 );

	GDALDataset *pDDst = pDriver->Create(sOutFile.c_str(), iNewWidth, iNewHeight, iNewBandCount, dataType, NULL);
	if (pDDst == NULL)
	{
		GDALClose((GDALDatasetH) pDSrc);
		return RE_CREATEFILE;
	}

	pDDst->SetProjection(pszWkt);               // Changed by chenhao 20141216
	pDDst->SetGeoTransform(dGeoTrans);

	//	OGRFree((void *)pszWkt) ;

	int *pSrcBand = NULL;
	int *pDstBand = NULL;
	int iBandSize = 0;
	if (pBandIndex != NULL && pBandCount != NULL)
	{
		iBandSize = *pBandCount;
		pSrcBand = new int[iBandSize];
		pDstBand = new int[iBandSize];

		for (int i=0; i<iBandSize; i++)
		{
			pSrcBand[i] = pBandIndex[i];
			pDstBand[i] = i+1;
		}
	}
	else
	{
		iBandSize = iBandCount;
		pSrcBand = new int[iBandSize];
		pDstBand = new int[iBandSize];

		for (int i=0; i<iBandSize; i++)
		{
			pSrcBand[i] = i+1;
			pDstBand[i] = i+1;
		}
	}

	void *hTransformArg = NULL, *hGenImgPrjArg = NULL;
	//    hTransformArg = hGenImgPrjArg = GDALCreateGenImgProjTransformer2((GDALDatasetH) pDSrc, (GDALDatasetH) pDDst, NULL);
	hTransformArg = GDALCreateGenImgProjTransformer((GDALDatasetH)pDSrc, pszWkt,(GDALDatasetH)pDDst, pszWkt,FALSE,0.0,1);
	if (hTransformArg == NULL)
	{
		GDALClose((GDALDatasetH) pDSrc);
		GDALClose((GDALDatasetH) pDDst);
		return RE_PARAMERROR;
	}

	GDALTransformerFunc pFnTransformer = GDALGenImgProjTransform;
	GDALWarpOptions *psWo = GDALCreateWarpOptions();

	psWo->papszWarpOptions = CSLDuplicate(NULL);
	psWo->eWorkingDataType = dataType;
	psWo->eResampleAlg = nResampleMode;

	psWo->hSrcDS = (GDALDatasetH) pDSrc;
	psWo->hDstDS = (GDALDatasetH) pDDst;

	psWo->pfnTransformer = pFnTransformer;
	psWo->pTransformerArg = hTransformArg;

	psWo->pfnProgress = GDALTermProgress;

	psWo->nBandCount = iNewBandCount;
	psWo->panSrcBands = (int *) CPLMalloc(iNewBandCount*sizeof(int));
	psWo->panDstBands = (int *) CPLMalloc(iNewBandCount*sizeof(int));
	for (int i=0; i<iNewBandCount; i++)
	{
		psWo->panSrcBands[i] = pSrcBand[i];
		psWo->panDstBands[i] = pDstBand[i];
	}

	delete pSrcBand ;
	delete pDstBand ;

	GDALWarpOperation oWo;
	if (oWo.Initialize(psWo) != CE_None)
	{
		GDALClose((GDALDatasetH) pDSrc);
		GDALClose((GDALDatasetH) pDDst);

		return RE_PARAMERROR;
	}

	std::cout<<"   Resampling: " ;
	oWo.ChunkAndWarpImage(0, 0, iNewWidth, iNewHeight);
	std::cout<<std::endl ;

	GDALDestroyGenImgProjTransformer(psWo->pTransformerArg);
	GDALDestroyWarpOptions( psWo );
	GDALClose((GDALDatasetH) pDSrc);
	GDALClose((GDALDatasetH) pDDst);

	return RE_SUCCESS;
}

int CMODISProcess::ProcessMODISMask(std::string sFileIn, std::string sFileOut)
{
	char *pcFilePath = CUtility::GetFilePath(sFileOut.c_str()) ;
	std::string sFileOutTmp = std::string(pcFilePath) + "FileOutTmp.img" ;
	std::string sFileInTmp = std::string(pcFilePath) + "FileInTmp.img" ;
	// Translate Codes
	int status = TranslateMaskCode(sFileIn, sFileOutTmp) ;
	if (status<0)
	{
		std::cout<< "Translating failed.\n" ;
		return RE_PARAMERROR ;
	}

	// Reproject image
	status = ReprojectMODIS(sFileOutTmp, sFileInTmp) ;
	if (status<0)
	{
		std::cout<< "Reprojection failed.\n" ;
		return RE_PARAMERROR ;
	}

	// Resample image
	LT_Envelope enExtent;
	enExtent.setToNull();
	enExtent.init(m_pLandsatInfo->m_daCornerUL[0], m_pLandsatInfo->m_daCornerLR[0], 
					m_pLandsatInfo->m_daCornerUL[1], m_pLandsatInfo->m_daCornerLR[1]) ;
//	status = ResampleMODIS(sFileInTmp, sFileOut, 30, 30, GRA_NearestNeighbour, &enExtent, NULL, NULL, "ENVI") ;

status = GetMODISMaskQuickview(sFileInTmp, sFileOut, 30, 30, GRA_NearestNeighbour, &enExtent, NULL, NULL, "ENVI") ;
	if (status<0)
	{
		std::cout<< "Resampling failed.\n" ;
		return RE_PARAMERROR ;
	}

	free(pcFilePath) ;
	return RE_SUCCESS ;

}

int CMODISProcess::TranslateMaskCode(std::string sFileIn, std::string sFileOut) 
{
	GDALDatasetH hSrcDS, hDstDS ;
	GDALDriverH hDriver ;
	int i,j ;

	int nMaskNum = 2 * (m_nMaskNum-1) ;

	GDALAllRegister(); 
	hSrcDS = GDALOpen( sFileIn.c_str(), GA_ReadOnly );
	CPLAssert( hSrcDS != NULL );

	char ** papszSUBDATASETS = GDALGetMetadata( hSrcDS, "SUBDATASETS");

	std::string tmpstr = std::string(papszSUBDATASETS[nMaskNum]);
	tmpstr = tmpstr.substr(tmpstr.find_first_of("=") + 1);
	const char *tmpc_str = tmpstr.c_str();	

	GDALClose(hSrcDS) ;

	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8","NO");     
	hSrcDS = GDALOpen(tmpc_str, GA_ReadOnly);    
	if (hSrcDS == NULL)    
	{    
		return RE_NOFILE ;    
	}    
	int nWidth= GDALGetRasterXSize(hSrcDS) ;
	int nHeight=GDALGetRasterXSize(hSrcDS) ;  
	int nBandCount = GDALGetRasterCount(hSrcDS) ;     
	GDALDataType dataType = GDALGetRasterDataType(GDALGetRasterBand(hSrcDS,1)) ;
	char *pszSrcWKT = NULL ;  
	pszSrcWKT = const_cast<char *>( GDALGetProjectionRef(hSrcDS) );  
	double dGeoTrans[6] ;
	GDALGetGeoTransform(hSrcDS, dGeoTrans) ;

	hDriver = GetGDALDriverManager()->GetDriverByName("ENVI") ;    
	if (hDriver == NULL)    
	{    
		GDALClose(hSrcDS);    
		return RE_PARAMERROR ;    
	} 
	hDstDS = GDALCreate(hDriver, sFileOut.c_str(), nWidth, nHeight, nBandCount, GDT_Byte, NULL) ;
	if (hDstDS == NULL)    
	{   
		GDALClose(hSrcDS);    
		return RE_PARAMERROR ;    
	} 
	GDALSetGeoTransform(hDstDS, dGeoTrans) ;
	GDALSetProjection(hDstDS, pszSrcWKT) ;

	GDALRasterBandH hSrcRasterBand = GDALGetRasterBand(hSrcDS, 1) ;
	GDALRasterBandH hDstRasterBand = GDALGetRasterBand(hDstDS, 1) ;

	cv::Mat rowMatInt32, rowMatByte8 ;
	rowMatInt32.create(1, nWidth, CUtility::OpencvDataType(dataType)) ;
	rowMatByte8.create(1, nWidth, CV_8S) ;
	//unsigned int *pValueInt32 = (unsigned int *)(rowMatInt32.data) ;
unsigned short *pValueInt32 = (unsigned short *)(rowMatInt32.data) ;       // �ĳ���16λ���������� -20150827
	char *pValueByte8 = (char *)(rowMatByte8.data) ;
	//unsigned int codeInt32 ;
unsigned short codeInt32 ;    // �ĳ���16λ���������� -20150827

	for (i=0; i<nHeight; i++)
	{
		GDALRasterIO(hSrcRasterBand, GF_Read, 0, i, nWidth, 1, rowMatInt32.data, nWidth, 1, dataType, 0, 0) ;
		for (j=0; j<nWidth; j++)
		{
			codeInt32 = *(pValueInt32+j) ;
			*(pValueByte8+j) = Code2Mask(codeInt32) ;		
		}
		GDALRasterIO(hDstRasterBand, GF_Write, 0, i, nWidth, 1, rowMatByte8.data, nWidth, 1, GDALDataType(CV_8S), 0, 0) ;
	}

	GDALClose(hSrcDS) ;
	GDALClose(hDstDS) ;

	return RE_SUCCESS ;

}

//char CMODISProcess::Code2Mask(unsigned int n)
char CMODISProcess::Code2Mask(unsigned short n)  // �ĳ���16λ���������� -20150827
{
	/*int i=0 ;     
	while(n && i<6)   
	{
		if( i>1 && n%2==1 )
			return 0;
		n /= 2;
		i++ ;
	}*/

	//int i ;                // 20150508�ӣ�ͨ��6�����εĲ�������Ĥ
	//for (i=2; i<18; i++)
	//{
	//	if((n>>i)&1)
	//		return 0 ;
	//}

	//for (i=26; i<30; i++)
	//{
	//	if((n>>i)&1)
	//		return 0 ;
	//}
	//return 1 ;

	if( (n>>14)&1 )             // 20150826 ������ȡ12ͼ��BRDF����
		return 1 ;
	return 0 ;

}

std::string CMODISProcess::FormatOutputFileName(std::string sFileName)
{
	std::string sTime = sFileName.substr(9,7) ;
	std::string sOutputFileName = "M" + sTime ;

	return sOutputFileName ;
}


int CMODISProcess::GetMODISMaskQuickview(std::string sSrcFile, std::string sOutFile, double dSizeX , double dSizeY, GDALResampleAlg nResampleMode,
						  LT_Envelope* pExtent, int* pBandIndex, int *pBandCount, const char* pszFormat) 
{
	GDALAllRegister();
	GDALDataset *pDSrc = (GDALDataset *)GDALOpen(sSrcFile.c_str(), GA_ReadOnly);
	if (pDSrc == NULL)
	{
		return RE_NOFILE;
	}

	GDALDriver *pDriver = GetGDALDriverManager()->GetDriverByName(pszFormat);
	if (pDriver == NULL)
	{
		GDALClose((GDALDatasetH) pDSrc);
		return RE_CREATEFILE;
	}

	int iBandCount = pDSrc->GetRasterCount();
	const char *pszWkt = pDSrc->GetProjectionRef();   // Changed by chenhao 20141216

	GDALDataType dataType = pDSrc->GetRasterBand(1)->GetRasterDataType();

	double dGeoTrans[6] = {0};
	pDSrc->GetGeoTransform(dGeoTrans);

	int iNewBandCount = iBandCount;
	if (pBandIndex != NULL && pBandCount != NULL)
	{
		int iMaxBandIndex = pBandIndex[0];    //�ҳ����Ĳ����������
		for (int i=1; i<*pBandCount; i++)
		{
			if (iMaxBandIndex < pBandIndex[i])
				iMaxBandIndex = pBandIndex[i];
		}

		if(iMaxBandIndex > iBandCount)
		{
			GDALClose((GDALDatasetH) pDSrc);
			return RE_PARAMERROR;
		}

		iNewBandCount = *pBandCount;
	}

	LT_Envelope enExtent;
	enExtent.setToNull();

	if (pExtent == NULL)    //ȫͼ����
	{
		double dPrj[4] = {0};    //x1,x2,y1,y2
		CUtility::ImageRowCol2Projection(dGeoTrans, 0, 0, dPrj[0], dPrj[2]);
		CUtility::ImageRowCol2Projection(dGeoTrans, pDSrc->GetRasterXSize(), pDSrc->GetRasterYSize(), dPrj[1], dPrj[3]);
		enExtent.init(dPrj[0], dPrj[1], dPrj[2], dPrj[3]);

		pExtent = &enExtent;
	}

	dGeoTrans[0] = pExtent->getMinX();
	dGeoTrans[3] = pExtent->getMaxY();
	dGeoTrans[1] = dSizeX ;
	dGeoTrans[5] = -dSizeY ;

	int iNewWidth  = static_cast<int>( (pExtent->getMaxX() - pExtent->getMinX() )/ ABS(dGeoTrans[1]) + 0.5 );
	int iNewHeight = static_cast<int>( (pExtent->getMaxY() - pExtent->getMinY() )/ ABS(dGeoTrans[5]) + 0.5 );

	GDALDataset *pDDst = pDriver->Create(sOutFile.c_str(), iNewWidth, iNewHeight, iNewBandCount, dataType, NULL);
	if (pDDst == NULL)
	{
		GDALClose((GDALDatasetH) pDSrc);
		return RE_CREATEFILE;
	}

	pDDst->SetProjection(pszWkt);               // Changed by chenhao 20141216
	pDDst->SetGeoTransform(dGeoTrans);

	//	OGRFree((void *)pszWkt) ;

	int *pSrcBand = NULL;
	int *pDstBand = NULL;
	int iBandSize = 0;
	if (pBandIndex != NULL && pBandCount != NULL)
	{
		iBandSize = *pBandCount;
		pSrcBand = new int[iBandSize];
		pDstBand = new int[iBandSize];

		for (int i=0; i<iBandSize; i++)
		{
			pSrcBand[i] = pBandIndex[i];
			pDstBand[i] = i+1;
		}
	}
	else
	{
		iBandSize = iBandCount;
		pSrcBand = new int[iBandSize];
		pDstBand = new int[iBandSize];

		for (int i=0; i<iBandSize; i++)
		{
			pSrcBand[i] = i+1;
			pDstBand[i] = i+1;
		}
	}

	void *hTransformArg = NULL, *hGenImgPrjArg = NULL;
	//    hTransformArg = hGenImgPrjArg = GDALCreateGenImgProjTransformer2((GDALDatasetH) pDSrc, (GDALDatasetH) pDDst, NULL);
	hTransformArg = GDALCreateGenImgProjTransformer((GDALDatasetH)pDSrc, pszWkt,(GDALDatasetH)pDDst, pszWkt,FALSE,0.0,1);
	if (hTransformArg == NULL)
	{
		GDALClose((GDALDatasetH) pDSrc);
		GDALClose((GDALDatasetH) pDDst);
		return RE_PARAMERROR;
	}

	GDALTransformerFunc pFnTransformer = GDALGenImgProjTransform;
	GDALWarpOptions *psWo = GDALCreateWarpOptions();

	psWo->papszWarpOptions = CSLDuplicate(NULL);
	psWo->eWorkingDataType = dataType;
	psWo->eResampleAlg = nResampleMode;

	psWo->hSrcDS = (GDALDatasetH) pDSrc;
	psWo->hDstDS = (GDALDatasetH) pDDst;

	psWo->pfnTransformer = pFnTransformer;
	psWo->pTransformerArg = hTransformArg;

	psWo->pfnProgress = GDALTermProgress;

	psWo->nBandCount = iNewBandCount;
	psWo->panSrcBands = (int *) CPLMalloc(iNewBandCount*sizeof(int));
	psWo->panDstBands = (int *) CPLMalloc(iNewBandCount*sizeof(int));
	for (int i=0; i<iNewBandCount; i++)
	{
		psWo->panSrcBands[i] = pSrcBand[i];
		psWo->panDstBands[i] = pDstBand[i];
	}

	delete pSrcBand ;
	delete pDstBand ;

	GDALWarpOperation oWo;
	if (oWo.Initialize(psWo) != CE_None)
	{
		GDALClose((GDALDatasetH) pDSrc);
		GDALClose((GDALDatasetH) pDDst);

		return RE_PARAMERROR;
	}

	std::cout<<"   Resampling: " ;
	oWo.ChunkAndWarpImage(0, 0, iNewWidth, iNewHeight);
	std::cout<<std::endl ;

	GDALDestroyGenImgProjTransformer(psWo->pTransformerArg);
	GDALDestroyWarpOptions( psWo );

	// �����Ӱ���GDALDataset: jpeg ֻ֧��8bits����12bit��������
	GDALDataset *pMemDs = GetGDALDriverManager()->GetDriverByName("MEM")->Create("", iNewWidth, iNewHeight, 1, GDT_Byte, NULL) ;  // ��dT�޸�ΪGDT_Byte
	if(NULL==pMemDs)
	{
		return -1 ;
	}
	cv::Rect dstRect(0, 0, iNewWidth, iNewHeight) ;
	cv::Mat tempMatByte ;
	CUtility::RasterDataIO( pDDst, 1, GF_Read, dstRect, tempMatByte) ;
	CUtility::RasterDataIO( pMemDs, 1, GF_Write, dstRect, tempMatByte) ;

	// �� MEM GdalDataset ������ JPEG GdalDataset
	char *pszFilePath = CUtility::GetFilePath(sOutFile.c_str()) ;
	char *pszFileName = CUtility::GetFileName(sOutFile.c_str()) ;
	std::string sDstFile(pszFilePath) ;
	sDstFile = sDstFile + pszFileName + ".jpg" ;
	GDALDataset *poDstDs = GetGDALDriverManager()->GetDriverByName("JPEG")->CreateCopy(sDstFile.c_str(), pMemDs, 0, NULL, NULL, NULL) ;
	if (NULL==poDstDs)
	{
		return -1 ;
	}
	GDALClose((GDALDatasetH) poDstDs);
	GDALClose((GDALDatasetH) pMemDs);
	free(pszFilePath) ;
	free(pszFileName) ;

	GDALClose((GDALDatasetH) pDSrc);
	GDALClose((GDALDatasetH) pDDst);



	return RE_SUCCESS;
}