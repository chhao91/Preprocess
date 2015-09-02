#include "Utility.h"

CUtility::CUtility(void)
{
}

CUtility::~CUtility(void)
{
}


char* CUtility::GetFileName(const char* fullpathname) 
{
	char* save_name, *pos_begin, *pos_end; 
	int name_len; 
	name_len = strlen(fullpathname); 
	pos_begin = const_cast<char*>(fullpathname) + name_len ;
	while(*pos_begin != '\\' && pos_begin != fullpathname) 
		pos_begin--; 
	if( *pos_begin == '\\') 
	{ 
		pos_begin = pos_begin + 1 ;  
	} 
	else if(pos_begin == fullpathname )
	{
		pos_begin = const_cast<char*>(fullpathname) ;
	}

	pos_end = const_cast<char*>(fullpathname) + name_len ;
	while(*pos_end != '.' && pos_end != fullpathname) 
		pos_end--; 
	if(pos_end == fullpathname) 
	{ 
		pos_end = const_cast<char*>(fullpathname) + name_len ;  
	} 
	name_len = strlen(pos_begin) - strlen(pos_end) ; 
	save_name = (char*) malloc(name_len+1); 
	memcpy(save_name,pos_begin,name_len); 
	save_name[name_len] = '\0' ;

	return save_name; 
}


/************************************************************************
* @param fullpathname - 为完整路径                                     
* 	 G:\\Programs\\VS2010\\fileName.txt                                                               
*	   \\Programs\\VS2010\\fileName.txt                                 
*                       \\fileName.txt                                 
*                         fileName.txt                                 
* @return 返回文件路径 G:\6_Programs\VS2010\                           
* @PS: 返回的路径用完后记得释放文件名所占内存：free(str);             
/************************************************************************/
char * CUtility::GetFilePath(const char* fullpathname) 
{
	char* save_name, *pos; 
	int name_len; 
	name_len = strlen(fullpathname); 
	pos = const_cast<char*>(fullpathname) + name_len ;
	while(*pos != '\\' && pos != fullpathname) 
		pos--; 
	if( *pos == '\\' ) 
	{ 
		pos = pos + 1 ; 
	} 
	else if( pos == fullpathname )
	{
		return 0 ;
	}
	name_len =  pos - const_cast<char*>(fullpathname) ;
	// '\fileName.txt'
	if(name_len==1)
		return 0 ;

	save_name = (char*) malloc(name_len+1); 
	memcpy(save_name,fullpathname,name_len); 
	save_name[name_len] = '\0' ;

	return save_name; 
}


bool CUtility::Projection2ImageRowCol(double *adfGeoTransform, double dProjX, double dProjY, int &iCol, int &iRow)
{
	try
	{
		double dTemp = adfGeoTransform[1]*adfGeoTransform[5] - adfGeoTransform[2]*adfGeoTransform[4];
		double dCol = 0.0, dRow = 0.0;
		dCol = (adfGeoTransform[5]*(dProjX - adfGeoTransform[0]) - 
			adfGeoTransform[2]*(dProjY - adfGeoTransform[3])) / dTemp + 0.5;
		dRow = (adfGeoTransform[1]*(dProjY - adfGeoTransform[3]) - 
			adfGeoTransform[4]*(dProjX - adfGeoTransform[0])) / dTemp + 0.5;

		iCol = static_cast<int>(dCol);
		iRow = static_cast<int>(dRow);
		return true;
	}
	catch(...)
	{
		return false;
	}
}

bool CUtility::ImageRowCol2Projection(double *adfGeoTransform, int iCol, int iRow, double &dProjX, double &dProjY)
{
	//adfGeoTransform[6]  数组adfGeoTransform保存的是仿射变换中的一些参数，分别含义见下
	//adfGeoTransform[0]  左上角x坐标 
	//adfGeoTransform[1]  东西方向分辨率
	//adfGeoTransform[2]  旋转角度, 0表示图像 "北方朝上"
	//adfGeoTransform[3]  左上角y坐标 
	//adfGeoTransform[4]  旋转角度, 0表示图像 "北方朝上"
	//adfGeoTransform[5]  南北方向分辨率

	try
	{
		dProjX = adfGeoTransform[0] + adfGeoTransform[1] * iCol + adfGeoTransform[2] * iRow;
		dProjY = adfGeoTransform[3] + adfGeoTransform[4] * iCol + adfGeoTransform[5] * iRow;
		return true;
	}
	catch(...)
	{
		return false;
	}
}

int CUtility::OpencvDataType(GDALDataType gdalDataType)
{
	//#define CV_8U   0
	//#define CV_8S   1
	//#define CV_16U  2
	//#define CV_16S  3
	//#define CV_32S  4
	//#define CV_32F  5
	//#define CV_64F  6
	//#define CV_USRTYPE1 7
	switch (gdalDataType)
	{
	case GDT_Byte:
		return 0;					  
		break;
	case GDT_UInt16:
		return CV_16U;
		break;
	case GDT_Int16:
		return CV_16S;
		break;
	case GDT_UInt32:
		return CV_32S;
		break;
	case GDT_Int32:
		return CV_32S;
		break;
	case GDT_Float32:
		return CV_32F;
		break;
	case GDT_Float64:
		return CV_64F;
		break;
	default:
		return 7;
		break;
	}
}

GDALDataType CUtility::GdalDataType(int opencvDataType)
{
	//#define CV_8U   0
	//#define CV_8S   1
	//#define CV_16U  2
	//#define CV_16S  3
	//#define CV_32S  4
	//#define CV_32F  5
	//#define CV_64F  6
	//#define CV_USRTYPE1 7
	switch (opencvDataType)
	{
	case CV_8U:
		return GDT_Byte;					  
		break;
	case CV_16U:
		return GDT_UInt16;
		break;
	case CV_16S:
		return GDT_Int16;
		break;
	/*case CV_32S:
		return GDT_UInt32;*/
		break;
	case CV_32S:
		return GDT_Int32;
		break;
	case CV_32F:
		return GDT_Float32;
		break;
	case CV_64F:
		return GDT_Float64;
		break;
	default:
		return GDT_Unknown;
		break;
	}
}

/*********************************************************************************************************************************
* 从GDALDataset中读取数据时，按照GDALDataset中的数据类型写入matrix；
* 向GDALDataset中写入数据时，按照GDALDataset中的数据类型读取matrix；
*********************************************************************************************************************************/
bool CUtility::RasterDataIO(GDALDataset *pDataset, int gdalBandId, GDALRWFlag rwFlag, cv::Rect const &imgRect,cv::Mat &matrix)
{
	if (NULL==pDataset)
		return false;

	int iBandNum=pDataset->GetRasterCount();
	if (gdalBandId>iBandNum)
	{
		return false;
	}
	GDALRasterBand *pBand=pDataset->GetRasterBand(gdalBandId);
	if (NULL==pBand)
	{
		return false;
	}

	int imgXSize=pBand->GetXSize();
	int imgYSize=pBand->GetYSize();
	if (imgRect.width+imgRect.x>imgXSize||imgRect.height+imgRect.y>imgYSize)
	{
		return false;
	}

	GDALDataType gdalDateType= pDataset->GetRasterBand(1)->GetRasterDataType() ;
	int opencvDataType = OpencvDataType(gdalDateType) ;
	if (rwFlag==GF_Read)
	{
		matrix.create(imgRect.height,imgRect.width,opencvDataType);
		char *pDataBuffer=(char *)matrix.data;
		pBand->RasterIO(GF_Read,imgRect.x,imgRect.y,imgRect.width,imgRect.height,pDataBuffer,imgRect.width,imgRect.height,gdalDateType,0,0);
	} 
	else
	{
		char *pDataBuffer=(char *)matrix.data;
		pBand->RasterIO(GF_Write,imgRect.x,imgRect.y,imgRect.width,imgRect.height,pDataBuffer,imgRect.width,imgRect.height,gdalDateType,0,0);
	}
	return true;
}


