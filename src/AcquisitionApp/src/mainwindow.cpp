#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "imagedisplayer.h"

#include <QPushButton>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), m_ui(new Ui::MainWindow) {
    m_ui->setupUi(this);

    m_ui->graphicsView->setScene(new ImageDisplayer());

    m_logger.setLog(m_ui->log);

#ifdef BUILD_ACQ
    // set acquisitor logger
    Acquisitor::get()->setLog(&m_logger);

    // connect Acquisitor initializer
    connect(m_ui->initialize, &QPushButton::clicked, [=] {
        // Start acquisitor initialization and disable button
        m_ui->initialize->setEnabled(false);
        m_ui->initialize->setText("Initializing...");
        Acquisitor::get()->initialize();
    });

    connect(Acquisitor::get(), &Acquisitor::initialized, [=](bool state) {
        if (state) {
            // Acquisitor is initialized, disable button and set text
            m_ui->initialize->setEnabled(false);
            m_ui->initialize->setText("Initialized");
        } else {
            // Acquisitor is no longer initialized/initialization failed
            m_ui->initialize->setEnabled(true);
            m_ui->initialize->setText("Initialize framegrabber");
        }
    });
    // Disable start-acq button by default
    m_ui->start->setEnabled(false);

    // Connect start functionality for frame grabber
    connect(m_ui->start, &QPushButton::toggled, [=](bool state) {
        if (state) {
            Acquisitor::get()->startAcq();
        } else {
            Acquisitor::get()->stopAcq();
        }
    });
    connect(Acquisitor::get(), &Acquisitor::acquisitionStateChanged, [=](bool state) {
        if (state) {
            m_ui->start->setText("Stop acquisition");
        } else {
            m_ui->start->setText("Start acquisition");
        };
    });
#endif
}

MainWindow::~MainWindow() {
    delete m_ui;
}

void MainWindow::on_actionExit_triggered() {
    QApplication::exit();
}