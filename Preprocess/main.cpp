
#include <string>
#include <vector>

#include "LandsatInfo.h"
#include "LandsatProcess.h"
#include "MODISProcess.h"
#include "GenerateInputFiles.h"
#include "GenerateQuickView.h"
#include "GetEvaluateIndexes.h"

#include "Utility.h"

/****************************************************************************************************************
*              ����Ŀ���ڶ�MODIS��hdf���ݺ�Landsat�Ľ�ѹ���ݽ���Ԥ����
* ��Landsat���ݵ�Ԥ���������
*   a. ��ȡ��һ��Ӱ���е�xml�ļ��е�LE���ꡢͶӰ�������š�����������Ϣ����Ԫ��С��
*   b. ��LandsatӰ�����.img��ʽ�����һ��Ӱ����ͬ����������ͬ�Ŀռ串�Ƿ�Χ��Ӱ��
*   c. ������Ӱ��ͳһ����ָ��Ŀ¼��ƽ���ļ���landsat_p�У�
* 
* ��MODIS���ݵ�Ԥ���������
*   a. ��¼LandsatӰ������ʹ�õ�ͶӰ�����кš���Ԫ��С��Ϣ��
*   b. ��ȡhdf�ļ������ü�¼����Ϣ��MODIS���κ���ģͬʱ������ͶӰ���ز����Ϳռ䷶Χ�ü���ʹ֮��Landsatһһ��Ӧ��
*   c. �����Ľ��ͳһ����ָ��Ŀ¼��ƽ���ļ���modis_p�У�
* 
* ��ǰ�汾��v2.1
* �޸����ڣ�20150403
*
* ��ǰ�汾��v2.0
* �������ڣ�20150322
* ���ߣ�    �� ��
* ��ϵ��ʽ��    chhao91@163.com
* 
****************************************************************************************************************/

using namespace std ;

int ProcessMOD09GA(CLandsatinfo* pLandsatInfo, string sRoot="") ;
int ProcessMOD09A1(CLandsatinfo* pLandsatInfo, string sRoot="") ;
int ProcessLandsat(CLandsatinfo* pLandsatInfo, string sRoot="") ;

int test() ;

int main(int argc, char* argv[]) 
{
	// �����Ŀ��LandsatӰ����Ϣ�������ļ���
	//string sPath = "J:\\fu_20150324\\landsat\\LC80350262013267-SC20150319213927\\LC80350262013267LGN00.xml" ;
	//string sPath = "J:\\fu_20150324\\LC80360262013274-SC20150319213837\\LC80360262013274LGN00.xml" ;
	//string sPath = "J:\\pstarfm_20150311\\landsat\\LE70370372000062-SC20150310080856\\LE70370372000062EDC00.xml" ;
	//CLandsatinfo landsatInfo ;
	//landsatInfo.InitializeXML(sPath) ;
	
	// ��envi��׼�ļ���ʼ��CLandsatinfo
	string sPath = "E:\\0\\resize.img" ;
	CLandsatinfo landsatInfo ;
	int stat = landsatInfo.InitializeENVI(sPath) ;
	if (stat<0)
	{
		return -1 ;
	}

	//string sRoot = "J:\\pstarfm_20150330\\" ;
	//ProcessLandsat(&landsatInfo, sRoot) ;
	//ProcessMOD09A1(&landsatInfo, sRoot) ;
	//ProcessMOD09GA(&landsatInfo, sRoot) ;

	// ������Ҫ����ʱ���ںϴ����ͷ�ļ�
	 //string sInputFile = "J:\\gaofeng_test_starfm_c\\InputFiles_2.txt" ;
	 //CGenerateInputFiles generateInputFiles ;
	 //generateInputFiles.Processing(sInputFile, &landsatInfo) ;

	// ʱ������
	//string sInputFile = "J:\\pstarfm_20150330\\input_txt3\\matchlist.txt" ;
	//CGenerateInputFiles generateInputFiles ;
	//generateInputFiles.Processing2(sInputFile, &landsatInfo) ;

	// ���ɿ���ͼ
	//string sRoot = "J:\\pstarfm_20150330\\" ;
	//string sInputFile = "J:\\pstarfm_20150330\\quickView\\matchlist.txt" ;
	//CGenerateQuickView generateQuickView ;
	//generateQuickView.Processing(sRoot, sInputFile, &landsatInfo) ;

	// ��������ָ�� ������Ҫ���е��ԣ�
	//int nvStartNums[] = {32, 40, 49, 57} ;
	//string sRoot = "J:\\pstarfm_20150330\\" ;
	//string sLandsatPreFolder = "J:\\pstarfm_20150330\\result_20150409\\" ;
	//string sInputFile = "J:\\pstarfm_20150330\\matchlist.txt" ;
	//CGetEvaluateIndexes getEvaluateIndx ;
	//getEvaluateIndx.Precessing(sRoot, sLandsatPreFolder, sInputFile, &landsatInfo, nvStartNums, 4) ;

	//test() ;

	// ����MOD09A1��BRDFͼ�㣨��12�㣬����14��
	string sRoot = "J:\\pstarfm_20150330\\" ;
	ProcessMOD09A1(&landsatInfo, sRoot) ;

	system("pause") ;
	return 0 ;
}

int ProcessMOD09GA(CLandsatinfo* pLandsatInfo, string sRoot)
{
	// ����MOD09GA
	string sRootPath, sOutputPath ; 

	if (sRoot.empty())
	{
		sRootPath = "J:\\fu_20150324\\modis\\" ;
		sOutputPath = "J:\\fu_20150324\\modis_p\\";
	}
	else
	{
		sRootPath = sRoot + "modis\\" ;
		sOutputPath = sRoot + "modis_p\\" ;
	}
	int naHdfBands[] = {11, 12, 13, 14, 15, 16, 17} ;
	vector<int> nvHdfBands(naHdfBands, naHdfBands+7) ;
	int naOutputBands[] = {1,2,3,4,5,6,7} ;
	vector<int> nvOutputBands(naOutputBands, naOutputBands+7) ;
	CMODISProcess modisProcess ;
	modisProcess.Initialize(pLandsatInfo, &nvHdfBands, &nvOutputBands, 18, sRootPath, sOutputPath ) ;
	modisProcess.Processing() ;

	return 0 ;
}

int ProcessMOD09A1(CLandsatinfo* pLandsatInfo, string sRoot)
{
	// ����MOD09A1
	string sRootPath, sOutputPath ; 

	if (sRoot.empty())
	{
		sRootPath = "J:\\pstarfm_20150311\\mod09a1\\" ;
		sOutputPath = "J:\\pstarfm_20150311\\temp\\";
	}
	else
	{
		sRootPath = sRoot + "modis\\" ;
		sOutputPath = sRoot + "modis_p\\" ;
	}
	int naHdfBands[] = {1,2, 3, 4, 5, 6, 7} ;
	vector<int> nvHdfBands(naHdfBands, naHdfBands+7) ;
	int naOutputBands[] = {1,2,3,4,5,6,7} ;
	vector<int> nvOutputBands(naOutputBands, naOutputBands+7) ;
	CMODISProcess modisProcess ;
//	modisProcess.Initialize(pLandsatInfo, &nvHdfBands, &nvOutputBands, 8, sRootPath, sOutputPath ) ;
	modisProcess.Initialize(pLandsatInfo, &nvHdfBands, &nvOutputBands, 12, sRootPath, sOutputPath ) ;  // ����12ͼ���BRDF -20150827
	modisProcess.Processing() ;

	return 0 ;
}

int ProcessLandsat(CLandsatinfo* pLandsatInfo, string sRoot)
{
	// ����Landsat
	string sRootPath, sOutputPath ;

	if (sRoot.empty())
	{
		sRootPath = "J:\\fu_20150324\\landsat\\" ;
		sOutputPath = "J:\\fu_20150324\\landsat_p\\" ;
	}
	else
	{
		sRootPath = sRoot + "landsat\\" ;
		sOutputPath = sRoot + "landsat_p\\" ;
	}
	CLandsatProcess landsatProcess ;
	landsatProcess.Initialize( pLandsatInfo, sRootPath, sOutputPath) ; // Ĭ�����·�������ã�������
	landsatProcess.Processing() ;

	return 0;
}



int test()
{
	//string sSrcFile = "C:\\Users\\chenhao\\Desktop\\2001MOD\\MOD09A1.A2001185.h08v05.hdf" ;
	//string sFileOut = "C:\\Users\\chenhao\\Desktop\\2001MOD\\temp.img" ;

	//int nBandNum = 2 * (1-1) ;

	//GDALAllRegister();

	//GDALDatasetH hSrcDS, hDstDS ;
	//GDALDriverH hDriver ;
	//int i ;

	////
	//GDALAllRegister(); 
	//hSrcDS = GDALOpen( sSrcFile.c_str(), GA_ReadOnly );
	//CPLAssert( hSrcDS != NULL );

	//char ** papszSUBDATASETS = GDALGetMetadata( hSrcDS, "SUBDATASETS");

	//std::string tmpstr = std::string(papszSUBDATASETS[nBandNum]);
	//tmpstr = tmpstr.substr(tmpstr.find_first_of("=") + 1);
	//const char *tmpc_str = tmpstr.c_str();	

	//GDALClose(hSrcDS) ;

	//CPLSetConfigOption("GDAL_FILENAME_IS_UTF8","NO");     
	//hSrcDS = GDALOpen(tmpc_str, GA_ReadOnly);    
	//if (hSrcDS == NULL)    
	//{    
	//	return RE_NOFILE ;    
	//}    
	//int nWidth= GDALGetRasterXSize(hSrcDS) ;
	//int nHeight=GDALGetRasterXSize(hSrcDS) ;  
	//int nBandCount = GDALGetRasterCount(hSrcDS) ;     
	//GDALDataType dataType = GDALGetRasterDataType(GDALGetRasterBand(hSrcDS,1)) ;
	//char *pszSrcWKT = NULL ;  
	//pszSrcWKT = const_cast<char *>( GDALGetProjectionRef(hSrcDS) );  
	//double dGeoTrans[6] ;
	//GDALGetGeoTransform(hSrcDS, dGeoTrans) ;

	//hDriver = GetGDALDriverManager()->GetDriverByName("ENVI") ;    
	//if (hDriver == NULL)    
	//{    
	//	GDALClose(hSrcDS);    
	//	return RE_PARAMERROR ;    
	//} 
	//hDstDS = GDALCreate(hDriver, sFileOut.c_str(), nWidth, nHeight, nBandCount, GDT_Byte, NULL) ;
	//if (hDstDS == NULL)    
	//{   
	//	GDALClose(hSrcDS);    
	//	return RE_PARAMERROR ;    
	//} 
	//GDALSetGeoTransform(hDstDS, dGeoTrans) ;
	//GDALSetProjection(hDstDS, pszSrcWKT) ;

	//GDALRasterBandH hSrcRasterBand = GDALGetRasterBand(hSrcDS, 1) ;
	//GDALRasterBandH hDstRasterBand = GDALGetRasterBand(hDstDS, 1) ;

	//cv::Mat rowMatInt32, rowMatByte8 ;
	//rowMatInt32.create(1, nWidth, CUtility::OpencvDataType(dataType)) ;
	//rowMatByte8.create(1, nWidth, CV_8S) ;
	//unsigned int *pValueInt32 = (unsigned int *)(rowMatInt32.data) ;
	//char *pValueByte8 = (char *)(rowMatByte8.data) ;
	//unsigned int codeInt32 ;

	//for (i=0; i<nHeight; i++)
	//{
	//	GDALRasterIO(hSrcRasterBand, GF_Read, 0, i, nWidth, 1, rowMatInt32.data, nWidth, 1, dataType, 0, 0) ;
	//	for (j=0; j<nWidth; j++)
	//	{
	//		codeInt32 = *(pValueInt32+j) ;
	//		*(pValueByte8+j) = Code2Mask(codeInt32) ;		
	//	}
	//	GDALRasterIO(hDstRasterBand, GF_Write, 0, i, nWidth, 1, rowMatByte8.data, nWidth, 1, GDALDataType(CV_8S), 0, 0) ;
	//}

	//GDALClose(hSrcDS) ;
	//GDALClose(hDstDS) ;
	
	return 1 ;
}
