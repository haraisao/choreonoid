/**
   @author Isao Hara
*/

#include "SimpleGraphicsView.h"

using namespace std;
using namespace cnoid;


/*
  Methods of SimpleGraphicsViewImpl 
*/
SimpleGraphicsViewImpl::SimpleGraphicsViewImpl(SimpleGraphicsView* self)
	:self(self)
{
	self->setDefaultLayoutArea(View::CENTER);

	QVBoxLayout* vbox = new QVBoxLayout();
	this->setScene(&scene);
	vbox->addWidget(this);
	self->setLayout(vbox);
	this->setAcceptDrops(false);
	this->setBackgroundBrush(QBrush(Qt::gray));


}

SimpleGraphicsViewImpl::~SimpleGraphicsViewImpl() {

}

void SimpleGraphicsViewImpl::dragEnterEvent(QDragEnterEvent *event)
{
	return;
}


void SimpleGraphicsViewImpl::dragMoveEvent(QDragMoveEvent *event)
{

	return;
}


void SimpleGraphicsViewImpl::dragLeaveEvent(QDragLeaveEvent *event) 
{
//    MessageView::instance()->putln(_("Drag and drop has been canceled. Please be operation again."));
	return;
}


void SimpleGraphicsViewImpl::dropEvent(QDropEvent *event)
{
	return;
}

/*
void SimpleGraphicsViewImpl::onnsViewItemSelectionChanged(const list<NamingContextHelper::ObjectInfo>& items)
{
	return;
}
*/

void SimpleGraphicsViewImpl::mouseMoveEvent   (QMouseEvent*     event){
	return;
}

void SimpleGraphicsViewImpl::mousePressEvent  (QMouseEvent*     event){
	return;
}

void SimpleGraphicsViewImpl::mouseReleaseEvent(QMouseEvent*     event){
	return;
}

void SimpleGraphicsViewImpl::keyPressEvent    (QKeyEvent*       event){
	return;
}

void SimpleGraphicsViewImpl::onTime(){
	return;
}

void SimpleGraphicsViewImpl::onActivated(bool on){
	return;
}

void SimpleGraphicsViewImpl::updateView(){
	return;
}

/*
  Methods of SimpleGraphicsView 
*/

void SimpleGraphicsView::initializeClass(ExtensionManager* ext){
	// Menu setting etc.
	ext->viewManager().registerClass<SimpleGraphicsView>("SimpleGraphicsView", "SimpleGraphics", ViewManager::SINGLE_OPTIONAL);

	return;
}

SimpleGraphicsView::SimpleGraphicsView(){
	this->impl = new SimpleGraphicsViewImpl(this);
}

SimpleGraphicsView::~SimpleGraphicsView(){
	if(this->impl){
		delete this->impl;
		this->impl=NULL;
	}
}

void SimpleGraphicsView::updateView(){
	this->impl->updateView();
	return;
}

void SimpleGraphicsView::onSimpleGraphicsTriggered() {
	return;
}