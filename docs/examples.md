# Examples

Each sketch is in [`examples/`](../examples), and appears in the Arduino IDE under **File → Examples → MQGasSensor**. CI compiles every one on each change: for an Arduino Mega, except the ESP32 and ESP8266 sketches, which build for their own boards.

Most single-sensor sketches follow the same pattern: set a curve, average ten calibration readings in `setup()`, then read and print with `printDebug()` twice a second.

## One sensor, one gas

- `MQ-2`: an MQ-2 reading LPG.
- `MQ-3`: an MQ-3 reading alcohol, in mg/L.
- `MQ-4`: an MQ-4 reading methane.
- `MQ-5`: an MQ-5 reading hydrogen.
- `MQ-6`: an MQ-6 reading methane.
- `MQ-7`: an MQ-7 reading CO through its heater cycle, switched by a MOSFET on pin 5.
- `MQ-8`: an MQ-8 reading hydrogen.
- `MQ-9`: an MQ-9 reading LPG, with the heater control pins defined.
- `MQ-131`: an MQ-131 reading ozone, with `setRatio(MQRatio::R0OverRs)`.
- `MQ-135`: an MQ-135 reading NH4.
- `MQ-136`: an MQ-136 reading hydrogen sulphide.
- `MQ-303A`: an MQ-303A reading iso-butane, with `setSupplyOffset(0.45)`.
- `MQ-309A`: an MQ-309A reading CO through its heater cycle.

## Several gases from one sensor

- `MQ-4-ALL`: an MQ-4 switching `setCurve` between five gases each loop and printing them as a table.
- `MQ-9-ALL`: an MQ-9 reading LPG, methane and CO.
- `MQ-135-ALL`: an MQ-135 reading CO, alcohol, CO2, toluene, NH4 and acetone.
- `LinearVsExponential`: one MQ-4 reading through a log-linear curve (`MQModel::LogLinear`) and a power law side by side.
- `MQ-4-LINEAR`: an MQ-4 read through a log-linear curve.

## Applications

- `Alcoholimeter`: an MQ-3 breath alcohol reading in mg/L.
- `smokeDetector`: an MQ-4 on its smoke curve, with a warning above 1000 ppm.
- `MQ-Board`: eight sensors, MQ-2 to MQ-9, read by one Mega.
- `MQ-135-CorrectionFactorDHT11`: an MQ-135 corrected for temperature and humidity from a DHT11, through `setCorrection`.

## Calibration and testing

- `Calibration`: prints a fresh R0 for an MQ-2 every 400 ms, to watch it settle in clean air.
- `AlgorithmTester`: compares `ppmFromRatio` against points read off several datasheets, and prints the error.
- `UnitTesting`: the library's unit tests, run on a simulated Mega by `just test`.

## Other hardware

- `ESP32`: an MQ-3 reading benzene on an ESP32, 12-bit ADC at 3.3 V.
- `ESP32/ESP32_WROOM_32D`: an MQ-2 on an ESP32 with Wi-Fi on, read through an ADC1 pin (GPIO 36), since Wi-Fi disables ADC2.
- `ESP8266`: an MQ-3 reading benzene on an ESP8266, 10-bit ADC at 3.3 V.
- `MQ135-ADS1115`: an MQ-135 read through an ADS1115 external ADC with `updateFromVoltage`.
- `external_A2D`: any external converter, passing its raw value to `updateFromRaw`.
- `Digital_Input`: the module's digital output, which needs no library: a comparator against the module's potentiometer.
