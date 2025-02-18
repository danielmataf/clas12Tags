// G4 headers
#include "G4Poisson.hh"
#include "Randomize.hh"

#include <CCDB/Calibration.h>
#include <CCDB/Model/Assignment.h>
#include <CCDB/CalibrationGenerator.h>
using namespace ccdb;

// gemc headers
#include "myatof_hitprocess.h"

// CLHEP units
#include "CLHEP/Units/PhysicalConstants.h"
using namespace CLHEP;

static myatofConstants initializeMYATOFConstants(int runno, string digiVariation = "default") {
	myatofConstants atc;
	
	// do not initialize at the beginning, only after the end of the first event,
	// with the proper run number coming from options or run table
	if (runno == -1) return atc;
	
	atc.runNo = runno;
	atc.date = "2020-04-20";
	if (getenv("CCDB_CONNECTION") != NULL)
		atc.connection = (string) getenv("CCDB_CONNECTION");
	else
		atc.connection = "mysql://clas12reader@clasdb.jlab.org/clas12";
	
	return atc;
}


// this methos is for implementation of digitized outputs and the first one that needs to be implemented.
map<string, double> myatof_HitProcess::integrateDgt(MHit* aHit, int hitn) {
	
	// digitized output
	map<string, double> dgtz;
	// hit ids
	vector<identifier> identity = aHit->GetId();

	/*
	int sector;
	int superlayer;
	int layer;
	int paddle;
	double adc = -1.0;
	double time = -1.0;
	*/

	if(aHit->isBackgroundHit == 1) {

		vector<double>        stepTime    = aHit->GetTime();
			cout << " This is a background hit with time " << stepTime[0] << endl;
		dgtz["sector"]     = 0;
		dgtz["superlayer"]      = 0;
		dgtz["layer"]      = 0;
		dgtz["paddle"]       = 0;
		dgtz["time"]        = stepTime[0];
		dgtz["hitn"]       = hitn;

		if(filterDummyBanks == false) {
			dgtz["adc"]       = 0;
		}
		return dgtz;
	}
	
	trueInfos tInfos(aHit);

	double length = aHit->GetDetector().dimensions[0];
	//cout << "paddle half-length in Z (mm) = " << length << endl;
	// Need to know the (x,y) coordinate of the paddle vertices for the largest radius
	double dim_3, dim_4, dim_5, dim_6, l_topXY, l_a, l_b;
	
	dim_3 = aHit->GetDetector().dimensions[3];
	dim_4 = aHit->GetDetector().dimensions[4];
	dim_5 = aHit->GetDetector().dimensions[5];
	dim_6 = aHit->GetDetector().dimensions[6];

	l_topXY = sqrt( pow((dim_3 - dim_5),2) + pow((dim_4 - dim_6),2) );
	//cout << "paddle top length in XY (l_topXY in mm) = " << l_topXY << endl;	

	vector<G4double>      Edep        = aHit->GetEdep();
	// local variable for each step
	vector<G4ThreeVector> Lpos        = aHit->GetLPos();
	vector<double> times = aHit->GetTime();
	
	double adc_CC_front, adc_CC_back, adc_CC_top, tdc_CC_front, tdc_CC_back, tdc_CC_top;

	double LposX=0.0;
	double LposY=0.0;
	double LposZ=0.0;

	//Simple output not equal to real physics, just to feel the adc, time values
	//Should be: double energy = tInfos.eTot*att;

	double totEdep=0.0;
	
	//This is for superlayer 0 paddles!!!
	//Distance calculation from the hit to the front or back SIPM, superlayer 0!
	double dFront = 0.0;
	double dBack = 0.0; 
	double e_Front = 0.0;
	double e_Back = 0.0;
	double E_tot_Front = 0.0;
	double E_tot_Back = 0.0;
	
	//For superlayer 1, only one SiPM per paddle, and on the top!
	double H_hit_SiPM = 0.0;
	double e_Top = 0.0;
	double E_tot_Top = 0.0;

	double attlength = 1600.0; // here in mm! because all lengths from the volume are in mm! EJ-204 160 cm
	double pmtPEYld = 1400.0; // EJ-204 10400 (photons / [1MeV*e-])
	double dEdxMIP = 1.956; // energy deposited by MIP per cm of scintillator material, to be adapted for SiPM case, it is a function of ?

	//Variables for tdc calculation (time)
	double EtimesTime_Front=0.0;
	double EtimesTime_Back=0.0;
	double EtimesTime_Top=0.0;
	double  v_eff_Front = 200.0; // mm/ns! CND v_eff = 16 cm/ns
	double  v_eff_Back = 200.0;
	double  v_eff_Top = 200.0;

	/*
	double dist_h_SiPMFront =0.0;
	double dist_h_SiPMBack =0.0;
	double dist_h_SiPMTop =0.0;
	*/
	cout << "First loop on steps begins" << endl;
	    	for(unsigned int s=0; s<tInfos.nsteps; s++)
		{
			LposX = Lpos[s].x();
			LposY = Lpos[s].y();
			LposZ = Lpos[s].z();
	
			if(identity[1].id == 0)
			{
				dFront = length - LposZ;
				dBack = length + LposZ;
				e_Front = Edep[s] *exp(-dFront/attlength); // value for just one step, in MeV!
				e_Back = Edep[s] *exp(-dBack/attlength);
				E_tot_Front = E_tot_Front + e_Front; // to sum over all the steps of the hit
				E_tot_Back = E_tot_Back + e_Back;

				// to check the totEdep MC truth value
				totEdep = totEdep + Edep[s];

				// times[s] is in ns!
				EtimesTime_Front = EtimesTime_Front + (times[s] + dFront/v_eff_Front)*e_Front;
				EtimesTime_Back = EtimesTime_Back + (times[s] + dBack/v_eff_Back)*e_Back;
						
			//cout << "Distance from hit to Front SIPM, to Back SiPM (mm): " << dFront << ", "<< dBack << endl;
				/*
				if ( dist_h_SiPMFront <= dFront )
				{
					dist_h_SiPMFront = dFront; // mm!!!
				}
				if ( dist_h_SiPMBack <= dBack )
				{
					dist_h_SiPMBack = dBack; // mm!!!
				}
				*/
			}
			else
			{
				l_a = sqrt( pow((dim_3 - LposX),2) + pow((dim_4 - LposY),2) );
				l_b = sqrt( pow((dim_5 - LposX),2) + pow((dim_6 - LposY),2) );
			cout << "l_a & l_b (mm): " << l_a << ", " << l_b << endl;

				// to check the totEdep MC truth value
				totEdep = totEdep + Edep[s];

				if( (l_a + l_b) == l_topXY) 
				{
					H_hit_SiPM = 0.0;
					e_Top = Edep[s] *1.0; // H=0.0 -> exp() = 1.0
					E_tot_Top = E_tot_Top + e_Top;
				}	
				else
				{
					H_hit_SiPM = l_a * sqrt( 1 - ((l_topXY*l_topXY + l_a*l_a - l_b*l_b)/(2*l_a*l_topXY)) );
					e_Top = Edep[s] *exp(-H_hit_SiPM/attlength);
					E_tot_Top = E_tot_Top + e_Top;
				}

			EtimesTime_Top = EtimesTime_Top + (times[s] + H_hit_SiPM/v_eff_Top)*e_Top;
			//cout << "Distance from hit to Top SiPM (mm): " << H_hit_SiPM << endl;
			/*
			if ( dist_h_SiPMTop <= H_hit_SiPM )
				{
					dist_h_SiPMTop = H_hit_SiPM; // mm!!!
				}
			*/
			}	
		}
	cout << "First loop on steps ends" << endl;
	if (identity[1].id == 0)
	{	
		//test factor for calibration coeff. conversion
		adc_CC_front = 10.0;	
		adc_CC_back = 10.0;
		tdc_CC_front = 1.0;	
		tdc_CC_back = 1.0;
	}
	else 
	{
		adc_CC_top = 10.0;
		tdc_CC_top = 1.0;
	}

	double adc_front = 0.00000;
	double adc_back = 0.00000;
	double adc_top = 0.00000;
	double tdc_front = 0.00000;
	double tdc_back = 0.00000;
	double tdc_top = 0.00000;
	double time_front = 0.00000;
	double time_back = 0.00000;
	double time_top = 0.00000;
	double sigma_time = 0.1; // in ns! 100 ps = 0.1 ns

	
	
	if ((E_tot_Front > 0.0) || (E_tot_Back > 0.0)) 
	{
		double nphe_fr = G4Poisson(E_tot_Front*pmtPEYld);
		double energy_fr = nphe_fr/pmtPEYld;

		double nphe_bck = G4Poisson(E_tot_Back*pmtPEYld);
		double energy_bck = nphe_bck/pmtPEYld;	

		adc_front = energy_fr *adc_CC_front *(1/(dEdxMIP*0.3)); // 3 mm sl0 (radial) thickness in XY -> 0.3 cm
		adc_back = energy_bck *adc_CC_back *(1/(dEdxMIP*0.3));

		
		time_front = EtimesTime_Front/E_tot_Front;
		time_back = EtimesTime_Back/E_tot_Back;
		tdc_front = G4RandGauss::shoot(time_front, sigma_time) / tdc_CC_front; 
		tdc_back = G4RandGauss::shoot(time_back, sigma_time) / tdc_CC_back;
	}
	if(E_tot_Top > 0.0)
	{
		double nphe_top = G4Poisson(E_tot_Top*pmtPEYld);
		double energy_top = nphe_top/pmtPEYld;

		adc_top = energy_top *adc_CC_top *(1/(dEdxMIP*2.0)); // 20 mm sl1 (radial) thickness in XY -> 2.0 cm
		time_top = EtimesTime_Top/E_tot_Top;
		tdc_top  = G4RandGauss::shoot(time_top, sigma_time) / tdc_CC_top;
		
	}

	/* // this is for previous simple version
	if (energy > 0) {
		adc = energy * adc_CC;
		
		double hit_time = tInfos.time;
		//tdc  = G4RandGauss::shoot(hit_time, sqrt(2) * tdc_CC);
		time  = hit_time;
		
	}
	*/

			dgtz["sector"] = identity[0].id;
			dgtz["superlayer"] = identity[1].id;
			dgtz["layer"] = identity[2].id;
			dgtz["paddle"] = identity[3].id;
			dgtz["adc_front"] = adc_front;
			dgtz["adc_back"] = adc_back;
			dgtz["adc_top"] = adc_top;
			dgtz["tdc_front"] = tdc_front;
			dgtz["tdc_back"] = tdc_back;
			dgtz["tdc_top"] = tdc_top;
			dgtz["time_front"] = time_front;
			dgtz["time_back"] = time_back;
			dgtz["time_top"] = time_top;
			dgtz["E_tot_Front"] = E_tot_Front;
			dgtz["E_tot_Back"] = E_tot_Back;
			dgtz["E_tot_Top"] = E_tot_Top;
			dgtz["totEdep_MC"] = totEdep;
			dgtz["hitn"] = hitn;		//(2202,99)
		
		//cout << " start of the ATOF hit " << endl;
		//cout << " sector = " << identity[0].id << endl;	
		//cout << " superlayer = " << identity[1].id << endl;
		//cout << " layer = " << identity[2].id << endl;
		//cout << " paddle = " << identity[3].id << endl;
		//cout << " E_tot_Front energy value = " << E_tot_Front << endl;
		//cout << " E_tot_Back energy value = " << E_tot_Back << endl;
		//cout << " E_tot_Top energy value = " << E_tot_Top << endl;
		//cout << " totEdep MC info energy (MeV) = " << totEdep << endl;
		//cout << " adc_front = " << adc_front << endl;
		//cout << " adc_back = " << adc_back << endl;
		//cout << " adc_top = " << adc_top << endl;
		//cout << " tdc_front = : " << tdc_front << endl;
		//cout << " tdc_back = : " << tdc_back << endl;
		//cout << " tdc_top = : " << tdc_top << endl;
		//cout << " value in hitn var: " << hitn << endl;
		//cout << " end of the ATOF hit " << endl;



	// decide if write an hit or not
	writeHit = true;
	// define conditions to reject hit
	if (rejectHitConditions) {
		writeHit = false;
	}
	
	return dgtz;
}

// this method is to locate the hit event, it returns a hitted wire or paddle; this is also the one that needs to be implemented at first.
vector<identifier> myatof_HitProcess::processID(vector<identifier> id, G4Step* aStep, detector Detector) {
	
	id[id.size()-1].id_sharing = 1;
	return id;


}

// - electronicNoise: returns a vector of hits generated / by electronics.

vector<MHit*> myatof_HitProcess::electronicNoise() {
	vector<MHit*> noiseHits;
	
	// first, identify the cells that would have electronic noise
	// then instantiate hit with energy E, time T, identifier IDF:
	//
	// MHit* thisNoiseHit = new MHit(E, T, IDF, pid);
	
	// push to noiseHits collection:
	// noiseHits.push_back(thisNoiseHit)
	
	return noiseHits;
}

map< string, vector <int> > myatof_HitProcess::multiDgt(MHit* aHit, int hitn) {
	map< string, vector <int> > MH;
	
	return MH;
}

// - charge: returns charge/time digitized information / step

map< int, vector <double> > myatof_HitProcess::chargeTime(MHit* aHit, int hitn) {
	map< int, vector <double> >  CT;

	return CT;
}

// - voltage: returns a voltage value for a given time. The inputs are:
// charge value (coming from chargeAtElectronics)
// time (coming from timeAtElectronics)

double myatof_HitProcess::voltage(double charge, double time, double forTime) {
	return 0.0;
}

void myatof_HitProcess::initWithRunNumber(int runno)
{
	string digiVariation = gemcOpt.optMap["DIGITIZATION_VARIATION"].args;

	if (atc.runNo != runno) {
		cout << " > Initializing " << HCname << " digitization for run number " << runno << endl;
		atc = initializeMYATOFConstants(runno, digiVariation);
		atc.runNo = runno;
	}
}

// this static function will be loaded first thing by the executable
myatofConstants myatof_HitProcess::atc = initializeMYATOFConstants(-1);




