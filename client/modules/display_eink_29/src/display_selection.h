// Display Library example for SPI e-paper panels from Dalian Good Display and boards from Waveshare.
// Requires HW SPI and Adafruit_GFX. Caution: the e-paper panels require 3.3V supply AND data lines!
//
// Display Library based on Demo Example from Good Display: http://www.e-paper-display.com/download_list/downloadcategoryid=34&isMode=false.html
//
// Author: Jean-Marc Zingg
//
// Version: see library.properties
//
// Library: https://github.com/ZinggJM/GxEPD2

// Supporting Arduino Forum Topics:
// Waveshare e-paper displays with SPI: http://forum.arduino.cc/index.php?topic=487007.0
// Good Display ePaper for Arduino: https://forum.arduino.cc/index.php?topic=436411.0


#if defined(ESP32)
// select one and adapt to your mapping, can use full buffer size (full HEIGHT)
//GxEPD2_BW<GxEPD2_290_T94_V2, GxEPD2_290_T94_V2::HEIGHT> display(GxEPD2_290_T94_V2(/*CS=5*/ SS, /*DC=*/ 17, /*RST=*/ 16, /*BUSY=*/ 4)); // GDEM029T94 128x296, SSD1680, Waveshare 2.9" V2 variant
//GxEPD2_BW<GxEPD2_290_BS, GxEPD2_290_BS::HEIGHT> display(GxEPD2_290_BS(/*CS=5*/ SS, /*DC=*/ 17, /*RST=*/ 16, /*BUSY=*/ 4)); // DEPG0290BS 128x296, SSD1680
//GxEPD2_BW<GxEPD2_290_M06, GxEPD2_290_M06::HEIGHT> display(GxEPD2_290_M06(/*CS=5*/ SS, /*DC=*/ 17, /*RST=*/ 16, /*BUSY=*/ 4)); // GDEW029M06  128x296, UC8151D
//GxEPD2_BW<GxEPD2_260, GxEPD2_260::HEIGHT> display(GxEPD2_260(/*CS=5*/ SS, /*DC=*/ 17, /*RST=*/ 16, /*BUSY=*/ 4)); // GDEW026T0 152x296, UC8151 (IL0373)
//GxEPD2_BW<GxEPD2_260_M01, GxEPD2_260_M01::HEIGHT> display(GxEPD2_260_M01(/*CS=5*/ SS, /*DC=*/ 17, /*RST=*/ 16, /*BUSY=*/ 4)); // GDEW026M01 152x296, UC8151 (IL0373)
//GxEPD2_BW<GxEPD2_266_BN, GxEPD2_266_BN::HEIGHT> display(GxEPD2_266_BN(/*CS=5*/ SS, /*DC=*/ 17, /*RST=*/ 16, /*BUSY=*/ 4)); // DEPG0266BN 152x296, SSD1680, TTGO T5 V2.4.1
//GxEPD2_BW<GxEPD2_270, GxEPD2_270::HEIGHT> display(GxEPD2_270(/*CS=5*/ SS, /*DC=*/ 17, /*RST=*/ 16, /*BUSY=*/ 4)); // GDEW027W3 176x264, EK79652 (IL91874)
//GxEPD2_BW<GxEPD2_270_T91, GxEPD2_270_T91::HEIGHT> display(GxEPD2_270_T91(/*CS=5*/ SS, /*DC=*/ 17, /*RST=*/ 16, /*BUSY=*/ 4)); // GDEY027T91 176x264, SSD1680
//GxEPD2_BW<GxEPD2_371, GxEPD2_371::HEIGHT> display(GxEPD2_371(/*CS=5*/ SS, /*DC=*/ 17, /*RST=*/ 16, /*BUSY=*/ 4)); // GDEW0371W7 240x416, UC8171 (IL0324)
//GxEPD2_BW<GxEPD2_370_TC1, GxEPD2_370_TC1::HEIGHT> display(GxEPD2_370_TC1(/*CS=5*/ SS, /*DC=*/ 17, /*RST=*/ 16, /*BUSY=*/ 4)); // ED037TC1 280x480, SSD1677, Waveshare 3.7"
//GxEPD2_BW<GxEPD2_420, GxEPD2_420::HEIGHT> display(GxEPD2_420(/*CS=5*/ SS, /*DC=*/ 17, /*RST=*/ 16, /*BUSY=*/ 4)); // GDEW042T2 400x300, UC8176 (IL0398)
//GxEPD2_BW<GxEPD2_420_M01, GxEPD2_420_M01::HEIGHT> display(GxEPD2_420_M01(/*CS=5*/ SS, /*DC=*/ 17, /*RST=*/ 16, /*BUSY=*/ 4)); // GDEW042M01 400x300, UC8176 (IL0398)
//GxEPD2_BW<GxEPD2_583, GxEPD2_583::HEIGHT> display(GxEPD2_583(/*CS=5*/ SS, /*DC=*/ 17, /*RST=*/ 16, /*BUSY=*/ 4)); // GDEW0583T7 600x448, UC8179 (IL0371)
//GxEPD2_BW<GxEPD2_583_T8, GxEPD2_583_T8::HEIGHT> display(GxEPD2_583_T8(/*CS=5*/ SS, /*DC=*/ 17, /*RST=*/ 16, /*BUSY=*/ 4)); // GDEW0583T8 648x480, GD7965
//GxEPD2_BW<GxEPD2_750, GxEPD2_750::HEIGHT> display(GxEPD2_750(/*CS=5*/ SS, /*DC=*/ 17, /*RST=*/ 16, /*BUSY=*/ 4)); // GDEW075T8 640x384, UC8179 (IL0371)
//GxEPD2_BW<GxEPD2_750_T7, GxEPD2_750_T7::HEIGHT> display(GxEPD2_750_T7(/*CS=5*/ SS, /*DC=*/ 17, /*RST=*/ 16, /*BUSY=*/ 4)); // GDEW075T7 800x480, GD7965
//GxEPD2_BW < GxEPD2_1160_T91, GxEPD2_1160_T91::HEIGHT / 2> display(GxEPD2_1160_T91(/*CS=5*/ SS, /*DC=*/ 17, /*RST=*/ 16, /*BUSY=*/ 4)); // GDEH116T91 960x640, SSD1677
// 3-color e-papers
//GxEPD2_3C<GxEPD2_154c, GxEPD2_154c::HEIGHT> display(GxEPD2_154c(/*CS=5*/ SS, /*DC=*/ 17, /*RST=*/ 16, /*BUSY=*/ 4)); // GDEW0154Z04 200x200, IL0376F, no longer available
//GxEPD2_3C<GxEPD2_154_Z90c, GxEPD2_154_Z90c::HEIGHT> display(GxEPD2_154_Z90c(/*CS=5*/ SS, /*DC=*/ 17, /*RST=*/ 16, /*BUSY=*/ 4)); // GDEH0154Z90 200x200, SSD1681
//GxEPD2_3C<GxEPD2_213c, GxEPD2_213c::HEIGHT> display(GxEPD2_213c(/*CS=5*/ SS, /*DC=*/ 17, /*RST=*/ 16, /*BUSY=*/ 4)); // GDEW0213Z16 104x212, UC8151 (IL0373)
//GxEPD2_3C<GxEPD2_213_Z19c, GxEPD2_213_Z19c::HEIGHT> display(GxEPD2_213_Z19c(/*CS=5*/ SS, /*DC=*/ 17, /*RST=*/ 16, /*BUSY=*/ 4)); // GDEH0213Z19 104x212, UC8151D
//GxEPD2_3C<GxEPD2_213_Z98c, GxEPD2_213_Z98c::HEIGHT> display(GxEPD2_213_Z98c(/*CS=5*/ SS, /*DC=*/ 17, /*RST=*/ 16, /*BUSY=*/ 4)); // GDEY0213Z98 122x250, SSD1680
//GxEPD2_3C<GxEPD2_290c, GxEPD2_290c::HEIGHT> display(GxEPD2_290c(/*CS=5*/ SS, /*DC=*/ 17, /*RST=*/ 16, /*BUSY=*/ 4)); // GDEW029Z10 128x296, UC8151 (IL0373)
//GxEPD2_3C<GxEPD2_290_Z13c, GxEPD2_290_Z13c::HEIGHT> display(GxEPD2_290_Z13c(/*CS=5*/ SS, /*DC=*/ 17, /*RST=*/ 16, /*BUSY=*/ 4)); // GDEH029Z13 128x296, UC8151D
//GxEPD2_3C<GxEPD2_290_C90c, GxEPD2_290_C90c::HEIGHT> display(GxEPD2_290_C90c(/*CS=5*/ SS, /*DC=*/ 17, /*RST=*/ 16, /*BUSY=*/ 4)); // GDEM029C90 128x296, SSD1680
//GxEPD2_3C<GxEPD2_266c, GxEPD2_266c::HEIGHT> display(GxEPD2_266c(/*CS=5*/ SS, /*DC=*/ 17, /*RST=*/ 16, /*BUSY=*/ 4)); // GDEY0266Z90 152x296, SSD1680
//GxEPD2_3C<GxEPD2_270c, GxEPD2_270c::HEIGHT> display(GxEPD2_270c(/*CS=5*/ SS, /*DC=*/ 17, /*RST=*/ 16, /*BUSY=*/ 4)); // GDEW027C44 176x264, IL91874
//GxEPD2_3C<GxEPD2_420c, GxEPD2_420c::HEIGHT> display(GxEPD2_420c(/*CS=5*/ SS, /*DC=*/ 17, /*RST=*/ 16, /*BUSY=*/ 4)); // GDEW042Z15 400x300, UC8176 (IL0398)
//GxEPD2_3C<GxEPD2_420c_Z21, GxEPD2_420c_Z21::HEIGHT> display(GxEPD2_420c_Z21(/*CS=5*/ SS, /*DC=*/ 17, /*RST=*/ 16, /*BUSY=*/ 4)); // GDEQ042Z21 400x300, UC8276
//GxEPD2_3C<GxEPD2_583c, GxEPD2_583c::HEIGHT> display(GxEPD2_583c(/*CS=5*/ SS, /*DC=*/ 17, /*RST=*/ 16, /*BUSY=*/ 4)); // GDEW0583Z21 600x448, UC8179 (IL0371)
//GxEPD2_3C<GxEPD2_583c_Z83, GxEPD2_583c_Z83::HEIGHT> display(GxEPD2_583c_Z83(/*CS=5*/ SS, /*DC=*/ 17, /*RST=*/ 16, /*BUSY=*/ 4)); // GDEW0583Z83 648x480, GD7965
//GxEPD2_3C<GxEPD2_750c, GxEPD2_750c::HEIGHT> display(GxEPD2_750c(/*CS=5*/ SS, /*DC=*/ 17, /*RST=*/ 16, /*BUSY=*/ 4)); // GDEW075Z09 640x384, UC8179 (IL0371)
//GxEPD2_3C<GxEPD2_750c_Z08, GxEPD2_750c_Z08::HEIGHT> display(GxEPD2_750c_Z08(/*CS=5*/ SS, /*DC=*/ 17, /*RST=*/ 16, /*BUSY=*/ 4)); // GDEW075Z08 800x480, GD7965
//GxEPD2_3C<GxEPD2_750c_Z90, GxEPD2_750c_Z90::HEIGHT / 2> display(GxEPD2_750c_Z90(/*CS=5*/ SS, /*DC=*/ 17, /*RST=*/ 16, /*BUSY=*/ 4)); // GDEH075Z90 880x528, SSD1677
// 7-color e-paper
//GxEPD2_3C < GxEPD2_565c, GxEPD2_565c::HEIGHT / 2 > display(GxEPD2_565c(/*CS=5*/ SS, /*DC=*/ 17, /*RST=*/ 16, /*BUSY=*/ 4)); // Waveshare 5.65" 7-color (3C graphics)
//GxEPD2_7C < GxEPD2_565c, GxEPD2_565c::HEIGHT / 2 > display(GxEPD2_565c(/*CS=5*/ SS, /*DC=*/ 17, /*RST=*/ 16, /*BUSY=*/ 4)); // Waveshare 5.65" 7-color

// grey levels parallel IF e-papers on Waveshare e-Paper IT8951 Driver HAT
// HRDY -> 4, RST -> 16, CS -> SS(5), SCK -> SCK(18), MOSI -> MOSI(23), MISO -> MISO(19), GND -> GND, 5V -> 5V
// note: 5V supply needs to be exact and strong; 5V over diode from USB (e.g. Wemos D1 mini) doesn't work!
//GxEPD2_BW<GxEPD2_it60, GxEPD2_it60::HEIGHT> display(GxEPD2_it60(/*CS=5*/ SS, /*DC=*/ 17, /*RST=*/ 16, /*BUSY=*/ 4));
//GxEPD2_BW < GxEPD2_it60_1448x1072, GxEPD2_it60_1448x1072::HEIGHT / 4 > display(GxEPD2_it60_1448x1072(/*CS=5*/ SS, /*DC=*/ 17, /*RST=*/ 16, /*BUSY=*/ 4));
//GxEPD2_BW < GxEPD2_it78_1872x1404, GxEPD2_it78_1872x1404::HEIGHT / 8 > display(GxEPD2_it78_1872x1404(/*CS=5*/ SS, /*DC=*/ 17, /*RST=*/ 16, /*BUSY=*/ 4));
//GxEPD2_BW < GxEPD2_it103_1872x1404, GxEPD2_it103_1872x1404::HEIGHT / 8 > display(GxEPD2_it103_1872x1404(/*CS=5*/ SS, /*DC=*/ 17, /*RST=*/ 16, /*BUSY=*/ 4));

// Waveshare 12.48 b/w SPI display board and frame or Good Display 12.48 b/w panel GDEW1248T3
// general constructor for use with all parameters, e.g. for Waveshare ESP32 driver board mounted on connection board
//GxEPD2_BW < GxEPD2_1248, GxEPD2_1248::HEIGHT / 4 >
//display(GxEPD2_1248(/*sck=*/ 13, /*miso=*/ 12, /*mosi=*/ 14, /*cs_m1=*/ 23, /*cs_s1=*/ 22, /*cs_m2=*/ 16, /*cs_s2=*/ 19,
//                             /*dc1=*/ 25, /*dc2=*/ 17, /*rst1=*/ 33, /*rst2=*/ 5, /*busy_m1=*/ 32, /*busy_s1=*/ 26, /*busy_m2=*/ 18, /*busy_s2=*/ 4));
// Waveshare 12.48 b/w/r SPI display board and frame or Good Display 12.48 b/w/r panel GDEY1248Z51
// general constructor for use with all parameters, e.g. for Waveshare ESP32 driver board mounted on connection board
//GxEPD2_3C < GxEPD2_1248c, GxEPD2_1248c::HEIGHT / 4 >
//display(GxEPD2_1248c(/*sck=*/ 13, /*miso=*/ 12, /*mosi=*/ 14, /*cs_m1=*/ 23, /*cs_s1=*/ 22, /*cs_m2=*/ 16, /*cs_s2=*/ 19,
//                             /*dc1=*/ 25, /*dc2=*/ 17, /*rst1=*/ 33, /*rst2=*/ 5, /*busy_m1=*/ 32, /*busy_s1=*/ 26, /*busy_m2=*/ 18, /*busy_s2=*/ 4));
#endif

