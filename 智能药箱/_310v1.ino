#include <Servo.h> 
#include <Wire.h>
#include <TimeLib.h>
#include <DS1307RTC.h>
#include <DHT.h>  
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#define HC_TX       10
#define HC_RX       11
#define Tri           9
#define Echo          8
#define ALA           13       // 蜂鸣器
#define DHTPIN 7     //定义针脚
#define DHTTYPE DHT11//定义类型，DHT11或者其它
#define led          6
DHT dht(DHTPIN, DHTTYPE);//进行初始设置 

SoftwareSerial mySerial(HC_RX, HC_TX);   //RX, TX
LiquidCrystal_I2C lcd(0x27,16,2); 
String comdata = "";
String s = "";           
int b = 60;//用于闹钟响铃时间
int dis = 0;//记录距离
int H = 0;//设定闹钟：时
int M = 0;//设定闹钟：分
int flag = 0;    //用于判断盒子开启状态 ,1 为开启
int lock = 0;
tmElements_t tm;

Servo myservo;  
int pos = 0;    

void show();
int checkifTake();

void setup() {
  // put your setup code here, to run once:
  lcd.init(); // initialize the lcd 
  lcd.backlight(); //Open the backlight
  dht.begin();
  myservo.attach(3); 
  
  mySerial.begin(38400);
  Serial.begin(115200);
  pinMode(ALA,OUTPUT);//蜂鸣器引脚
  pinMode(Tri,OUTPUT); //超声波模块Tri引脚
  pinMode(Echo,INPUT);//超声波模块Echo引脚
  pinMode(led,OUTPUT);//小灯
  myservo.write(30);
}

void loop() {
  while(mySerial.available())
  {
    comdata += char(mySerial.read());
    delay(2);
  }
  if(comdata.length() > 0)
  {
    if(comdata[0]=='a')
    {
       Serial.println("open!");
       open();
    }
    if(comdata[0]=='b')
    {
       Serial.write("close!");
       close();
    }
    Serial.println(comdata);
    H=(comdata[0]-'0')*10+(comdata[1]-'0');
    //comdata[2]是：或者其他分隔符
    M=(comdata[3]-'0')*10+(comdata[4]-'0');
    Serial.print(H);
    Serial.print(":");
    Serial.println(M);
    comdata = "";    
  }
    
  //时钟部分
  if(RTC.read(tm))
  {
    lcd.clear();
    show();
    delay(1000);
    Serial.print(tm.Hour);
    Serial.print(":");
    Serial.print(tm.Minute);
    Serial.print(":");
    Serial.println(tm.Second);
    if(H==tm.Hour && M==tm.Minute && flag==0 && lock==0)     // tm.Second 不能用
    {
     //舵机升起
     open();  
     flag=1;
     while(!checkifTake())
     {
       Serial.println("未拿药");
       delay(500);
       //wifi.print("It's time to take medicine!");
       //RTC.read(tm);
       show();
       lcd.setCursor(0,1);
       lcd.print("take medicine!  ");
       digitalWrite(ALA,HIGH);
       digitalWrite(led,HIGH);
       delay(500);
       if(checkifTake())
         break;
     }
     digitalWrite(ALA,LOW);
     digitalWrite(led,LOW);
    }     
  }
  if(flag==1 && !checkifTake() && lock==0)
  {
     close();     // 关闭
     flag=0;
     lock=1; 
  }
  if(tm.Minute>M)
     lock = 0;
} 

int checkifTake()
{
    digitalWrite(Tri,LOW);
    delayMicroseconds(2);
    digitalWrite(Tri,HIGH);
    delayMicroseconds(10);
    digitalWrite(Tri,LOW);
    float dis=pulseIn(Echo,HIGH)/58.0;
    Serial.println(dis);
    if(dis>5)
     return 1;
    else
     return 0;
}

void show()
{
    lcd.setCursor(0,0);
    lcd.print(tmYearToCalendar(tm.Year));
    lcd.print('-');
    lcd.print(tm.Month);
    lcd.print('-');
    lcd.print(tm.Day);
    lcd.print(' ');
    lcd.print(tm.Hour);
    lcd.print(':');
    if(tm.Minute<10)
    {
      lcd.print('0');
      lcd.print(tm.Minute);
    }
    else
      lcd.print(tm.Minute);

    int h = dht.readHumidity();//读湿度
    int t = dht.readTemperature();//读温度，默认为摄氏度
    
    lcd.setCursor(0,1);
    lcd.print("H:");//湿度
    lcd.print(h);
    lcd.print("%RH");
    lcd.print("  T:");//温度
    lcd.print(t);
    lcd.print("%C");

}

void open()
{
   //打开
   for(pos = 30; pos < 120; pos += 1)  
   {                                  
     myservo.write(pos);             
     delay(15);                      
   }     
}

void close()
{
  //关闭
   for(pos = 120; pos>=30; pos-=1)     
   {                                
     myservo.write(pos);              
     delay(15);                      
   }    
}
