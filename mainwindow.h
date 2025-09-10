#pragma once
#include <QMainWindow>

class QSpinBox;
class QDoubleSpinBox;
class QFrame;
class QSlider;
class QLineEdit;
class QPushButton;

namespace Color { struct RGB; }

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onXyzChanged();
    void onLabChanged();
    void onHsvChanged();

private:
    // Вспомогательные методы
    void updatePreview(const Color::RGB &rgb);
    void updateFromRGB(const Color::RGB &rgb, bool showWarning);

    // Флаг защиты от рекурсии
    bool m_updating = false;

    // HSV
    QDoubleSpinBox *spinH = nullptr;
    QDoubleSpinBox *spinS = nullptr;
    QDoubleSpinBox *spinV = nullptr;
    QSlider *hSlider = nullptr;
    QSlider *sSlider = nullptr;
    QSlider *vSlider = nullptr;

    // XYZ
    QDoubleSpinBox *spinX = nullptr;
    QDoubleSpinBox *spinY = nullptr;
    QDoubleSpinBox *spinZ = nullptr;
    QSlider *xSlider = nullptr;
    QSlider *ySlider = nullptr;
    QSlider *zSlider = nullptr;

    // Lab
    QDoubleSpinBox *spinL = nullptr;
    QDoubleSpinBox *spina = nullptr;
    QDoubleSpinBox *spinb = nullptr;
    QSlider *lSlider = nullptr;
    QSlider *aSlider = nullptr;
    QSlider *bbSlider = nullptr;

    QFrame *previewFrame = nullptr;

    static constexpr int SL_SCALE = 1000;

    void setHSVui(double H_deg, double S_pct, double V_pct);
    void setXYZui(double X, double Y, double Z);
    void setLabui(double L, double a, double b);
};
