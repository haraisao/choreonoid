/*!
  @author Shin'ichiro Nakaoka
*/

#include "../MainWindow.h"
#include "../ExtensionManager.h"
#include "../ToolBar.h"
#include "../ToolBarArea.h"
#include "../ViewArea.h"
#include "../View.h"
#include <cnoid/PyUtil>

using namespace boost::python;
using namespace cnoid;

// for MSVC++2015 Update3
CNOID_PYTHON_DEFINE_GET_POINTER(MainWindow)
CNOID_PYTHON_DEFINE_GET_POINTER(ToolBarArea)
CNOID_PYTHON_DEFINE_GET_POINTER(ViewArea)

namespace cnoid {

void exportPyMainWindow()
{
<<<<<<< HEAD:src/Base/python/PyMainWindow.cpp
	class_<MainWindow, MainWindow*, bases<QMainWindow>, boost::noncopyable>("MainWindow", no_init)
		.def("instance", &MainWindow::instance, return_value_policy<reference_existing_object>()).staticmethod("instance")
		.def("setProjectTitle", &MainWindow::setProjectTitle)
		.def("toolBarArea", &MainWindow::toolBarArea, return_internal_reference<>())
		.def("viewArea", &MainWindow::viewArea, return_internal_reference<>())
		.def("addToolBar", &MainWindow::addToolBar)
		.def("getExtensionManager", &MainWindow::getExtensionManager, return_internal_reference<>())
		;


/**
    class_<MainWindow, MainWindow*, bases<QMainWindow>, boost::noncopyable>("MainWindow", no_init)
        .def("instance", &MainWindow::instance, return_value_policy<reference_existing_object>()).staticmethod("instance")
        .def("setProjectTitle", &MainWindow::setProjectTitle)
        //.def("toolBarArea", &MainWindow::toolBarArea)
        .def("viewArea", &MainWindow::viewArea, return_value_policy<reference_existing_object>())
        .def("addToolBar", &MainWindow::addToolBar);

*/
//>>>>>>> master:src/Base/boostpython/PyMainWindow.cpp
    class_<ToolBarArea, ToolBarArea*, bases<QWidget>, boost::noncopyable>("ToolBarArea", no_init)
        .def("addToolBar", &ToolBarArea::addToolBar)
        .def("removeToolBar", &ToolBarArea::removeToolBar);

//<<<<<<< HEAD:src/Base/python/PyMainWindow.cpp

//>>>>>>> master:src/Base/boostpython/PyMainWindow.cpp
    class_<ViewArea, ViewArea*, bases<QWidget>, boost::noncopyable>("ViewArea", no_init)
        .def("addView", &ViewArea::addView)
        .def("removeView", &ViewArea::removeView)
        .def("numViews", &ViewArea::numViews);
//<<<<<<< HEAD:src/Base/python/PyMainWindow.cpp

}

}
