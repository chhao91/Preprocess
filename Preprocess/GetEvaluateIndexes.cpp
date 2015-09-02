#include "GetEvaluateIndexes.h"

CGetEvaluateIndexes::CGetEvaluateIndexes(void)
{
}

CGetEvaluateIndexes::~CGetEvaluateIndexes(void)
{
}

int CGetEvaluateIndexes::Precessing(std::string sRoot, std::string sLandsatPreFolder, std::string sParameterFile, CLandsatinfo* pLandsatInfo, int *pnStartNums, int num, std::string sOutputPath)
{
	// ��һ������ȡmatchlist�ļ����ҵ�Ԥ���������ӦLandsatӰ������ڣ����ɶ�Ӧ���ļ����б�
	// �ڶ���������forѭ���������������ָ������Ҫ���м����
	// �������������������ָ�꣬Ȼ�󱣴���txt���á�,������
	// һ������������н������һ��txt�ļ���
	// txt��y�����ʾʱ�䣬x�����ǵ�����࣬ÿ���������6�����Σ�ÿ�������а���5������ָ�꣬����һ����7*6*5��  �ģ�6*7*5
	// 

	// �÷���Ĭ�ϴӸ�Ŀ¼sRoot�µ�landsat_pĿ¼�¶�ȡlandsatӰ��
	// �������sOutputPath�����Ĭ�Ͻ����������ڲ����ļ�����Ŀ¼sParameterFile��

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

	// ƴ�Ӽ�������ָ���Ӱ��������ļ�����Ȼ����м���
	double dmEvaluateIndexes[CLASSNUM][5] ;
	int nvBands[] = {1,2,3,4,5,7} ;   // ����landsatӰ���Ӧ�Ĳ���
	std::string sLandsatReal, sLandsatMask, sLandsatPre, sLandCover;
	int i, j, k, iband, nStart, iNum ;
	char bufferPath[200] ;
	for(iNum=0; iNum<num; iNum++)  // iNum �����
	{
		// �򿪼�¼�ļ�
		char bufferName[200] ;
		sprintf(bufferName,"%sEvaluateIndx_start%d.txt", sRoot.c_str(), iNum+1) ;
		std::string sEvaluateIndxPath = bufferName ;
		FILE *pEvaluateIndxFile = fopen(sEvaluateIndxPath.c_str(), "w") ;
		if(pEvaluateIndxFile==NULL)
		{
			return -1 ;
		}

		nStart = pnStartNums[iNum] ;
		for (i=nStart+1; i<nTimeLength; i++)  // matchlist���еı�ţ���Ŵ�  ��ʼ
		{
			for(j=0; j<6; j++)  // j ���α��
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

				// ��ʼ��������ָ�꣬ע�����ｫӰ���С�޶�Ϊ[1000,1200]��Ӱ����������Ϊint16����Ĥ�����ظ�����������Ϊuint8
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
			// ����
			fprintf(pEvaluateIndxFile, "\n") ;

		} // for i

		fclose(pEvaluateIndxFile) ;
		
	} // iNum




	return 0;
}

int CGetEvaluateIndexes::CalcEvaluateIndexes(std::string sRealPath, std::string sPrePath, std::string sMask, std::string sLandcover, int *pRowsCols, double dmEvaluateIndexes[][5])
{
	// �ٶ���ʵ��LandsatӰ������Ӧ��ͷ�ļ���Ϣ����ô������Щ��Ϣ�Ϳ���֪��Ԥ��Ӱ����ĤӰ�����������Ӱ����ļ���Ϣ
	// ע���޸�һ�£����ǵ��������ʵ��forѭ���У�����һ��gdal��ȡmeta��Ϣ���ӳ�����ʱ�䣬��ֱ�ӽ����������кź��������͸����������Լ���ȡ��

	// �������Ǽ�����������ָ�꣺R2, RMSE, MAE, Slope, Intercept
	// �������ظ��������趨ΪCLASSNUM 7

	// ֱ������C���Ժ���fopen, freadһ��һ�ж�ȡӰ���ڴ��м����м����
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
	
	unsigned char lcValue = 0 ;  // land cover һ�����ص��ֵ

	// �����ڴ滺��������
	cv::Mat matReal(1, nCols, CV_16S) ;
	cv::Mat matMask(1, nCols, CV_8U) ;
	cv::Mat matPre(1, nCols, CV_16S) ;
	cv::Mat matLC(1, nCols, CV_8U) ;

	// ������󻺳���ָ��
	char *pDataReal = (char *)matReal.data ;
	char *pDataMask = (char *)matMask.data ;
	char *pDataPre = (char *)matPre.data ;
	char *pDataLC = (char *)matLC.data ;

	// �����У���������ָ��
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

	// �ر��ļ�
	fclose(pFidReal) ;
	fclose(pFidMask) ;
	fclose(pFidPre) ;
	fclose(pFidLandcover) ;

	// ��ʼ�����м������������ָ��
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