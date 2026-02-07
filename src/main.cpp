#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <time.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "config.h"

// OLED 설정
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_SDA 14
#define OLED_SCL 12

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// 전역 변수
unsigned long lastPageChange = 0;
int currentPage = 0;
const int totalPages = 28; 

String seoulTemp = "Waiting...";
unsigned long lastWeatherUpdate = 0;

// --- 유틸리티 ---
void drawHeader(const char* title) {
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(title);
  display.drawFastHLine(0, 10, 128, SSD1306_WHITE);
}

void updateWeather() {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client; HTTPClient http;
    http.setTimeout(8000);
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    http.setUserAgent("curl/7.64.1");
    if (http.begin(client, "http://wttr.in/Seoul?format=%t")) {
      int httpCode = http.GET();
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        seoulTemp = http.getString(); seoulTemp.trim();
        seoulTemp.replace("°C", " C"); seoulTemp.replace("°", " ");
      }
      http.end();
    }
  }
}

// --- Classic Game Animations ---

// 10. TETRIS
void drawTetris() {
  drawHeader("TETRIS CLASSIC");
  static int state = 0; // 0: falling, 1: clearing
  static float y = 12;
  static unsigned long lastUpdate = 0;
  
  if (millis() - lastUpdate > 100) {
    if (state == 0) {
      y += 2;
      if (y >= 48) { y = 48; state = 1; }
    } else {
      static int flash = 0;
      flash++;
      if (flash > 6) { state = 0; y = 12; flash = 0; }
    }
    lastUpdate = millis();
  }

  // Well
  display.drawRect(40, 12, 48, 50, SSD1306_WHITE);
  // Stack
  display.fillRect(40, 52, 48, 10, SSD1306_WHITE);
  display.fillRect(48, 54, 32, 6, SSD1306_BLACK); // Hole

  // Falling block (I-piece)
  if (state == 0) {
    display.fillRect(60, y, 8, 16, SSD1306_WHITE);
    display.drawRect(60, y, 8, 4, SSD1306_BLACK);
    display.drawRect(60, y+4, 8, 4, SSD1306_BLACK);
    display.drawRect(60, y+8, 8, 4, SSD1306_BLACK);
  } else {
    // Clear animation (flash)
    if ((millis()/100)%2 == 0) display.fillRect(40, 52, 48, 10, SSD1306_WHITE);
  }
}

// 11. LODE RUNNER
void drawLodeRunner() {
  drawHeader("LODE RUNNER DIG");
  static int x = 20;
  static bool digging = false;
  static int holeSize = 0;
  static unsigned long lastUpdate = 0;
  
  if (millis() - lastUpdate > 150) {
    if (!digging) {
      x += 3;
      if (x > 60) digging = true;
    } else {
      holeSize++;
      if (holeSize > 15) { x = 20; digging = false; holeSize = 0; }
    }
    lastUpdate = millis();
  }

  // Floor
  display.fillRect(0, 50, 128, 14, SSD1306_WHITE);
  if (digging) {
    display.fillRect(x + 10, 50, 8, 8, SSD1306_BLACK); // Digging hole
  }

  // Character
  int cy = 40;
  display.fillRect(x, cy, 6, 10, SSD1306_WHITE); // Body
  display.fillRect(x+1, cy-3, 4, 4, SSD1306_WHITE); // Head
  if (digging) {
    display.drawLine(x+5, cy+5, x+12, cy+10, SSD1306_WHITE); // Shovel
  }
}

// 12. PRINCE OF PERSIA
void drawPrinceOfPersia() {
  drawHeader("PRINCE JUMP");
  static float px = 0, py = 40;
  static bool jumping = false;
  static float t = 0;
  static unsigned long lastUpdate = 0;

  if (millis() - lastUpdate > 50) {
    if (!jumping) {
      px += 4;
      if (px > 40) jumping = true;
    } else {
      t += 0.1;
      px += 4;
      py = 40 - 25 * sin(t * PI / 2.0); // Jump arc
      if (t >= 2.0) { jumping = false; t = 0; px = 0; py = 40; }
    }
    lastUpdate = millis();
  }

  // Pit
  display.fillRect(0, 50, 50, 14, SSD1306_WHITE);
  display.fillRect(90, 50, 38, 14, SSD1306_WHITE);
  
  // Prince
  display.fillRect(px, py, 6, 10, SSD1306_WHITE); // Body
  display.fillRect(px+1, py-4, 4, 4, SSD1306_WHITE); // Head
  display.drawLine(px-2, py+2, px, py+5, SSD1306_WHITE); // Cape/Arms
}

// --- Rest of the Content ---

void drawBigAnalogClock() {
  int cx = 64, cy = 32, r = 31;
  for(int i=0; i<12; i++) {
    float angle = i * 30 * PI / 180;
    display.drawLine(cx + (r-3)*cos(angle), cy + (r-3)*sin(angle), cx + r*cos(angle), cy + r*sin(angle), SSD1306_WHITE);
  }
  display.drawCircle(cx, cy, r, SSD1306_WHITE);
  time_t now = time(nullptr); struct tm* t = localtime(&now);
  float s_a = (t->tm_sec * 6 - 90) * PI / 180;
  display.drawLine(cx, cy, cx + 28 * cos(s_a), cy + 28 * sin(s_a), SSD1306_WHITE);
  float m_a = (t->tm_min * 6 - 90) * PI / 180;
  display.drawLine(cx, cy, cx + 24 * cos(m_a), cy + 24 * sin(m_a), SSD1306_WHITE);
  float h_a = ((t->tm_hour % 12) * 30 + t->tm_min / 2.0 - 90) * PI / 180;
  display.drawLine(cx, cy, cx + 18 * cos(h_a), cy + 18 * sin(h_a), SSD1306_WHITE);
  display.fillCircle(cx, cy, 2, SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 56); display.print("ANALOG");
  display.setCursor(95, 56); display.print("CLOCK");
}

void drawBigClock() {
  drawHeader("CURRENT TIME");
  time_t now = time(nullptr); struct tm* t = localtime(&now);
  display.setTextSize(2); display.setCursor(16, 25);
  display.printf("%02d:%02d:%02d", t->tm_hour, t->tm_min, t->tm_sec);
  display.setTextSize(1); display.setCursor(32, 50);
  display.printf("%04d-%02d-%02d", t->tm_year+1900, t->tm_mon+1, t->tm_mday);
}

void drawSystemInfo() {
  drawHeader("SYSTEM INFORMATION");
  display.setCursor(0, 20); display.printf("CPU: %d MHz", ESP.getCpuFreqMHz());
  display.setCursor(0, 31); display.printf("RAM: %d Bytes", ESP.getFreeHeap());
  display.setCursor(0, 42); display.printf("IP: %s", WiFi.localIP().toString().c_str());
  display.setCursor(0, 53); display.printf("Uptime: %lu s", millis()/1000);
}

void drawWeatherPage() {
  drawHeader("SEOUL WEATHER");
  display.setTextSize(1); display.setCursor(10, 20); display.println("South Korea, Seoul");
  display.setTextSize(3); display.setCursor(20, 38); display.print(seoulTemp);
}

void drawSpectrumVisualizer() {
  drawHeader("AUDIO SPECTRUM");
  static int values[16], peaks[16]; static unsigned long lu = 0;
  if (millis() - lu > 50) {
    for (int i = 0; i < 16; i++) {
      int target = random(5, 45);
      if (values[i] < target) values[i] += 5; else values[i] -= 3;
      if (values[i] > peaks[i]) peaks[i] = values[i]; else if (peaks[i] > 0) peaks[i] -= 1;
    }
    lu = millis();
  }
  for (int i = 0; i < 16; i++) {
    int x = i * 8 + 1, h = values[i], y = 60 - h;
    display.fillRect(x, y, 6, h, SSD1306_WHITE);
    display.drawFastHLine(x, 60 - peaks[i] - 2, 6, SSD1306_WHITE);
  }
}

void drawBarGraph() {
  drawHeader("LOAD MONITOR");
  static int levels[4] = {40, 60, 20, 80};
  for(int i=0; i<4; i++) {
    levels[i] += random(-5, 6);
    if(levels[i] < 0) levels[i] = 0; if(levels[i] > 100) levels[i] = 100;
    int h = map(levels[i], 0, 100, 0, 40);
    int x = 15 + i*30;
    display.drawRect(x, 20, 10, 42, SSD1306_WHITE);
    display.fillRect(x, 62-h, 10, h, SSD1306_WHITE);
    display.setCursor(x-2, 12); display.print(levels[i]);
  }
}

void drawFighterRadar() {
  drawHeader("FIGHTER RADAR");
  int cx = 64, cy = 37, r = 25;
  display.drawCircle(cx, cy, r, SSD1306_WHITE);
  display.drawCircle(cx, cy, r/2, SSD1306_WHITE);
  display.drawLine(cx-r, cy, cx+r, cy, SSD1306_WHITE);
  display.drawLine(cx, cy-r, cx, cy+r, SSD1306_WHITE);
  static float ang = 0; float x = cx + cos(ang) * r, y = cy + sin(ang) * r;
  display.drawLine(cx, cy, x, y, SSD1306_WHITE);
  static int bx=0, by=0; static unsigned long lb=0;
  if(millis()-lb>2000){ bx=cx+random(-r+5,r-5); by=cy+random(-r+5,r-5); lb=millis(); }
  if(bx!=0) display.fillCircle(bx,by,2,SSD1306_WHITE);
  ang += 0.1;
}

void drawArrowTarget() {
  drawHeader("BULLSEYE SHOT");
  int cx = 100, cy = 37;
  display.drawCircle(cx, cy, 15, SSD1306_WHITE);
  display.drawCircle(cx, cy, 10, SSD1306_WHITE);
  display.drawCircle(cx, cy, 5, SSD1306_WHITE);
  display.fillCircle(cx, cy, 2, SSD1306_WHITE);
  static float ax = 0; static bool hit = false; static unsigned long lr = 0;
  if (millis() - lr > 3000) { ax = 0; hit = false; lr = millis(); }
  if (!hit) { ax += 4; if (ax >= cx - 5) { ax = cx - 5; hit = true; } }
  display.drawLine(ax - 20, cy, ax, cy, SSD1306_WHITE);
  display.drawLine(ax, cy, ax - 4, cy - 3, SSD1306_WHITE);
  display.drawLine(ax, cy, ax - 4, cy + 3, SSD1306_WHITE);
  display.drawLine(ax - 20, cy, ax - 24, cy - 4, SSD1306_WHITE);
  display.drawLine(ax - 20, cy, ax - 24, cy + 4, SSD1306_WHITE);
}

void drawLightsaberDuel() {
  drawHeader("LIGHTSABER DUEL");
  static float ofs = 0; float pos = sin(ofs) * 30;
  int x1 = 64 - 10 + pos; int x2 = 64 + 10 - pos;
  display.drawLine(x1, 15, x1 + 5, 55, SSD1306_WHITE);
  display.drawLine(x2, 15, x2 - 5, 55, SSD1306_WHITE);
  if (abs(x1 - x2) < 5) {
    int cx = (x1 + x2) / 2;
    for(int i=0; i<8; i++) {
      float a = i * 45 * PI / 180;
      display.drawLine(cx, 35, cx + cos(a)*10, 35 + sin(a)*10, SSD1306_WHITE);
    }
  }
  ofs += 0.15;
}

void drawSunrise() {
  drawHeader("MOUNTAIN SUNRISE");
  display.drawLine(0, 63, 40, 30, SSD1306_WHITE); display.drawLine(40, 30, 80, 63, SSD1306_WHITE);
  display.drawLine(60, 63, 100, 40, SSD1306_WHITE); display.drawLine(100, 40, 128, 63, SSD1306_WHITE);
  static float sy = 70; static unsigned long lr = 0;
  if(millis() - lr > 5000) { sy = 70; lr = millis(); }
  if(sy > 30) sy -= 0.3;
  display.drawCircle(64, sy, 8, SSD1306_WHITE);
  if (sy < 55) { for(int i=0; i<8; i++) { float a = i * 45 * PI / 180; display.drawLine(64 + cos(a)*10, sy + sin(a)*10, 64 + cos(a)*14, sy + sin(a)*14, SSD1306_WHITE); } }
}

void drawLissajous() {
  drawHeader("LISSAJOUS CURVES"); static float a=3, b=2, d=0; float xp, yp;
  for(int i=0;i<=80;i++){ float t=i*0.0785, x=64+45*sin(a*t+d), y=37+22*sin(b*t); if(i>0)display.drawLine(xp,yp,x,y,SSD1306_WHITE); xp=x; yp=y; }
  d+=0.05;
}

void drawLorenz() {
  drawHeader("LORENZ ATTRACTOR");
  static float x = 0.1, y = 0, z = 0; float dt = 0.01;
  float dx = (10 * (y - x)) * dt; float dy = (x * (28 - z) - y) * dt; float dz = (x * y - (8.0/3.0) * z) * dt;
  x+=dx; y+=dy; z+=dz; static int pts[80][2], ph=0;
  pts[ph][0]=64+x*2; pts[ph][1]=60-z*1; ph=(ph+1)%80;
  for(int i=0;i<79;i++){ int c=(ph+i)%80, n=(ph+i+1)%80; if(pts[c][0]!=0)display.drawLine(pts[c][0],pts[c][1],pts[n][0],pts[n][1],SSD1306_WHITE); }
}

void drawSierpinski() {
  drawHeader("SIERPINSKI FRACTAL"); int tx[]={64,10,118}, ty[]={15,60,60}; static int px=64, py=37;
  for(int i=0;i<100;i++){ int r=random(3); px=(px+tx[r])/2; py=(py+ty[r])/2; display.drawPixel(px,py,SSD1306_WHITE); }
}

void drawTorus() {
  drawHeader("3D WIRE TORUS"); static float A=0, B=0;
  for(float p=0;p<6.28;p+=0.8){ for(float t=0;t<6.28;t+=0.4){
    float x=(2+cos(t))*cos(p), y=(2+cos(t))*sin(p), z=sin(t), nx=x*cos(B)-y*sin(B), ny=x*sin(B)+y*cos(B); x=nx; y=ny;
    float nz=y*sin(A)+z*cos(A); ny=y*cos(A)-z*sin(A); y=ny; z=nz; display.drawPixel(64+x*12,37+y*12,SSD1306_WHITE);
  } } A+=0.05; B+=0.03;
}

void drawMoire() {
  drawHeader("MOIRE INTERFERENCE"); static float a=0;
  for(int i=0;i<128;i+=4)display.drawLine(i,12,64+cos(a)*64,64+sin(a)*64,SSD1306_WHITE); a+=0.02;
}

void drawRotating3D() {
  static float a=0; static int type=0; static unsigned long ls=0; if(millis()-ls>2000){type=!type;ls=millis();}
  drawHeader(type==0?"3D WIRE CUBE":"3D WIRE PYRAMID");
  float nodes[8][3]; int edges[12][2]; int nn, ne;
  if(type==0){ float n[8][3]={{-1,-1,-1},{1,-1,-1},{1,1,-1},{-1,1,-1},{-1,-1,1},{1,-1,1},{1,1,1},{-1,1,1}}; int e[12][2]={{0,1},{1,2},{2,3},{3,0},{4,5},{5,6},{6,7},{7,4},{0,4},{1,5},{2,6},{3,7}}; nn=8; ne=12; memcpy(nodes,n,sizeof(n)); memcpy(edges,e,sizeof(e)); }
  else { float n[5][3]={{0,-1.2,0},{-1,1,-1},{1,1,-1},{1,1,1},{-1,1,1}}; int e[8][2]={{0,1},{0,2},{0,3},{0,4},{1,2},{2,3},{3,4},{4,1}}; nn=5; ne=8; memcpy(nodes,n,sizeof(float)*5*3); memcpy(edges,e,sizeof(int)*8*2); }
  int p[8][2]; for(int i=0;i<nn;i++){ float x=nodes[i][0],y=nodes[i][1],z=nodes[i][2],nx=x*cos(a)+z*sin(a),nz=-x*sin(a)+z*cos(a); p[i][0]=64+nx*20; p[i][1]=37+y*20; }
  for(int i=0;i<ne;i++)display.drawLine(p[edges[i][0]][0],p[edges[i][0]][1],p[edges[i][1]][0],p[edges[i][1]][1],SSD1306_WHITE); a+=0.05;
}

void drawDNA() {
  drawHeader("3D DNA HELIX"); static float o=0;
  for(int i=0; i<20; i++){ float a=i*0.4+o; int x1=64+cos(a)*20,y=15+i*2.2,x2=64+cos(a+PI)*20; display.drawPixel(x1,y,SSD1306_WHITE); display.drawPixel(x2,y,SSD1306_WHITE); if(i%2==0)display.drawLine(x1,y,x2,y,SSD1306_WHITE); }
  o+=0.1;
}

void drawPlexus() {
  drawHeader("NEURAL PLEXUS"); const int nd=10; static float x[nd],y[nd],dx[nd],dy[nd]; static bool in=false;
  if(!in){for(int i=0;i<nd;i++){x[i]=random(128);y[i]=random(12,64);dx[i]=(random(5,10)/10.0)*(random(2)?1:-1);dy[i]=(random(5,10)/10.0)*(random(2)?1:-1);}in=true;}
  for(int i=0; i<nd; i++){x[i]+=dx[i];y[i]+=dy[i];if(x[i]<0||x[i]>128)dx[i]*=-1;if(y[i]<12||y[i]>64)dy[i]*=-1;display.drawPixel(x[i],y[i],SSD1306_WHITE);
  for(int j=i+1; j<nd; j++){float d=sqrt(pow(x[i]-x[j],2)+pow(y[i]-y[j],2));if(d<25) display.drawLine(x[i],y[i],x[j],y[j],SSD1306_WHITE);}}
}

void drawMatrix() {
  drawHeader("THE MATRIX"); static int r[15]; static unsigned long lu=0;
  if(millis()-lu>50){for(int i=0;i<15;i++){r[i]+=random(1,4);if(r[i]>64)r[i]=0;}lu=millis();}
  for(int i=0;i<15;i++){int x=i*9;for(int j=0;j<5;j++){int y=r[i]-j*8;if(y>10&&y<64)display.drawPixel(x,y,SSD1306_WHITE);}}
}

void drawWarp() {
  drawHeader("WARP SPEED"); static float s[20][3]; static bool in=false;
  if(!in){for(int i=0;i<20;i++){s[i][0]=random(-64,64);s[i][1]=random(-32,32);s[i][2]=random(1,128);}in=true;}
  for(int i=0; i<20; i++){s[i][2]-=3;if(s[i][2]<=0)s[i][2]=128;int sx=64+(s[i][0]*128/s[i][2]),sy=32+(s[i][1]*128/s[i][2]);if(sx>=0&&sx<128&&sy>=12&&sy<64)display.drawPixel(sx,sy,SSD1306_WHITE);}
}

void drawWave() {
  drawHeader("SINE WAVEFORM"); static float o=0;
  for(int i=0; i<128; i++){int y1=30+sin(i*0.1+o)*10,y2=42+sin(i*0.05-o*1.2)*8;display.drawPixel(i,y1,SSD1306_WHITE);display.drawPixel(i,y2,SSD1306_WHITE);}o+=0.15;
}

void drawSpiral() {
  drawHeader("GOLDEN SPIRAL"); static float r=0;
  for(int n=0;n<70;n++){float a=n*137.5*PI/180+r,d=3.5*sqrt(n);int x=64+d*cos(a),y=37+d*sin(a);if(x>=0&&x<128&&y>=12&&y<64)display.drawPixel(x,y,SSD1306_WHITE);}r+=0.02;
}

void drawOrbit() {
  drawHeader("PLANETARY ORBIT"); display.drawCircle(64,37,6,SSD1306_WHITE); static float a1=0,a2=0;
  display.drawCircle(64,37,18,SSD1306_WHITE); display.fillCircle(64+cos(a1)*18,37+sin(a1)*18,2,SSD1306_WHITE);
  display.drawCircle(64,37,26,SSD1306_WHITE); display.fillCircle(64+cos(a2)*26,37+sin(a2)*26,2,SSD1306_WHITE); a1+=0.05; a2-=0.03;
}

void drawMinimalClock() {
  drawHeader("MINIMAL CLOCK"); int cx=64,cy=37,r=24; display.drawCircle(cx,cy,r,SSD1306_WHITE);
  time_t n=time(nullptr); struct tm* t=localtime(&n);
  float s=(t->tm_sec*6-90)*PI/180, m=(t->tm_min*6-90)*PI/180, h=((t->tm_hour%12)*30+t->tm_min/2.0-90)*PI/180;
  display.drawLine(cx,cy,cx+r*0.9*cos(s),cy+r*0.9*sin(s),SSD1306_WHITE);
  display.drawLine(cx,cy,cx+r*0.7*cos(m),cy+r*0.7*sin(m),SSD1306_WHITE);
  display.drawLine(cx,cy,cx+r*0.5*cos(h),cy+r*0.5*sin(h),SSD1306_WHITE);
}

void drawHeart() {
  drawHeader("LINE ART HEART"); static float sz=12; static bool up=true; if(up){sz+=0.4;if(sz>=20)up=false;}else{sz-=0.4;if(sz<=12)up=true;}
  int cx=64,cy=30; for(int d=0;d<2;d++){int r=sz/2-d;display.drawCircleHelper(cx-sz/2,cy,r,1|2,SSD1306_WHITE);display.drawCircleHelper(cx+sz/2,cy,r,1|2,SSD1306_WHITE);display.drawLine(cx-sz+d,cy,cx,cy+sz+d,SSD1306_WHITE);display.drawLine(cx+sz-d,cy,cx,cy+sz+d,SSD1306_WHITE);}
}

void setup() {
  Serial.begin(74880); Wire.begin(OLED_SDA, OLED_SCL); 
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) for(;;);
  display.clearDisplay(); display.display(); WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  int c=0; while(WiFi.status()!=WL_CONNECTED && c<20){delay(500); c++;}
  configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);
  updateWeather();
}

void loop() {
  unsigned long now = millis();
  if (now - lastWeatherUpdate > 600000 || lastWeatherUpdate == 0) { updateWeather(); lastWeatherUpdate = now; }
  if (now - lastPageChange > 3500) { currentPage = (currentPage + 1) % totalPages; lastPageChange = now; }
  display.clearDisplay(); display.setTextColor(SSD1306_WHITE);
  switch(currentPage) {
    case 0: drawBigAnalogClock(); break;
    case 1: drawBigClock(); break;
    case 2: drawSystemInfo(); break;
    case 3: drawWeatherPage(); break;
    case 4: drawSpectrumVisualizer(); break;
    case 5: drawBarGraph(); break;
    case 6: drawFighterRadar(); break;
    case 7: drawArrowTarget(); break;
    case 8: drawLightsaberDuel(); break;
    case 9: drawSunrise(); break;
    case 10: drawTetris(); break;
    case 11: drawLodeRunner(); break;
    case 12: drawPrinceOfPersia(); break;
    case 13: drawLissajous(); break;
    case 14: drawLorenz(); break;
    case 15: drawSierpinski(); break;
    case 16: drawTorus(); break;
    case 17: drawMoire(); break;
    case 18: drawRotating3D(); break;
    case 19: drawDNA(); break;
    case 20: drawPlexus(); break;
    case 21: drawMatrix(); break;
    case 22: drawWarp(); break;
    case 23: drawWave(); break;
    case 24: drawSpiral(); break;
    case 25: drawOrbit(); break;
    case 26: drawMinimalClock(); break;
    case 27: drawHeart(); break;
  }
  display.display(); delay(10);
}
