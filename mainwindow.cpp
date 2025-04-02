#include "mainwindow.h"
#include "analysisdialog.h"
#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include <QSplitter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QTextCodec>
#include <algorithm>
#include <QtMath>
#include <QApplication>       // 新增
#include <QDesktopWidget>     // 新增
#include <QScreen>
#include <QOpenGLWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    yAxisManual(false),
    visibleItemCount(20),
    crosshairXLine(nullptr),
    crosshairYLine(nullptr),
    infoLabel(new QLabel(this))


{
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

    QSplitter *mainSplitter = new QSplitter(Qt::Horizontal, this);
    setCentralWidget(mainSplitter);



    QWidget *chartWidget = new QWidget;
    QVBoxLayout *mainLayout = new QVBoxLayout(chartWidget);
    mainLayout->setContentsMargins(2, 2, 2, 2);

    QHBoxLayout *chartLayout = new QHBoxLayout;
    chartLayout->setSpacing(2);

    vScrollBar = new QScrollBar(Qt::Vertical);
    vScrollBar->setEnabled(false);
    chartLayout->addWidget(vScrollBar);

    chartView = new QChartView();
    chartView->setRubberBand(QChartView::RectangleRubberBand);
    chartView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    chartLayout->addWidget(chartView, 1);

    hScrollBar = new QScrollBar(Qt::Horizontal);
    hScrollBar->setEnabled(false);

    mainLayout->addLayout(chartLayout, 1);
    mainLayout->addWidget(hScrollBar);

    mainSplitter->addWidget(chartWidget);

    QWidget *controlWidget = new QWidget;
    QVBoxLayout *controlLayout = new QVBoxLayout(controlWidget);
    controlLayout->setContentsMargins(5, 5, 5, 5);

    QPushButton *openBtn = new QPushButton(tr("打开CSV"));

    // 创建缩放按钮布局
    QHBoxLayout *zoomLayout = new QHBoxLayout();
    zoomInBtn = new QPushButton("+");
    zoomOutBtn = new QPushButton("-");
    resetYAxisBtn = new QPushButton(tr("复位"));

    // 设置按钮样式
    zoomInBtn->setFixedSize(40, 30);
    zoomOutBtn->setFixedSize(40, 30);
    resetYAxisBtn->setFixedSize(60, 30);

    zoomLayout->addWidget(resetYAxisBtn);
    zoomLayout->addWidget(zoomOutBtn);
    zoomLayout->addWidget(zoomInBtn);

    controlLayout->addWidget(openBtn);
    controlLayout->addLayout(zoomLayout);
    controlLayout->addStretch();

    mainSplitter->addWidget(controlWidget);
    mainSplitter->setStretchFactor(0, 8);
    mainSplitter->setStretchFactor(1, 1);

    initChart();

    // 连接信号槽
    connect(openBtn, &QPushButton::clicked, this, &MainWindow::onOpenButtonClicked);
    connect(hScrollBar, &QScrollBar::valueChanged, this, &MainWindow::onScrollBarValueChanged);
    connect(vScrollBar, &QScrollBar::valueChanged, this, &MainWindow::onYScrollValueChanged);
    connect(zoomInBtn, &QPushButton::clicked, this, &MainWindow::onZoomInClicked);
    connect(zoomOutBtn, &QPushButton::clicked, this, &MainWindow::onZoomOutClicked);
    connect(resetYAxisBtn, &QPushButton::clicked, this, &MainWindow::onResetYAxis);

    // 初始化十字线
    crosshairXLine = new QGraphicsLineItem();
    crosshairYLine = new QGraphicsLineItem();
    crosshairXLine->setPen(QPen(Qt::gray, 1, Qt::DashLine));
    crosshairYLine->setPen(QPen(Qt::gray, 1, Qt::DashLine));
    crosshairXLine->setZValue(10);
    crosshairYLine->setZValue(10);
    chartView->scene()->addItem(crosshairXLine);
    chartView->scene()->addItem(crosshairYLine);
    crosshairXLine->hide();
    crosshairYLine->hide();

    // 信息标签初始化
    infoLabel->setStyleSheet("background-color: white; border: 1px solid black; padding: 2px;");
    infoLabel->setAlignment(Qt::AlignCenter);
    infoLabel->hide();
    infoLabel->setWindowFlags(Qt::ToolTip);

    chartView->setMouseTracking(true);
    chartView->viewport()->installEventFilter(this);
}

MainWindow::~MainWindow()
{
    delete chart;
}

void MainWindow::initChart()
{
    chart = new QChart();
    chart->setAnimationOptions(QChart::NoAnimation);
    chart->setMargins(QMargins(40, 10, 20, 30));
    chartView->setChart(chart);

    series = new QBoxPlotSeries();
    chart->addSeries(series);

    axisX = new QBarCategoryAxis();
    axisY = new QValueAxis();
    axisY->setLabelFormat("%.2f");
    axisY->setTitleText("价格");
    axisY->setLabelsAngle(-90);

    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);

    series->attachAxis(axisX);
    series->attachAxis(axisY);
    chart->legend()->hide();
    chartView->setRenderHint(QPainter::Antialiasing);
}

void MainWindow::onOpenButtonClicked()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("打开CSV文件"),
        "",
        tr("CSV文件 (*.csv)"));

    if (!fileName.isEmpty()) {
        loadCSV(fileName);
    }
}

void MainWindow::loadCSV(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;

    QTextStream in(&file);
    allDataItems.clear();
    currentCategories.clear();
    yAxisManual = false;

    hScrollBar->setPageStep(visibleItemCount); // 确保页步长初始化
    hScrollBar->setRange(0, qMax(0, allDataItems.size() - visibleItemCount));

    globalDataYMin = std::numeric_limits<double>::max();
    globalDataYMax = -std::numeric_limits<double>::max();

    if (!in.atEnd()) in.readLine();

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        QStringList fields = line.split('\t');

        if (fields.size() < 6) continue;

        QDateTime dt = QDateTime::fromString(
            fields[0] + " " + fields[1],
            "yyyy.MM.dd hh:mm:ss"
            );

        if (!dt.isValid()) continue;

        bool ok;
        DataItem item;
        item.time = dt;
        item.open = fields[2].toDouble(&ok);
        item.high = fields[3].toDouble(&ok);
        item.low = fields[4].toDouble(&ok);
        item.close = fields[5].toDouble(&ok);

        if (ok) {
            allDataItems.append(item);
            currentCategories << dt.toString("MM/dd hh:mm");
            globalDataYMin = qMin(globalDataYMin, item.low);
            globalDataYMax = qMax(globalDataYMax, item.high);
        }
    }

    series->clear();
    axisX->clear();
    axisX->append(currentCategories);

    foreach (const DataItem &item, allDataItems) {
        QtCharts::QBoxSet *box = new QtCharts::QBoxSet();
        box->setValue(QtCharts::QBoxSet::LowerExtreme, item.low);
        box->setValue(QtCharts::QBoxSet::UpperExtreme, item.high);
        box->setValue(QtCharts::QBoxSet::LowerQuartile, qMin(item.open, item.close));
        box->setValue(QtCharts::QBoxSet::UpperQuartile, qMax(item.open, item.close));
        box->setValue(QtCharts::QBoxSet::Median, item.close);

        box->setBrush(QBrush(item.close > item.open ? Qt::green : Qt::red));
        series->append(box);
    }

    hScrollBar->setRange(0, qMax(0, allDataItems.size() - visibleItemCount));
    hScrollBar->setPageStep(visibleItemCount);
    hScrollBar->setEnabled(allDataItems.size() > visibleItemCount);

    if (!allDataItems.isEmpty()) {
        updateVisibleRange(0);
        axisX->setRange(currentCategories.first(),
                        currentCategories.at(qMin(visibleItemCount-1, currentCategories.size()-1)));
    }

    updateYScrollBar();
    file.close();
}

void MainWindow::onScrollBarValueChanged(int value) {
    if (allDataItems.isEmpty()) return;

    yAxisManual = false;
    int startIndex = value;
    int endIndex = qMin(startIndex + visibleItemCount - 1, allDataItems.size() - 1); // 使用当前可见项数

    axisX->setRange(currentCategories.at(startIndex), currentCategories.at(endIndex));
    updateVisibleRange(startIndex);
}
void MainWindow::onYScrollValueChanged(int value)
{
    if (yAxisManual) return;

    yAxisManual = true;
    double currentRange = axisY->max() - axisY->min();
    double minValue = globalDataYMin + (value / 1000.0);
    axisY->setRange(minValue, minValue + currentRange);
}

void MainWindow::onZoomInClicked()
{
    yAxisManual = true;
    const double zoomFactor = 0.8;

    QPointF center = chart->mapToValue(chart->plotArea().center());
    double newHeight = (axisY->max() - axisY->min()) * zoomFactor;

    axisY->setRange(center.y() - newHeight/2, center.y() + newHeight/2);
    updateYScrollBar();
}

void MainWindow::onZoomOutClicked()
{
    yAxisManual = true;
    const double zoomFactor = 1.2;

    QPointF center = chart->mapToValue(chart->plotArea().center());
    double newHeight = (axisY->max() - axisY->min()) * zoomFactor;

    axisY->setRange(center.y() - newHeight/2, center.y() + newHeight/2);
    updateYScrollBar();
}

void MainWindow::onResetYAxis()
{
    yAxisManual = false;
    updateVisibleRange(hScrollBar->value());
}

void MainWindow::updateVisibleRange(int startIndex)
{
    if (yAxisManual) return;

    int endIndex = qMin(startIndex + visibleItemCount - 1, allDataItems.size() - 1);

    double localYMin = allDataItems[startIndex].low;
    double localYMax = allDataItems[startIndex].high;

    for(int i = startIndex; i <= endIndex; ++i) {
        const DataItem &item = allDataItems[i];
        localYMin = qMin(localYMin, item.low);
        localYMax = qMax(localYMax, item.high);
    }

    const double buffer = (localYMax - localYMin) * 0.05;
    axisY->setRange(localYMin - buffer, localYMax + buffer);
    updateYScrollBar();
}

void MainWindow::updateYScrollBar()
{
    vScrollBar->blockSignals(true);

    double totalRange = globalDataYMax - globalDataYMin;
    double currentRange = axisY->max() - axisY->min();

    if (totalRange <= currentRange) {
        vScrollBar->setMaximum(0);
        vScrollBar->setEnabled(false);
    } else {
        int max = static_cast<int>((totalRange - currentRange) * 1000);
        vScrollBar->setMaximum(max);
        vScrollBar->setEnabled(true);
        int value = static_cast<int>((axisY->min() - globalDataYMin) * 1000);
        vScrollBar->setValue(qBound(0, value, max));
    }

    vScrollBar->blockSignals(false);
}
bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == chartView->viewport()) {

        if (event->type() == QEvent::MouseMove) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            QPoint pos = mouseEvent->pos();
            QRectF plotArea = chart->plotArea();

            // 将鼠标位置转换为场景坐标
            QPointF scenePos = chartView->mapToScene(pos);
            // 将绘图区域坐标转换为场景坐标
            QPointF plotAreaTopLeft = chartView->mapToScene(plotArea.topLeft().toPoint());
            QPointF plotAreaBottomRight = chartView->mapToScene(plotArea.bottomRight().toPoint());

            // 创建基于场景坐标的绘图区域矩形
            QRectF scenePlotArea(plotAreaTopLeft, plotAreaBottomRight);

            if (scenePlotArea.contains(scenePos)) {
                // 计算相对于绘图区域左上角的X坐标偏移
                qreal mouseXInPlot = scenePos.x() - plotAreaTopLeft.x();
                // 计算每个数据项的宽度
                qreal widthPerItem = scenePlotArea.width() / visibleItemCount;
                // 计算当前数据项索引
                int startIndex = hScrollBar->value();
                int index = startIndex + static_cast<int>(mouseXInPlot / widthPerItem);

                if (index >= 0 && index < allDataItems.size()) {
                    const DataItem &item = allDataItems[index];
                    // 转换Y坐标到实际值
                    qreal yValue = chart->mapToValue(QPointF(pos.x(), pos.y())).y();

                    /* 更新十字线 - 使用场景坐标 */
                    // 垂直十字线（覆盖整个绘图区域高度）
                    crosshairXLine->setLine(scenePos.x(), plotAreaTopLeft.y(),
                                            scenePos.x(), plotAreaBottomRight.y());
                    // 水平十字线（覆盖整个绘图区域宽度）
                    crosshairYLine->setLine(plotAreaTopLeft.x(), scenePos.y(),
                                            plotAreaBottomRight.x(), scenePos.y());
                    crosshairXLine->show();
                    crosshairYLine->show();

                    /* 更新信息标签 */
                    QString timeStr = item.time.toString("MM/dd hh:mm");
                    QString highStr = QString::number(item.high, 'f', 2);
                    QString lowStr = QString::number(item.low, 'f', 2);
                    QString closeStr = QString::number(item.close, 'f', 2);

                    QString info = QString::fromUtf8(u8"时间: %1\n 最高: %2\n 最低: %3\n 收盘: %4")
                                       .arg(timeStr, highStr, lowStr, closeStr);

                    infoLabel->setText(info);
                    infoLabel->adjustSize();

                    // 将标签位置限制在窗口内
                    QPoint labelPos = mouseEvent->globalPos() + QPoint(20, 20);
                    QRect screenGeometry = QApplication::desktop()->screenGeometry();
                    if (labelPos.x() + infoLabel->width() > screenGeometry.right()) {
                        labelPos.setX(mouseEvent->globalPos().x() - infoLabel->width() - 30);
                    }
                    if (labelPos.y() + infoLabel->height() > screenGeometry.bottom()) {
                        labelPos.setY(mouseEvent->globalPos().y() - infoLabel->height() - 30);
                    }
                    infoLabel->move(labelPos);
                    infoLabel->show();
                }
            } else {
                crosshairXLine->hide();
                crosshairYLine->hide();
                infoLabel->hide();
            }
            return false; // 事件已处理


        }
        else if (event->type() == QEvent::Leave) {
            crosshairXLine->hide();
            crosshairYLine->hide();
            infoLabel->hide();
            return false;
        }
        if (event->type() == QEvent::Wheel) {
            QWheelEvent *wheelEvent = static_cast<QWheelEvent*>(event);



            if (wheelEvent->modifiers() & Qt::ControlModifier) {
                int delta = wheelEvent->angleDelta().y() > 0 ? -1 : 1;
                int step = 5; // 每次调整5个项
                int newCount = visibleItemCount + delta * step;
                newCount = qBound(5, newCount, allDataItems.size()); // 限制最小和最大值

                if (newCount != visibleItemCount) {
                    visibleItemCount = newCount;

                    // 更新水平滚动条参数
                    hScrollBar->setPageStep(visibleItemCount);
                    int maxScroll = qMax(0, allDataItems.size() - visibleItemCount);
                    hScrollBar->setRange(0, maxScroll);

                    // 确保当前滚动值在范围内
                    int currentScroll = hScrollBar->value();
                    if (currentScroll > maxScroll) {
                        hScrollBar->setValue(maxScroll);
                        currentScroll = maxScroll;
                    }

                    // 更新图表显示范围
                    int startIndex = currentScroll;
                    int endIndex = qMin(startIndex + visibleItemCount - 1, allDataItems.size() - 1);
                    axisX->setRange(currentCategories.at(startIndex), currentCategories.at(endIndex));
                    updateVisibleRange(startIndex);
                }

                return true; // 事件已处理
            }

        }
        return QMainWindow::eventFilter(watched, event);
    }
}
