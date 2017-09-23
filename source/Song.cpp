#include "../Song.h"
#include <sstream>
double Constrain(double in_, double from, double to)
{
	if (in_ < from)
	{
		return from;
	}
	else if (in_ > to)
	{
		return to;
	}
	else
	{
		return in_;
	}
}
int DOUBLE_TO_INT_MID(double in_f)
{
	int temp = (int)(in_f);
	double temp1 = in_f - temp;

	int out_i;
	if (temp1 > 0.5)
	{
		out_i = temp + 1;
	}
	else if (temp1 < (-0.5))
	{
		out_i = temp - 1;
	}
	else
	{
		out_i = temp;
	}
	return out_i;
}
/**
* double×ª»»Îªstring
*/
string DoubleToString(double d) {
	ostringstream os;
	if (os << d)
		return os.str();
	return "invalid conversion";
}
/**
* double×ªstring
*/
double StringToDouble(string str) {
	istringstream iss(str);
	double x;
	if (iss >> x)
		return x;
	return 0.0;
}