# 相机控制参数API {#camera_control_params}

## 打开或关闭自动曝光

```↩
/** Auto-exposure enabled or not  default enabled*/
bool AutoExposureControl(bool enable);    see "camera.h"
```

## 打开或关闭自动白平衡

```
/** Auto-white-balance enabled or not  default enabled*/
bool AutoWhiteBalanceControl(bool enable);    see "camera.h"
```

## 设置 IR 强度

```
/** set infrared(IR) intensity [0, 10] default 4*/
void SetIRIntensity(const std::uint16_t &value);     see "camera.h"
```

## 设置全局增益

注意:: 需要关闭自动曝光

```
/** Set global gain [1 - 16]
 * value -- global gain value
 * */
void SetGlobalGain(const float &value);    see "camera.h"
```

## 设置曝光时间

注意:: 需要关闭自动曝光

```
/** Set exposure time [1ms - 2000ms]
 * value -- exposure time value
 * */
void SetExposureTime(const float &value);    see "camera.h"
```

