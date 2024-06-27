## 环境
- GCC 版本：Arm GNU Toolchain arm-none-eabi\13.2 Rel1
- 仿真器：cmsis-dap
- ide：vscode + EIDE

## 注意事项
- dap 需要连接3V3，否则不工作
- flash 需要在 vscode 终端中选择运行任务 - flash

## 使用
1. 更改构建配置-链接脚本
2. 更改项目属性-预处理宏定义
3. 更改start_up文件
3. 更改vscode中openocd相关配置


## Puya PY32F0 Family

PY32F0 are cost-effective Arm Cortex-M0+ microcontrollers featured with wide range operating voltage from 1.7V to 5.5V. Datesheets and Reference Manuals can be found at somewhere.

## PY32F0xx

Frequency up to 48 MHz, 16 to 64 Kbytes of Flash memory, 3 to 8 Kbytes of SRAM.

* PY32F002A
  * PY32F002Ax5(20KB Flash/3KB RAM)
* PY32F003
  * PY32F003x4(16KB Flash/2KB RAM), PY32F003x6(32KB Flash/4KB RAM), PY32F003x8(64KB Flash/8KB RAM)
* PY32F030
  * PY32F030x4(16KB Flash/2KB RAM), PY32F030x6(32KB Flash/4KB RAM), PY32F030x8(64KB Flash/8KB RAM)