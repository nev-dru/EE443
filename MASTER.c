/********************************************************************************************/
/********************************************************************************************/
/** David Hurlbut   																       **/
/** Andrew Neville  																       **/
/** DIGITAL SIGNAL PROCESSING AND APPLICATIONS (EE 443)                                    **/
/** DESCRIPTION:                                                                           **/
/** This is the C code used to implement our real-time audio effects and tone generation.  **/
/** The effects include Tremolo, Auto-Panning, Gating, Delay and a Bandpass Filter.        **/
/** This code was developed for use on a TI c6713 board with an RTDX connection to Matlab. **/
/** Effect specification is controlled with a Matlab GUI.                                  **/
/**                                                                                        **/										                                                  
/********************************************************************************************/
/********************************************************************************************/



/******************************************************************************************/
/********************************        INCLUDES          ********************************/
/******************************************************************************************/
#include <rtdx.h>
#include <stdio.h>
#include "target.h"
#include "DSK6713_AIC23.h"
#include <math.h>
#include "MASTER_GLOBALS.h"

/******************************************************************************************/
/******************************************************************************************/
/**										  MAIN                                           **/
/******************************************************************************************/
/******************************************************************************************/
void main()
{
	/**************************************************************************************/
	/******************************     INITIALIZATIONS      ******************************/
	/**************************************************************************************/
	comm_poll();
	TARGET_INITIALIZE();

	// enable RTDX channel to recieve X and Y coordinates
	RTDX_enableInput(&ichan1);
	while(RTDX_channelBusy(&ichan1)) 
	{
		// waiting
	}

	// enable RTDX channel for effect specifications
	RTDX_enableInput(&ichan2);
	while(RTDX_channelBusy(&ichan2)) 
	{
		// waiting
	}
	
	// enable RTDX channel for BandPass filter coefficients 
	RTDX_enableInput(&ichan3);
	while(RTDX_channelBusy(&ichan3)) 
	{
		// waiting
	}

	// all 0 or 1 for boolean
	applyTone    = effects[0]; 
	applyBPF     = effects[1];
	applyAutoPan = effects[2]; 
	applyDelay   = effects[3];
	applyGating  = effects[4];
	applyTremolo = effects[5];

	holdTone    = effects[6];
	holdTremolo = effects[7];
	holdGating  = effects[8];
	holdAutoPan = effects[9];
	holdDelay   = effects[10];
	holdBPF     = effects[11];
	
	normX=normY=0.5;

	/*****************************    init tone generation    *****************************/
	newfreq = 1000;
	oldfreq = 1000;
	amplitude = 500;
	sinewave = 0;
	sinecount = 0;
	num_harmonics = 1;
	h_count = 0;

	/*****************************    init bandpass filter    *****************************/
	xn1=0;
	xn2=0;
	yn1=0;
	yn2=0;
	yn=0; //init difference eqn.
	
	cxn  = 0;
	cxn1 = 0;
	cxn2 = 0;
	cyn1 = 0;
	cyn2 = 0;

	/********************************    init auto pan    *********************************/
	pan=0;      
	APcount = 0;  
	pan_DEPTH = 0;  
	panFreq = 20;
	
	/*********************************    init delay    ***********************************/
	delay_gain = 0.3;
	buflength = 5512;
	delayIndex = 0;
	resetDelay = 0;

	for(delayIndex=0 ; delayIndex<MAX_BUF_SIZE ; delayIndex++) 
	{   // clear buffer
		delayBuffRIGHT[delayIndex] = 0;
		delayBuffLEFT[delayIndex] = 0;
	}

	/*********************************    init gating    **********************************/
	voidLength=intervalVoid=500;//int
	gateCount = 0;
	
	/********************************    init tremolo    **********************************/
	trem=0; //double
	trem_DEPTH = 0.7; //float
	tremCount = 0; //int
	smooth = 5; //int


    /**************************************************************************************/
    /******************************     INFINITE LOOP        ******************************/
    /**************************************************************************************/
	while(1)
	{
        /**************************************************************************************/
		/**********************************    GET INPUT    ***********************************/
        /**************************************************************************************/

		inputLEFT  = input_left_sample();
  		inputRIGHT = input_right_sample();

	    /**************************************************************************************/
	    /******************************        EFFECTS           ******************************/
	    /**************************************************************************************/
		
		/******************************    TONE GENERATION       ******************************/
		if (applyTone) 
		{
			if (!holdTone)
			{
				newfreq   = 3 * XY[0] - 100;
				num_harmonics = (short)(XY[1] / 100);
			}
			if (newfreq != oldfreq)
			{
				sinewave = (short)((amplitude/2)*sin(2*PI*sinecount*newfreq/FS));
				oldfreq = newfreq;
			}
			else
			{
				sinewave = (short)((amplitude)*sin(2*PI*sinecount*newfreq/FS));
			}

			for (h_count = 2; h_count <= 2*num_harmonics; h_count+=2)
			{
				sinewave += (short)(amplitude*sin(2*PI*sinecount*newfreq*(h_count+1)/FS));
			}
			
			sinewave = (short)(sinewave * 4 / PI);

			inputLEFT = sinewave;
			inputRIGHT = sinewave;
			
			sinecount++;
			if (sinecount > (FS/newfreq))
			{
				sinecount = 0;
			}
		}

		/******************************    BAND PASS FILTER      ******************************/
		if (applyBPF) 
		{
			xn = inputLEFT;
			yn = (short)( (cxn*xn) + 
				          (cxn1*xn1) + 
			              (cxn2*xn2) - 
			              (cyn1*yn1) - 
			              (cyn2*yn2) );
			//update values
			yn2=yn1;
			yn1=yn;
			yn=0;
			xn2=xn1;
			xn1=xn;
			//output
			inputLEFT = yn1;
			inputRIGHT = yn1;
		}
	
		/*****************************      AUTO PANNING        *******************************/
		if (applyAutoPan) 
		{
			if (!holdAutoPan)
			{
				// set xy
				panFreq = (short)(29*normX + 1);
				pan_DEPTH = (float)normY;
			}
			pan = pan_DEPTH*sin(2*PI*panFreq*APcount/FS);
			if (pan < 0) 
			{
				inputRIGHT = (pan+1)*inputRIGHT;
			} 
			else if (pan > 0) 
			{
				inputLEFT = (1-pan)*inputLEFT;  
			}
			
			inputLEFT = (short)inputLEFT;
			inputRIGHT = (short)inputRIGHT;

			//APcount is the length of 1 full period of the pan wave
			APcount++;
			if (APcount > (FS/panFreq)) 
			{
				APcount = 0;
			}
		}

		/*******************************        TREMOLO         *******************************/
		if (applyTremolo)
		{
			if (!holdTremolo)
			{
				// set xy
				smooth = (int)(29*normX + 1);
				trem_DEPTH = normY;
			}
			// tremolo
			trem = (1-trem_DEPTH) + trem_DEPTH*sin(2*PI*smooth*tremCount/FS);
			inputLEFT = (short)inputLEFT*trem;
			inputRIGHT = (short)inputRIGHT*trem;

			//tremCount is the length of 1 full period of the tremolo wave
			tremCount++;
			if (tremCount > (FS/smooth)) 
			{
				tremCount = 0;
			}
		}

		/********************************        GATING          ******************************/
		if (applyGating) 
		{
			if (!holdGating)
			{
				// sest xy
				voidLength = (int)2200*normX;
				intervalVoid = (int)4400*normY;
			}
			if (gateCount < intervalVoid) 
			{
				inputLEFT = 0;  
				inputRIGHT = 0;
			} 
			gateCount++;
			if (gateCount >= (voidLength + intervalVoid)) 
			{
				gateCount = 0;
			}
		}

		/*******************************        DELAY           *******************************/
		if (applyDelay) 
		{
			if (!holdDelay)
			{
				// set  xy
				buflength = (int)(22000*normX);
				delay_gain =  (float)(0.9*normY);
			}
			// read delayed value from buffer
			delayedRIGHT = delayBuffRIGHT[delayIndex];         
			delayedLEFT = delayBuffLEFT[delayIndex];

			// output sum of input and delayed values
			outLEFT = inputLEFT + delayedLEFT; 
			outRIGHT = inputRIGHT + delayedRIGHT;    

			// store new input and a fraction of the delayed value in buffer
			delayBuffRIGHT[delayIndex] =  outRIGHT*delay_gain; 
			delayBuffLEFT[delayIndex] = outLEFT*delay_gain;

			inputLEFT = outLEFT;
			inputRIGHT = outRIGHT;
			// test for end of buffer                             
			if(++delayIndex >= MAX_BUF_SIZE)
			{
				delayIndex = MAX_BUF_SIZE - buflength;	
			} 

			resetDelay = 1;
		}

		if (!applyDelay && resetDelay)
		{
			for (delayIndex = 0; delayIndex < MAX_BUF_SIZE; delayIndex++)
			{
				delayBuffRIGHT[delayIndex] = 0;
				delayBuffLEFT[delayIndex] = 0;
			}
			resetDelay = 0;
			delayIndex = 0;
		}


		/******************************************************************************/
		/********************************    OUTPUT    ********************************/
		/******************************************************************************/

		data.channel[LEFT]  = (short)inputLEFT;
		data.channel[RIGHT] = (short)inputRIGHT;

		output_sample(data.combo); 


		/******************************************************************************/
		/****************************    READ NEW VALUES    ***************************/
		/******************************************************************************/

		// Read x and y values associated with the touch screen
		if (!RTDX_channelBusy(&ichan1)) 
	    {
			RTDX_readNB(&ichan1,&XY,sizeof(XY));
	    }

	    // Read values to determine what effects to apply
	    if (!RTDX_channelBusy(&ichan2)) 
	    {
			RTDX_readNB(&ichan2,&effects,sizeof(effects));
	    }

	    // Read bandpass filter coefficients 
	    if (!RTDX_channelBusy(&ichan3)) 
	    {
			RTDX_readNB(&ichan3,&BPfCoeff,sizeof(BPfCoeff));
	    }

	    // normalize x and y to be 0-1
	    normX = ((float)XY[0] - MIN_X)/(MAX_X - MIN_X);
	    normY = ((float)XY[1] - MIN_Y)/(MAX_Y - MIN_Y);

		// all 0 or 1 for boolean
		applyTone    = effects[0]; 
		applyTremolo = effects[1];
		applyGating  = effects[2];
		applyAutoPan = effects[3]; 
		applyDelay   = effects[4];
		applyBPF     = effects[5];

		holdTone    = effects[6];
		holdTremolo = effects[7];
		holdGating  = effects[8];
		holdAutoPan = effects[9];
		holdDelay   = effects[10];
		holdBPF     = effects[11];
		
		if (!holdBPF)
		{
			cxn  = BPfCoeff[0];
			cxn1 = BPfCoeff[1];
			cxn2 = BPfCoeff[2];
			cyn1 = BPfCoeff[3];
			cyn2 = BPfCoeff[4];
		}

	} // END WHILE(1)
}// END MAIN


