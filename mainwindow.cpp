#include "mainwindow.h"
#include "ColorModels.h"
#include "AppStyle.h"

#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QFrame>
#include <QStatusBar>
#include <QSignalBlocker>
#include <QSlider>
#include <QHBoxLayout>
#include <QtMath>
#include <QPushButton>
#include <QColorDialog>
#include <QPainterPath>

#include <QStyleFactory>
#include <QPalette>
#include <QLabel>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QWidget *central = new QWidget(this);
    setCentralWidget(central);
    setFixedSize(700,700);

    QVector<QLabel*> formLabels;

    // helpers
    auto row = [](QWidget* parent, QWidget* w1, QWidget* w2){
        auto wrap = new QWidget(parent);
        auto h = new QHBoxLayout(wrap);
        h->setContentsMargins(0,0,0,0);
        h->setSpacing(6);
        h->addWidget(w1, 0, Qt::AlignVCenter);
        h->addWidget(w2, 1, Qt::AlignVCenter);
        return wrap;
    };

    auto makeFormLabel = [&](QWidget* parent, const QString& text){
        auto *lbl = new QLabel(text, parent);
        lbl->setObjectName("formLabel");
        lbl->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        formLabels.push_back(lbl);
        return lbl;
    };

    auto normalizeSpin = [](QDoubleSpinBox* sb){
        sb->setFixedHeight(32);
        sb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sb->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    };
    auto normalizeSlider = [](QSlider* sl){
        sl->setFixedHeight(22);
        sl->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    };

    auto unifySpinWidths = [&](std::initializer_list<QAbstractSpinBox*> boxes){
        int maxW = 0;
        for (auto *b : boxes) maxW = std::max(maxW, b->sizeHint().width());
        for (auto *b : boxes) {
            b->setMinimumWidth(maxW);
            b->setMaximumWidth(maxW);
        }
    };

    // ===== HSV ===========================================================
    auto *hsvGroup = new QGroupBox(tr("HSV"), this);
    QFont font("Segoe Print", 16, QFont::Black);
    hsvGroup->setFont(font);

    spinH = new QDoubleSpinBox(this);
    spinS = new QDoubleSpinBox(this);
    spinV = new QDoubleSpinBox(this);
    normalizeSpin(spinH); normalizeSpin(spinS); normalizeSpin(spinV);

    spinH->setRange(0.0, 360.0);  spinH->setDecimals(1);  spinH->setSingleStep(1.0);
    spinS->setRange(0.0, 100.0);  spinS->setDecimals(1);  spinS->setSingleStep(1.0);
    spinV->setRange(0.0, 100.0);  spinV->setDecimals(1);  spinV->setSingleStep(1.0);
    spinH->setKeyboardTracking(true);
    spinS->setKeyboardTracking(true);
    spinV->setKeyboardTracking(true);

    hSlider = new QSlider(Qt::Horizontal, this);
    sSlider = new QSlider(Qt::Horizontal, this);
    vSlider = new QSlider(Qt::Horizontal, this);
    normalizeSlider(hSlider); normalizeSlider(sSlider); normalizeSlider(vSlider);
    hSlider->setRange(0,360); sSlider->setRange(0,100); vSlider->setRange(0,100);
    auto bindD = [&](QDoubleSpinBox* d, QSlider* s){
        s->setRange(int(std::floor(d->minimum())), int(std::ceil(d->maximum())));
        connect(s, &QSlider::valueChanged, this, [=](int v){
            QSignalBlocker b(d);
            d->setValue(v);
            onHsvChanged();
        });
#if QT_VERSION >= QT_VERSION_CHECK(5,7,0)
        connect(d, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [=](double v){
            QSignalBlocker b(s);
            s->setValue(int(std::round(v)));
            onHsvChanged();
        });
#else
        connect(d, SIGNAL(valueChanged(double)), this, [=](double v){
            QSignalBlocker b(s);
            s->setValue(int(std::round(v)));
            onHsvChanged();
        });
#endif
    };

    bindD(spinH, hSlider); bindD(spinS, sSlider); bindD(spinV, vSlider);

    auto *hsvLayout = new QFormLayout;


    hsvLayout->addRow(makeFormLabel(hsvGroup, tr("H(°):")), row(hsvGroup, spinH, hSlider));
    hsvLayout->addRow(makeFormLabel(hsvGroup, tr("S(%):")), row(hsvGroup, spinS, sSlider));
    hsvLayout->addRow(makeFormLabel(hsvGroup, tr("V(%):")), row(hsvGroup, spinV, vSlider));
    auto *pickBtn = new QPushButton(tr("Palette"), this);
    { QFont f = pickBtn->font(); f.setPointSize(16); pickBtn->setFont(f); }
    hsvLayout->addRow(QString(), pickBtn);
    hsvGroup->setLayout(hsvLayout);
    connect(pickBtn, &QPushButton::clicked, this, [this]{
        QColor c = QColorDialog::getColor(Qt::white, this, tr("Color Picker"));
        if (!c.isValid()) return;
        Color::HSV hsv = Color::RGB_to_HSV({c.red(),c.green(),c.blue()});
        QSignalBlocker bh(spinH), bs(spinS), bv(spinV);
        spinH->setValue(hsv.h); spinS->setValue(hsv.s*100.0); spinV->setValue(hsv.v*100.0);
        onHsvChanged();
    });

    // ===== XYZ ===========================================================
    auto *xyzGroup = new QGroupBox(tr("XYZ"), this);
     xyzGroup->setFont(font);

    spinX = new QDoubleSpinBox(this);
    spinY = new QDoubleSpinBox(this);
    spinZ = new QDoubleSpinBox(this);
    normalizeSpin(spinX); normalizeSpin(spinY); normalizeSpin(spinZ);

    spinX->setRange(0.0,150.0); spinY->setRange(0.0,150.0); spinZ->setRange(0.0,150.0);
    spinX->setDecimals(3);      spinY->setDecimals(3);      spinZ->setDecimals(3);
    spinX->setSingleStep(0.001); spinY->setSingleStep(0.001); spinZ->setSingleStep(0.001);
    spinX->setKeyboardTracking(true); spinY->setKeyboardTracking(true); spinZ->setKeyboardTracking(true);

    xSlider = new QSlider(Qt::Horizontal, this);
    ySlider = new QSlider(Qt::Horizontal, this);
    zSlider = new QSlider(Qt::Horizontal, this);
    normalizeSlider(xSlider); normalizeSlider(ySlider); normalizeSlider(zSlider);

    auto bindDblSlider = [&](QDoubleSpinBox* d, QSlider* s, bool callXyzSlot){
        s->setRange(int(std::round(d->minimum()*SL_SCALE)),
                    int(std::round(d->maximum()*SL_SCALE)));
        connect(s, &QSlider::valueChanged, this, [=](int v){
            QSignalBlocker b(d);
            d->setValue(double(v)/SL_SCALE);
            if (callXyzSlot) onXyzChanged(); else onLabChanged();
        });
#if QT_VERSION >= QT_VERSION_CHECK(5,7,0)
        connect(d, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [=](double val){
            QSignalBlocker b(s);
            s->setValue(int(std::round(val*SL_SCALE)));
            if (callXyzSlot) onXyzChanged(); else onLabChanged();
        });
#else
        connect(d, SIGNAL(valueChanged(double)), this, [=](double val){
            QSignalBlocker b(s);
            s->setValue(int(std::round(val*SL_SCALE)));
            if (callXyzSlot) onXyzChanged(); else onLabChanged();
        });
#endif
    };


    auto *xyzLayout = new QFormLayout;
    xyzLayout->addRow(makeFormLabel(xyzGroup, tr("X:   ")), row(xyzGroup, spinX, xSlider));
    xyzLayout->addRow(makeFormLabel(xyzGroup, tr("Y:   ")), row(xyzGroup, spinY, ySlider));
    xyzLayout->addRow(makeFormLabel(xyzGroup, tr("Z:   ")), row(xyzGroup, spinZ, zSlider));
    auto *pickXyzBtn = new QPushButton(tr("Palette"), this);
    { QFont f = pickXyzBtn->font(); f.setPointSize(16); pickXyzBtn->setFont(f); }
    xyzLayout->addRow(QString(), pickXyzBtn);
    xyzGroup->setLayout(xyzLayout);
    connect(pickXyzBtn, &QPushButton::clicked, this, [this]{
        QColor c = QColorDialog::getColor(Qt::white, this, tr("Color Picker"));
        if (!c.isValid()) return;
        Color::XYZ xyz = Color::RGB_to_XYZ({c.red(),c.green(),c.blue()});
        QSignalBlocker bx(spinX), by(spinY), bz(spinZ);
        spinX->setValue(xyz.X); spinY->setValue(xyz.Y); spinZ->setValue(xyz.Z);
        onXyzChanged();
    });

    bindDblSlider(spinX, xSlider, true);
    bindDblSlider(spinY, ySlider, true);
    bindDblSlider(spinZ, zSlider, true);

    // ===== LAB ===========================================================
    auto *labGroup = new QGroupBox(tr("LAB"), this);
    labGroup->setFont(font);

    spinL = new QDoubleSpinBox(this);
    spina = new QDoubleSpinBox(this);
    spinb = new QDoubleSpinBox(this);
    normalizeSpin(spinL); normalizeSpin(spina); normalizeSpin(spinb);

    spinL->setRange(0.0,100.0);
    spina->setRange(-200.0,200.0);
    spinb->setRange(-200.0,200.0);
    spinL->setDecimals(3); spina->setDecimals(3); spinb->setDecimals(3);
    spinL->setSingleStep(0.1); spina->setSingleStep(0.1); spinb->setSingleStep(0.1);
    spinL->setKeyboardTracking(true); spina->setKeyboardTracking(true); spinb->setKeyboardTracking(true);

    lSlider  = new QSlider(Qt::Horizontal, this);
    aSlider  = new QSlider(Qt::Horizontal, this);
    bbSlider = new QSlider(Qt::Horizontal, this);
    normalizeSlider(lSlider); normalizeSlider(aSlider); normalizeSlider(bbSlider);

    auto *labLayout = new QFormLayout;
    labLayout->addRow(makeFormLabel(labGroup, tr("L:   ")), row(labGroup, spinL,  lSlider));
    labLayout->addRow(makeFormLabel(labGroup, tr("A:   ")), row(labGroup, spina, aSlider));
    labLayout->addRow(makeFormLabel(labGroup, tr("B:   ")), row(labGroup, spinb, bbSlider));
    auto *pickLabBtn = new QPushButton(tr("Palette"), this);
    { QFont f = pickLabBtn->font(); f.setPointSize(16); pickLabBtn->setFont(f); }
    labLayout->addRow(QString(), pickLabBtn);
    labGroup->setLayout(labLayout);
    connect(pickLabBtn, &QPushButton::clicked, this, [this]{
        QColor c = QColorDialog::getColor(Qt::white, this, tr("Color Picker"));
        if (!c.isValid()) return;
        Color::XYZ xyz = Color::RGB_to_XYZ({c.red(),c.green(),c.blue()});
        Color::Lab lab = Color::XYZ_to_Lab(xyz);
        QSignalBlocker bl(spinL), ba(spina), bb(spinb);
        spinL->setValue(lab.L); spina->setValue(lab.a); spinb->setValue(lab.b);
        onLabChanged();
    });

    bindDblSlider(spinL,  lSlider,  false);
    bindDblSlider(spina,  aSlider,  false);
    bindDblSlider(spinb,  bbSlider, false);

    // ===== превью и общий грид ==========================================
    previewFrame = new QFrame(this);
    previewFrame->setMinimumSize(300, 550);

    previewFrame->setAutoFillBackground(true);

    QWidget *previewWrapper = new QWidget(this);
    QVBoxLayout *pvLayout = new QVBoxLayout(previewWrapper);
    pvLayout->addWidget(previewFrame, 0, Qt::AlignCenter);
    pvLayout->setContentsMargins(10, 24, 10, 10);

    unifySpinWidths({
        spinH, spinS, spinV,
        spinX, spinY, spinZ,
        spinL, spina, spinb
    });

    // общий вид форм
    for (auto form : {hsvLayout, xyzLayout, labLayout}) {
        form->setContentsMargins(8, 8, 8, 8);
        form->setHorizontalSpacing(10);
        form->setVerticalSpacing(10);
        form->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
        form->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
        form->setFormAlignment(Qt::AlignLeft | Qt::AlignTop);
    }

    auto unifyLabelWidths = [&](const QVector<QLabel*>& labels){
        int maxW = 0;
        for (auto *l : labels) maxW = std::max(maxW, l->sizeHint().width());
        for (auto *l : labels) l->setFixedWidth(maxW);
    };
    unifyLabelWidths(formLabels);

    auto *grid = new QGridLayout;
    grid->addWidget(hsvGroup, 0, 0);
    grid->addWidget(xyzGroup, 1, 0);
    grid->addWidget(labGroup, 2, 0);
    grid->addWidget(previewWrapper, 0, 1, 3, 1);
    grid->setColumnStretch(0, 1);
    grid->setColumnStretch(1, 1);
    central->setLayout(grid);

    spinH->setValue(0.0);
    spinS->setValue(100.0);
    spinV->setValue(100.0);
    onHsvChanged();

    AppStyle::applyAppStyle(this);
}





MainWindow::~MainWindow() = default;

void MainWindow::setHSVui(double H_deg, double S_pct, double V_pct) {
    QSignalBlocker bh(spinH), bs(spinS), bv(spinV);
    QSignalBlocker hh(hSlider), hs(sSlider), hv(vSlider);
    spinH->setValue(H_deg);
    spinS->setValue(S_pct);
    spinV->setValue(V_pct);
    hSlider->setValue(int(std::round(H_deg)));
    sSlider->setValue(int(std::round(S_pct)));
    vSlider->setValue(int(std::round(V_pct)));
}

void MainWindow::setXYZui(double X, double Y, double Z) {
    QSignalBlocker bx(spinX), by(spinY), bz(spinZ);
    QSignalBlocker hx(xSlider), hy(ySlider), hz(zSlider);
    spinX->setValue(X);  spinY->setValue(Y);  spinZ->setValue(Z);
    xSlider->setValue(int(std::round(X*SL_SCALE)));
    ySlider->setValue(int(std::round(Y*SL_SCALE)));
    zSlider->setValue(int(std::round(Z*SL_SCALE)));
}

void MainWindow::setLabui(double L, double a, double b) {
    QSignalBlocker bl(spinL), ba(spina), bb(spinb);
    QSignalBlocker hl(lSlider), ha(aSlider), hb(bbSlider);
    spinL->setValue(L); spina->setValue(a); spinb->setValue(b);
    lSlider->setValue(int(std::round(L*SL_SCALE)));
    aSlider->setValue(int(std::round(a*SL_SCALE)));
    bbSlider->setValue(int(std::round(b*SL_SCALE)));
}


// ===== ВСПОМОГАТЕЛЬНОЕ =====================================================

void MainWindow::updatePreview(const Color::RGB &rgb)
{
    previewFrame->setStyleSheet(
        QString("background-color: rgb(%1,%2,%3); border: 1px solid #888;")
            .arg(rgb.r).arg(rgb.g).arg(rgb.b));
    previewFrame->update();

}

// ===== СЛОТЫ ===============================================================

void MainWindow::onXyzChanged() {
    if (m_updating) return;
    m_updating = true;

    Color::XYZ xyz{spinX->value(), spinY->value(), spinZ->value()};
    auto conv = Color::XYZ_to_RGB(xyz);
    Color::RGB rgb = conv.first;
    Color::ConvertFlags flags = conv.second;
    Color::Lab lab = Color::XYZ_to_Lab(xyz);
    Color::HSV hsv = Color::RGB_to_HSV(rgb);

    setHSVui(hsv.h, hsv.s * 100.0, hsv.v * 100.0);
    setLabui(lab.L, lab.a, lab.b);

    if (flags.outOfGamut)
        statusBar()->showMessage(tr("Out of sRGB gamut — values have been clipped."));
    else
        statusBar()->clearMessage();

    updatePreview(rgb);
    m_updating = false;
}


void MainWindow::onLabChanged() {
    if (m_updating) return;
    m_updating = true;

    Color::Lab lab{spinL->value(), spina->value(), spinb->value()};
    Color::XYZ xyz = Color::Lab_to_XYZ(lab);
    auto conv = Color::XYZ_to_RGB(xyz);
    Color::RGB rgb = conv.first;
    Color::ConvertFlags flags = conv.second;
    Color::HSV hsv = Color::RGB_to_HSV(rgb);

    setXYZui(xyz.X, xyz.Y, xyz.Z);
    setHSVui(hsv.h, hsv.s * 100.0, hsv.v * 100.0);

    if (flags.outOfGamut)
        statusBar()->showMessage(tr("Out of sRGB gamut — values have been clipped."));
    else
        statusBar()->clearMessage();

    updatePreview(rgb);
    m_updating = false;
}


void MainWindow::onHsvChanged() {
    if (m_updating) return;
    m_updating = true;

    Color::HSV hsv{ spinH->value(), spinS->value() / 100.0, spinV->value() / 100.0 };
    Color::RGB rgb = Color::HSV_to_RGB(hsv);
    Color::XYZ xyz = Color::RGB_to_XYZ(rgb);
    Color::Lab lab = Color::XYZ_to_Lab(xyz);

    setXYZui(xyz.X, xyz.Y, xyz.Z);
    setLabui(lab.L, lab.a, lab.b);

    statusBar()->clearMessage();

    updatePreview(rgb);
    m_updating = false;
}



