# API

Everything public in [`src/MQGasSensor.h`](../src/MQGasSensor.h). The site build fails if this page and the header disagree about which methods exist.

Units throughout: volts, kilohms, and the concentration unit of the curve, which is ppm for most curves and mg/L for the MQ-3's.

## Types

`MQModel` is the shape of a curve:

| Value | Concentration |
|---|---|
| `MQModel::PowerLaw` | `a × ratio^b`, the form every fitted coefficient uses |
| `MQModel::LogLinear` | `10^((log10(ratio) − b) / a)`, a straight line on log-log axes written with its slope `a` and intercept `b` |

`MQRatio` is the ratio a curve is plotted against:

| Value | Ratio |
|---|---|
| `MQRatio::RsOverR0` | Rs/R0, the default, which most datasheets plot |
| `MQRatio::R0OverRs` | R0/Rs, which the MQ-131 datasheet plots |

## Construction and setup

### `MQGasSensor(pin, adcRefVolts = 5.0, adcBits = 10, name = "MQ")`

`pin` is the analog input the sensor's output is wired to. `adcRefVolts` and `adcBits` describe the ADC that reads it, and turn a raw reading into volts. The sensor's supply voltage starts equal to `adcRefVolts`. `name` is only shown by `printDebug`; up to 11 characters are kept.

### `begin()`

Sets the pin as an input. Skip it when an external ADC reads the sensor and the pin is used for something else.

### `setLoadResistance(kOhm)`

RL, the resistor the sensor drives on its module. Default 10 kΩ.

### `setSupplyVoltage(volts)`

The voltage across the sensor and RL, used in Rs. Default: the ADC reference.

### `setAdcReference(volts)`

The ADC's reference voltage, used to turn a raw reading into volts.

### `setSupplyOffset(volts)`

Subtracted from the supply voltage wherever Rs is computed, including in `calibrate`. The MQ-303A examples set 0.45. Default 0.

### `setCurve(a, b, model = MQModel::PowerLaw)`

The curve `readPpm` and `ppmFromRatio` use. Every call sets the model too, so a sketch that switches between curves passes it each time. `a` is limited to ±10^30 and `b` to ±100; a value that is not a finite number becomes 0.

### `setRatio(ratio)`

Which ratio `readPpm` computes: `MQRatio::RsOverR0` (the default) or `MQRatio::R0OverRs`. `calibrate` always takes the clean-air ratio as Rs/R0, whichever is set.

### `setCorrection(factor)`

`readPpm` divides the ratio by `factor` before the curve, for an environmental correction such as temperature and humidity. Default 1.0, no correction.

## Calibration

### `calibrate(cleanAirRatio, r0Offset = 0.0)`

Sets R0 to Rs from the last update divided by `cleanAirRatio`, the datasheet's Rs/R0 in clean air, plus `r0Offset`, and returns it. A negative result becomes 0. If the last update read 0 V, R0 is infinite; if it read the supply voltage or more, Rs and so R0 are 0.

### `setR0(kOhm = 10)`

Sets R0 directly, for a value calibrated earlier and stored.

## Reading

### `update()`

Reads the pin twice, 20 ms apart, and keeps the average as the sensor voltage. It takes about 40 ms. Returns `false` if that is 0 V, which means the sensor is disconnected or unpowered.

### `updateFromVoltage(volts)`

Uses a voltage measured elsewhere, such as by an external ADC, as the sensor voltage. Returns `false` if it is 0 or less.

### `updateFromRaw(raw)`

Uses a raw ADC value measured elsewhere, converted with the reference and resolution given to the constructor. Returns `false` if it is 0.

### `readPpm()`

Computes Rs from the last update, the ratio (`setRatio`) divided by the correction (`setCorrection`), and the concentration from the curve, and returns the concentration. Returns `NAN` if the last update read 0 V. A concentration too large for a `float` is returned as `FLT_MAX`.

### `ppmFromRatio(ratio)`

The concentration the curve gives for `ratio`, without reading anything. 0 for a ratio of 0 or less, or before a curve is set.

## Inspection

### `rs()`

Rs in kΩ for the last update's voltage. Infinite at 0 V; 0 when the voltage reaches the supply.

### `ratio()`

The ratio the last `readPpm` used, after the correction. `NAN` if that read found 0 V.

### `voltage()`

The sensor voltage from the last update.

### `r0()`

R0 in kΩ.

### `a()`

The curve's `a`, after limiting.

### `b()`

The curve's `b`, after limiting.

### `printDebug(out = Serial)`

Prints one line to `out`, any `Print` such as `Serial` or an LCD library's object:

```
MQ-2: V=1.00 V, Rs=40.00 kOhm, R0=4.21 kOhm, ratio=9.51, ppm=4.44 (a*ratio^b, a=564.77, b=-2.15)
```

That line is from a simulated Mega 2560: an MQ-2 on its LPG curve, calibrated with `calibrate(9.51)` at 1.00 V and read at the same voltage.

Rs is computed from the current voltage; the ratio and ppm are those of the last `readPpm`.
