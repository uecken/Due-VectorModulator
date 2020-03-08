# Due-VectorModulator
Use HMC630LP3E and Arduino Due

# Function
- set gain,phase by "-20,90" via serial.
- callibration by "cal,"
- Phase Accuracy Check by "phase,-13"
- Gain Accuracy Check by "Gain,0"
- Cancellation changing Gain and Phase.
- (Calibration, like  "cal,Q,1.35,1.36,0.01,1.49" via serial. 
  Need to find minimum gain by searching I/Q. The I/Q is Vmi/Vmq.)

# Accuracy
- Phase Error: 3 degree OK @-32~--13dB
- Aplitude Error: 2dB  NG @-32~--13dB

![image](https://github.com/uecken/Due-VectorModulator/blob/master/Due-Modulator.gif?raw=true)
![Picture](https://github.com/uecken/Due-VectorModulator/blob/master/accuracy.png?raw=true)
![Picture2](https://github.com/uecken/Due-VectorModulator/blob/master/Picture.png?raw=true)
