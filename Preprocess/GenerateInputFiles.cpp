#include "GenerateInputFiles.h"

CGenerateInputFiles::CGenerateInputFiles(void)
{
}

CGenerateInputFiles::~CGenerateInputFiles(void)
{
}

/************************************************************************************************************
* 直接根据“1,1,2,282,1,274”这样格式的文件生成input输入参数文件；
* 这个参数文件以“begin”开始，以“end”结束；
************************************************************************************************************/
int CGenerateInputFiles::Processing(std::string sParameterFile, CLandsatinfo* pLandsatInfo, std::string sOutputPath)
{	
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

	fscanf(pFile, "%s", buffer) ;
	if (stricmp(buffer, "begin") != 0)
	{
		printf("This is an invalid file.") ;
		fclose(pFile) ;
		return false ;
	}

	// 跳过一个空格
	fgets(buffer, MAX_STR_LEN, pFile) ;

	while( fgets(buffer, MAX_STR_LEN, pFile) != NULL )
	{
		if (stricmp(buffer, "end") == 0)
		{
			break;
		}

		/* get string token */
		tokenptr = strtok(buffer, pszSeparator) ;
		// 前5个一定不为空
		int i;
		for (i=0; i< 5;i++)
		{
			if (NULL==tokenptr)
			{
				return false ;
			}
			m_naPredictPattern[i] = atoi(tokenptr) ;
			tokenptr = strtok(NULL, pszSeparator) ;
		}
		for (i; i<5+m_naPredictPattern[4]; i++)
		{
			if (NULL==tokenptr)
			{
				return false ;
			}
			m_naPredictPattern[i] = atoi(tokenptr) ;
			tokenptr = strtok(NULL, pszSeparator) ;
		}

		// 生成input文件
		Generating() ;

	}// while

	fclose(pFile) ;
	return 0;
}

int CGenerateInputFiles::Generating()
{
	char buffer[MAX_STR_LEN] ;
	int nLandsatBandNum[] = {1,2,3,4,5,7} ;    // Landsat-7
	//int nLandsatBandNum[] = {2,3,4,5,6,7} ;  // Landsat-8
	int nModisBandNum[] = {3,4,1,2,6,7} ;      // MODIS
	int i,iband ;

	for (iband=0; iband<6; iband++)
	{
		sprintf(buffer,"%sinput_%d_%d_%d_%03d_band%d.txt", m_sOutputPath.c_str(), m_naPredictPattern[0],
			m_naPredictPattern[1],
			m_naPredictPattern[2],
			m_naPredictPattern[3],
			nLandsatBandNum[iband]) ;

		std::cout<< buffer <<std::endl ;
		FILE* pFileInput = fopen(buffer, "w") ;

		// 开始写入
		fprintf(pFileInput, "STARFM_PARAMETER_START\n") ;
		fprintf(pFileInput, "# number of input pairs of fine and coarse resolution image\n") ;
		fprintf(pFileInput, "  NUM_IN_PAIRS = %d\n", m_naPredictPattern[4]) ;

		fprintf(pFileInput, "# input coarse resolution data (saved in 2 bytes / pixel)\n") ;
		fprintf(pFileInput, "  IN_PAIR_MODIS_FNAME =") ;
		for (i=0; i<m_naPredictPattern[4]; i++)
		{
			fprintf(pFileInput, " M%03d_band%d.img", m_naPredictPattern[5+i], nModisBandNum[iband]) ;

		}
		fprintf(pFileInput, "\n") ;

		fprintf(pFileInput, "# input fine resolution data\n") ;
		fprintf(pFileInput, "  IN_PAIR_LANDSAT_FNAME =") ;
		for (i=0; i<m_naPredictPattern[4]; i++)
		{
			fprintf(pFileInput, " L%03d_band%d.img", m_naPredictPattern[5+i], nLandsatBandNum[iband]) ;

		}
		fprintf(pFileInput, "\n") ;

		fprintf(pFileInput, "# input coarse resolution data mask (saved in 1 byte / pixel)\n") ;
		fprintf(pFileInput, "#  IN_PAIR_MODIS_MASK =") ;
		for (i=0; i<m_naPredictPattern[4]; i++)
		{
			fprintf(pFileInput, " M%03d_mask.img", m_naPredictPattern[5+i]) ;

		}
		fprintf(pFileInput, "\n") ;

		fprintf(pFileInput, "# input fine resolution data mask (saved in 1 pixel /pixel)\n") ;
		fprintf(pFileInput, "#  IN_PAIR_LANDSAT_MASK =") ;
		for (i=0; i<m_naPredictPattern[4]; i++)
		{
			fprintf(pFileInput, " L%03d_cfmask.img", m_naPredictPattern[5+i]) ;

		}
		fprintf(pFileInput, "\n") ;

		fprintf(pFileInput, "# coarse resolution data for the prediction day\n") ;
		fprintf(pFileInput, "  IN_PDAY_MODIS_FNAME = M%03d_band%d.img\n", m_naPredictPattern[3], nModisBandNum[iband]) ;

		fprintf(pFileInput, "# the output fine resolution prediction\n") ;
		fprintf(pFileInput, "  OUT_PDAY_LANDSAT_FNAME = pre_%d_%d_%d_%03d_band%d.img\n", m_naPredictPattern[0],
			m_naPredictPattern[1],
			m_naPredictPattern[2],
			m_naPredictPattern[3],
			nLandsatBandNum[iband]) ;
		fprintf(pFileInput, "# number of rows (same for all inputs)\n") ;
		fprintf(pFileInput, "  NROWS = %d\n",m_pLandsatInfo->m_nRows) ;

		fprintf(pFileInput, "# number of columns (same for all inputs)\n") ;
		fprintf(pFileInput, "  NCOLS = %d\n",m_pLandsatInfo->m_nCols) ;

		fprintf(pFileInput, "# spatial resolution (same for all inputs), coarse resolution data\n") ;
		fprintf(pFileInput, "# should first be resampled to fine resolution\n") ;
		fprintf(pFileInput, "  RESOLUTION = %d\n", static_cast<int>(m_pLandsatInfo->m_dPixelXSize+0.5) ) ;

		fprintf(pFileInput, "# define data scale factor\n") ;
		fprintf(pFileInput, "  SCALE_FACTOR = 10000\n") ;
		fprintf(pFileInput, "# define metadata for fine resolution input \n") ;
		fprintf(pFileInput, "  LANDSAT_FILLV = -9999\n") ;
		fprintf(pFileInput, "  LANDSAT_DATA_RANGE = 0, 10000\n") ;
		fprintf(pFileInput, "  LANDSAT_UNCERTAINTY = 50\n") ;
		fprintf(pFileInput, "# define metadata for coarse resolution input\n") ;
		fprintf(pFileInput, "  MODIS_FILLV = -9999\n") ;
		fprintf(pFileInput, "  MODIS_DATA_RANGE = 0, 10000 \n") ;
		fprintf(pFileInput, "  MODIS_UNCERTAINTY = 50\n") ;
		fprintf(pFileInput, "# 1 = use spatial information (suggested)\n") ;
		fprintf(pFileInput, "# 0 = not use spatial information (just use information from current pixel)\n") ;
		fprintf(pFileInput, "  USE_SPATIAL_FLAG = 1\n") ;
		fprintf(pFileInput, "# maximum distance to searching for the spatial information \n") ;
		fprintf(pFileInput, "  MAX_SEARCH_DISTANCE = 750\n") ;
		fprintf(pFileInput, "# number of slice for spectral similar testing\n") ;
		fprintf(pFileInput, "  NUM_SLICE_PURE_TEST = 40\n") ;
		fprintf(pFileInput, "# minimum acceptable percent of samples within searching window\n") ;
		fprintf(pFileInput, "  MIN_SAMPLE_PERCENT = 2\n") ;
		fprintf(pFileInput, "STARFM_PARAMETER_END\n") ;

		fclose(pFileInput) ;
	}// iband
	
	return 0;
}

/***************************************************************************************************************
* 从matchlist文件中读取配对的landsat和modis影像的“年”和“DOY”，以此来生成input输入参数文件；
* 这个matchlist文件每行包含landsat的“年”和“DOY”，以及modis的“年”和“DOY”，例如：“2000,230,2000,233”；
****************************************************************************************************************/
int CGenerateInputFiles::Processing2(std::string sParameterFile, CLandsatinfo* pLandsatInfo,std::string sOutputPath)
{
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

	// 缺一个从四个季节中寻找起点的函数
	int naStarPreTime[MAX_TIME_SERIES_LENGTH] ;
	int nStarPreTimeLen = 0 ;
//	GetSeasonMidDay(m_naTimeSeries, nTimeLength, naStarPreTime, nStarPreTimeLen) ;

	int i,j, k ;
	//for (i=0; i<nStarPreTimeLen; i++)
	//{
	//	for (j=naStarPreTime[i]+1; j<nTimeLength; j++) // j表示终点的时间序列编号
	//	{
	//		m_naPredictPattern[0] = 1 ;
	//		m_naPredictPattern[1] = i + 1 ;
	//		m_naPredictPattern[2] = j - naStarPreTime[i] ;
	//		m_naPredictPattern[3] = j ;
	//		m_naPredictPattern[4] = 1 ;
	//		m_naPredictPattern[5] = naStarPreTime[i] ;

	//		Generating2() ;
	//	}

	//}
	//int index[] = {4, 5, 6, 7} ;
	//for (k=0; k<4; k++)
	//{
	//	i=index[k] ;            // i表示起点时间序列中的编号
	//	for (j=naStarPreTime[i]+1; j<nTimeLength; j++) // j表示终点的时间序列编号
	//	{
	//		m_naPredictPattern[0] = 1 ;
	//		m_naPredictPattern[1] = i + 1 ;
	//		m_naPredictPattern[2] = j - naStarPreTime[i] ;
	//		m_naPredictPattern[3] = j ;
	//		m_naPredictPattern[4] = 1 ;
	//		m_naPredictPattern[5] = naStarPreTime[i] ;

	//		Generating2() ;
	//	}

	//}

	//int index[] = {46,47,50,51,52,53,55,56} ;
	//for (k=0; k<4; k++)
	//{
	//	for (j=k+1; j<8; j++)
	//	{
	//		m_naPredictPattern[0] = 1 ;       //
	//		m_naPredictPattern[1] = k + 1 ;  // 起点编号
	//		m_naPredictPattern[2] = j - k ;  // 该起点预测编号
	//		m_naPredictPattern[3] = index[j] ;   // 终点
	//		m_naPredictPattern[4] = 1 ;   // 表示一对
	//		m_naPredictPattern[5] = index[k] ;   // 起点

	//		Generating2() ;
	//	}
	//}

	// index 编号从1开始，故应该减1
	int index[] = {32,34,35,38,42,46,47,51,53,57,59,63, 97,99,101,104,106,109,110,113,115,119,122,125, 245,247,248,249,251,252,253,254,257,258,261,262,264,266,267,268,269,271,272,274,276} ;
	for (k=0; k<45; k++)
	{
		for (j=index[k]; j<382; j++)  // j 终点编号，j起点为0
		{
			m_naPredictPattern[0] = 1 ;       //
			m_naPredictPattern[1] = k + 1 ;  // 起点编号
			m_naPredictPattern[2] = j - index[k] + 1 ;  // 该起点预测编号
			m_naPredictPattern[3] = j ;   // 终点
			m_naPredictPattern[4] = 1 ;   // 表示一对
			m_naPredictPattern[5] = index[k]-1 ;   // 起点

			Generating2() ;
		}
	}

	return 0 ;
}


int CGenerateInputFiles::Generating2()
{
	char buffer[MAX_STR_LEN] ;
	int nLandsatBandNum[] = {1,2,3,4,5,7} ;    // Landsat-7
	//int nLandsatBandNum[] = {2,3,4,5,6,7} ;  // Landsat-8
	int nModisBandNum[] = {3,4,1,2,6,7} ;      // MODIS
	int i,iband ;

	for (iband=0; iband<6; iband++)
	{
		sprintf(buffer,"%sinput_%d_%d_%d_%d%03d_band%d.txt", m_sOutputPath.c_str(), m_naPredictPattern[0],
			m_naPredictPattern[1],
			m_naPredictPattern[2],
			m_naTimeSeries[m_naPredictPattern[3]][0],
			m_naTimeSeries[m_naPredictPattern[3]][1],
			nLandsatBandNum[iband]) ;

		std::cout<< buffer <<std::endl ;
		FILE* pFileInput = fopen(buffer, "w") ;

		// 开始写入
		fprintf(pFileInput, "STARFM_PARAMETER_START\n") ;
		fprintf(pFileInput, "# number of input pairs of fine and coarse resolution image\n") ;
		fprintf(pFileInput, "  NUM_IN_PAIRS = %d\n", m_naPredictPattern[4]) ;

		fprintf(pFileInput, "# input coarse resolution data (saved in 2 bytes / pixel)\n") ;
		fprintf(pFileInput, "  IN_PAIR_MODIS_FNAME =") ;
		for (i=0; i<m_naPredictPattern[4]; i++)
		{
			fprintf(pFileInput, " M%d%03d_band%d.img", m_naTimeSeries[m_naPredictPattern[5+i]][2], m_naTimeSeries[m_naPredictPattern[5+i]][3], nModisBandNum[iband]) ;

		}
		fprintf(pFileInput, "\n") ;

		fprintf(pFileInput, "# input fine resolution data\n") ;
		fprintf(pFileInput, "  IN_PAIR_LANDSAT_FNAME =") ;
		for (i=0; i<m_naPredictPattern[4]; i++)
		{
			fprintf(pFileInput, " L%d%03d_band%d.img", m_naTimeSeries[m_naPredictPattern[5+i]][0], m_naTimeSeries[m_naPredictPattern[5+i]][1], nLandsatBandNum[iband]) ;

		}
		fprintf(pFileInput, "\n") ;

		fprintf(pFileInput, "# input coarse resolution data mask (saved in 1 byte / pixel)\n") ;
		fprintf(pFileInput, "  IN_PAIR_MODIS_MASK =") ;   // modis mask
		for (i=0; i<m_naPredictPattern[4]; i++)
		{
			fprintf(pFileInput, " M%d%03d_mask.img", m_naTimeSeries[m_naPredictPattern[5+i]][2], m_naTimeSeries[m_naPredictPattern[5+i]][3]) ;

		}
		fprintf(pFileInput, "\n") ;

		fprintf(pFileInput, "# input fine resolution data mask (saved in 1 pixel /pixel)\n") ;
		fprintf(pFileInput, "  IN_PAIR_LANDSAT_MASK =") ;   // landsat mask
		for (i=0; i<m_naPredictPattern[4]; i++)
		{
			fprintf(pFileInput, " L%d%03d_cfmask.img", m_naTimeSeries[m_naPredictPattern[5+i]][0], m_naTimeSeries[m_naPredictPattern[5+i]][1]) ;

		}
		fprintf(pFileInput, "\n") ;

		fprintf(pFileInput, "# coarse resolution data for the prediction day\n") ;
		fprintf(pFileInput, "  IN_PDAY_MODIS_FNAME = M%d%03d_band%d.img\n", m_naTimeSeries[m_naPredictPattern[3]][2], m_naTimeSeries[m_naPredictPattern[3]][3], nModisBandNum[iband]) ;

		fprintf(pFileInput, "# the output fine resolution prediction\n") ;
		fprintf(pFileInput, "  OUT_PDAY_LANDSAT_FNAME = pre_%d_%d_%d_%d%03d_band%d.img\n", m_naPredictPattern[0],
			m_naPredictPattern[1],
			m_naPredictPattern[2],
			m_naTimeSeries[m_naPredictPattern[3]][0],
			m_naTimeSeries[m_naPredictPattern[3]][1],
			nLandsatBandNum[iband]) ;
		fprintf(pFileInput, "# number of rows (same for all inputs)\n") ;
		fprintf(pFileInput, "  NROWS = %d\n",m_pLandsatInfo->m_nRows) ;

		fprintf(pFileInput, "# number of columns (same for all inputs)\n") ;
		fprintf(pFileInput, "  NCOLS = %d\n",m_pLandsatInfo->m_nCols) ;

		fprintf(pFileInput, "# spatial resolution (same for all inputs), coarse resolution data\n") ;
		fprintf(pFileInput, "# should first be resampled to fine resolution\n") ;
		fprintf(pFileInput, "  RESOLUTION = %d\n", static_cast<int>(m_pLandsatInfo->m_dPixelXSize+0.5) ) ;

		fprintf(pFileInput, "# define data scale factor\n") ;
		fprintf(pFileInput, "  SCALE_FACTOR = 10000\n") ;
		fprintf(pFileInput, "# define metadata for fine resolution input \n") ;
		fprintf(pFileInput, "  LANDSAT_FILLV = -9999\n") ;
		fprintf(pFileInput, "  LANDSAT_DATA_RANGE = 0, 10000\n") ;
		fprintf(pFileInput, "  LANDSAT_UNCERTAINTY = 50\n") ;
		fprintf(pFileInput, "# define metadata for coarse resolution input\n") ;
		fprintf(pFileInput, "  MODIS_FILLV = -9999\n") ;
		fprintf(pFileInput, "  MODIS_DATA_RANGE = 0, 10000 \n") ;
		fprintf(pFileInput, "  MODIS_UNCERTAINTY = 50\n") ;
		fprintf(pFileInput, "# 1 = use spatial information (suggested)\n") ;
		fprintf(pFileInput, "# 0 = not use spatial information (just use information from current pixel)\n") ;
		fprintf(pFileInput, "  USE_SPATIAL_FLAG = 1\n") ;
		fprintf(pFileInput, "# maximum distance to searching for the spatial information \n") ;
		fprintf(pFileInput, "  MAX_SEARCH_DISTANCE = 750\n") ;
		fprintf(pFileInput, "# number of slice for spectral similar testing\n") ;
		fprintf(pFileInput, "  NUM_SLICE_PURE_TEST = 40\n") ;
		fprintf(pFileInput, "# minimum acceptable percent of samples within searching window\n") ;
		fprintf(pFileInput, "  MIN_SAMPLE_PERCENT = 2\n") ;
		fprintf(pFileInput, "STARFM_PARAMETER_END\n") ;

		fclose(pFileInput) ;
	}// iband

	return 0 ;
}

/****************************************************************************************************************************
* naStartPreTime[] -记录的是每个季节中Landsat影像的中间时间影像列表的编号（即naTimeSeries的行编号），且从0开始编号； 
*
****************************************************************************************************************************/
int CGenerateInputFiles::GetSeasonMidDay(int naTimeSeries[][4], int nActualLen, int naStartPreTime[], int &nStartPreTimeLen)
{
	// 春分，夏至，秋分，冬至的日期假设是固定的
	int SpringEquinox = 81 ;
	int	SummerSolstice = 173 ;
	int FallEquinox	= 267 ;
	int WinterSolstice = 358 ;

	int nCursor1 =0, nCursor2 = 0, nStart = 0, nEnd = 0 ;
	while (nCursor1<nActualLen)
	{
		// 判断前冬天
		if (naTimeSeries[nCursor1][1]<SpringEquinox)
		{
			nStart = nCursor1 ;
			while( naTimeSeries[++nCursor1][0] == naTimeSeries[nStart][0] && naTimeSeries[nCursor1][1] < SpringEquinox )
				;
			nEnd = nCursor1 - 1 ;
			naStartPreTime[nCursor2] = (nStart + nEnd)/2 ;
			nCursor2++ ;
		}// if

		// 判断春天
		if (naTimeSeries[nCursor1][1]<SummerSolstice)
		{
			nStart = nCursor1 ;
			while( naTimeSeries[++nCursor1][0] == naTimeSeries[nStart][0] && naTimeSeries[nCursor1][1] < SummerSolstice )
				;
			nEnd = nCursor1 - 1 ;
			naStartPreTime[nCursor2] = (nStart + nEnd)/2 ;
			nCursor2++ ;
		}// if

		// 判断夏天
		if (naTimeSeries[nCursor1][1]<FallEquinox)
		{
			nStart = nCursor1 ;
			while( naTimeSeries[++nCursor1][0] == naTimeSeries[nStart][0] && naTimeSeries[nCursor1][1] < FallEquinox )
				;
			nEnd = nCursor1 - 1 ;
			naStartPreTime[nCursor2] = (nStart + nEnd)/2 ;
			nCursor2++ ;
		}// if

		// 判断秋天
		if (naTimeSeries[nCursor1][1]<WinterSolstice)
		{
			nStart = nCursor1 ;
			while( naTimeSeries[++nCursor1][0] == naTimeSeries[nStart][0] && naTimeSeries[nCursor1][1] < WinterSolstice )
				;
			nEnd = nCursor1 - 1 ;
			naStartPreTime[nCursor2] = (nStart + nEnd)/2 ;
			nCursor2++ ;
		}// if

		// 判断后冬天
		if (naTimeSeries[nCursor1][1]>=WinterSolstice)
		{
			nStart = nCursor1 ;
			while( (naTimeSeries[++nCursor1][0] == naTimeSeries[nStart][0] && naTimeSeries[nCursor1][1] >=WinterSolstice) ||
						(naTimeSeries[nCursor1][0] == 1 + naTimeSeries[nStart][0] && naTimeSeries[nCursor1][1] < SpringEquinox ) )
				;
			nEnd = nCursor1 - 1 ;
			naStartPreTime[nCursor2] = (nStart + nEnd)/2 ;
			nCursor2++ ;
		}// if

	}// while
	nStartPreTimeLen = nCursor2 ;

	return 0;
}
