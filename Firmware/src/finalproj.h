volatile unsigned char AdjUpdate = 0;   // Flag to indicate a need to update the LCD for Adjust
volatile short Threshold_Local; // Variable to store the local threshold
volatile short Threshold_Remote;    // Variable to store the remote threshold
volatile long distance_x10 = 0; // Variable to store the distance in mm
volatile long distance_x10_remote = 0;  // Variable to store the remote distance in mm
unsigned char bntsts;

enum states { ACQUIRE, ADJUST };
enum ThresSet { LOCAL, REMOTE };