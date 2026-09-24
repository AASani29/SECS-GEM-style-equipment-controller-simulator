#include "MainWindow.h"

#include <QComboBox>
#include <QFontDatabase>
#include <QHBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTime>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle(QStringLiteral("SECS/GEM-Style Equipment Simulator"));
    resize(820, 560);

    // --- Create the widgets ---

    stateLabel_ = new QLabel(this);
    stateLabel_->setAlignment(Qt::AlignCenter);

    connectionLabel_ = new QLabel(this);

    logView_ = new QPlainTextEdit(this);
    logView_->setReadOnly(true);
    logView_->setMaximumBlockCount(1000); // don't let the log grow forever
    logView_->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

    connectButton_ = new QPushButton(QStringLiteral("Connect Host"), this);
    areYouThereButton_ = new QPushButton(QStringLiteral("Send S1F1 (Are You There)"), this);

    commandComboBox_ = new QComboBox(this);
    commandComboBox_->addItem(QStringLiteral("START"));
    commandComboBox_->addItem(QStringLiteral("RESET"));
    sendCommandButton_ = new QPushButton(QStringLiteral("Send S2F41 Remote Command"), this);

    alarmButton_ = new QPushButton(QStringLiteral("Trigger Alarm"), this);
    alarmButton_->setStyleSheet(QStringLiteral("background-color: #dc2626; color: white; padding: 6px 12px;"));

    // --- Lay them out ---

    QHBoxLayout* buttonRow = new QHBoxLayout;
    buttonRow->addWidget(connectButton_);
    buttonRow->addWidget(areYouThereButton_);
    buttonRow->addWidget(commandComboBox_);
    buttonRow->addWidget(sendCommandButton_);
    buttonRow->addStretch();
    buttonRow->addWidget(alarmButton_);

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(new QLabel(QStringLiteral("Equipment state"), this));
    mainLayout->addWidget(stateLabel_);
    mainLayout->addWidget(connectionLabel_);
    mainLayout->addLayout(buttonRow);
    mainLayout->addWidget(logView_, 1); // stretch factor 1: the log gets the spare space

    QWidget* centralWidget = new QWidget(this);
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);

    // --- Buttons only emit signals; they never call other threads' objects ---

    connect(connectButton_, &QPushButton::clicked, this, &MainWindow::connectRequested);
    connect(areYouThereButton_, &QPushButton::clicked, this, &MainWindow::areYouThereRequested);
    connect(alarmButton_, &QPushButton::clicked, this, &MainWindow::alarmRequested);
    connect(sendCommandButton_, &QPushButton::clicked, this, [this]() {
        emit remoteCommandRequested(commandComboBox_->currentText());
    });

    // The equipment always starts out IDLE, and the host starts out disconnected.
    setStateLabel(StateId::Idle);
    onConnectionChanged(false);
}

void MainWindow::appendLog(const QString& text) {
    const QString timestamp = QTime::currentTime().toString(QStringLiteral("HH:mm:ss.zzz"));
    logView_->appendPlainText(QStringLiteral("[%1] %2").arg(timestamp, text));
}

void MainWindow::onEquipmentStateChanged(StateId oldState, StateId newState) {
    setStateLabel(newState);
    appendLog(QStringLiteral("Equipment state changed: %1 -> %2")
                  .arg(QString::fromStdString(toString(oldState)),
                       QString::fromStdString(toString(newState))));
}

void MainWindow::onConnectionChanged(bool connected) {
    connectionLabel_->setText(connected ? QStringLiteral("Host link: connected")
                                        : QStringLiteral("Host link: disconnected"));

    // Requests that need a live link are only available once connected.
    connectButton_->setEnabled(!connected);
    areYouThereButton_->setEnabled(connected);
    commandComboBox_->setEnabled(connected);
    sendCommandButton_->setEnabled(connected);
}

void MainWindow::setStateLabel(StateId state) {
    QString color = QStringLiteral("#6b7280");
    switch (state) {
        case StateId::Idle:       color = QStringLiteral("#6b7280"); break;
        case StateId::Setup:      color = QStringLiteral("#d97706"); break;
        case StateId::Processing: color = QStringLiteral("#2563eb"); break;
        case StateId::Alarm:      color = QStringLiteral("#dc2626"); break;
        case StateId::Complete:   color = QStringLiteral("#16a34a"); break;
    }

    stateLabel_->setText(QString::fromStdString(toString(state)));
    stateLabel_->setStyleSheet(
        QStringLiteral("font-size: 28px; font-weight: bold; color: white; "
                       "background-color: %1; border-radius: 6px; padding: 12px;")
            .arg(color));
}
