#include <M5Core2.h>
#include <TinyGPS++.h>
#define LGFX_AUTODETECT
#include "SD.h"
#include <LovyanGFX.hpp>
#include <SoftwareSerial.h>
#define pi 3.141592653589793

static LGFX lcd;
static LGFX_Sprite canvas(&lcd);
static const int RXPin = 33, TXPin = 32;
static const uint32_t GPSBaud = 9600;

// The TinyGPSPlus object
TinyGPSPlus gps;

// The serial connection to the GPS device
SoftwareSerial ss(RXPin, TXPin);
//ズーム倍率
int z = 12;

bool wasPressdBtnC = false;
int center = 50;

//WiFi初期設定-----------------
#include "WiFiClient.h"
WiFiClient client;
const char* ssid = "iPhone (165)"; //wifiのssid
const char* password = "qsgeqiemb48m9"; //wifiのパスワード
//2.4GHzで接続すること

//ambient初期設定-------------------------------------
#include "Ambient.h" // Ambientのヘッダーをインクルード
Ambient ambient; // Ambientオブジェクトを定義
unsigned int channelId = 74583; // AmbientのチャネルID
const char* writeKey = "53b959fc5b0e9e16"; // ライトキー

char speedbuf[12], altitudebuf[12], latbuf[12], lngbuf[12];

void drawMap(){

  //変数の定義
  double la = gps.location.lat();
  double ln = gps.location.lng();
  double L = 85.05112878;
  //緯度経度→ピクセル座標の変換計算
  double px = int(pow(2.0, z + 7.0) * ((ln / 180.0) + 1.0));
  double py = int(pow(2.0, z + 7.0) * (-1 * atanh(sin(pi * la / 180.0)) + atanh(sin(pi * L / 180.0))) / pi);
  //ピクセル座標→タイル座標の変換計算
  int tx = px / 256;
  int ty = py / 256;
  //タイル画像の中の座標を計算
  int x = int(px) % 256;
  int y = int(py) % 256;

  //画像9枚のファイルアドレスを用意
  String filename[9];
  filename[0] = String("/cyberjapandata.gsi.go.jp/xyz/std/" + String(z) + "/" + String(tx - 1) + "/" + String(ty-1) + ".jpg");
  filename[1] = String("/cyberjapandata.gsi.go.jp/xyz/std/" + String(z) + "/" + String(tx) + "/" + String(ty-1) + ".jpg");
  filename[2] = String("/cyberjapandata.gsi.go.jp/xyz/std/" + String(z) + "/" + String(tx + 1) + "/" + String(ty-1) + ".jpg");
  filename[3] = String("/cyberjapandata.gsi.go.jp/xyz/std/" + String(z) + "/" + String(tx - 1) + "/" + String(ty) + ".jpg");
  filename[4] = String("/cyberjapandata.gsi.go.jp/xyz/std/" + String(z) + "/" + String(tx) + "/" + String(ty) + ".jpg");
  filename[5] = String("/cyberjapandata.gsi.go.jp/xyz/std/" + String(z) + "/" + String(tx + 1) + "/" + String(ty) + ".jpg");
  filename[6] = String("/cyberjapandata.gsi.go.jp/xyz/std/" + String(z) + "/" + String(tx - 1) + "/" + String(ty+1) + ".jpg");
  filename[7] = String("/cyberjapandata.gsi.go.jp/xyz/std/" + String(z) + "/" + String(tx) + "/" + String(ty+1) + ".jpg");
  filename[8] = String("/cyberjapandata.gsi.go.jp/xyz/std/" + String(z) + "/" + String(tx + 1) + "/" + String(ty+1) + ".jpg");

  /*
  画像9枚の並びはこのようになっている
  [0][1][2]
  [3][4][5]
  [6][7][8]
  */

  //Stringからchar配列に変換
  for (int i = 0; i < 9; i++)
  {
    int str_len = filename[i].length() + 1;//ファイル名の文字数より1多いchar型の配列を用意する
    char file[9][str_len];
    filename[i].toCharArray(file[i], str_len);
  }

  //filename[4]を中心として画像を描画
  int mainx = -1 * (x-160), mainy = -1 * (y-120);//(mainx,maiy)はx,yを画面の真ん中に配置した時のタイル画像の書き出し位置
  lcd.drawJpgFile(SD, filename[4], mainx, mainy);
  lcd.fillCircle(160, 120, 10, TFT_CYAN);
  lcd.fillTriangle(160, 120 - 8, 160, 120 + 4, 160 - 6, 120 + 8,TFT_BLUE);
  lcd.fillTriangle(160, 120 - 8, 160 + 6, 120 + 8, 160, 120 + 4,TFT_BLUE);
  //他8枚の画像を描画

  if (mainx > 0 && mainy > 0)//[0]が画面に入る時
  {
    lcd.drawJpgFile(SD, filename[0], mainx - 256, mainy-256);
  }
  if (mainy > 0)//[1]が画面に入る時
  {
    lcd.drawJpgFile(SD, filename[1], mainx , mainy-256);
  }
  if (mainx + 256 < 320 && mainy >0)//[2]が画面に入る時
  {
    lcd.drawJpgFile(SD, filename[2], mainx + 256, mainy - 256);
  }


  if (mainx > 0)//[3]が画面に入る時
  {
    lcd.drawJpgFile(SD, filename[3], mainx - 256, mainy);
  }
  if (mainx + 256 < 320)//[5]が画面に入る時
  {
    lcd.drawJpgFile(SD, filename[5], mainx + 256, mainy);
  }


  if (mainx > 0 && mainy < -26)//[6]が画面に入る時
  {
    lcd.drawJpgFile(SD, filename[6], mainx - 256, mainy + 256);
  }
  if (mainy < -26)//[7]が画面に入る時
  {
    lcd.drawJpgFile(SD, filename[7], mainx, mainy + 256);
  }
  if (mainx + 256 < 320 && mainy < -26)//[8]が画面に入る時
  {
    lcd.drawJpgFile(SD, filename[8], mainx + 256, mainy + 256);
  }

  //中心に印をつける
  // lcd.fillCircle(160, 120, 8, TFT_CYAN);
  // lcd.fillCircle(160, 120, 5, TFT_BLUE);
  // lcd.fillCircle(160, 120, 10, TFT_CYAN);
  // lcd.fillTriangle(160, 120 - 8, 160, 120 + 4, 160 - 6, 120 + 8,TFT_BLUE);
  // lcd.fillTriangle(160, 120 - 8, 160 + 6, 120 + 8, 160, 120 + 4,TFT_BLUE);
  //canvas.pushSprite(160,120);

  // Batterylevel();
  // printTimeMini(gps.time);
}





unsigned long lastUpdateTime = 0;//最後に座標を更新した時間
unsigned long lastUpdateAvgTime = 0;//最後に平均速度を更新した時間
double totalDistance = 0.0;//総走行距離
double avgspeed = 0.0;//平均速度
double lat1 = 0;
double lng1 = 0;
/*GPSデータのエンコードが適切に行われ、遅延中に新しいデータが処理されるようにする。
通常のdelay()関数を使うと、一時停止する間にデータが逃れてしまう可能性があるが、この smartDelay() 関数ではその対策をしている。
*/
static void smartDelay(unsigned long ms) {
    unsigned long start = millis();  // millis()関数で現在のミリ秒数を取得
    do {
        while (ss.available())        // UARTからのデータがある場合
            gps.encode(ss.read());     // GPSオブジェクトにデータをエンコード

    } while (millis() - start < ms);  // 指定したミリ秒数経過するまでループ
}

static void printSpeed(float s){//速度を表示
  lcd.setTextSize(1);
  lcd.setFont(&fonts::Font4);
  lcd.setCursor(20, 20);  lcd.printf("Kph   %.1fkm/h",s);
  smartDelay(0);
}

static void printTime(TinyGPSTime &t) {//時間表示
    lcd.setTextSize(1);
    lcd.setCursor(20, 80);
    lcd.setFont(&fonts::Font4);
    if (!t.isValid()) {
        Lcd.print(F("******** "));
    } else {
        char sz[32];
        int h =  t.hour() + 9;//時差9時間を追加
        if(h >= 24){
          h = h - 24;
        }
        sprintf(sz, "Time   %02d:%02d:%02d ", h, t.minute(), t.second());
        Lcd.print(sz);
    }

    smartDelay(0);
}

static void printTimeMini(TinyGPSTime &t) {//時間表示
    lcd.setTextSize(1);
    lcd.setCursor(254,0);
    lcd.setFont(&fonts::Font2);
    if (!t.isValid()) {
        canvas.print(F("******** "));
    } else {
        char sz[32];
        int h =  t.hour() + 9;//時差9時間を追加
        if(h >= 24){
          h = h - 24;
        }
        sprintf(sz, "%02d:%02d", h, t.minute());
        lcd.print(sz);
        //lcd.pushSprite(254,0);
    }

    smartDelay(0);
}

#define R 6371.0  // 地球の半径（単位: km）

double haversine(double lat1, double lon1, double lat2, double lon2) {//地球上の2地点の距離を計算
    // 度をラジアンに変換
    lat1 = radians(lat1);
    lon1 = radians(lon1);
    lat2 = radians(lat2);
    lon2 = radians(lon2);

    // Haversine式に基づく距離計算
    double dlat = lat2 - lat1;
    double dlon = lon2 - lon1;
    double a = sq(sin(dlat / 2.0)) + cos(lat1) * cos(lat2) * sq(sin(dlon / 2.0));
    double c = 2.0 * atan2(sqrt(a), sqrt(1.0 - a));

    // 距離を計算して返す(単位:km）
    double distance = R * c;
    return distance;
}

void printMileage(){//総走行距離を表示
  lcd.setTextSize(1);
  lcd.setCursor(20, 140);
  lcd.setFont(&fonts::Font4);
  lcd.printf("Mileage   %.2fkm",totalDistance);
}

void printAvgSpeed(){//平均速度表示
  lcd.setTextSize(1);
  lcd.setCursor(20, 200);
  lcd.setFont(&fonts::Font4);
  lcd.printf("Avg   %.1fkm/h",avgspeed);
}
//リチュウムイオン電池は100%充電で4.2V,0%で3.2Vとするのが一般的らしい
void Batterylevel(){
  float battery;    // バッテリー残量表示用
  battery = (M5.Axp.GetBatVoltage() - 3.2) * 100;      // バッテリー残量取得、換算
  if (battery > 100) {                              // 換算値が100以上なら
    battery = 100;                                  // 100にする
  }
  lcd.setTextSize(1);
  lcd.setCursor(287, 0); lcd.setFont(&fonts::Font2);  // 座標、フォント 2(16px)
  lcd.setTextColor(TFT_WHITE, TFT_BLACK);             // 文字色
  lcd.printf("%3.0f%%", battery);                // バッテリー残量表示

}



void setup()
{
  //M5,LovyanGFX,GPSの初期化
  M5.begin();
  lcd.init();
  ss.begin(GPSBaud);

  //wifi------------
  WiFi.begin(ssid, password);  //  Wi-Fi APに接続
  while (WiFi.status() != WL_CONNECTED) {  //  Wi-Fi AP接続待ち
    delay(100);
  }

  Serial.print("WiFi connected\r\nIP address: ");
  Serial.println(WiFi.localIP());
  
  //ambient---------------------------------------------------------------------------
  ambient.begin(channelId, writeKey, &client); // チャネルIDとライトキーを指定してAmbientの初期化

  M5.Lcd.setTextSize(3);
  M5.Lcd.setCursor(30,120);
  M5.Lcd.println("Now loading...");
}

void loop()
{
  //GPSの位置情報取得
  while (!gps.location.isUpdated())
  {
    while (ss.available() > 0)
    {
      if (gps.encode(ss.read()))
      {
        break;
      }
    }
  }
  if(lat1 == 0 && lng1 == 0){
    lat1 = gps.location.lat();
    lng1 = gps.location.lng();
  }
  M5.update();
  //Cボタンを押すと別画面に移る
  if(M5.BtnC.wasPressed()){
    if(wasPressdBtnC){
      wasPressdBtnC = false;
    }else{
      wasPressdBtnC = true;
      lcd.fillScreen(TFT_BLACK);           // 背景色
      lcd.setTextColor(TFT_WHITE , TFT_BLACK); // 文字色
      lcd.setFont(&fonts::Font4);      // フォント設定
    }
  }

  //Aボタンを押すとズーム倍率を上げる
  if(M5.BtnA.wasPressed()){
    if(z!=14){
    z+=2;
    }
  }
  //Bボタンを押すとズーム倍率を下げる
  if(M5.BtnB.wasPressed()){
    z-=2;
  }
  //10秒おきに走行距離を更新する
    if(millis() - lastUpdateTime >= 10000){
      lastUpdateTime = millis();
      
      if (gps.location.isValid()) {
        double distance = haversine(gps.location.lat(), gps.location.lng(), lat1, lng1);
        totalDistance += distance;
      }
      lat1 = gps.location.lat();
      lng1 = gps.location.lng();
    }


  if(wasPressdBtnC){

    printSpeed(gps.speed.kmph());
    printTime(gps.time);
    printMileage();
    printAvgSpeed();
    Batterylevel();
  }else{
    drawMap();
  }

  //1分おきに平均速度を求める
    if(millis() - lastUpdateAvgTime >= 60000){
      lastUpdateAvgTime = millis();
      avgspeed = totalDistance/(millis() / 3600000.0);

      // センサー値をAmbientに送信する
      dtostrf(gps.location.lat(), 11, 7, latbuf);
      ambient.set(9, latbuf);
      dtostrf(gps.location.lng(), 11, 7, lngbuf);
      ambient.set(10, lngbuf);
      dtostrf(gps.speed.kmph(), 4, 2, speedbuf);
      ambient.set(1, speedbuf);
      dtostrf(gps.altitude.meters(), 5, 1, altitudebuf);
      ambient.set(2, altitudebuf);
      ambient.send();
      smartDelay(5000);
    }

  smartDelay(1000);
}