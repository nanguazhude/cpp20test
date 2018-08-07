#include "include_all.hpp"

#include "MainWindow.hpp"
#include <QApplication>

std::future<void> goo() {
	std::cout << __func__ << std::endl;
	std::this_thread::sleep_for(1s);
	co_return;
}

std::future<void> goo1() {
	std::cout << __func__ << std::endl;
	co_return;
}

std::future<void> foo() {
	co_await goo();
	co_await goo1();
	std::cout << __func__ << std::endl;
	co_return;
}

class RunObject : public QObject {
	unsigned int runcount = 0;
public:
	void timerEvent(QTimerEvent *event) override {
		std::cout << "t,";
		++runcount;
		if (runcount == 10) {
			QTimer::singleShot(1ms, this, []() {foo(); });
		}
		return QObject::timerEvent(event);
	}
};

class Thread : public QThread {
public:
	void run() override {
		RunObject obj;
		obj.startTimer(10ms);
		promise.set_value(&obj);
		exec();
	}
	std::promise<RunObject *> promise;
};

class One final :public QObject, public std::enable_shared_from_this<One> {
private:
	One() { }
public:
	static std::shared_ptr<One> instance() { return std::shared_ptr<One>{ new One }; }
};

int main(int argc, char *argv[]) {
	QApplication app(argc, argv);

	auto thread = new Thread;
	thread->start();
	thread->promise.get_future().get();

	auto t = One::instance();
	QObject::connect(t.get(), &QObject::destroyed, t.get(), []() {}, Qt::QueuedConnection);
	
	MainWindow window;
	window.show();

	return app.exec();
}


