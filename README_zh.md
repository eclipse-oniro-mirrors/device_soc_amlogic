# device_soc_amlogic

## 介绍

本仓库下实现了Amlogic芯片系列的display hdi(Hardware Driver Interfaces)。其中ge2d为芯片平台支持的特性，用于辅助display进行硬件合成及图形处理加速。gpu用于硬件合成渲染显示。isp专用于平台的图像处理，实现Auto Exposure/Auto Focus/Auto White Balance，并配合media模块的进行图像的编解码。本库相关代码可直接复用于Amlogic芯片系列的产品。

## 约束

目前支持的芯片系列

| 芯片厂商    | 主芯片型号 | GPU型号    |
| ------- | ----- | -------- |
| Amlogic | A311D | Mali-G52 |

## 目录

```
device/soc/amlogic
├── a311d                 # A311D芯片目录
│   ├── hardware
│   │   ├── BUILD.gn
│   │   ├── display       # display hdi实现目录
│   │   ├── ge2d          # 2D加速模块目录
│   │   ├── gpu           # GPU库目录
│   │   ├── isp           # 图像处理相关和依赖库目录
│   │   └── media         # 媒体编解码库目录
│   └── soc.gni
├── common                # amlogic芯片平台公共目录
│   └── hal
│       └── usb
├── LICENSE
├── OAT.xml
└── README_zh.md
```

## 相关仓库

- device_soc_amlogic

- [vendor_unionman](https://gitee.com/openharmony-sig/vendor_unionman)

- [device_board_unionman](https://gitee.com/openharmony-sig/device_board_unionman)