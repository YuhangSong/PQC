#pragma once

#include <list>
#include <algorithm>
#include <functional>
#include <fstream>
#include <sstream>

using namespace std;

#define	GetCTU_Num 0
#define MaxFrame 800

#define MinQP 0
#define MaxQP 51

double Constrain(double in_, double from, double to);
int DOUBLE_TO_INT_MID(double in_f);
#define random(x) (rand()%x)


/**
* double×ª»»Îªstring
*/
string DoubleToString(double d);
/**
* double×ªstring
*/
double StringToDouble(string str);

class TopMassage_T
{
public:
	TopMassage_T() {
	}
	~TopMassage_T() {
	}

	void setCurQP(double qp) {
		CurQP = qp;
		Compute_nQP(qp);
	}
	void Compute_nQP(double qp) {
		if (GetCTU_Num == 1)
		{
			fprintf(stdout, "GetCTU_Num=%d\n", CTU_Pointer);
		}
		CTU_Pointer = 0;

		int temp = (int)qp;
		double fra = qp - temp;
		double want = fra*CTU_Num;
		int can = DOUBLE_TO_INT_MID(want);

		for (int i = 0; i < CTU_Num; i++)
		{
			Cur_nQP[i] = temp;
		}

		int plus_index[510];
		for (int i = 0; i < can; )
		{
			int temp_index = random(CTU_Num - 1);
			bool found = false;
			for (int ii = 0; ii < i; ii++)
			{
				if (plus_index[ii] == temp_index) {
					found = true;
				}
			}

			if (found == false)
			{
				plus_index[i] = temp_index;
				Cur_nQP[temp_index] = temp + 1;
				i++;
			}
		}
	}
	int getCur_nQP() {
		int re;
		if (GetCTU_Num==1)
		{
			re = DOUBLE_TO_INT_MID(CurQP);
		}
		else
		{
			re = Cur_nQP[CTU_Pointer];
		}
		return re;
	}
	int getCurQP() {
		int re = DOUBLE_TO_INT_MID(CurQP);
		return re;
	}
	void setCurFrame(int a) {
		CurFrame = a;
	}
	int getCurFrame() {
		return CurFrame;
	}
	void setCurPSNR(double a) {
		CurPSNR = a;
	}
	double getCurPSNR() {
		return CurPSNR;
	}
	void Init() {

		if ((getRunType()==PIDRUN_AutoTarget)||getRunType()==COMRUN)
		{
			long DataTypePointer = 0;
			int Cur_Char;
			int QP = 0;
			int Frame = 1;
			double PSNR = 0.0;

			double NextQP = 0.0;
			double CurQP = 0.0;
			double P_Term = 0.0;
			double D_Term = 0.0;


			double Curve_PSNR[MaxFrame];
			double Curve_QP[MaxFrame];

			double Curve_NextQP[MaxFrame];
			double Curve_CurQP[MaxFrame];
			double Curve_P_Term[MaxFrame];
			double Curve_D_Term[MaxFrame];

			int FrameTotal = 0;

			string tt = "G://DC/Project/RawData/" + getSequenceName() + "/" + DoubleToString(getCompareQp()) + "/" + "PSNR.txt";
			FILE *stream = fopen(tt.c_str(), "r+");
			int a, b, c;
			while (4 == fscanf(stream, "%ld\t%ld\t%ld\t%lf\n", &a, &b, &c, &Curve_PSNR[FrameTotal])) {
				FrameTotal++;
			}

			double Sum = 0.0;
			for (int i = 0; i < FrameTotal; i++)
			{
				Sum += Curve_PSNR[i];
			}
			double Avr = Sum / FrameTotal;

			setTarget(Avr);

			setCurQP(getCompareQp());
		}
		setCurQP(getCompareQp());
	}
	void RunControl() {

		if ((getRunType()==PIDRUN_AutoTarget)||(getRunType() == PIDRUN_CustomTarget)||(getRunType()==OrgRun)||(getRunType()==PIDRUN_RateControl))
		{
			static double i_wanted_qp_for_next_frame = getCompareQp();
			static double last_delt = 0;
			static bool first_run = true;

			int cur_frame = getCurFrame();

			double cur_qp = getCurQP();

			double cur_psnr=0.0;

			if (getRunType() == PIDRUN_RateControl) {
				cur_psnr = getBitRate();
			}
			else
			{
				cur_psnr = getCurPSNR();
			}
			

			 

			double target = 0.0;
			if (getRunType() == PIDRUN_AutoTarget)
			{
				target = getTarget();
			}
			else if(getRunType() == PIDRUN_CustomTarget)
			{
				target = getCustomTarget();
			}else if (getRunType() == PIDRUN_RateControl)
			{
				target = getTargetBitRate();
			}

			double cur_delt;
			if (getRunType() == PIDRUN_RateControl)
			{
				if (first_run == true)
				{
					cur_delt = 0.0;
				}
				else
				{
					cur_delt = cur_psnr - target;
				}
			}
			else
			{
				cur_delt = cur_psnr - target;
			}

			double P_Param = getKp(), I_Param = getKi(), D_Param = getKd();

			double pid_term[3];
			static int P = 0, I = 1, D = 2, DD = 3, C = 4;

			pid_term[P] = cur_delt;

			if (first_run == true)
			{
				pid_term[D] = 0;
			}
			else
			{
				pid_term[D] = cur_delt - last_delt;
			}

			double out = pid_term[P] * P_Param + pid_term[D] * D_Param;

			double wanted_qp_for_next_frame = out;


			i_wanted_qp_for_next_frame = Constrain((wanted_qp_for_next_frame + i_wanted_qp_for_next_frame), MinQP, MaxQP);

			if ((i_wanted_qp_for_next_frame == MinQP) || (i_wanted_qp_for_next_frame == MaxQP))
			{
				fprintf(stdout, "TouchEdge\n");
			}

			double next_QP = i_wanted_qp_for_next_frame;

			//record part
			last_delt = cur_delt;

			//store data
			store_data(getTarget(), next_QP, cur_qp, pid_term[P], pid_term[D]);

			if (getRunType()==OrgRun)
			{
				next_QP = getCompareQp();
			}
			setCurQP(next_QP);
			first_run = false;
		}
		else if(getRunType()==COMRUN)
		{
			static double lp;
			double cp = getCurPSNR();
			double tp = (cp + lp) / 2;

			int cur_frame = getCurFrame();

			double cur_qp = getCurQP();

			double cur_psnr = getCurPSNR();

			double next_QP = 0.0;
			if (cur_psnr>tp)
			{
				next_QP = cur_qp + 1;
			}
			else
			{
				next_QP = cur_qp - 1;
			}

			//record part
			lp = cp;

			//store data
			store_data(getCompareQp(), next_QP, cur_qp, 0, 0);

			setCurQP(next_QP);
		}

		totalpsnr += getCurPSNR();
		totalframe++;
		
	}
	int totalframe = 0;
	double totalpsnr = 0.0;
	void store_data(double target,double next_QP,double cur_qp,double pid_term_p,double pid_term_d) {
		//store data
		FILE *fp;
		const char *PathChar;

		string PathString;
		PathString = getFileStorePath();
		PathString = PathString + "/";
		PathString = PathString + getMainDescription();

		PathString = PathString + "-";
		PathString = PathString + getSequenceName();

		PathString = PathString + "-";
		PathString = PathString + "CompareQp";
		PathString = PathString + DoubleToString(getCompareQp());

		PathString = PathString + "-";
		PathString = PathString + "Target";
		PathString = PathString + DoubleToString(target);

		PathString = PathString + "-";
		PathString = PathString + "Kp";
		PathString = PathString + DoubleToString(getKp());

		PathString = PathString + "-";
		PathString = PathString + "Ki";
		PathString = PathString + DoubleToString(getKi());

		PathString = PathString + "-";
		PathString = PathString + "Kd";
		PathString = PathString + DoubleToString(getKd());

		PathString = PathString + ".txt";


		PathChar = PathString.c_str();
		fp = fopen(PathChar, "a+");
		fseek(fp, 0, SEEK_END);
		char to_write[100];
		sprintf(to_write, "%d;%d;%lf;%lf;%lf;%lf;%lf;%d\r\n", getCurQP(), getCurFrame(), getCurPSNR(), (double)next_QP, (double)cur_qp, pid_term_p, pid_term_d, getBitRate());
		fwrite(to_write, strlen(to_write), 1, fp);
		fclose(fp);
	}
	double getAvrPSNR() {
		return totalpsnr / totalframe;
	}
	void PushCTUPointer() {
		if (GetCTU_Num==1)
		{
			fprintf(stdout, "CTU_Pointer=%d\n", CTU_Pointer);
		}
		CTU_Pointer++;
	}
	void setTarget(double a) {
		Target = a;
		fprintf(stdout, "setTarget(%f)\n", Target);
	}
	double getTarget() {
		return Target;
	}
	double getLambda(int qp) {
		double re;
		//Int estQP = Int( 4.2005 * log( lambda ) + 13.7122 + 0.5 );
		re = exp(((double)qp - 13.7122 - 0.5) / 4.2005);
		//fprintf(stdout, "ttttttt\n");
		return re;
	}
	void setSequenceName(string s) {
		size_t iPos = 23;
		string s2 = s.substr(0, iPos);
		string s3 = s.substr(iPos + 1, s.length() - iPos - 1);

		s = s3;
		iPos = s.find(".");
		s2 = s.substr(0, iPos);
		s3 = s.substr(iPos + 1, s.length() - iPos - 1);
		SequenceName = s2;
	}
	string getSequenceName() {
		return SequenceName;
	}
	void setMainDescription(string a) {
		MainDescription = a;
	}
	string getMainDescription() {
		return MainDescription;
	}
	void setFileStorePath(string a) {
		FileStorePath = a;
	}
	string getFileStorePath(){
		return FileStorePath;
	}
	double getKp() {
		return Kp;
	}
	double getKi() {
		return Ki;
	}
	double getKd() {
		return Kd;
	}
	void setKp(double a) {
		Kp = a;
	}
	void setKi(double a) {
		Ki = a;
	}
	void setKd(double a) {
		Kd = a;
	}
	void setCompareQp(int a) {
		CompareQp = a;
	}
	int getCompareQp() {
		return CompareQp;
	}
	void setBitRate(unsigned int a) {
		BitRate = a;
	}
	unsigned int getBitRate() {
		return BitRate;
	}
	void setCTU_Num(int a) {
		CTU_Num = a;
	}
	int getCTU_Num() {
		return CTU_Num;
	}
	void setWidth(int a) {
		Width = a;
		if (Width == 1920)
		{
			setCTU_Num(510);
		}
		else if (Width==1280)
		{
			setCTU_Num(240);
		}
		else if (Width == 832)
		{
			setCTU_Num(104);
		}
		else if (Width == 416)
		{
			setCTU_Num(28);
		}
		else if (Width == 352)
		{
			setCTU_Num(30);
		}
	}
	int getWidth() {
		return Width;
	}
	static const int PIDRUN_AutoTarget = 1;
	static const int COMRUN = 2;
	static const int OrgRun = 3;
	static const int PIDRUN_CustomTarget = 4;
	static const int PIDRUN_RateControl = 5;
	void setRunType(int a) {
		RunType = a;
	}
	int getRunType() {
		return RunType;
	}
	void setCustomTarget(double a) {
		CustomTarget = a;
	}
	double getCustomTarget() {
		return CustomTarget;
	}
	void setTargetBitRate(double a) {
		TargetBitRate = a;
	}
	double getTargetBitRate() {
		return TargetBitRate;
	}
private:
	double CurQP;
	int CurFrame;
	double CurPSNR;
	int Cur_nQP[510];
	int CTU_Pointer = 0;
	double Target;
	string SequenceName;
	string FileStorePath = "G://DC/Project/FinalDataBase";
	string MainDescription;
	double Kp = 2.12, Ki=0.0, Kd = 0.6;
	int CompareQp;
	unsigned int BitRate;
	int CTU_Num;
	int Width;
	int RunType;
	double CustomTarget;
	double TargetBitRate;
	//1920 510
	//1280 240
	//832 104
	//426 28
};
