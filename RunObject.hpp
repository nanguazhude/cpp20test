#ifndef RUNOBJECT_HPP
#define RUNOBJECT_HPP

#include <memory>
#include <functional>
#include <QtCore/qobject.h>
#include <QtCore/qvariant.h>

class RunObject : public QObject {
	Q_OBJECT
public:
	RunObject(QObject * /**/ = nullptr);
public:
signals:
	void runFunction(const std::function<void(void)> &)/*DirectConnection*/;
	void asyncRunFunction(const std::function<void(void)> &)/*QueuedConnection*/;
    void blockRunFunction(const std::function<void(void)> &)/*BlockingQueuedConnection*/;
	void _private_blockRunFunction(const std::function<void(void)> &,QPrivateSignal)/*BlockingQueuedConnection*/;
private slots:
	void _private_runFunction(const std::function<void(void)> &);
	void _private_runWaitFunction(const std::function<void(void)> &);
};

Q_DECLARE_METATYPE(std::function<void(void)>)

#endif // RUNOBJECT_HPP
