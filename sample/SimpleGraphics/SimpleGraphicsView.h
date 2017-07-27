/**
   @author Isao Hara
*/

#ifndef CNOID_SIMPLE_GRAPHICS_VIEW_H_INCLUDED
#define CNOID_SIMPLE_GRAPHICS_VIEW_H_INCLUDED

#include <cnoid/View>
#include <cnoid/ViewManager>
#include <cnoid/MenuManager>
#include <cnoid/MessageView>
#include <cnoid/ConnectionSet>
#include <cnoid/Dialog>
#include <cnoid/ComboBox>
#include <cnoid/Timer>
#include <cnoid/ItemList>
#include <cnoid/ItemTreeView>
#include <cnoid/RootItem>
#include <cnoid/SimulatorItem>
#include <QGraphicsView>
#include <QVBoxLayout>
#include <QDropEvent>
#include <QGraphicsItemGroup>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsEffect>
#include <QLabel>
#include <QLineEdit>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsEffect>
#include <QStatusBar>

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#define USE_QT5
#endif

using namespace std;
using namespace cnoid;

namespace cnoid 
{
	class SimpleGraphicsViewImpl;
	/*

	*/
	class SimpleGraphicsView : public View
	{
		Q_OBJECT

	public:
		static void initializeClass(ExtensionManager* ext);

		SimpleGraphicsView();
		virtual	~SimpleGraphicsView();

		void updateView();

		public Q_SLOTS:
		void onSimpleGraphicsTriggered();

	protected:
		/*
		virtual void onActivated() override;
		virtual void onDeactivated() override;
		virtual bool storeState(Archive& archive) override;
		virtual bool restoreState(const Archive& archive) override;
		*/
	private:
		SimpleGraphicsViewImpl* impl = NULL;

	};

    /*

     */
	class SimpleGraphicsViewImpl : public QGraphicsView
	{

	public:
    	SimpleGraphicsView* self;
		QGraphicsScene scene;
		MenuManager  menuManager;
		Timer timer;
		QGraphicsLineItem* dragPortLine;

    	SimpleGraphicsViewImpl(SimpleGraphicsView* self);
    	~SimpleGraphicsViewImpl();

    	void dragEnterEvent   (QDragEnterEvent* event);
    	void dragMoveEvent    (QDragMoveEvent*  event);
    	void dragLeaveEvent   (QDragLeaveEvent* event);
    	void dropEvent        (QDropEvent*      event);
    	void mouseMoveEvent   (QMouseEvent*     event);
   		void mousePressEvent  (QMouseEvent*     event);
    	void mouseReleaseEvent(QMouseEvent*     event);
    	void keyPressEvent    (QKeyEvent*       event);
//    	void onnsViewItemSelectionChanged(const list<NamingContextHelper::ObjectInfo>& items);

    	void onTime();
    	void onActivated(bool on);
    	void updateView();
	};

}
#endif
