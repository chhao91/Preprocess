
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
*              此项目用于对MODIS的hdf数据和Landsat的解压数据进行预处理
* 对Landsat数据的预处理包括：
*   a. 读取第一幅影像中的xml文件中的LE坐标、投影、条带号、行列坐标信息，像元大小；
*   b. 将Landsat影像处理成.img格式、与第一幅影像相同行列数、相同的空间覆盖范围的影像；
*   c. 处理后的影像统一放在指定目录的平级文件夹landsat_p中；
* 
* 对MODIS数据的预处理包括：
*   a. 记录Landsat影像处理中使用的投影、行列号、像元大小信息；
*   b. 读取hdf文件，采用记录的信息将MODIS波段和掩模同时进行重投影、重采样和空间范围裁剪，使之与Landsat一一对应；
*   c. 处理后的结果统一放在指定目录的平级文件夹modis_p中；
* 
* 当前版本：v2.1
* 修改日期：20150403
*
* 当前版本：v2.0
* 创建日期：20150322
* 作者：    陈 浩
* 联系方式：    chhao91@163.com
* 
****************************************************************************************************************/

using namespace std ;

int ProcessMOD09GA(CLandsatinfo* pLandsatInfo, string sRoot="") ;
int ProcessMOD09A1(CLandsatinfo* pLandsatInfo, string sRoot="") ;
int ProcessLandsat(CLandsatinfo* pLandsatInfo, string sRoot="") ;

int test() ;

int main(int argc, char* argv[]) 
{
	// 请更改目标Landsat影像信息的完整文件名
	//string sPath = "J:\\fu_20150324\\landsat\\LC80350262013267-SC20150319213927\\LC80350262013267LGN00.xml" ;
	//string sPath = "J:\\fu_20150324\\LC80360262013274-SC20150319213837\\LC80360262013274LGN00.xml" ;
	//string sPath = "J:\\pstarfm_20150311\\landsat\\LE70370372000062-SC20150310080856\\LE70370372000062EDC00.xml" ;
	//CLandsatinfo landsatInfo ;
	//landsatInfo.InitializeXML(sPath) ;
	
	// 从envi标准文件初始化CLandsatinfo
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

	// 生成需要进行时空融合处理的头文件
	 //string sInputFile = "J:\\gaofeng_test_starfm_c\\InputFiles_2.txt" ;
	 //CGenerateInputFiles generateInputFiles ;
	 //generateInputFiles.Processing(sInputFile, &landsatInfo) ;

	// 时间序列
	//string sInputFile = "J:\\pstarfm_20150330\\input_txt3\\matchlist.txt" ;
	//CGenerateInputFiles generateInputFiles ;
	//generateInputFiles.Processing2(sInputFile, &landsatInfo) ;

	// 生成快视图
	//string sRoot = "J:\\pstarfm_20150330\\" ;
	//string sInputFile = "J:\\pstarfm_20150330\\quickView\\matchlist.txt" ;
	//CGenerateQuickView generateQuickView ;
	//generateQuickView.Processing(sRoot, sInputFile, &landsatInfo) ;

	// 计算评价指标 （还需要进行调试）
	//int nvStartNums[] = {32, 40, 49, 57} ;
	//string sRoot = "J:\\pstarfm_20150330\\" ;
	//string sLandsatPreFolder = "J:\\pstarfm_20150330\\result_20150409\\" ;
	//string sInputFile = "J:\\pstarfm_20150330\\matchlist.txt" ;
	//CGetEvaluateIndexes getEvaluateIndx ;
	//getEvaluateIndx.Precessing(sRoot, sLandsatPreFolder, sInputFile, &landsatInfo, nvStartNums, 4) ;

	//test() ;

	// 处理MOD09A1的BRDF图层（第12层，编码14）
	string sRoot = "J:\\pstarfm_20150330\\" ;
	ProcessMOD09A1(&landsatInfo, sRoot) ;

	system("pause") ;
	return 0 ;
}

int ProcessMOD09GA(CLandsatinfo* pLandsatInfo, string sRoot)
{
	// 处理MOD09GA
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
	// 处理MOD09A1
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
	modisProcess.Initialize(pLandsatInfo, &nvHdfBands, &nvOutputBands, 12, sRootPath, sOutputPath ) ;  // 处理12图层的BRDF -20150827
	modisProcess.Processing() ;

	return 0 ;
}

int ProcessLandsat(CLandsatinfo* pLandsatInfo, string sRoot)
{
	// 处理Landsat
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
	landsatProcess.Initialize( pLandsatInfo, sRootPath, sOutputPath) ; // 默认输出路径不可用，待修正
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
