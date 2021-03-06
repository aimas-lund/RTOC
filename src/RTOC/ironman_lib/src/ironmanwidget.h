#ifndef IRONMANWIDGET_H
#define IRONMANWIDGET_H

#include "../acquisitor_src/acquisitor.h"

#include "cameradisplayerwidget.h"
#include "logger.h"

#include <QTimer>
#include <QWidget>

#include "boost/serialization/nvp.hpp"
#include "boost/serialization/serialization.hpp"

#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>

namespace Ui {
class IronManWidget;
}

class IronManWidget : public QWidget {
    Q_OBJECT

public:
    explicit IronManWidget(QWidget* parent = 0);
    ~IronManWidget();

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) const;

private slots:
    void on_actionExit_triggered();
    void on_clearLog_clicked();

    void on_scale_currentIndexChanged(const QString& arg1);

    void on_spinBox_valueChanged(int arg1);

    void on_txtPathButton_clicked();
    void on_xmlPathButton_clicked();

private:
    CameraDisplayerWidget m_imageDisplayer;

    Ui::IronManWidget* m_ui;
    Logger* m_logger;

private slots:
    void initializeFramegrabber();
    void acqStateChanged(AcqState state);
    void setButtonStates(AcqState state);

private:
    // The acquisitor object is initialized to reside as a permanent object in a separate thread.
    // All communication is facilitated through signals/slots through QMetaObject::InvokeMethod
};

#endif  // IRONMANWIDGET_H
