/*!
  @author Shin'ichiro Nakaoka
*/

#include <cnoid/PySignal>
#include "../ToolBar.h"
#include "../TimeBar.h"

using namespace boost::python;
using namespace cnoid;

namespace {

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(TimeBar_stopPlayback_overloads, stopPlayback, 0, 1)

}

void exportToolBars()
{
    ToolButton* (ToolBar::*ToolBar_addButton1)(const QString& text, const QString& tooltip) = &ToolBar::addButton;
    ToolButton* (ToolBar::*ToolBar_addButton2)(const QIcon& icon, const QString& tooltip) = &ToolBar::addButton;
    ToolButton* (ToolBar::*ToolBar_addButton3)(const char* const* xpm, const QString& tooltip) = &ToolBar::addButton;
    ToolButton* (ToolBar::*ToolBar_addToggleButton1)(const QString& text, const QString& tooltip) = &ToolBar::addToggleButton;
    ToolButton* (ToolBar::*ToolBar_addToggleButton2)(const QIcon& icon, const QString& tooltip) = &ToolBar::addToggleButton;
    ToolButton* (ToolBar::*ToolBar_addToggleButton3)(const char* const* xpm, const QString& tooltip) = &ToolBar::addToggleButton;
    ToolButton* (ToolBar::*ToolBar_addRadioButton1)(const QString& text, const QString& tooltip) = &ToolBar::addRadioButton;
    ToolButton* (ToolBar::*ToolBar_addRadioButton2)(const QIcon& icon, const QString& tooltip) = &ToolBar::addRadioButton;
    ToolButton* (ToolBar::*ToolBar_addRadioButton3)(const char* const* xpm, const QString& tooltip) = &ToolBar::addRadioButton;

    //class_ < ToolBarArea, boost::noncopyable >("ToolBarArea", no_init);

    class_<ToolBar, bases<QWidget>, boost::noncopyable>("Toolbar", no_init)
        .def("addButton", ToolBar_addButton1, return_value_policy<reference_existing_object>())
        .def("addButton", ToolBar_addButton2, return_value_policy<reference_existing_object>())
        .def("addButton", ToolBar_addButton3, return_value_policy<reference_existing_object>())
        .def("addToggleButton", ToolBar_addToggleButton1, return_value_policy<reference_existing_object>())
        .def("addToggleButton", ToolBar_addToggleButton2, return_value_policy<reference_existing_object>())
        .def("addToggleButton", ToolBar_addToggleButton3, return_value_policy<reference_existing_object>())
        .def("requestNewRadioGroup", &ToolBar::requestNewRadioGroup)
        //.def("currentRadioGroup", &ToolBar::currentRadioGroup, return_value_policy<reference_existing_object>())
        .def("addRadioButton", ToolBar_addRadioButton1, return_value_policy<reference_existing_object>())
        .def("addRadioButton", ToolBar_addRadioButton2, return_value_policy<reference_existing_object>())
        .def("addRadioButton", ToolBar_addRadioButton3, return_value_policy<reference_existing_object>())
        //.def("addAction", &ToolBar::addAction)
        .def("addWidget", &ToolBar::addWidget)
        //.def("addLabel", &ToolBar::addLabel, return_value_policy<reference_existing_object>())
        //.def("addImage", &ToolBar::addImage, return_value_policy<reference_existing_object>())
        .def("addSeparator", &ToolBar::addSeparator, return_value_policy<reference_existing_object>())
        .def("addSpacing", &ToolBar::addSpacing)
        .def("setVisibleByDefault", &ToolBar::setVisibleByDefault)
        .def("isVisibleByDefault", &ToolBar::isVisibleByDefault)
        .def("setStretchable", &ToolBar::setStretchable)
        .def("isStretchable", &ToolBar::isStretchable, return_value_policy<return_by_value>())
        //.def("toolBarArea", &ToolBar::toolBarArea, return_value_policy<reference_existing_object>());
        ;

    {
        scope timeBarScope =
            class_<TimeBar, bases<ToolBar>, boost::noncopyable>("TimeBar", no_init)
            .def("instance", &TimeBar::instance, return_value_policy<reference_existing_object>()).staticmethod("instance")
            .def("sigPlaybackInitialized", &TimeBar::sigPlaybackInitialized)
            .def("sigPlaybackStarted", &TimeBar::sigPlaybackStarted)
            .def("sigTimeChanged", &TimeBar::sigTimeChanged)
            .def("sigPlaybackStopped", &TimeBar::sigPlaybackStopped)        
            .def("time", &TimeBar::time)
            .def("setTime", &TimeBar::setTime)
            .def("realPlaybackTime", &TimeBar::realPlaybackTime)
            .def("minTime", &TimeBar::minTime)
            .def("maxTime", &TimeBar::maxTime)
            .def("setTimeRange", &TimeBar::setTimeRange)
            .def("frameRate", &TimeBar::frameRate)
            .def("setFrameRate", &TimeBar::setFrameRate)
            .def("timeStep", &TimeBar::timeStep)
            .def("playbackSpeedScale", &TimeBar::playbackSpeedScale)
            .def("setPlaybackSpeedScale", &TimeBar::setPlaybackSpeedScale)
            .def("playbackFrameRate", &TimeBar::playbackFrameRate)
            .def("setPlaybackFrameRate", &TimeBar::setPlaybackFrameRate)
            .def("setRepeatMode", &TimeBar::setRepeatMode)
            .def("startPlayback", &TimeBar::startPlayback)
            .def("startPlaybackFromFillLevel", &TimeBar::startPlaybackFromFillLevel)
            .def("stopPlayback", &TimeBar::stopPlayback, TimeBar_stopPlayback_overloads())
            .def("isDoingPlayback", &TimeBar::isDoingPlayback)
            .def("startFillLevelUpdate", &TimeBar::startFillLevelUpdate)
            .def("updateFillLevel", &TimeBar::updateFillLevel)
            .def("stopFillLevelUpdate", &TimeBar::stopFillLevelUpdate)
            .def("setFillLevelSync", &TimeBar::setFillLevelSync)
            ;

        PySignalProxy<bool(double time), TimeBar::LogicalProduct>("SigPlaybackInitialized");
        PySignalProxy<bool(double time), TimeBar::LogicalSum>("SigTimeChanged");
    }
}
