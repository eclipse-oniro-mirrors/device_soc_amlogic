# device_soc_amlogic

#### 简介

本仓用于放置amlogic soc相关内容，包含媒体编解码及display南向接口实现、框架及对接层库目录。

#### 约束

目前支持amlogic a311d

#### 软件框架

```
device/soc/amlogic
├── a311d                 # A311D芯片平台
│   ├── hardware
│   │   ├── BUILD.gn
│   │   ├── display       # display hdi实现
│   │   ├── ge2d          # 2D加速模块
│   │   ├── gpu           # GPU相关
│   │   ├── isp           # 图像处理相关服务
│   │   └── media         # 媒体编解码库
│   └── soc.gni
├── common                # amlogic芯片平台公共目录
│   └── hal
│       └── usb
├── LICENSE
├── OAT.xml
└── README_zh.md
```

#### 相关仓库

- device_soc_amlogic

- [vendor_unionman](https://gitee.com/openharmony-sig/vendor_unionman)

- [device_board_unionman](https://gitee.com/openharmony-sig/device_board_unionman)