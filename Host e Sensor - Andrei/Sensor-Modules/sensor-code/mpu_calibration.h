#include<dmp.h>

#ifndef MPU_CALIBRATION_H
#define MPU_CALIBRATION_H


#define sampFreq 100 //Sampling frequency

//Variables for storing raw data from accelerometers gyroscope and magnetometer
int16_t ax, ay, az; //Accel
int16_t gx, gy, gz; //Gyro
int16_t mx, my, mz; //Mag
//Calibration variables
uint8_t xdata calibIt = 0; //Counter for the number of iterations in calibration
uint8_t xdata calibCounter = 0; //Counter for the number of samples read during calibration
uint8_t xdata calibStep = 1; //Determines which step of the calibration should be performed
uint8_t xdata accelTol = 0; //Error tolerance for accelerometer readings
uint8_t xdata gyroTol = 0; //Error tolerance for gyroscope readings
uint8_t xdata timeTol = 10; //Maximum duration of the calibration process (seconds)
int16_t xdata accelBuffer[3] = {0,0,0}; //Buffer to store the mean values from accel
int16_t xdata gyroBuffer[3] = {0,0,0};  //Buffer to store the mean values from gyro
/**
 * [calibrationRoutine description]
 * Everything should happen inside the timer
 * 1st step: Read raw sensor data for 1s to measure the mean values
 * 2nd step: Estimate the first offset values
 * 3rd step: Read 1s and check if the offsets are providing good values
 * according to the specified tolerance
 * 4th step: Keep looping through step 3 until the values are achieved
 * or the function runs for a specified amount of time
 */
void calibrationRoutine() {
  //Switch according to which step of the calibration should be performed
  switch (calibStep) {
    case 1:  //1st step: Measuring data to estimate the first offsets
    calibrationStepOne();
    break;
    case 2:  //2nd step: Iteratively update the offsets for accurate readings
    calibrationStepTwo();
    break;
  }
}

void calibrationStepOne(){
  //First readings to measure the mean raw values from accel and gyro
  //Measures during 2 seconds
  if(calibCounter < sampFreq*2)
  {
    //Reads the imu sensors
    getMotion6_variables(&ax, &ay, &az, &gx, &gy, &gz);
    //Iteratively updating the mean value for each sensor
    //Accel-X
    accelBuffer[0] = (calibCounter*accelBuffer[0] + ax)/(calibCounter+1);
    //Accel-Y
    accelBuffer[1] = (calibCounter*accelBuffer[1] + ay)/(calibCounter+1);
    //Accel-Z
    accelBuffer[2] = (calibCounter*accelBuffer[2] + az)/(calibCounter+1);
    //Gyro-X
    gyroBuffer[0] = (calibCounter*gyroBuffer[0] + gx)/(calibCounter+1);
    //Gyro-Y
    gyroBuffer[1] = (calibCounter*gyroBuffer[1] + gy)/(calibCounter+1);
    //Gyro-Z
    gyroBuffer[2] = (calibCounter*gyroBuffer[2] + gz)/(calibCounter+1);
    //Increments the sample counter
    calibCounter++;
  }
  else
  {
    //Setting the new offset values
    //Accelerometer offsets
    setXAccelOffset(-accelBuffer[0] / 8);
    setYAccelOffset(-accelBuffer[1] / 8);
    setZAccelOffset((16384 - accelBuffer[2]) / 8);
    //Gyroscope offsets
    setXGyroOffset(-gyroBuffer[0] / 4);
    setYGyroOffset(-gyroBuffer[1] / 4);
    setZGyroOffset(-gyroBuffer[2] / 4);
    //Goes to the next step of the calibration process
    calibStep = 2;
  }
}

void calibrationStepTwo(){

}























#endif /*MPU_CALIBRATION_H*/
