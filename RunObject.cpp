#include "RunObject.hpp"
#include <QtCore/qdebug.h>
#include <QtCore/qstring.h>
#include <QtCore/qthread.h>
#include <QtCore/qcoreapplication.h>
#include <QtCore/qabstracteventdispatcher.h>
#include <thread>
#include <future>
#include <chrono>
#include <atomic>

using namespace std::chrono_literals;

namespace {
	class ThisRunObject : public RunObject {
	public:
		std::atomic<bool> _private_is_quit{ false };
		std::shared_ptr<ThisRunObject> _private_tmp_runobject;
		RunObjectThread * getThread() const { return static_cast<RunObjectThread *>(this->thread()); }

		constexpr static const char * _private_name() { return  "\xF1" "\xF0" "\xA1" "sstd::" "watcher" "name"; }

		void addWatcher(QObject * obj) const override {
			if (obj == nullptr) { return; }
			if (dynamic_cast<RunObject*>(obj)) { return; }
			obj->setProperty(_private_name(), QVariant::fromValue<std::shared_ptr<RunObject>>(getWatcher()));
		}

		void removeWatcher(QObject *obj) const override {
			if (obj == nullptr) { return; }
			if (dynamic_cast<RunObject*>(obj)) { return; }
			obj->setProperty(_private_name(), QVariant::fromValue<std::shared_ptr<RunObject>>({}));
		}

		std::shared_ptr<RunObject> getWatcher() const override {
			return getThread()->getRunObject();
		}

	};


}/*namespace*/

class RunObjectThread::RunObjectThreadPrivate {
public:
	RunObjectThreadPrivate() {}
	std::promise<ThisRunObject *> * _private_runobjec_promise = nullptr;
	std::weak_ptr<RunObject> _private_runobject;
};

RunObjectThread * RunObjectThread::createThread() {
	auto * varAns = new RunObjectThread;
	varAns->moveToThread(qApp->thread()) /*移动到主线程*/;
	varAns->connect(varAns, &QThread::finished, qApp, &QThread::deleteLater, Qt::QueuedConnection) /*线程完成自动销毁*/;
	{
		std::promise<ThisRunObject *> varPromise;
		varAns->_private_data->_private_runobjec_promise = &varPromise;
		auto varFuture = varPromise.get_future();
		varAns->start() /*启动线程*/;
		varAns->_private_data->_private_runobject = varFuture.get()->_private_tmp_runobject;
	}
	return varAns;
}

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

RunObjectThread::~RunObjectThread() {
	delete _private_data;
}

RunObjectThread::RunObjectThread(QObject * a) :QThread(a) {
	_private_data = new RunObjectThreadPrivate;
}

void RunObjectThread::run() {
	std::unique_ptr<ThisRunObject> varRunObject_{ new ThisRunObject };
	{/*init varRunObject*/
		auto varLocker = new QEventLoopLocker(this);
		varRunObject_->_private_tmp_runobject = std::shared_ptr<ThisRunObject>(
			varRunObject_.get(), [varLocker](ThisRunObject *) {delete varLocker; });
	}
	std::weak_ptr<ThisRunObject> varRunObjectWeak = varRunObject_->_private_tmp_runobject;
	{
		_private_data->_private_runobjec_promise->set_value(varRunObject_.get());
		_private_data->_private_runobjec_promise = nullptr;
	}
	this->exec();
	varRunObject_->_private_is_quit.store(true);
	try {
		auto varRunObject = varRunObjectWeak.lock();
		if (varRunObject) {
			while (varRunObject.use_count() > 1) {
				this->eventDispatcher()->processEvents(QEventLoop::AllEvents);
				std::this_thread::sleep_for(1s);
			}
		}
	}
	catch (...) {}
}

std::shared_ptr<RunObject> RunObjectThread::getRunObject() {
	try {
		auto varAns = _private_data->_private_runobject.lock();
		if (varAns) {
			auto varPtr = static_cast<ThisRunObject*>(varAns.get());
			if (varPtr->_private_is_quit.load())return {};
			return std::move(varAns);
		}
	}
	catch (...) {}
	return {};
}

std::shared_ptr<RunObject> RunObject::createObject() {
	auto * varThread = RunObjectThread::createThread();
	std::shared_ptr<RunObject> varAns = varThread->getRunObject();
	{
		static_cast<ThisRunObject *>(varAns.get())->_private_tmp_runobject = {};
	}
	varAns->connect(varAns.get(), &RunObject::quitThread, varThread, &QThread::quit, Qt::QueuedConnection);
	varAns->connect(qApp, &QCoreApplication::aboutToQuit, varAns.get(), &RunObject::quitThread, Qt::QueuedConnection);
	return std::move(varAns);
}



