#include "RunObject.hpp"
#include <QtCore/qdebug.h>
#include <QtCore/qstring.h>
#include <QtCore/qthread.h>

RunObject::RunObject(QObject * arg) : QObject(arg) {
	connect(this, &RunObject::runFunction, this, &RunObject::_private_runFunction, Qt::DirectConnection);
	connect(this, &RunObject::asyncRunFunction, this, &RunObject::_private_runFunction, Qt::QueuedConnection);
	connect(this, &RunObject::blockRunFunction, this, &RunObject::_private_runWaitFunction, Qt::DirectConnection);
	connect(this, &RunObject::_private_blockRunFunction, this, &RunObject::_private_runWaitFunction, Qt::BlockingQueuedConnection);
}

void RunObject::_private_runFunction(const std::function<void(void)> &arg) {
	if (arg) {
		try {
			arg();
		}
		catch (...) {
			qDebug() << QStringLiteral("get exceptions but ignored @ RunObject::_private_runFunction");
		}
	}
}

void RunObject::_private_runWaitFunction(const std::function<void(void)> &arg) {
	if (this->thread() == QThread::currentThread()) {
		return _private_runFunction(arg);
	}
	_private_blockRunFunction(arg, {});
}

