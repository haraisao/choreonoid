/**
   @author Isao Hara
*/
#ifndef CNOID_SIMPLE_GRAPHICS_PLUGIN_H_INCLUDED
#define CNOID_SIMPLE_GRAPHICS_PLUGIN_H_INCLUDED

#include "SimpleGraphicsView.h"
#include <cnoid/Plugin>
#include <cnoid/MenuManager>
#include <cnoid/MessageView>

using namespace std;
using namespace cnoid;

namespace cnoid
{
	class SimpleGraphicsPlugin : public Plugin
	{
	public:

		SimpleGraphicsPlugin();

		virtual bool initialize();

		virtual bool finalize();

	private:
		void onSimpleGraphicsTriggered();

	};
}
#endif