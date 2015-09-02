

class LT_Envelope
{
	
	

public:
	LT_Envelope() ;
	~LT_Envelope() ;

	double getMinX() ;
	double getMaxX() ;
	double getMinY() ;
	double getMaxY() ;
	void init(double x1, double x2, double y1, double y2) ;
	void setToNull() ;

private:
	double m_array[4] ;
};