#include <WiFi.h>
#include <WiFiClient.h>
#include <Servo.h>

//巴法云服务器地址默认即可
#define TCP_SERVER_ADDR "bemfa.com"
//服务器端口，tcp创客云端口8344
#define TCP_SERVER_PORT "8344"

//********************需要修改的部分*******************//
#define DEFAULT_STASSID  "iamddd"     //WIFI名称，区分大小写，不要写错
#define DEFAULT_STAPSW   "88888888"  //WIFI密码
String UID = "";  //用户私钥，可在控制台获取,修改为自己的UID
String TOPIC =   "";         //主题名字，可在控制台新建
const int LED_Pin = 4;              //ESP32的GPIO4引脚
//**************************************************//

//最大字节数
#define MAX_PACKETSIZE 512
//设置心跳值30s
#define KEEPALIVEATIME 30*1000

//tcp客户端相关初始化，默认即可
WiFiClient TCPclient;
String TcpClient_Buff = "";
unsigned int TcpClient_BuffIndex = 0;
unsigned long TcpClient_preTick = 0;
unsigned long preHeartTick = 0;//心跳
unsigned long preTCPStartTick = 0;//连接
bool preTCPConnected = false;

//相关函数初始化
//连接WIFI
void doWiFiTick();
void startSTA();

//TCP初始化连接
void doTCPClientTick();
void startTCPClient();
void sendtoTCPServer(String p);

//led 控制函数
void turnOnServo();

/*
  *发送数据到TCP服务器
 */
void sendtoTCPServer(String p){

    if (!TCPclient.connected())
    {
        Serial.println("Client is not readly");
        return;
    }
    TCPclient.print(p);
    Serial.println("[Send to TCPServer]:String");
    Serial.println(p);
}

/*
  *初始化和服务器建立连接
*/
void startTCPClient(){
    if(TCPclient.connect(TCP_SERVER_ADDR, atoi(TCP_SERVER_PORT))){
        Serial.print("\nConnected to server:");
        Serial.printf("%s:%d\r\n",TCP_SERVER_ADDR,atoi(TCP_SERVER_PORT));

        String tcpTemp="";  //初始化字符串
        tcpTemp = "cmd=1&uid="+UID+"&topic="+TOPIC+"\r\n"; //构建订阅指令
        sendtoTCPServer(tcpTemp); //发送订阅指令
        tcpTemp="";//清空

        preTCPConnected = true;
        preHeartTick = millis();
        TCPclient.setNoDelay(true);
    }
    else{
        Serial.print("Failed connected to server:");
        Serial.println(TCP_SERVER_ADDR);
        TCPclient.stop();
        preTCPConnected = false;
    }
    preTCPStartTick = millis();
}

/*
  *检查数据，发送心跳
*/
void doTCPClientTick(){
    //检查是否断开，断开后重连
    if(WiFi.status() != WL_CONNECTED) return;

    if (!TCPclient.connected()) {//断开重连

        if(preTCPConnected == true){

            preTCPConnected = false;
            preTCPStartTick = millis();
            Serial.println();
            Serial.println("TCP Client disconnected.");
            TCPclient.stop();
        }
        else if(millis() - preTCPStartTick > 1*1000)//重新连接
            startTCPClient();
    }
    else
    {
        if (TCPclient.available()) {//收数据
            char c =TCPclient.read();
            TcpClient_Buff +=c;
            TcpClient_BuffIndex++;
            TcpClient_preTick = millis();

            if(TcpClient_BuffIndex>=MAX_PACKETSIZE - 1){
                TcpClient_BuffIndex = MAX_PACKETSIZE-2;
                TcpClient_preTick = TcpClient_preTick - 200;
            }
            preHeartTick = millis();
        }
        if(millis() - preHeartTick >= KEEPALIVEATIME){//保持心跳
            preHeartTick = millis();
            Serial.println("--Keep alive:");
            sendtoTCPServer("cmd=0&msg=keep\r\n");
        }
    }
    if((TcpClient_Buff.length() >= 1) && (millis() - TcpClient_preTick>=200))
    {//data ready
        TCPclient.flush();
        Serial.println("Buff");
        Serial.println(TcpClient_Buff);
        if((TcpClient_Buff.indexOf("&msg=on") > 0)) {
            turnOnServo();
        }else if((TcpClient_Buff.indexOf("&msg=off") > 0)) {
            turnOnServo();
        }
        TcpClient_Buff="";
        TcpClient_BuffIndex = 0;
    }
}

void startSTA(){
    WiFi.disconnect();
    WiFi.mode(WIFI_STA);
    WiFi.begin(DEFAULT_STASSID, DEFAULT_STAPSW);
}

/*
  WiFiTick
  检查是否需要初始化WiFi
  检查WiFi是否连接上，若连接成功启动TCP Client
  控制指示灯
*/
void doWiFiTick(){
    static bool startSTAFlag = false;
    static bool taskStarted = false;
    static uint32_t lastWiFiCheckTick = 0;

    if (!startSTAFlag) {
        startSTAFlag = true;
        startSTA();
        Serial.printf("Heap size:%d\r\n", ESP.getFreeHeap());
    }
    //未连接1s重连
    if ( WiFi.status() != WL_CONNECTED ) {
        if (millis() - lastWiFiCheckTick > 1000) {
            lastWiFiCheckTick = millis();
        }
    }
        //连接成功建立
    else {
        if (taskStarted == false) {
            taskStarted = true;
            Serial.print("\r\nGet IP Address: ");
            Serial.println(WiFi.localIP());
            startTCPClient();
        }
    }
}

//打开
void turnOnServo(){
    Serial.println("Turn ON");
    Servo servo1;
    delay(500);
    servo1.attach(LED_Pin);
    delay(500);
    //从0°转到180°
    for(int posDegrees = 0; posDegrees <= 180; posDegrees++) {
        servo1.write(posDegrees);
        Serial.println(posDegrees);
        delay(10);
    }
    delay(2500);
    //从180°转到0°
    for(int posDegrees = 180; posDegrees >= 0; posDeg+rees--) {
        servo1.write(posDegrees);
        Serial.println(posDegrees);
        delay(10);
    }
}

// 初始化，相当于main 函数
void setup() {
    Serial.begin(115200);
    Serial.println("Beginning...");
}

//循环
void loop() {
    doWiFiTick();
    doTCPClientTick();
}