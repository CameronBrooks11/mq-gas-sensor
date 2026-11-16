<!-- PROJECT SHIELDS -->
<!--
*** Markdown "reference style" links are used for readability.
*** Links are enclosed in brackets [ ] instead of parentheses ( ).
*** See the bottom for reference variables declarations (check-url, release-url, etc.).
*** For more information, visit: https://www.markdownguide.org/basic-syntax/#reference-style-links
-->

[![Check][check-shield]][check-url]
[![Release][release-shield]][release-url]
[![MIT License][license-shield]][license-url]

# MQGasSensor

**Documentation: <https://cameronbrooks11.github.io/mq-gas-sensor/>**

A robust Arduino library for interfacing with MQ-series gas sensors, enabling accurate gas concentration measurements with ease. Supports calibration, resistance calculations, and real-time gas concentration derivation for various gases.

**Status:** version 2.0.0, which replaced the 1.x API; the [release notes](https://github.com/CameronBrooks11/mq-gas-sensor/releases/tag/v2.0.0) map each old call to its replacement. Every example is compiled for its board and the unit tests run on a simulated Arduino Mega on each change. This version has not yet been validated against physical sensors. The library began as a fork of [MQSensorsLib](https://github.com/miguel5612/MQSensorsLib) and is now developed independently; see [Acknowledgment](#acknowledgment).

## Features

- Preheat and baseline calibration for MQ sensors.
- Calculate sensor resistance (Rs) and baseline resistance (R0).
- Support for multiple MQ-series sensors (e.g., MQ-2, MQ-7, MQ-135).
- Calibration coefficients traced to their datasheets: [every `a` and `b`](docs/coefficients.md) is listed with the datasheet curve it was fitted from, and can be [re-derived](docs/methods.md) with one command.

<!-- TABLE OF CONTENTS -->

## Table of Contents

- [MQGasSensor](#mqgassensor)
  - [Features](#features)
  - [Table of Contents](#table-of-contents)
  - [Getting Started](#getting-started)
  - [Library Usage](#library-usage)
    - [Supported Hardware](#supported-hardware)
    - [Serial Debugging](#serial-debugging)
  - [Hardware Setup](#hardware-setup)
    - [Sensor Considerations](#sensor-considerations)
    - [Wiring Hookup](#wiring-hookup)
      - [MQ-7 / MQ-309A](#mq-7--mq-309a)
      - [ESP Wiring](#esp-wiring)
      - [ESP32 WROOM 32D](#esp32-wroom-32d)
  - [Sensor Manufacturers](#sensor-manufacturers)
  - [Contributing](#contributing)
  - [License](#license)
  - [Acknowledgment](#acknowledgment)
    - [Attribution](#attribution)

## Getting Started

The values passed to `setCurve(a, b)` come from the sensor's datasheet. [docs/coefficients.md](docs/coefficients.md) lists them per sensor and gas, and [docs/methods.md](docs/methods.md) explains how they are derived.

```c++
#include <MQGasSensor.h>

#define RatioMQ4CleanAir (4.43) // Rs/R0 in clean air, from the datasheet's AIR line

MQGasSensor MQ4(A4, 5.0, 10, "MQ-4"); // analog pin, ADC reference (V), ADC bits, name

void setup() {
  Serial.begin(9600);
  MQ4.begin();
  MQ4.setCurve(990.22, -2.826); // CH4, from the MQ-4 datasheet fit in docs/coefficients.md
  MQ4.update();                    // in clean air, after the heater has warmed up
  MQ4.calibrate(RatioMQ4CleanAir); // stores R0 = Rs / 4.43; or setR0() with a saved value
}

void loop() {
  if (MQ4.update()) {             // false when the pin reads 0 V: check the wiring
    Serial.println(MQ4.readPpm()); // ppm of CH4
  }
  delay(500);
}
```

Version 2.0 changed every call above; the [2.0.0 release notes](https://github.com/CameronBrooks11/mq-gas-sensor/releases/tag/v2.0.0) map each 1.2 call to its replacement.

## Library Usage

Once you've completed your [hardware setup](#hardware-setup), start by cloning this repository to your computer:

```
git clone https://github.com/CameronBrooks11/mq-gas-sensor
```

under your Arduino libraries folder or download the '.zip'.

### Supported Hardware

This library is compatible with various development boards, including **Arduino**, **Genuino**, **ESP8266**, and **ESP32**, facilitating integration into diverse projects. It supports the following MQ-series gas sensors:

- **MQ-2**: Detects LPG, i-butane, propane, methane, alcohol, hydrogen, and smoke.
- **MQ-3**: Sensitive to alcohol vapor; commonly used in breathalyzers.
- **MQ-4**: High sensitivity to methane; suitable for natural gas detection.
- **MQ-5**: Detects natural gas and LPG; suitable for gas leakage detection.
- **MQ-6**: Sensitive to LPG and butane gas; used for gas leak detection.
- **MQ-7**: High sensitivity to carbon monoxide; used in CO detectors.
- **MQ-8**: Detects hydrogen gas; suitable for hydrogen leak detection.
- **MQ-9**: Sensitive to carbon monoxide and combustible gases; used in gas detecting equipment.
- **MQ-131**: High sensitivity to ozone; also responsive to chlorine and nitrogen dioxide.
- **MQ-135**: Detects air quality by sensing NH₃, NOₓ, alcohol, benzene, smoke, and CO₂.
- **MQ-136**: Sensitive to hydrogen sulfide (H₂S); used for detecting H₂S gas.
- **MQ-137**: High sensitivity to ammonia (NH₃); suitable for ammonia gas detection.
- **MQ-303A**: Detects alcohol (ethanol), hydrogen and iso-butane; set `setSupplyOffset(0.45)` for it.
- **MQ-309A**: Detects carbon monoxide, hydrogen, methane and alcohol; its heater alternates between two voltages, like the MQ-7.

This comprehensive support allows for the development of applications such as air quality monitors, gas leak detectors, and other sensor-based systems.

### Serial Debugging

After `readPpm()`, `printDebug()` prints one line to `Serial`, or to any `Print` you pass it, with the name, the measured voltage, Rs, R0, the ratio, the concentration and the curve:

```c++
MQ2.update();
MQ2.readPpm();
MQ2.printDebug(); // MQ-2: V=..., Rs=... kOhm, R0=... kOhm, ratio=..., ppm=... (a*ratio^b, a=..., b=...)
```

## Hardware Setup

### Sensor Considerations

**Important points:**

Sensor pins:

- VCC -> 5V Power supply (+) wire
- GND -> GND Ground (-) wire
- AO -> Analog Output of the sensor

![Wiring_MQSensor](docs/img/Points_explanation.jpeg)

Sensor characteristics:

- Find your RL Value in KOhms
  - From datasheet of your sensor, or your module's schematic; set it with `setLoadResistance(<kOhm>)` if it is not 10
- If the sensor uses a different supply voltage than the ADC reference, call `yourSensor.setSupplyVoltage(<voltage>);`
  - `setAdcReference()` sets the ADC reference only; the RS calculation uses the supply voltage
- Find your RS/R0 (Clean air - English)
  - Note: RS/R0 is equal to Ratio variable on the program
  - From datasheet of your sensor

![Graph from datasheet](docs/img/Graph_Explanation.jpeg)

![Arduino_Wiring_MQSensor](docs/img/MQ_Arduino.PNG)

### Wiring Hookup

#### MQ-7 / MQ-309A

**Note**: MQ-7 and MQ-309 needs two different voltages for heater, they can be supplied by PWM and DC Signal controlled by your controller, another option is to use two different power sources, you should use the best option for you, next i will show the PWM option and on the examples this will be the way.

![MQ-7_MQ-309](docs/img/MQ-309_MQ-7.PNG)

#### ESP Wiring

![ESP8266_Wiring_MQSensor](docs/img/MQ_ESP8266.PNG)

#### ESP32 WROOM 32D

The ESP32 WROOM 32D does not need an external power supply. A0 goes to PIN36, Vcc to 3v3 and GND to any GND port on the board. Check the **examples/ESP32/ESP32_WROOM_32D** folder for a fix to the measuring issue when connecting to wifi.

## Sensor Manufacturers

These links are for reading. The exact files the coefficients were derived from, with checksums and archived mirrors, are in [`datasheets/manifest.toml`](datasheets/manifest.toml); `just datasheets` downloads them. See [datasheets/README.md](datasheets/README.md).

| Sensor  | Manufacture        | URL Datasheet                                                                                                   |
| ------- | ------------------ | --------------------------------------------------------------------------------------------------------------- |
| MQ-2    | HANWEI Electronics | [datasheet](https://www.pololu.com/file/0J309/MQ2.pdf)                                                          |
| MQ-3    | HANWEI Electronics | [datasheet](https://www.sparkfun.com/datasheets/Sensors/MQ-3.pdf)                                               |
| MQ-4    | HANWEI Electronics | [datasheet](https://www.sparkfun.com/datasheets/Sensors/Biometric/MQ-4.pdf)                                     |
| MQ-5    | HANWEI Electronics | [datasheet](https://web.archive.org/web/20131102061827/http://www.parallax.com/sites/default/files/downloads/605-00009-MQ-5-Datasheet.pdf) |
| MQ-6    | HANWEI Electronics | [datasheet](https://web.archive.org/web/20120113122419/http://www.sparkfun.com/datasheets/Sensors/Biometric/MQ-6.pdf) |
| MQ-7    | HANWEI Electronics | [datasheet](https://www.sparkfun.com/datasheets/Sensors/Biometric/MQ-7.pdf)                                     |
| MQ-8    | HANWEI Electronics | [datasheet](https://web.archive.org/web/20130531234925/http://dlnmh9ip6v2uc.cloudfront.net/datasheets/Sensors/Biometric/MQ-8.pdf) |
| MQ-9    | HANWEI Electronics | [datasheet](http://www.haoyuelectronics.com/Attachment/MQ-9/MQ9.pdf)                                            |
| MQ-131  | HANWEI Electronics | [datasheet](http://www.sensorsportal.com/DOWNLOADS/MQ131.pdf)                                                   |
| MQ-135  | HANWEI Electronics | [datasheet](https://www.electronicoscaldas.com/datasheet/MQ-135_Hanwei.pdf)                                     |
| MQ-136  | HANWEI Electronics | [datasheet](https://github.com/miguel5612/MQSensorsLib_Docs/blob/master/Datasheets/MQ136%20-%20Hanwei.pdf)     |
| MQ-303A | HANWEI Electronics | [datasheet](http://www.kosmodrom.com.ua/pdf/MQ303A.pdf)                                                         |
| MQ-309A | HANWEI Electronics | [datasheet](http://www.sensorica.ru/pdf/MQ-309A.pdf)                                                            |

## Contributing

Please read [CONTRIBUTING.md](https://github.com/CameronBrooks11/mq-gas-sensor/blob/main/CONTRIBUTING.md) for details on the code of conduct, and the process for submitting pull requests.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details

## Acknowledgment

This library began as a fork of the original **MQSensorsLib** library, created and maintained by Miguel Angel Califa Urquiza, Ghiordy Contreras Contreras, and Yerson Ramiro Carrillo Amado. Their work laid the foundation for interfacing MQ-series gas sensors with Arduino, and we acknowledge their significant contributions to the community.

Original repository: [miguel5612/MQSensorsLib](https://github.com/miguel5612/MQSensorsLib)  
Last synced with the original at commit [`59a2e58`](https://github.com/miguel5612/MQSensorsLib/commit/59a2e5852ec12f5da3cf508be68f5a9405314003) (2025-06-04, its version 3.0.5). This project is developed independently from that point.  
Cite their work as:

- Califa Urquiza, Miguel Angel, Contreras Contreras, Ghiordy, & Carrillo Amado, Yerson Ramiro. (2019, September 3). miguel5612/MQSensorsLib: Arduino Preview V1.03 (Version 1.0.3). Zenodo. [http://doi.org/10.5281/zenodo.3384301](http://doi.org/10.5281/zenodo.3384301)

### Attribution

- **Authors**
  - _Miguel A. Califa U._ - [_GitHub_](https://github.com/miguel5612) - [CV](https://scienti.minciencias.gov.co/cvlac/visualizador/generarCurriculoCv.do?cod_rh=0000050477)
  - _Ghiordy F. Contreras C._ - [_GitHub_](https://github.com/Ghiordy) - [CV](https://scienti.minciencias.gov.co/cvlac/visualizador/generarCurriculoCv.do?cod_rh=0000050476)
  - _Yersson R. Carrillo A._ - [_GitHub_](https://github.com/Yercar18/Dronefenix) - [CV](https://scienti.minciencias.gov.co/cvlac/visualizador/generarCurriculoCv.do?cod_rh=0001637655)
- **Contributors**
  - _Andres A. Martinez._ - [_Github_](https://github.com/andresmacsi) - [CV](https://www.linkedin.com/in/andr%C3%A9s-acevedo-mart%C3%ADnez-73ab35185/?originalSubdomain=co)
  - _Juan A. Rodríguez._
  - _Mario A. Rodríguez O._ - [_GitHub_](https://github.com/MarioAndresR) - [CV](https://scienti.minciencias.gov.co/cvlac/visualizador/generarCurriculoCv.do?cod_rh=0000111304)
- **Reviewers**
- _PhD. Jacipt A Ramón V._ - [CV](https://scienti.minciencias.gov.co/cvlac/visualizador/generarCurriculoCv.do?cod_rh=0000512702)

See also the list of [contributors](https://github.com/miguel5612/MQSensorsLib/graphs/contributors) who participated in the original project.

<!-- MARKDOWN LINKS & IMAGES -->
<!-- https://www.markdownguide.org/basic-syntax/#reference-style-links -->

[check-shield]: https://img.shields.io/github/actions/workflow/status/CameronBrooks11/mq-gas-sensor/check.yml?branch=main&label=build%20%26%20tests
[check-url]: https://github.com/CameronBrooks11/mq-gas-sensor/actions/workflows/check.yml
[release-shield]: https://img.shields.io/github/v/release/CameronBrooks11/mq-gas-sensor
[release-url]: https://github.com/CameronBrooks11/mq-gas-sensor/releases/latest
[license-shield]: https://img.shields.io/github/license/CameronBrooks11/mq-gas-sensor
[license-url]: https://github.com/CameronBrooks11/mq-gas-sensor/blob/main/LICENSE
