#include <cstdio>
#include <fstream>
#include <opencv/cv.hpp>
#include "lib/analyzer.h"
#include "lib/timer.h"

#ifdef BUILD_GUI
#include <QApplication>
#include "../AcquisitionApp/src/acquisitionwidget.h"
#include "gui/mainwindow.h"
#endif

int main(int argc, char** argv) {
    Timer t;
    Analyzer analyzer;

    //// TEMP PLACE FOR TEST ////
    // Create 20 random containers, each with 20 objects ERROR: THOSE VALUES MUST BE EQUAL?
    //double randomNumber;
    //int numberOfContainers = 20;
    //int numberOfObjects = 20;
    //for (int i = 0; i < numberOfContainers; i++) {
    //    analyzer.m_Experiment.data.emplace_back(new DataContainer(0x3ffff));
    //    for (int j = 0; j < numberOfObjects; j++) {
    //        analyzer.m_Experiment.data[i]->appendNew();
    //        randomNumber = 1 + ( std::rand() % ( 100 - 1 + 1 ) );
    //        (*analyzer.m_Experiment.data[i])[j]->setValue(data::Area, randomNumber);
    //        randomNumber = 1 + ( std::rand() % ( 100 - 1 + 1 ) );
    //        (*analyzer.m_Experiment.data[i])[j]->setValue(data::ConvexArea, randomNumber);
    //        randomNumber = 1 + ( std::rand() % ( 100 - 1 + 1 ) );
    //        (*analyzer.m_Experiment.data[i])[j]->setValue(data::Circularity, randomNumber);
    //        randomNumber = 1 + ( std::rand() % ( 100 - 1 + 1 ) );
    //        (*analyzer.m_Experiment.data[i])[j]->setValue(data::Eccentricity, randomNumber);
    //        randomNumber = 1 + ( std::rand() % ( 100 - 1 + 1 ) );
    //        (*analyzer.m_Experiment.data[i])[j]->setValue(data::GradientScore, randomNumber);
    //        randomNumber = 1 + ( std::rand() % ( 100 - 1 + 1 ) );
    //        (*analyzer.m_Experiment.data[i])[j]->setValue(data::Major_axis, randomNumber);
    //        randomNumber = 1 + ( std::rand() % ( 100 - 1 + 1 ) );
    //        (*analyzer.m_Experiment.data[i])[j]->setValue(data::Minor_axis, randomNumber);
    //        randomNumber = 1 + ( std::rand() % ( 100 - 1 + 1 ) );
    //        (*analyzer.m_Experiment.data[i])[j]->setValue(data::Solidity, randomNumber);
    //        randomNumber = 1 + ( std::rand() % ( 100 - 1 + 1 ) );
    //        (*analyzer.m_Experiment.data[i])[j]->setValue(data::Symmetry, randomNumber);
    //        randomNumber = 1 + ( std::rand() % ( 100 - 1 + 1 ) );
    //        (*analyzer.m_Experiment.data[i])[j]->setValue(data::Perimeter, randomNumber);
    //        (*analyzer.m_Experiment.data[i])[j]->setValue(data::OutputValue, (i%2)*1.0);
    //    }
    //}
    //analyzer.exportExperiment();

#ifdef BUILD_GUI




    Q_INIT_RESOURCE(icons);
    QApplication a(argc, argv);

    MainWindow window(&analyzer);
    window.show();

    return a.exec();

#endif
#ifndef BUILD_GUI

    // Get path to pictures
    analyzer.loadExperimentPreset("../../../data/ImgD1/");

    t.tic();
    // Loop through all pictures to reject or accept
    std::cout << "Load images" << std::endl;
    analyzer.loadImagesFromFolder();
    t.toc();

    t.tic();
    // Select Background
    std::cout << "Select background" << std::endl;
    analyzer.selectBG();
    t.toc();

    t.tic();
    // Load processes preset
    std::cout << "Load RBC Preset" << std::endl;
    analyzer.loadRBCPreset();
    t.toc();

    t.tic();
    std::cout << "Run processes on accepted frames" << std::endl;
    Frame f;
    for (const Frame& frame : analyzer.m_Experiment.acc) {
        analyzer.m_img = frame.image;
        analyzer.runProcesses();

        f.filename = frame.filename;
        f.id = frame.id;
        f.image = analyzer.m_img;
        f.accepted = true;

        analyzer.m_Experiment.processed.push_back(f);
    }
    t.toc();
    return 0;
#endif
}
