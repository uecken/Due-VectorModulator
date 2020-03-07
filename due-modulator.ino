double Gain,Phase,Vi,Vq,Vmi,Vmq,Vrange,Gmax;
String msgs[6] = {"\0"};
double dacVmin,dacVmax, dacVmaxIndex,dacRange,dacResolution,dacOffset;
double pi = 3.14159;

void setup() {
  Serial.begin(9600);
  Serial.print("DUE-IQmodulator wake up.");
  analogWriteResolution(12); //Max?
  
  //Set Variable
  Vi=0;Vq=0;Vmi=0;Vmq=0;Vrange=0;Gmax=0;

  //Calibrated value
  dacVmin = 0.55; //@analogWrite(DAC0, 0)
  dacVmax = 2.5; //@analogWrite(DAC0, 3640)
  dacVmaxIndex = 3640; // Index at Vmax2.5V 
  dacResolution = (dacVmax-dacVmin)/(dacVmaxIndex+1);
  dacOffset = dacVmin;
  
  //Callibrated.
  Vmi = 1.49; Vmq =1.356; //Vrange = dacVmin - dacVmax
  Gmax = -10.00;

  //Init
  analogWrite(DAC0,int(((1.5-dacOffset)/dacResolution)));
  analogWrite(DAC1,int(((1.5-dacOffset)/dacResolution)));

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
      calIQ(msgs[1],msgs[2].toDouble(),msgs[3].toDouble(),msgs[4].toDouble(),msgs[5].toDouble());
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
// delay(1000);
}



//Refet to page6 from https://www.analog.com/media/en/technical-documentation/data-sheets/hmc630.pdf
//appendix https://www.analog.com/media/en/technical-documentation/application-notes/Vector_Modulators.pdf
double GainPhase2ViVq(double G,double Theta){
  double GmaxTrue = pow(10.0,(Gmax/10.0));
  double GTrue =  pow(10.0,(G/10.0));
  Vi = Vmi + 1.0 * (GTrue/GmaxTrue)*cos(2*pi*Theta/360);
  Vq = Vmq + 1.0 * (GTrue/GmaxTrue)*sin(2*pi*Theta/360);    
  return Vi,Vq;
}

void calIQ(String ChannelIQ, double Vstart, double Vend, double Vresol, double Vstatic){
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
