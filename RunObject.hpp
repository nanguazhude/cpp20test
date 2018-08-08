#ifndef RUNOBJECT_HPP
#define RUNOBJECT_HPP

#include <memory>
#include <functional>
#include <QtCore/qobject.h>
#include <QtCore/qvariant.h>
#include <QtCore/qthread.h>

class RunObject;
class RunObjectThread;

class RunObjectThread : public QThread {
	Q_OBJECT
public:
	~RunObjectThread();
	std::shared_ptr<RunObject> getRunObject();
public:
	RunObjectThread(const RunObjectThread &) = delete;
	RunObjectThread(RunObjectThread &&) = delete;
	RunObjectThread&operator=(const RunObjectThread &) = delete;
	RunObjectThread&operator=(RunObjectThread &&) = delete;
private:
	void run() override;
	class RunObjectThreadPrivate;
	RunObjectThreadPrivate * _private_data = nullptr;
	static RunObjectThread * createThread();
	friend class RunObject;
	RunObjectThread(QObject * /**/ = nullptr);
};

class RunObject : public QObject {
	Q_OBJECT
public:
	RunObject(QObject * /**/ = nullptr);
	static std::shared_ptr<RunObject> createObject();
	virtual void addWatcher(QObject *)    const =0;
	virtual void removeWatcher(QObject *) const =0;
	virtual std::shared_ptr<RunObject> getWatcher() const =0;
public:
signals:
	void runFunction(const std::function<void(void)> &)/*DirectConnection*/;
	void asyncRunFunction(const std::function<void(void)> &)/*QueuedConnection*/;
	void blockRunFunction(const std::function<void(void)> &)/*BlockingQueuedConnection*/;
	void _private_blockRunFunction(const std::function<void(void)> &, QPrivateSignal)/*BlockingQueuedConnection*/;
private slots:
	void _private_runFunction(const std::function<void(void)> &);
	void _private_runWaitFunction(const std::function<void(void)> &);
};

Q_DECLARE_METATYPE(std::function<void(void)>)
Q_DECLARE_METATYPE(std::shared_ptr<RunObject>)

#endif // RUNOBJECT_HPP
