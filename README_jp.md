# ATmega88高電圧パラレルヒューズライタ for Arduino

![FuseRescue](https://raw.github.com/wiki/hieromon/ISP_FuseRescue/images/FuseRescue.jpg)

## はじめに

普段Arduinoを「素のまま」の状態で使っていると気づかないのですが、AVRマイコンにはヒューズバイトなるものがあります。ヒューズバイトはチップの動作状態のコンフィグレーションを設定します。  
ATMEL Studioなどを使ってAVRマイコンをネイティブな状態で開発する場合、まれにうっかりヒューズビットを書き換えてしまうことがあります。特にクロックソースやブートローダーのためのリセットベクタにかかわるビットを書き換えてしまうと、ISPで外部と通信できなくなったり、ISPプログラミングができなくなってしまいます。Arduino IDEの場合、スケッチの書き込み処理が次の様なエラーになります。

    avrdude: stk500_recv(): programmer is not responding

このエラーはArduinoのATmega328チップに書き込んであるブートローダーが応答しない時に出ます。原因は色々あるのですが、ATmega328のヒューズバイトがArduinoの回路と整合せず、チップ自体がスタートしない場合でもこのようなエラーになります。こうなるとArduino用のブートローダーを書き込もうにも、チップそのものがロックした状態なので、シリアルプログラミング方式ではチップを元の状態に戻すことはできません。  
しかしチップ自体は破壊されていないので、ヒューズバイトを元に戻しさえすれば、また使えるようになります。これには12Vの電圧を掛けながら、ヒューズバイトを8ビットパラレルで書き込んでやる必要があります。


[ATmega88/168/328のデータシート](http://www.atmel.com/images/doc8161.pdf "ATmega48PA/88PA/168PA/328P - Complete(Revision D, 448 pages)")には高圧パラレルプログラミングに関する仕様が記述されており、その手順に従ってヒューズを書き込んでやればいいです。そのためには何らかの回路や書込用のソフトウェアが必要です。AVRマイコンの高電圧パラレルプログラミングのためのツールは、既に様々な人が実装されていて、[SatE-oさんのAVR-ATmega48,88,168リセッター](http://zampoh.cocolog-nifty.com/blog/2008/09/avr---atmega488.html)や、[ゆきの研究室さんのAVRライタATmega8対応](http://yuki-lab.jp/hw/index.html)などが公開されています。


**ATmega88高電圧パラレルヒューズライタ for Arduino**は、高電圧パラレルヒューズ書込のためのArduinoシールドとスケッチです。

* ATmega88A/88PA/168A/168PA/328/328P 28pin DIP 対応
* Arduinoシールドになっている(別途Arduino Unoが必要)
* PCからシリアル通信を使って対話的に操作する
* [Arduino as ISP](https://www.arduino.cc/en/Tutorial/ArduinoISP "Using an Arduino as an AVR ISP")のライタシールドとしても使える(シールド基板のスライドスイッチでヒューズライタとISPライタを切替)

また、高電圧パラレルヒューズライタとして次の機能があります。

* ヒューズバイトおよび拡張ヒューズバイト読み書き
* Arduinoブートローダー用のヒューズバイト書き込み
* ロックビット読み書き
* チップ消去

## 回路図

![schematic](https://raw.github.com/wiki/hieromon/ISP_FuseRescue/images/ISP_FuseRescue_C.sch.jpg)

回路は単純です。Arduinoの+5V端子からパラレルプログラミングに必要な+12Vを作り出すチャージポンプと、高電圧パラレルプログラマとISPライタを切り替える信号選択用の[2-Chアナログマルチプレクサ](http://www.ti.com/lit/ds/symlink/cd74hc4053.pdf "74HC4053")だけです。  
チャージポンプには[MC34063](http://www.onsemi.com/pub_link/Collateral/MC34063A-D.PDF "ON Semiconductor MC34063")を使っていますが、定数さえ合わせれば[NJM2360](http://www.njr.co.jp/products/semicon/products/NJM2360.html "JRC NJM2360")などのセカンドソースでも使えます。  
アナログマルチプレクサのゲート選択端子は、スライドスイッチの+5V/GND選択端子とヒューズバイトを書き換える対象のAtmega88チップのリセット端子に繋がっています。ゲート選択が+5Vの時に、パラレルプログラミングに必要な全ての信号ラインは、母体のArudinoと対象チップに接続されます。また、スライドスイッチをGND側にすると、ArduinoのISP信号ラインが対象チップに接続されます。

## Arduinoスケッチ

高電圧パラレルプログラマとして使う場合もArduino as ISPライタとして使う場合も一つのスケッチで動作します。Arduinoのスケッチは3つのモジュールから構成されます。  

* ISPFuseRescue : メインスケッチ
* FuseRescue : 高電圧パラレルプログラミングスケッチ
* ArduinoISP : Arduino as ISPスケッチ (Arduino IDEに付属しているExampleスケッチそのもの)

ISPFuseRescueはArduinoのスケッチフォルダへ、またFuseRescureとArduinoISPはArduinoのユーザーlibrariesフォルダへ格納します。

## 使い方

はじめにISPFuseRescureのスケッチをコンパイルしてライタとして使うArudino Unoへ書き込みます。そして高電圧パラレルヒューズライタシールドをArduino Unoに搭載してPCとUSBで接続します。

![Arduino Operation](https://raw.github.com/wiki/hieromon/ISP_FuseRescue/images/FuseRescue_arduino.jpg)

PCでターミナルソフトを起動してArudinoにシリアル接続します。ターミナル画面に下のようなメッセージが表示されて、プログラミング用のコマンドが入力できるようになります。

![Terminal Operation](https://raw.github.com/wiki/hieromon/ISP_FuseRescue/images/FuseRescue_terminal.jpg)

### コマンド

**L** : 下位ヒューズバイト書込  
**H** : 上位ヒューズバイト書込  
**X** : 拡張ヒューズバイト書込  
**K** : ロックビット書込  
**W** : デフォルト(工場出荷値)ヒューズバイト書込  
**A** : Arduino用ヒューズバイト書込  
**E** : チップ消去  
**V** : ヒューズバイト・ロックビット読出し  

### ヒューズバイトとロックビット

#### ATmega88A/168A拡張ヒューズバイト
名称|ビット|意味|既定値
----|------|----|------
- |7～3|(予約)|1
BOOTSZ1|2|ブートローダ容量選択|0
BOOTSZ0|1|ブートローダ容量選択|0
BOOTRST|0|リセットベクタ領域選択|1 (アプリケーション領域)

#### ATmega88A/168A上位ヒューズバイト
名称|ビット|意味|既定値
----|------|----|------
RSTDISBL|7|PC6 RESETピン選択|1 (RESET)
DWEN|6|デバッグWIRE許可|1 (不許可)
SPIEN|5|シリアルプログラミング許可|0 (シリアルプログラミング許可)
WDTON|4|ウォッチドッグタイマ常時有効|1 (WDTCSRで許可)
EESAVE|3|チップ消去動作からEEPROMを保護|1 (保護しない)
BODLEVEL2|2|BODリセット制御|1
BODLEVEL1|1|BODリセット制御|1
BODLEVEL0|0|BODリセット制御|1

#### ATmega328拡張ヒューズバイト
名称|ビット|意味|既定値
----|------|----|------
- |7～3|(予約)|1
BODLEVEL2|2|BODリセット制御|1
BODLEVEL1|1|BODリセット制御|1
BODLEVEL0|0|BODリセット制御|1

#### ATmega328上位ヒューズバイト
名称|ビット|意味|既定値
----|------|----|------
RSTDISBL|7|PC6 RESETピン選択|1 (RESET)
DWEN|6|デバッグWIRE許可|1 (不許可)
SPIEN|5|シリアルプログラミング許可|0 (シリアルプログラミング許可)
WDTON|4|ウォッチドッグタイマ常時有効|1 (WDTCSRで許可)
EESAVE|3|チップ消去動作からEEPROMを保護|1 (保護しない)
BOOTSZ1|2|ブートローダ容量|0
BOOTSZ0|1|ブートローダ容量|0
BOOTRST|0|リセットベクタ領域選択|1 (アプリケーション領域)

#### 下位ヒューズバイト
名称|ビット|意味|既定値
----|------|----|------
CKDIV8|7|システムクロック8分周選択|0 (8分周)
CKOUT|6|システムクロック出力|1 (出力しない)
SUT1|5|起動遅延時間|1
SUT2|4|起動遅延時間|0
CKSEL3|3|クロック選択|0
CKSEL2|2|クロック選択|0
CKSEL1|1|クロック選択|1
CKSEL0|0|クロック選択|0

#### ロックビット
名称|ビット|意味|既定値|
----|------|----|------|
- |7～6|(予約)|1|
BLB12|5|ブートローダ領域保護|1|
BLB11|4|ブートローダ領域保護|1|
BLB02|3|アプリケーション領域保護|1|
BLB01|2|アプリケーション領域保護|1|
LB2|1|フラッシュ・EEPROM保護|1|
LB1|0|フラッシュ・EEPROM保護|1|

#### Arduinoのヒューズバイト
ATmega328などをArduinoとして動かす場合は、ヒューズバイトでクロックソースとブートローダを指定する必要があります。その値は`<Arduinoシステムフォルダ>\hardware\arduino\avr\boards.txt`ファイルに記述されています。※Arduino IDE 1.6.0の場合のパス

````
uno.bootloader.low_fuses=0xFF
uno.bootloader.high_fuses=0xDE
uno.bootloader.extended_fuses=0x05
uno.bootloader.unlock_bits=0x3F
uno.bootloader.lock_bits=0x0F
````

### 使うときの注意

高電圧パラレルヒューズライタとPCをUSBで接続したままISPライタとの切替スイッチを操作してはいけません。ショートします。高電圧パラレルヒューズライタとISPライタの切替操作は、必ず電源が供給されていない状態で行ってください。

## 資料

#### Eagle CADデータ(回路図とPCBデザイン)

参考までに、今回実装したPCBのEagleファイルをアップしておきます。これは表面実装部品で構成したPCBです。

[Eagel CADファイル](https://github.com/Hieromon/ISP_FuseRescue/blob/master/ISP_FuseRescue_C.zip "Eagle CADファイル") (zip)

![PCB\_LAYOUT](https://raw.github.com/wiki/hieromon/ISP_FuseRescue/images/FuseRescue_pcblayout.jpg)
![PCB](https://raw.github.com/wiki/hieromon/ISP_FuseRescue/images/FuseRescue_pcb.jpg)

#### 部品表

Part|Value|Device|Package
----|-----|------|-------
C1|330μ|電解コンデンサ|Φ6.3mm x 7.7mm
C2|100μ|電解コンデンサ|Φ8.0mm x 10.2mm
C3|680pF|積層セラミックコンデンサ|2012 SMD
C4|0.1μF|積層セラミックコンデンサ|2012 SMD
C5|22pF|積層セラミックコンデンサ|1608 SMD
C6|22pF|積層セラミックコンデンサ|1608 SMD
C7|0.1μF|積層セラミックコンデンサ|2012 SMD
C8|0.1μF|積層セラミックコンデンサ|2012 SMD
D1|RSX101VA-30TR|ショットキーバリアダイオード|TUMD2
PGM|緑|LED|2012 SMD
PWR|緑|LED|2012 SMD
ERR|赤|LED|2012 SMD
HB|黄|LED|2012 SMD
RX|黄|LED|2012 SMD
TX|黄|LED|2012 SMD
IC1|74HC4053|3回路2-Chアナログマルチプレクサ|TSSOP-16
IC2|74HC4053|3回路2-Chアナログマルチプレクサ|TSSOP-16
L1|100μH|インダクタ|3225 SMD
Q1|SI6544DQ|N/P-Ch複合パワーMOSFET|TSSOP-8
Q2|16MHz|クリスタル発振子|HS-49/S
R1|1Ω|抵抗|2012 SMD
R2|1Ω|抵抗|2012 SMD
R3|200Ω|抵抗|2012 SMD
R4|1.1KΩ|抵抗|2012 SMD
R5|100KΩ|抵抗|2012 SMD
R6|130kΩ|抵抗|2012 SMD
R7|3.3KΩ|抵抗|2012 SMD
R8|10KΩ|抵抗|2012 SMD
R9|15KΩ|抵抗|2012 SMD
R10|10KΩ|抵抗|2012 SMD
R11|1KΩ|抵抗|2012 SMD
R12|1KΩ|抵抗|2012 SMD
R13|1KΩ|抵抗|2012 SMD
R14|1KΩ|抵抗|2012 SMD
R15|1MΩ|抵抗|2012 SMD
R16|1KΩ|抵抗|2012 SMD
R17|1KΩ|抵抗|2012 SMD
R18|1KΩ|抵抗|2012 SMD
R19|10KΩ|抵抗|2012 SMD
R20|10KΩ|抵抗|2012 SMD
R21|10KΩ|抵抗|2012 SMD
S1|SS-22SDP2|2回路スライドスイッチ|
S2|RESET|タクトスイッチ|
T1|2SC4116|NPNトランジスタ|SOT-23(SC-70)
T2|2SC4116|NPNトランジスタ|SOT-23(SC-70)
U2|MC34063ADG|DC-DCコンバータ|SIOC-8
U3|ICソケット|28ピン|300MIL DIP
