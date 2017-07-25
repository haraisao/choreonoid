/**
   @author Isao Hara
*/

#include "SimpleGraphicsPlugin.h"

using namespace std;
using namespace cnoid;

SimpleGraphicsPlugin::SimpleGraphicsPlugin()
	: Plugin("SimpleGraphics")
{
}

 bool SimpleGraphicsPlugin::initialize()
{
        Action* menuItem = menuManager().setPath("/View").addItem("SimpleGraphics");
        menuItem->sigTriggered().connect(bind(&SimpleGraphicsPlugin::onSimpleGraphicsTriggered, this));

		SimpleGraphicsView::initializeClass(this);

		this->setUnloadable(true);

        return true;
 }

 bool SimpleGraphicsPlugin::finalize(){
		return menuManager().setPath("/View").removeItem("SimpleGraphics");
}

void SimpleGraphicsPlugin::onSimpleGraphicsTriggered()
    {
        MessageView::instance()->putln("Hello SimpleGraphics !");
    }

CNOID_IMPLEMENT_PLUGIN_ENTRY(SimpleGraphicsPlugin)
