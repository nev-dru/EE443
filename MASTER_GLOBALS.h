/********************************************************************************************/
/********************************************************************************************/
/** David Hurlbut   																       **/
/** Andrew Neville  																       **/
/** DIGITAL SIGNAL PROCESSING AND APPLICATIONS (EE 443)                                    **/
/** DESCRIPTION:                                                                           **/
/** This is the h file used with MASTER.c to implement our real-time audio effects         **/
/** The effects include Tremolo, Auto-Panning, Gating, Delay and a Bandpass Filter         **/
/** and tone generation. This code was developed for use on a TI c6713 board with an       **/
/** RTDX connection to Matlab. Effect specification is controlled with a Matlab GUI.       **/
/**                                                                                        **/										                                                  
/********************************************************************************************/
/********************************************************************************************/

/******************************************************************************************/
/********************************        DEFINES           ********************************/
/******************************************************************************************/
#define MAX_X 900 // Max x input from arduino
#define MIN_X 100 // Min x input from arduin0

#define MAX_Y 900 // Max y input from arduin0
#define MIN_Y 100 // Min y input from arduin0

#define DSK6713_AIC23_INPUT_MIC 0x0015 // specifies mic input (not used)
#define DSK6713_AIC23_INPUT_LINE 0x0011 // specifies line input
#define LEFT 0 // left channel
#define RIGHT 1 // right channel
#define FS 44100 // sampling rate
#define PI 3.14159265358979323846 // pi
#define MAX_BUF_SIZE 44100 // max size of delay buffer



/******************************************************************************************/
/********************************      DECLARATIONS        ********************************/
/******************************************************************************************/
Uint32 fs=DSK6713_AIC23_FREQ_44KHZ;
Uint16 inputsource=DSK6713_AIC23_INPUT_LINE;
RTDX_CreateInputChannel(ichan1); //create input channel for x and y values
RTDX_CreateInputChannel(ichan2); //create input channel for choosing effects
RTDX_CreateInputChannel(ichan3); //create input channel for Bandpass filter coefficients 



/******************************************************************************************/
/********************************     GLOBAL VARIABLES     ********************************/
/******************************************************************************************/
union {Uint32 combo; short channel[2];} data;     //input data
short effects[12] = {0}; // 0 or 1 boolean for choosing effects
short applyTone; // boolean to apply Tone generation effect
short applyBPF; // boolean to apply Bandpass filter effect 
short applyAutoPan; // boolean to apply Auto panning effect 
short applyDelay; // boolean to apply Delay effect 
short applyGating; // boolean to apply Gating effect
short applyTremolo; // boolean to apply Tremolo effect
short holdTone;  // determins whether or not to hold previous effect parameters 
short holdTremolo; // determins whether or not to hold previous effect parameters
short holdGating; // determins whether or not to hold previous effect parameters
short holdAutoPan; // determins whether or not to hold previous effect parameters
short holdDelay; // determins whether or not to hold previous effect parameters
short holdBPF; // determins whether or not to hold previous effect parameters
short XY[2] = {0}; // X and Y values from the touch screen 
float normX, normY; // normalized values for x and y [0:1]
short inputLEFT; // input left sample  
short inputRIGHT; // input right sample 

/********************************    FROM TONE GENERATION    ********************************/
short newfreq; // frequency of tone
short oldfreq; // last tone frequency seen
short amplitude; // amplitude of the tone
short sinewave, sinecount, num_harmonics, h_count; // tone harmonic variables

/********************************    FROM BP FILTER    ********************************/
float BPfCoeff[5] = {0}; // bandpass filter coefficients
short xn, yn, xn1, yn1, xn2, yn2; // bandpass difference equation values
int gCount, dCount; // gate and delay effect counter
float cxn;  // difference equation coefficient  
float cxn1; // difference equation coefficient 
float cxn2; // difference equation coefficient 
float cyn1; // difference equation coefficient 
float cyn2; // difference equation coefficient 

/********************************    FROM DELAY    ********************************/
float delay_gain;
int buflength; // determines the length of time between delayed samples
short delayBuffRIGHT[MAX_BUF_SIZE], delayBuffLEFT[MAX_BUF_SIZE]; // storage for previous samples
short outRIGHT,outLEFT,delayedRIGHT,delayedLEFT; // output and delayes samples
int delayIndex; // index of delay buffer
short resetDelay; // used for determining when to reset delay buffer to zero

/********************************    AUTO PAN    ********************************/
int APcount; // counter for Auto pan effect
float pan; // left/right volume
float pan_DEPTH; // amount of panning to left/right colume cutoff 
short panFreq; // rate of auto pan from left/right

/********************************    FROM TREMOLO    ********************************/
int smooth; // frequency of tremolo in Hz
float trem_DEPTH; // Depth of tremolo cutoff
int tremCount; // tremolo variables
double trem;

/********************************   FROM GATING   ********************************/
int gateCount; // counter for gating effect
int voidLength; // length of gate cutoff
int intervalVoid; // length inbatween gate cutoff



