#include "GetEvaluateIndexes.h"

CGetEvaluateIndexes::CGetEvaluateIndexes(void)
{
}

CGetEvaluateIndexes::~CGetEvaluateIndexes(void)
{
}

int CGetEvaluateIndexes::Precessing(std::string sRoot, std::string sLandsatPreFolder, std::string sParameterFile, CLandsatinfo* pLandsatInfo, int *pnStartNums, int num, std::string sOutputPath)
{
	// 第一步：读取matchlist文件，找到预测结果及其对应Landsat影像的日期，生成对应的文件名列表
	// 第二步：利用for循环，计算各个评价指标所需要的中间参量
	// 第三步：计算各个评价指标，然后保存至txt，用“,”隔开
	// 一个起点计算的所有结果放在一个txt文件中
	// txt中y方向表示时间，x方向是地物分类，每个分类包含6个波段，每个波段中包含5个评价指标，所以一共有7*6*5列  改：6*7*5
	// 

	// 该方法默认从根目录sRoot下的landsat_p目录下读取landsat影像；
	// 如果参数sOutputPath不填，则默认将计算结果放在参数文件所在目录sParameterFile；

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

	// 拼接计算评价指标的影像的完整文件名，然后进行计算
	double dmEvaluateIndexes[CLASSNUM][5] ;
	int nvBands[] = {1,2,3,4,5,7} ;   // 这是landsat影像对应的波段
	std::string sLandsatReal, sLandsatMask, sLandsatPre, sLandCover;
	int i, j, k, iband, nStart, iNum ;
	char bufferPath[200] ;
	for(iNum=0; iNum<num; iNum++)  // iNum 起点编号
	{
		// 打开记录文件
		char bufferName[200] ;
		sprintf(bufferName,"%sEvaluateIndx_start%d.txt", sRoot.c_str(), iNum+1) ;
		std::string sEvaluateIndxPath = bufferName ;
		FILE *pEvaluateIndxFile = fopen(sEvaluateIndxPath.c_str(), "w") ;
		if(pEvaluateIndxFile==NULL)
		{
			return -1 ;
		}

		nStart = pnStartNums[iNum] ;
		for (i=nStart+1; i<nTimeLength; i++)  // matchlist中行的编号，编号从  开始
		{
			for(j=0; j<6; j++)  // j 波段编号
			{
				// real landsat
				iband = nvBands[j] ;
				sprintf(bufferPath, "%slandsat_p\\L%d%03d_band%d.img", sRoot.c_str(), m_naTimeSeries[i][0], m_naTimeSeries[i][1], iband) ;
				sLandsatReal = bufferPath ;

				// landsat mask
				sprintf(bufferPath, "%slandsat_p\\L%d%03d_cfmask.img", sRoot.c_str(), m_naTimeSeries[i][0], m_naTimeSeries[i][1]) ;
				sLandsatMask = bufferPath ;

				// predicted landsat
				sprintf(bufferPath, "%spre_1_%d_%d_%d%03d_band%d.img", sLandsatPreFolder.c_str(), 5+i, i-nStart, m_naTimeSeries[i][0], m_naTimeSeries[i][1], iband) ;
				sLandsatPre = bufferPath ;

				// land cover
				sprintf(bufferPath, "%slc_%d_pheonix_combd.img", sRoot.c_str(), GetNearestLCYear(m_naTimeSeries[i][0])) ;
				sLandCover = bufferPath ;

				// 开始计算评价指标，注意这里将影像大小限定为[1000,1200]，影像数据类型为int16，掩膜和土地覆盖数据类型为uint8
				int nvRowsCols[] = {1000, 1200} ;
				int status = CalcEvaluateIndexes(sLandsatReal, sLandsatPre, sLandsatMask, sLandCover, nvRowsCols, dmEvaluateIndexes) ;
				if (status<0)
				{
					return -1 ;
				}

				// 
				for (k=0; k<CLASSNUM; k++)
				{
					fprintf(pEvaluateIndxFile, "%09.4f,%09.4f,%09.4f,%09.4f,%09.4f,", dmEvaluateIndexes[k][0], dmEvaluateIndexes[k][1], dmEvaluateIndexes[k][2], dmEvaluateIndexes[k][3], dmEvaluateIndexes[k][4]) ;
				}

			} // for j
			// 换行
			fprintf(pEvaluateIndxFile, "\n") ;

		} // for i

		fclose(pEvaluateIndxFile) ;
		
	} // iNum




	return 0;
}

int CGetEvaluateIndexes::CalcEvaluateIndexes(std::string sRealPath, std::string sPrePath, std::string sMask, std::string sLandcover, int *pRowsCols, double dmEvaluateIndexes[][5])
{
	// 假定真实的Landsat影像有相应的头文件信息，那么根据这些信息就可以知道预测影像，掩膜影像和土地类型影像的文件信息
	// 注意修改一下：考虑到这个函数实在for循环中，增加一个gdal读取meta信息会延长计算时间，故直接将基本的行列号和数据类型给出，不再自己获取；

	// 这里我们计算五种评价指标：R2, RMSE, MAE, Slope, Intercept
	// 这里土地覆盖类型设定为CLASSNUM 7

	// 直接利用C语言函数fopen, fread一行一行读取影像到内存中计算中间参量
	FILE *pFidReal = NULL ;
	FILE *pFidMask = NULL ;
	FILE *pFidPre = NULL ;
	FILE *pFidLandcover = NULL ;

	double dvSumReal[CLASSNUM] = {0} ;
	double dvSum2Real[CLASSNUM] = {0} ;
	double dvSumPre[CLASSNUM] = {0} ;
	double dvSum2Pre[CLASSNUM] = {0} ;
	double dvSumRealPre[CLASSNUM] = {0} ;
	int nvNum[CLASSNUM] = {0} ;
	double dvMAE[CLASSNUM] = {0} ;
	double dvSumDiff2[CLASSNUM] = {0} ;
	double dAbsDiff = 0 ;

	pFidReal = fopen(sRealPath.c_str(), "r") ;
	pFidMask = fopen(sMask.c_str(), "r") ;
	pFidPre = fopen(sPrePath.c_str(), "r") ;
	pFidLandcover = fopen(sLandcover.c_str(), "r") ;

	if ( NULL==pFidReal || NULL==pFidMask || NULL==pFidPre || NULL==pFidLandcover )
	{
		return -1 ;
	}
	
	int nRows = pRowsCols[0] ;
	int nCols = pRowsCols[1] ;
	
	unsigned char lcValue = 0 ;  // land cover 一个像素点的值

	// 定义内存缓冲区定义
	cv::Mat matReal(1, nCols, CV_16S) ;
	cv::Mat matMask(1, nCols, CV_8U) ;
	cv::Mat matPre(1, nCols, CV_16S) ;
	cv::Mat matLC(1, nCols, CV_8U) ;

	// 定义矩阵缓冲区指针
	char *pDataReal = (char *)matReal.data ;
	char *pDataMask = (char *)matMask.data ;
	char *pDataPre = (char *)matPre.data ;
	char *pDataLC = (char *)matLC.data ;

	// 遍历行，计算评价指标
	int i,j;
	for (i=0; i<nRows; i++)
	{
		fread(pDataReal, 2, nCols, pFidReal) ;
		fread(pDataMask, 1, nCols, pFidMask) ;
		fread(pDataPre,2, nCols, pFidPre); ;
		fread(pDataLC, 1, nCols, pFidLandcover) ;

		for (j=0; j<nCols; j++)
		{
			if ( matReal.at<int>(j)==-9999 || matMask.at<unsigned char>(j)==0 || matPre.at<int>(j)==-9999 )
			{
				continue;
			}
			lcValue = matLC.at<unsigned char>(j) ;
			if (lcValue<=0)
			{
				continue;
			}
			//
			dvSumReal[lcValue] += matReal.at<int>(j) ;
			dvSumPre[lcValue] += matPre.at<int>(j) ;
			dvSumRealPre[lcValue] += matReal.at<int>(j) * matPre.at<int>(j) ;
			dvSum2Real[lcValue] += matReal.at<int>(j) * matReal.at<int>(j)  ;
			dvSum2Pre[lcValue] += matPre.at<int>(j) * matPre.at<int>(j) ;
			dAbsDiff = abs(matReal.at<int>(j) - matPre.at<int>(j)) ;
			if (dvMAE[lcValue]<dAbsDiff)
			{
				dvMAE[lcValue] = dAbsDiff ;
			}
			dvSumDiff2[lcValue] +=  dAbsDiff * dAbsDiff ;
			nvNum[lcValue]++ ;

		} // for j

	} // for i

	// 关闭文件
	fclose(pFidReal) ;
	fclose(pFidMask) ;
	fclose(pFidPre) ;
	fclose(pFidLandcover) ;

	// 开始利用中间参量计算评价指标
	for(i=0; i<CLASSNUM; i++)
	{
		if (nvNum[i]<MIMIMUMPIXELS)
		{
			dmEvaluateIndexes[i][0] = 0 ;
			dmEvaluateIndexes[i][1] = 0 ;
			dmEvaluateIndexes[i][2] = 0 ;
			dmEvaluateIndexes[i][3] = 0 ;
			dmEvaluateIndexes[i][4] = 0 ;
		} 
		else
		{
			// R2
			dmEvaluateIndexes[i][0] = pow((nvNum[i]*dvSumRealPre[i]-dvSumReal[i]*dvSumPre[i]),2) / ( ( nvNum[i]*dvSum2Real[i]-pow(dvSumReal[i],2) )*( nvNum[i]*dvSum2Pre[i]-pow(dvSumPre[i],2) ) ) ;
			// rmse
			dmEvaluateIndexes[i][1] = sqrt(dvSumDiff2[i])/nvNum[i] ;
			// mae
			dmEvaluateIndexes[i][2] = dvMAE[i] ;
			// slope
			dmEvaluateIndexes[i][3] = (nvNum[i]*dvSumRealPre[i]-dvSumReal[i]*dvSumPre[i]) / ( nvNum[i]*dvSum2Real[i]-pow(dvSumReal[i],2) ) ;
			// intercept
			dmEvaluateIndexes[i][4] = dvSumPre[i]/nvNum[i] - dmEvaluateIndexes[i][3]*dvSumReal[i]/nvNum[i] ;

		}
	}

	return 0 ;
}

int CGetEvaluateIndexes::GetNearestLCYear(int nCurrentYear) 
{
	int nvYears[] = {2001, 2006, 2011} ;
	int i;
	for(i=0; i<3; i++)
	{
		if ( abs(nCurrentYear-nvYears[i])<=2 )
		{
			return nvYears[i] ;
		}
		
	}

	return 0 ;
}