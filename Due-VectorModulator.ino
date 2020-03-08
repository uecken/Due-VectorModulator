//Use Vi,Vq as global. function changes the Vi,Vq.

double Gain,Phase,Vi,Vq,Vmi,Vmq,Vrange,Gmax;
double Vimin,Vqmin;
String msgs[6] = {"\0"};
double dacVmin,dacVmax, dacVmaxIndex,dacRange,dacResolution,dacOffset;
double pi = 3.14159;



void setup() {
  analogWriteResolution(12);
  analogReadResolution(12);
  
  Serial.begin(9600);
  Serial.print("DUE-IQmodulator wake up.");
  Serial.println();
 
  //Set Variable
  Vi=0;Vq=0;Vmi=0;Vmq=0;Vrange=0;Gmax=0;

  //Calibrated value
  dacVmin = 0.58; //@analogWrite(DAC0, 0)
  dacVmax = 2.5; //@analogWrite(DAC0, 3640)
  dacVmaxIndex = 3640; // Index at Vmax2.5V 
  dacResolution = (dacVmax-dacVmin)/(dacVmaxIndex+1);
  dacOffset = dacVmin;
  
  //Callibrated.
  Vmi = 1.47; Vmq =1.382; //Vrange = dacVmin - dacVmax
  Vrange = 2*(Vmq - dacVmin); //radius = Vrange/2
  Gmax = -13.00;

  //Init
  analogWrite(DAC0,int(((1.3-dacOffset)/dacResolution)));
  analogWrite(DAC1,int(((1.5-dacOffset)/dacResolution)));
  Serial.println("dacVmin,dacVmax,dacVmaxIndex,dacResolution,dacOffset,Vm,Vmq,Vrange,Gmax");
  Serial.println(String(dacVmin)+","+String(dacVmax)+","+String(dacVmaxIndex)+","+String(dacResolution)+","+String(dacOffset)+","+String(Vmi)+","+String(Vmq)+","+String(Vrange)+","+String(Gmax));

  double GmaxTrue = pow(2.718,(Gmax/10.0));
  Serial.println("GmaxTrue:"+String(GmaxTrue));
}

void loop() {
  if(Serial.available() > 0){
    String msg =  Serial.readString();
    Serial.print("Serial Input: " + msg);  //gain,phase
    int len = split(msg,',',msgs);    
    /*for(int i=0;i<len;i++){
      Serial.print(String(i) + ":" + msgs[i]);
    }*/
    
    if(msgs[0] == "cal"){
      //Ex: cal,Q,1.35,1.36,0.01,1.49
      //calIQ(msgs[1],msgs[2].toDouble(),msgs[3].toDouble(),msgs[4].toDouble(),msgs[5].toDouble());//Set Vi,Vq
      Vmi,Vmq = calIQ();
    }
    else if(msgs[0] == "cancel"){
      Vimin,Vqmin = cancel(0.25);
      writeIQ(Vimin,Vqmin);
    }
    else if(msgs[0] == "phase"){
      checkPhaseAtGain(msgs[1].toDouble());
    }
    else if(msgs[0] == "gain"){
      checkGainAtPhase(msgs[1].toDouble());
    }
    else if(msgs[0] == "power"){
      Serial.print(readPower());
    }
    else{
      Gain = msgs[0].toDouble();
      Phase = msgs[1].toDouble();
      Serial.print("Gain,Phase=");
      Serial.println(String(Gain) + ',' + String(Phase));
      
      Vi,Vq = GainPhase2ViVq(Gain,Phase);    
      Serial.print("Vi,Vq=");
      Serial.println(String(Vi) + ',' + String(Vq));

      analogWrite(DAC0,int(((Vi-dacOffset)/dacResolution)));
      analogWrite(DAC1,int(((Vq-dacOffset)/dacResolution)));  
    }
   }
//Serial.println("power: " + String(readPower()));
delay(1000);
}

void writeIQ(double I,double Q){
      analogWrite(DAC0,int(((I-dacOffset)/dacResolution)));
      analogWrite(DAC1,int(((Q-dacOffset)/dacResolution)));  
}

double cancel(double gstep){
  //double gstep = 1.0;
  double pstep = 1.0;
  double minPower,minGain,minPhase;
  minPower = +100;
  for(double gain=Gmax; gain > -30 ; gain = gain - gstep){
    Serial.print("gain = " + String(gain)+"   "); 
    for(double phase=0.0; phase < 360 ; phase = phase + pstep){
      GainPhase2ViVq(gain,phase); //Set Vi,Vq
      //Serial.println(String(Vi)+","+String(Vq));
      delay(1);
      analogWrite(DAC0,int(((Vi-dacOffset)/dacResolution)));
      analogWrite(DAC1,int(((Vq-dacOffset)/dacResolution)));
      double tmpPower = avgPower(5);
      if(minPower > tmpPower ){
        minPower = tmpPower;
        minGain = gain;
        minPhase = phase;
        Vimin = Vi;
        Vqmin = Vq;
        //Serial.println(minGain); 
      }
      //Serial.println(phase); 
    }
    Serial.println("minPower,minGain,minPhase,Vimin,Vqmin," + String(minPower)+ "," + String(minGain) +","+String(minPhase,3)+","+String(Vimin,3)+","+String(Vqmin,3)); 
  }
    Serial.println("-->minPower,minGain,minPhase,Vimin,Vqmin," + String(minPower)+ "," + String(minGain) +","+String(minPhase,3)+","+String(Vimin,3)+","+String(Vqmin,3)); 
    Serial.println("-->Set Vi,Vq = : " + String(Vimin,3)+","+String(Vqmin,3));  
  return Vimin,Vqmin ;//Global Variable can be 2 returned
}

void checkPhaseAtGain(double gain){
    for(double phase=0.0; phase < 360 ; phase = phase + 4){
      GainPhase2ViVq(gain,phase); //Set Vi,Vq
      delay(50);
      analogWrite(DAC0,int(((Vi-dacOffset)/dacResolution)));
      analogWrite(DAC1,int(((Vq-dacOffset)/dacResolution)));
      Serial.println("phase,power,Vi,Vq,"+String(phase,3)+"," + String(avgPower(5))+","+String(Vi)+","+String(Vq));
 }
}

void checkGainAtPhase(double phase){
    for(double gain=Gmax; gain > -40 ; gain = gain - 0.5){
      GainPhase2ViVq(gain,phase); //Set Vi,Vq
      delay(50);
      analogWrite(DAC0,int(((Vi-dacOffset)/dacResolution)));
      analogWrite(DAC1,int(((Vq-dacOffset)/dacResolution)));
      Serial.println("gain,phase,power,Vi,Vq,"+String(gain,3)+","+String(phase,3)+"," + String(avgPower(5))+","+String(Vi)+","+String(Vq));
 }
}

double calIQ(){
  double Vstep = 0.0025;
  double minPower;
  minPower = +100000;
  for(Vi=1.4; Vi <1.5 ; Vi=Vi+Vstep){
    Serial.print("Vi = " + String(Vi,3)+"  "); 
    for(Vq=1.3; Vq <1.5 ; Vq=Vq+Vstep){
      //Serial.println(String(Vi)+","+String(Vq));
      delay(1);
      analogWrite(DAC0,int(((Vi-dacOffset)/dacResolution)));
      analogWrite(DAC1,int(((Vq-dacOffset)/dacResolution)));
      double tmpPower = avgPower(5);
      if(minPower > tmpPower ){
        minPower = tmpPower;
        Vmi = Vi;
        Vmq = Vq;
      }
    }
    Serial.println("minPower,Vmi,Vmq," + String(minPower)+","+String(Vmi,3)+","+String(Vmq,3)); 
  }
    Serial.println("-->minPower,Vmi,Vmq," + String(minPower)+","+String(Vmi,3)+","+String(Vmq,3));
    Serial.println("-->Set Vmi,Vmq = ," + String(Vmi,3)+","+String(Vmq,3));  
  return Vmi,Vmq ;//Global Variable can be 2 returned
}
double readPower(){
 // double power = (analogRead(0)*3.3/1023.0 - 0.35)/0.022; //AD8317 DUE Voltage =3.3
  double power = -((analogRead(0)*3.3/4095.0)-0.4)/0.018; //AD8317 DUE Voltage =3.3
    //+0dBm = 0.4V
    //-20dBm = 0.76V
    //(NoSignale = 1.74V)
    //==>18 mV/dB 
  return power;
}

double avgPower(int N){
  double sum=0;
  for(int i=0; i<N;i++){
    sum = sum + readPower();
  }
  double avgpower = sum/N;
  return avgpower;
}

//Refet to page6 from https://www.analog.com/media/en/technical-documentation/data-sheets/hmc630.pdf
//appendix https://www.analog.com/media/en/technical-documentation/application-notes/Vector_Modulators.pdf
double GainPhase2ViVq(double G,double Theta){
  if(G>Gmax){
    Serial.print("NG: G>Gmax");
    return Vmi,Vmq;
  }
  double GmaxTrue = pow(2.718,(Gmax/10.0)); // It's not log10 loge
  double GTrue =  pow(2.718,(G/10.0));
  //Serial.println("GmaxTrue;"+String(GmaxTrue));
  //Vi = Vmi + 1.0 * (GTrue/GmaxTrue)*cos(2*pi*Theta/360.0);
  //Vq = Vmq + 1.0 * (GTrue/GmaxTrue)*sin(2*pi*Theta/360.0);
  Vi = Vmi + (Vrange/2) * (GTrue/GmaxTrue)*cos(2*pi*Theta/360.0);
  Vq = Vmq + (Vrange/2) * (GTrue/GmaxTrue)*sin(2*pi*Theta/360.0);
  //Serial.println(Vi);
  return Vi,Vq;
}

/*
double GainPhase2ViorVq(String IorQ,double G,double Theta){
  double ViorVq;
  if(G>Gmax){
    Serial.print("NG: G>Gmax");
    return Vmi;
  }
  double GmaxTrue = pow(10.0,(Gmax/10.0));
  double GTrue =  pow(10.0,(G/10.0));
  if(IorQ="I"){
    ViorVq = Vmi + 1.0 * (GTrue/GmaxTrue)*cos(2*pi*Theta/360.0);
  }else if(IorQ="Q"){
    ViorVq = Vmq + 1.0 * (GTrue/GmaxTrue)*sin(2*pi*Theta/360.0);
  }
  return ViorVq;
}
*/


//Example: cal,Q,1.35,1.36,0.01,1.49
void calIorQ(String ChannelIQ, double Vstart, double Vend, double Vresol, double Vstatic){
  double VariableChannel,StaticChannel;
  if(ChannelIQ == "I"){
    VariableChannel = DAC0;
    StaticChannel = DAC1;
  }else if( ChannelIQ == "Q"){
    VariableChannel = DAC1;
    StaticChannel = DAC0;
  }
  
  analogWrite(StaticChannel,int((Vstatic-dacOffset)/dacResolution));
  for(double V=Vstart ; V<Vend ; V=V+Vresol){
    analogWrite(VariableChannel,int(((V-dacOffset)/dacResolution)));
    Serial.println(V,5);
    delay(1000);
  } 
}

// Thanks to https://algorithm.joho.info/arduino/string-split-delimiter/
int split(String data, char delimiter, String *dst){
    //文字列配列の初期化
    for (int j=0; j< sizeof(dst); j++){
      //Serial.println(dst(j));
      dst[j] = {""}; 
    }
    //msgs[3] = {"\0"};
  
    int index = 0;
    int arraySize = (sizeof(data)/sizeof((data)[0]));  
    int datalength = data.length();
    for (int i = 0; i < datalength; i++) {
        char tmp = data.charAt(i);
        if ( tmp == delimiter ) {
            index++;
            if ( index > (arraySize - 1)) return -1;
        }
        else dst[index] += tmp; //区切り文字が来るまで1Byteづつ連結
        //Serial.print("dbg: "+dst[index]);
    }
    return (index + 1);
}
