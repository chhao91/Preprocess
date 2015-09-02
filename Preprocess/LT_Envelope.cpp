#include "LT_Envelope.h"

LT_Envelope::LT_Envelope()
{
	// x1, x2, y1, y2
	m_array[0] = 0 ;
	m_array[1] = 0 ;
	m_array[2] = 0 ;
	m_array[3] = 0 ;
}

LT_Envelope::~LT_Envelope()
{

}

double LT_Envelope::getMinX()
{
	return m_array[0] ;
}
double LT_Envelope::getMaxX()
{
	return m_array[1] ;
}
double LT_Envelope::getMinY()
{
	return m_array[3] ;
}
double LT_Envelope::getMaxY()
{
	return m_array[2] ;
}
void LT_Envelope::init(double x1, double x2, double y1, double y2)
{
	m_array[0] = x1 ;
	m_array[1] = x2 ;
	m_array[2] = y1 ;
	m_array[3] = y2 ;
}
void LT_Envelope::setToNull()
{
	m_array[0] = 0 ;
	m_array[1] = 0 ;
	m_array[2] = 0 ;
	m_array[3] = 0 ;
}
