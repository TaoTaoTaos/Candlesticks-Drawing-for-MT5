# Candlesticks-Drawing-for-MT5
 
# MT5-K线图查看器
<div style="text-align:center;">     <img src="https://picture-1323909691.cos.ap-nanjing.myqcloud.com/test/202504021654362.png" style="height:314px; width:auto;"> </div>
A Qt-based application for visualizing financial candlestick/box plot charts with interactive features.  
基于Qt的金融K线图可视化工具，支持多种交互功能。

## Features 功能特性
- Load CSV data with time series and OHLC values  
  加载包含时间序列和OHLC值的CSV数据
- Interactive horizontal/vertical scrolling  
  交互式水平/垂直滚动
- Dynamic zoom in/out operations  
  动态缩放操作
- Auto-range Y axis with manual override  
  自动范围Y轴（支持手动覆盖）
- Crosshair tracking with real-time info display  
  十字线跟踪实时信息显示
- Adjustable visible items via Ctrl+MouseWheel  
  通过Ctrl+鼠标滚轮调整可见数据项
- Responsive UI layout with control panel  
  带控制面板的响应式UI布局
- Support Chinese/English localization  
  支持中英文本地化

## Technical Highlights 技术要点
- Qt Charts module integration  
  集成Qt Charts模块
- Custom event filtering for mouse interactions  
  自定义事件过滤器处理鼠标交互
- Dynamic axis range calculation  
  动态坐标轴范围计算
- Efficient data loading and rendering  
  高效数据加载与渲染
- QGraphicsScene-based crosshair implementation  
  基于QGraphicsScene的十字线实现
- Adaptive scrollbar management  
  自适应滚动条管理

## Dependencies 依赖
- Qt 5.15+  
- Qt Charts module  
- C++17 compiler  


## Usage 使用方法
1. Click "Open CSV" to load data  
   点击"打开CSV"加载数据
2. Use mouse wheel to scroll vertically  
   使用鼠标滚轮垂直滚动
3. Ctrl+MouseWheel to adjust time scale  
   Ctrl+鼠标滚轮调整时间尺度
4. Click +/- buttons for Y-axis zoom  
   点击+/-按钮进行Y轴缩放
5. Hover mouse to show crosshair  
   鼠标悬停显示十字线

## Data Format 数据格式
CSV with columns:
日期 时间 开盘价 最高价 最低价 收盘价
EXP：
2024.01.01 09:30:00 100.0 105.0 98.0 102.5
