// G4 headers
#include "G4Poisson.hh"
#include "Randomize.hh"

// gemc headers
#include "cnd_hitprocess.h"

// CLHEP units
#include "CLHEP/Units/PhysicalConstants.h"
using namespace CLHEP;

// ccdb
#include <CCDB/Calibration.h>
#include <CCDB/Model/Assignment.h>
#include <CCDB/CalibrationGenerator.h>
using namespace ccdb;

static cndConstants initializeCNDConstants(int runno, string digiVariation = "default", string digiSnapshotTime = "no", bool accountForHardwareStatus = false)
{
	// all these constants should be read from CCDB
	cndConstants cndc;
	if(runno == -1) return cndc;
	string timestamp = "";
	if(digiSnapshotTime != "no") {
		timestamp = ":"+digiSnapshotTime;
	}

	cout<<"Entering initializeCNDConstants"<<endl;
	
	// database
	cndc.runNo = runno;
	cndc.date       = "2017-07-13";
	if(getenv ("CCDB_CONNECTION") != nullptr)
		cndc.connection = (string) getenv("CCDB_CONNECTION");
	else
		cndc.connection = "mysql://clas12reader@clasdb.jlab.org/clas12";

	int isec,ilay,istr;
	
	vector<vector<double> > data;
	
	unique_ptr<Calibration> calib(CalibrationGenerator::CreateCalibration(cndc.connection));
	cout<<"Connecting to " << cndc.connection << "/calibration/cnd"<<endl;

	if(accountForHardwareStatus) {
		cout<<"CND:Getting status" << endl;
		sprintf(cndc.database,"/calibration/cnd/Status_LR:%d:%s%s", cndc.runNo, digiVariation.c_str(), timestamp.c_str());
		data.clear(); calib->GetCalib(data,cndc.database);
		for(unsigned row = 0; row < data.size(); row++)
		{
			isec   = data[row][0]; ilay   = data[row][1]; istr   = data[row][2];
			cndc.status_L[isec-1][ilay-1][istr-1]=data[row][3];
			cndc.status_R[isec-1][ilay-1][istr-1]=data[row][4];
		}
	}

	cout<<"CND:Getting TDC slope"<<endl;
	sprintf(cndc.database,"/calibration/cnd/TDC_conv:%d:%s%s", cndc.runNo, digiVariation.c_str(), timestamp.c_str());
	data.clear(); calib->GetCalib(data,cndc.database);
	for(unsigned row = 0; row < data.size(); row++)
	{
		isec   = data[row][0]; ilay   = data[row][1]; istr   = data[row][2];
		cndc.slope_L[isec-1][ilay-1][istr-1]=data[row][3];
		cndc.slope_R[isec-1][ilay-1][istr-1]=data[row][5];
	}
	
	cout<<"CND:Getting attenuation"<<endl;
	sprintf(cndc.database,"/calibration/cnd/Attenuation:%d:%s%s", cndc.runNo, digiVariation.c_str(), timestamp.c_str());
	data.clear(); calib->GetCalib(data,cndc.database);
	for(unsigned row = 0; row < data.size(); row++)
	{
		isec   = data[row][0]; ilay   = data[row][1]; istr   = data[row][2];
		cndc.attlen_L[isec-1][ilay-1][istr-1]=data[row][3];
		cndc.attlen_R[isec-1][ilay-1][istr-1]=data[row][5];
	}
	
	cout<<"CND:Getting effective_velocity"<<endl;
	sprintf(cndc.database,"/calibration/cnd/EffV:%d:%s%s", cndc.runNo, digiVariation.c_str(), timestamp.c_str());
	data.clear(); calib->GetCalib(data,cndc.database);
	for(unsigned row = 0; row < data.size(); row++)
	{
		isec   = data[row][0]; ilay   = data[row][1]; istr   = data[row][2];
		cndc.veff_L[isec-1][ilay-1][istr-1]=data[row][3];
		cndc.veff_R[isec-1][ilay-1][istr-1]=data[row][5];
	}
	
	cout<<"CND:Getting energy calibration"<<endl;
	sprintf(cndc.database,"/calibration/cnd/Energy:%d:%s%s", cndc.runNo, digiVariation.c_str(), timestamp.c_str());
	data.clear(); calib->GetCalib(data,cndc.database);
	for(unsigned row = 0; row < data.size(); row++)
	{
		isec   = data[row][0]; ilay   = data[row][1]; istr   = data[row][2];
		cndc.mip_dir_L[isec-1][ilay-1][istr-1]=data[row][3];
		cndc.mip_indir_L[isec-1][ilay-1][istr-1]=data[row][5];
		cndc.mip_dir_R[isec-1][ilay-1][istr-1]=data[row][7];
		cndc.mip_indir_R[isec-1][ilay-1][istr-1]=data[row][9];
	}
	
	cout<<"CND:Getting u-turn delay"<<endl;
	sprintf(cndc.database,"/calibration/cnd/UturnTloss:%d:%s%s", cndc.runNo, digiVariation.c_str(), timestamp.c_str());
	data.clear(); calib->GetCalib(data,cndc.database);
	for(unsigned row = 0; row < data.size(); row++)
	{
		isec   = data[row][0]; ilay   = data[row][1]; istr = data[row][2];
		cndc.uturn_tloss[isec-1][ilay-1][0]=data[row][3];
	}
	
	cout<<"CND:Getting time offset LR"<<endl;
	sprintf(cndc.database,"/calibration/cnd/TimeOffsets_LR:%d:%s%s", cndc.runNo, digiVariation.c_str(), timestamp.c_str());
	data.clear(); calib->GetCalib(data,cndc.database);
	for(unsigned row = 0; row < data.size(); row++)
	{
		isec   = data[row][0]; ilay   = data[row][1]; istr   = data[row][2];
		cndc.time_offset_LR[isec-1][ilay-1][0]=data[row][3];
	}
	
	cout<<"CND:Getting time offset layer"<<endl;
	sprintf(cndc.database,"/calibration/cnd/TimeOffsets_layer:%d:%s%s", cndc.runNo, digiVariation.c_str(), timestamp.c_str());
	data.clear(); calib->GetCalib(data,cndc.database);
	for(unsigned row = 0; row < data.size(); row++)
	{
		isec   = data[row][0]; ilay   = data[row][1]; istr   = data[row][2];
		cndc.time_offset_layer[isec-1][ilay-1][0]=data[row][3];
	}
	
	return cndc;
}

map<string, double> cnd_HitProcess :: integrateDgt(MHit* aHit, int hitn)
{

	map<string, double> dgtz;

	double dEdxMIP = 1.956;         // energy deposited by MIP per cm of scintillator material
	double thickness = 3;           // thickness of each CND paddle

	// MARK TO DELETE LATER
	double sigmaTD = 0.24;          // direct signal
	double sigma   = 0.24;          // direct signal
	
	vector<identifier> identity = aHit->GetId();
	
	int sector = identity[0].id; // paddle number
	int layer  = identity[1].id; // layer
	int paddle = identity[2].id; // side: 1 = left, 2 = right
	int side   = identity[2].id; // side: 1 = left, 2 = right
	int direct = identity[3].id; // direct = 0, indirect = 1


	if(aHit->isBackgroundHit == 1) {

		return dgtz;
	}
	
	trueInfos tInfos(aHit);
	
	
	// Get the paddle length: in CND paddles are along z
	double length = aHit->GetDetector().dimensions[0];     // this is actually the half-length! Units: mm
	
	
	// TDC smearing for the time resolution (needs to know which layer it's in) --
	// factor for direct and indirect signals is tuned on cosmic ray measurements and simulations, so should be hard-coded for now:
	
	double uturn_scale[3];     // to account for nominal energy-loss in uturn, for the 3 layers -- used only for the hard-coded resolutions
	uturn_scale[0] = 0.65;
	uturn_scale[1] = 0.6;
	uturn_scale[2] = 0.5;
	double neigh_scale = uturn_scale[layer-1] * exp(-2*length/cm/150.);   // energy scaling due to nominal propagation in neighbour
	
	// MARK TO DELETE LATER
	double sigmaTN = sigmaTD / sqrt(neigh_scale);   // indirect signal in neighbour
	if ( direct == 1) {
		sigma = sigmaTD / sqrt(neigh_scale);   // indirect signal in neighbour
	}
	
	// To calculate ADC values:

	// estimated yield of photoelectrons at the photocathode of PMT:
	// assumes 10,000 photons/MeV, LG length 1.4m with attenuation length 9.5m, 30% losses at junctions and PMT QE = 0.2
	
	// MARK TO DELETE LATER
	double pmtPEYldD = 1210;                      // for the direct signal
	double pmtPEYldN = pmtPEYldD * neigh_scale;   // for the indirect signal, additional loss in u-turn and attenuation in neighbour


	double pmtPEYld = 1210;  // for the direct signal
	if(direct == 1) {
		pmtPEYld = pmtPEYld * neigh_scale; // for the indirect signal, additional loss in u-turn and attenuation in neighbour
	}

	
	// Get info about detector material to eveluate Birks effect
	double birks_constant=aHit->GetDetector().GetLogical()->GetMaterial()->GetIonisation()->GetBirksConstant();
	
	vector<G4ThreeVector> Lpos   = aHit->GetLPos();   // local position wrt centre of the detector piece (ie: paddle): in mm
	vector<double>        Edep   = aHit->GetEdep();     // deposited energy in the hit, in MeV
	vector<int>           charge = aHit->GetCharges();        // charge for each step
	vector<double>        times  = aHit->GetTime();
	vector<double>        dx     = aHit->GetDx();              // step length
	
	unsigned nsteps = times.size();                 // total number of steps in the hit

	// variables for each step:

	double distanceTravelled = 0; // distance travelled by the light along the paddle. For indirect, it will then go through the u-turn and along the neighbouring paddle
	double attenuatedEnergy  = 0;  // attenuated energy as it arrives at the edge of the hit paddle

	// MARK TO DELETE LATER
	double dUp    = 0.;       // distance travelled by the light along the paddle UPSTREAM (direct light)
	double dDown  = 0.;     // distance travelled by the light along the paddle DOWNSTREAM (it will then go through the u-turn and along the neighbouring paddle)
	double e_up   = 0.;      // attenuated energy as it arrives at the upstream edge of the hit paddle
	double e_down = 0.;    // attenuated energy as it arrives at the downstream edge of the hit paddle


	double Edep_B = 0.;    // Energy deposit, scaled in accordance with Birk's effect

	
	// Variables for each hit:

	double eTotalTime = 0;    // total energy*time for each hit propagated to end paddle (direct = upstream, indirect = downstream)

	// MARK TO DELETE LATER
	double et_D = 0.;         // total energy*time for each hit propagated to upstream end of hit paddle
	double et_N = 0.;         // total energy*time for each hit propagated to upstream end of the neighbour paddle
	

	double eTotal = 0;   // total energy of hit propagated to end paddle (direct = upstream, indirect = downstream)
	double eTime  = 0;   // hit times measured at the upstream edges of the two paddles

	// MARK TO DELETE LATER

	double etotUp = 0.;       // total energy of hit propagated to the upstream end of paddle
	double etotDown = 0.;     // total energy of hit propagated to the downstream end of the paddle
	double timeD = 0.;        // hit times measured at the upstream edges of the two paddles
	double timeN = 0.;
	
	int TDCmax = 16384;       // max value of the ADC readout

	int ADC = 0;
	int TDC = TDCmax;

	// MARK TO DELETE LATER
	int ADCD = 0;
	int ADCN = 0;
	int TDCD = TDCmax;
	int TDCN = TDCmax;
	
//	int ADCL = 0;
//	int ADCR = 0;
//	int TDCL = TDCmax;
//	int TDCR = TDCmax;
	
	
	double attlength = 0.;
	double slope     = 0.;
	// int status       = 0;
	double v_eff     = 0.;
	double adc_mip   = 0.;


	// MARK TO DELETE LATER
	double attlength_D = 0.;  // attlength_N was not needed
 	//	double attlength_N = 0.;
	double slope_D   = 0.;
	double slope_N   = 0.;
	int status_D     = 0;
	int status_N     = 0;
	double v_eff_D   = 0.;
	double v_eff_N   = 0.;
	double adc_mip_D = 0.;
	double adc_mip_N = 0.;

	double t_u            = cndc.uturn_tloss[sector-1][layer-1][0];
	double t_offset_LR    = cndc.time_offset_LR[sector-1][layer-1][0];
	double t_offset_layer = cndc.time_offset_layer[sector-1][layer-1][0];


	if ( side == 1 ){   // hit is in paddle L
		
		attlength = cndc.attlen_L[sector-1][layer-1][0];

		if ( direct == 0 ) {
			slope   = cndc.slope_L[sector-1][layer-1][0];
			// status  = cndc.status_L[sector-1][layer-1][0];
			v_eff   = cndc.veff_L[sector-1][layer-1][0];
			adc_mip = cndc.mip_dir_L[sector-1][layer-1][0];
		} else {
			slope   = cndc.slope_R[sector-1][layer-1][0];
			// status  = cndc.status_R[sector-1][layer-1][0];
			v_eff   = cndc.veff_R[sector-1][layer-1][0];
			adc_mip = cndc.mip_indir_L[sector-1][layer-1][0];
		}


		attlength_D = cndc.attlen_L[sector-1][layer-1][0];

		slope_D = cndc.slope_L[sector-1][layer-1][0];
		slope_N = cndc.slope_R[sector-1][layer-1][0];

		status_D = cndc.status_L[sector-1][layer-1][0];
		status_N = cndc.status_R[sector-1][layer-1][0];

		v_eff_D = cndc.veff_L[sector-1][layer-1][0];
		v_eff_N = cndc.veff_R[sector-1][layer-1][0];

		adc_mip_D = cndc.mip_dir_L[sector-1][layer-1][0];
		adc_mip_N = cndc.mip_indir_L[sector-1][layer-1][0];
	}
	
	else if ( side == 2 ) {   // hit is in paddle R
		
		attlength = cndc.attlen_R[sector-1][layer-1][0];

		if ( direct == 0 ) {
			slope   = cndc.slope_R[sector-1][layer-1][0];
			// status  = cndc.status_R[sector-1][layer-1][0];
			v_eff   = cndc.veff_R[sector-1][layer-1][0];
			adc_mip = cndc.mip_dir_R[sector-1][layer-1][0];
		} else {
			slope   = cndc.slope_L[sector-1][layer-1][0];
			// status  = cndc.status_L[sector-1][layer-1][0];
			v_eff   = cndc.veff_L[sector-1][layer-1][0];
			adc_mip = cndc.mip_indir_R[sector-1][layer-1][0];
		}

		attlength_D = cndc.attlen_R[sector-1][layer-1][0];

		slope_D = cndc.slope_R[sector-1][layer-1][0];
		slope_N = cndc.slope_L[sector-1][layer-1][0];

		status_D = cndc.status_R[sector-1][layer-1][0];
		status_N = cndc.status_L[sector-1][layer-1][0];

		v_eff_D = cndc.veff_R[sector-1][layer-1][0];
		v_eff_N = cndc.veff_L[sector-1][layer-1][0];

		adc_mip_D = cndc.mip_dir_R[sector-1][layer-1][0];
		adc_mip_N = cndc.mip_indir_R[sector-1][layer-1][0];
	}
	else {
		cout <<"/n Help, do not recognise paddle number " << paddle << "!!!" << endl;
	}


	if(tInfos.eTot>0) {
		for(unsigned int s=0; s<nsteps; s++) {

			// apply Birks effect to Edep this step
			Edep_B = BirksAttenuation(Edep[s], dx[s], charge[s], birks_constant);

			// Distances travelled through the paddles to the upstream (direct) or downstream (indirect) edges
			if ( direct == 0 ) {
				distanceTravelled = (length + Lpos[s].z());
			} else {
				distanceTravelled = (length - Lpos[s].z());
			}

			dUp   = (length + Lpos[s].z());
			dDown = (length - Lpos[s].z());


			// Calculate attenuated energy which will reach the upstream and downstream edges of the hit paddle:
			attenuatedEnergy = Edep_B * 0.5 * exp(-distanceTravelled/cm/attlength);
			e_up   = Edep_B * 0.5 * exp(-dUp/cm/attlength_D);
			e_down = Edep_B * 0.5 * exp(-dDown/cm/attlength_D);

			// Integrate energy over entire hit. These values are used for time-smearing:
			eTotal = eTotal + attenuatedEnergy;

			etotUp   = etotUp + e_up;
			etotDown = etotDown + e_down;

			
			/****** Time of hit calculation *******/
			// In all the methods below, the assumption is that the time taken to travel along the light-guides,
			// through PMTs and the electronics to the ADC units is the same for all paddles, therefore this time offset is ignored.
			// Pick whichever method you like:
			
			// Method 1: this takes average time of all the steps with energy deposit above 0:
			//if (Edep[s] > 0.){
			//   timeD = timeD + (times[s] + dUp/cm/v_eff_D) / nsteps;
			//   timeN = timeN + (times[s] + dDown/cm/v_eff_D + t_u + 2*length/cm/v_eff_N) / nsteps;
			//}
			
			// Method 2: This takes the time of the first step (in order of creation) with energy deposit above 0 (can set another threshold):
			// if (flag_counted == 0 && Edep[s] > 0.){
			//   timeD = times[s] + dUp/cm/v_eff_D;
			//   timeN = times[s] + dDown/cm/v_eff_D + t_u + 2*length/cm/v_eff_N;
			//   flag_counted = 1;   // so that subsequent steps are not counted in the hit
			// }
			
			// Method 3: This calculates the total energy * time value at the upstream edges of the hit paddle and its neighbour,
			// will be used to get the energy-weighted average times (should correspond roughly to the peak energy deposit):
			
			// cout<<"times[s] (ns) "<<times[s]/ns<<endl;


			if ( direct == 0 ) {
				eTotalTime = eTotalTime + ((times[s] + distanceTravelled/cm/v_eff) * attenuatedEnergy);
			} else {
				eTotalTime = eTotalTime + ((times[s] + distanceTravelled/cm/v_eff + t_u + 2*length/cm/v_eff) * attenuatedEnergy);
			}

			et_D = et_D + ((times[s] + dUp/cm/v_eff_D) * e_up);
			et_N = et_N + ((times[s] + dDown/cm/v_eff_D + t_u + 2*length/cm/v_eff_N) * e_down);

		}   // close loop over steps s
		
		/**** The following calculates the time based on energy-weighted average of all step times ****/

		eTime = eTotalTime / eTotal;
		timeD = et_D / etotUp;      // sum(energy*time) /  sum(energy)
		timeN = et_N / etotDown;
		
		// "Actual" GEMC hit timings, propagated to paddle edges and not:
		//timeD = tInfos.time + dUp/cm/v_eff_D;
		//timeN = tInfos.time + dDown/cm/v_eff_D + t_u + 2*length/cm/v_eff_N;
		//timeD = tInfos.time;
		//timeN = tInfos.time + t_u + 2*length/cm/v_eff_N;
		
		//	cout<<"timeD "<<timeD<<endl;
		//	cout<<"timeN "<<timeN<<endl;
		//	cout<<"timeD-timeN "<<timeD-timeN<<endl;
		
		// Apply offsets between left and right paddles (these are only applied on paddle 2, which is R):

		if ( (direct == 0 && side == 2) || (direct == 1 && side == 1) ) {
			eTime = eTime + t_offset_LR;
		}

		if (paddle == 2) {
			timeD = timeD + t_offset_LR;
		} else if (paddle == 1) {
			timeN = timeN + t_offset_LR;
		}

		eTime = eTime + t_offset_layer;

		timeD = timeD + t_offset_layer;
		timeN = timeN + t_offset_layer;


//		cout << " side: " << side << ", direct: " << direct << ", etotUp: " << etotUp << ", etotDown: " << etotDown << ", eTotal: " << eTotal;
//		cout << ", timeD: " << timeD << ", timeN: " << timeN << ", eTime: " << eTime << endl;


		/******** end timing determination ***********/
		
		// cout << "Half-length of paddle(cm): "<<length/cm<<endl;
		// cout << "etotUp, etotDown (in MeV): " << etotUp/MeV << ", " << etotDown/MeV << endl;
		// cout << "timeD, timeN (in ns): " << timeD/ns << ", " << timeN/ns << endl;
		// cout << "Reconstructed time (ns): " << ((timeD + timeN - 2*length/cm/v_eff_N - t_u)/2.)/ns << endl;
		// cout << "Reconstructed z (cm, wrt paddle center): " << (length/cm/v_eff_N + t_u + timeD - timeN)*v_eff_D/2./cm << endl;
		
		// Check the full output:
		// cout << "Total steps in this hit: " << nsteps << endl;
		// for (int s=0; s<nsteps; s++)
		//  {
		//    cout << "\n Edep (in MeV): "  << Edep[s] << " time (in ns): " << times[s] << " Lpos-x (in mm): "
		//          << Lpos[s].x() << " Lpos-y: " << Lpos[s].y() << " Lpos-z: " << Lpos[s].z() << " pos-x (in mm): "
		// 		         << pos[s].x() << " pos-y: "  << pos[s].y()  << " pos-z: " << pos[s].z() << endl;
		//  }
		//  cout << "Total for the hit:" << endl;
		//  cout << "etotUp (in MeV): " << etotUp << " etotDown: " << etotDown  << " timeD (in ns): " << timeD << " timeN: " << timeN << endl;
		
		
		/**** Actual digitisation happens here! *****/
		
		// MARK: TO delete later
		if (etotUp > 0.) {
			TDCD = (int) ( (G4RandGauss::shoot(timeD, sigmaTD/sqrt(etotUp)) ) / slope_D);
			double npheD = G4Poisson(etotUp*pmtPEYldD);
			double eneD = npheD/pmtPEYldD;
			ADCD = (int) (eneD*adc_mip_D*2./(dEdxMIP*thickness));
		}
		if (etotDown > 0.) {
			TDCN = (int) ( (G4RandGauss::shoot(timeN, sigmaTN/sqrt(etotDown)) ) / slope_N);
			double npheN = G4Poisson(etotDown*pmtPEYldN);
			double eneN = npheN/pmtPEYldN;
			ADCN = (int) (eneN*adc_mip_N*2./(dEdxMIP*thickness));
		}


		TDC = (int) ((G4RandGauss::shoot(eTime, sigma/sqrt(eTotal)))/slope);
		double nphe = G4Poisson(eTotal*pmtPEYld);
		double ene  = nphe/pmtPEYld;
		ADC = (int) (ene*adc_mip*2./(dEdxMIP*thickness));



		// MARK: TO delete later
		if (TDCD < 0) TDCD = 0;
		else if (TDCD > TDCmax) TDCD = TDCmax;
		if (TDCN < 0) TDCN = 0;
		else if (TDCN > TDCmax) TDCN = TDCmax;
		
		if (ADCD < 0) ADCD = 0;
		if (ADCN < 0) ADCN = 0;



		if (TDC < 0) {
			TDC = 0;
		} else if (TDC > TDCmax) {
			TDC = TDCmax;
		}

		if (ADC < 0) ADC = 0;


	}  // closes tInfos.eTot>0
	
	
	if(verbosity>4) {
		cout << " layer: " << layer    << ", paddle: " << paddle  << " x=" << tInfos.x/cm << "cm, y=" << tInfos.y/cm << "cm, z=" << tInfos.z/cm << "cm" << endl;
		cout << " Etot=" << tInfos.eTot/MeV     << "MeV, average time=" << tInfos.time  << "ns"  << endl;
		cout << " timeD=" << timeD     << ",   offset=" << t_offset_layer  << "ns"  << endl;
		cout << " TDCD= " << TDCD     << ", TDCN= " << TDCN    << ", ADCD= " << ADCD << ", ADCN= " << ADCN << endl;
	}
	
	// Status flags
	if(accountForHardwareStatus) {
		switch (status_D)
		{
			case 0:
				break;
			case 1:
				ADCD = 0;
				break;
			case 2:
				TDCD = 0;
				break;
			case 3:
				ADCD = TDCD = 0;
				break;
			case 5:
				break;

			default:
				cout << " > Unknown CND status: " << status_D << " for sector " << sector << ",  layer " << layer << ", paddle " << paddle << endl;
		}
		switch (status_N)
		{
			case 0:
				break;
			case 1:
				ADCN = 0;
				break;
			case 2:
				TDCN = 0;
				break;
			case 3:
				ADCN = TDCN = 0;
				break;
			case 5:
				break;

			default:
				cout << " > Unknown CND status: " << status_N << " for sector " << sector << ",  layer " << layer << ", paddle " << 3 - paddle << endl;
		}
	}

	
//	if (paddle == 1) {
//		ADCL = (int) ADCD;
//		ADCR = (int) ADCN;
//		TDCL = (int) TDCD;
//		TDCR = (int) TDCN;
//	} else if (paddle == 2) {
//		ADCL = (int) ADCN;
//		ADCR = (int) ADCD;
//		TDCL = (int) TDCN;
//		TDCR = (int) TDCD;
//	}

//	cout << " sector: " << sector << ", layer: " << layer <<  ", side: " << side << ", direct: " << direct << ", TDCD: " << TDCD << ", TDCN: " << TDCN << ", TDC: " << TDC;
//	cout << ", ADCD: " << ADCD << ", ADCN: " << ADCN << ", ADC: " << ADC << endl;

	
	// Apply global offsets for each paddle-pair (a.k.a. component):


	dgtz["hitn"]      = hitn;
	dgtz["sector"]    = sector;
	dgtz["layer"]     = layer;
	dgtz["component"] = 1;
	dgtz["ADC_order"] = direct;
	dgtz["ADC_ADC"]   = ADC;
	dgtz["ADC_time"]  = TDC;   // no conversion
	dgtz["ADC_ped"]   = 0;
	dgtz["TDC_order"] = direct + 2;
	dgtz["TDC_TDC"]   = TDC;

	// decide if write an hit or not
	writeHit = true;
	// define conditions to reject hit
	if(rejectHitConditions) {
		writeHit = false;
	}
	
	return dgtz;
}


// vector<identifier> identity = aHit->GetId();
// sector = identity[0].id; // sector (paddle number)
// layer  = identity[1].id; // layer
// side   = identity[2].id; // side: 1 = left, 2 = right
// direct = identity[3].id; // direct = 0, indirect = 1  << what we need to duplcate to 1 here

vector<identifier>  cnd_HitProcess :: processID(vector<identifier> id, G4Step *step, detector Detector)
{
	vector<identifier> yid = id;
	yid[0].id_sharing = 1; // sector (paddle number)
	yid[1].id_sharing = 1; // layer
	yid[2].id_sharing = 1; // side: 1 = left, 2 = right
	yid[3].id_sharing = 1; // direct = 0, indirect = 1  << what we need to duplcate to 1 here

	if (yid[3].id != 0) {
		cout << "*****WARNING***** in cnd_HitProcess :: processID, identifier \"direct\" of the original hit should be 0 but is " << yid[3].id << endl;
		cout << "yid[3].id = " << yid[3].id << endl;
	}

	// Now we want to have similar identifiers, but the only difference be id direct to be 1, instead of 0
	identifier this_id = yid[0];
	yid.push_back(this_id);
	this_id = yid[1];
	yid.push_back(this_id);
	this_id = yid[2];
	yid.push_back(this_id);
	this_id = yid[3];
	this_id.id = 1;
	yid.push_back(this_id);

	return yid;

}


double cnd_HitProcess::BirksAttenuation(double destep, double stepl, int charge, double birks)
{
	//Example of Birk attenuation law in organic scintillators.
	//adapted from Geant3 PHYS337. See MIN 80 (1970) 239-244
	//
	// Taken from GEANT4 examples advanced/amsEcal and extended/electromagnetic/TestEm3
	//
	double response = destep;
	if (birks*destep*stepl*charge != 0.) {
		response = destep/(1. + birks*destep/stepl);
	}
	return response;
}


map< string, vector <int> >  cnd_HitProcess :: multiDgt(MHit* aHit, int hitn)
{
	map< string, vector <int> > MH;
	
	return MH;
}

// - charge: returns charge/time digitized information / step
map< int, vector <double> > cnd_HitProcess :: chargeTime(MHit* aHit, int hitn)
{
	map< int, vector <double> >  CT;
	
	return CT;
}

// - voltage: returns a voltage value for a given time. The inputs are:
// charge value (coming from chargeAtElectronics)
// time (coming from timeAtElectronics)
double cnd_HitProcess :: voltage(double charge, double time, double forTime)
{
	return 0.0;
}


void cnd_HitProcess::initWithRunNumber(int runno)
{
	string digiVariation    = gemcOpt.optMap["DIGITIZATION_VARIATION"].args;
	string digiSnapshotTime = gemcOpt.optMap["DIGITIZATION_TIMESTAMP"].args;

	if(cndc.runNo != runno) {
		cout << " > Initializing " << HCname << " digitization for run number " << runno << endl;
		cndc = initializeCNDConstants(runno, digiVariation, digiSnapshotTime, accountForHardwareStatus);
		cndc.runNo = runno;
	}
}


// - electronicNoise: returns a vector of hits generated / by electronics.
vector<MHit*> cnd_HitProcess :: electronicNoise()
{
	vector<MHit*> noiseHits;
	
	// first, identify the cells that would have electronic noise
	// then instantiate hit with energy E, time T, identifier IDF:
	//
	// MHit* thisNoiseHit = new MHit(E, T, IDF, pid);
	
	// push to noiseHits collection:
	// noiseHits.push_back(thisNoiseHit)
	
	return noiseHits;
}

// this static function will be loaded first thing by the executable
cndConstants cnd_HitProcess::cndc = initializeCNDConstants(-1);

