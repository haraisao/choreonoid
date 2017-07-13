/*!
  @author Shin'ichiro Nakaoka
*/

#include "../Plugin.h"
#include "../PluginManager.h"
#include "../MenuManager.h"
#include "../Action.h"
#include <boost/python.hpp>

using namespace boost::python;
using namespace cnoid;

namespace {

bool PluginManager_unloadPlugin2(PluginManager& self, const std::string& name) { return self.unloadPlugin(name); }

}

namespace cnoid {

void exportPyPluginManager()
{
	boost::python::import("cnoid.QtCore");
	boost::python::import("cnoid.QtGui");

    class_<PluginManager, PluginManager*, boost::noncopyable>("PluginManager", no_init)
        .def("instance", &PluginManager::instance, return_value_policy<reference_existing_object>()).staticmethod("instance")
        .def("unloadPlugin", PluginManager_unloadPlugin2)
        .def("reloadPlugin", &PluginManager::reloadPlugin)
		.def("numPlugins", &PluginManager::numPlugins)
		.def("findPlugin", &PluginManager::findPlugin, return_internal_reference<>())
		.def("pluginPath", &PluginManager::pluginPath, return_value_policy<copy_const_reference>())
		.def("pluginName", &PluginManager::pluginName, return_value_policy<copy_const_reference>())
		.def("pluginStatus", &PluginManager::pluginStatus)
        ;

	class_<ExtensionManager, ExtensionManager*, boost::noncopyable>("ExtensionManager", no_init)
		.def("name", &ExtensionManager::name, return_value_policy<copy_const_reference>())
		.def("notifySystemUpdate", &ExtensionManager::notifySystemUpdate)
		//		.def("itemManager", &ExtensionManager::itemManager, return_internal_reference<>())
		//		.def("timeSyncItemEngineManger", &ExtensionManager::timeSyncItemEngineManger, return_internal_reference<>())
		//		.def("viewManager", &ExtensionManager::viewManager, return_internal_reference<>())
		.def("menuManager", &ExtensionManager::menuManager, return_internal_reference<>())
		//		.def("optionManager", &ExtensionManager::optionManager, return_internal_reference<>())
		;

	class_<Plugin, bases<ExtensionManager>, boost::noncopyable>("Plugin", no_init)
		.def("name", &Plugin::name, return_value_policy<return_by_value>())
		.def("numRequisites", &Plugin::numRequisites)
		;

	class_<MenuManager, boost::noncopyable>("MenuManager", no_init)
		.def("setPath", &MenuManager::setPath, return_internal_reference<>())
		.def("numItems", &MenuManager::numItems)
		.def("addAction", &MenuManager::addAction)
		.def("addItem", (Action*(MenuManager::*)(const QString&))&MenuManager::addItem, return_internal_reference<>())
		;

	class_<Action, Action*, bases<QAction>, boost::noncopyable>("Action", no_init)
		.def("sigTriggered", &Action::sigTriggered, return_internal_reference<>())
		.def("sigToggled", &Action::sigToggled, return_internal_reference<>())
		;
}

}
