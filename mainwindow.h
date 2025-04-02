#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDateTime>
#include <QtCharts/QChartView>
#include <QtCharts/QBoxPlotSeries>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>
#include <QScrollBar>
#include <QPushButton>
#include <QStringList>
#include <QGraphicsLineItem>
#include <QLabel>
#include <QMouseEvent>

QT_CHARTS_USE_NAMESPACE

    struct DataItem {
    QDateTime time;
    double open;
    double high;
    double low;
    double close;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void onOpenButtonClicked();
    void onScrollBarValueChanged(int value);
    void onYScrollValueChanged(int value);
    void onZoomInClicked();
    void onZoomOutClicked();
    void onResetYAxis();

private:
    void loadCSV(const QString &filename);
    void initChart();
    void updateVisibleRange(int startIndex);
    void updateYScrollBar();

    // UI 组件
    QChartView *chartView;
    QBoxPlotSeries *series;
    QChart *chart;
    QBarCategoryAxis *axisX;
    QValueAxis *axisY;
    QScrollBar *hScrollBar;
    QScrollBar *vScrollBar;
    QPushButton *zoomInBtn;
    QPushButton *zoomOutBtn;
    QPushButton *resetYAxisBtn;
    QGraphicsLineItem *crosshairXLine;
    QGraphicsLineItem *crosshairYLine;
    QLabel *infoLabel;

    // 数据相关
    QList<DataItem> allDataItems;
    QStringList currentCategories;

    // 状态参数
    int visibleItemCount;
    bool yAxisManual;
    double globalDataYMin;
    double globalDataYMax;
};

#endif // MAINWINDOW_H
