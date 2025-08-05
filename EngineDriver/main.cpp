#include "dbg/custom_assert.h"
#include "common/code_timer.h"
#include "common/observer.h"
using namespace CommonUtil::Notification;

NotificationManager notifMgr;

class RandomClass {
public:
	void someFunc(const float& myVal) {
		DOOBIUS_CLOG(info) << "This callback got called with value = " << myVal;
	}

	void regNotif() {
		NotificationCallback randomCb = notifMgr.makeCallback<float>(
			"RandomNotifiee",
			[this](const float& val) { this->someFunc(val); }
		);

		notifMgr.registerCbForNotif<float>("MyRandomFloat", randomCb);
	}
};

int main(int argc, char* argv[]) {
	BOOST_LOG_NAMED_SCOPE("EntryPoint");
	// TODO: Do I need to pass $(TargetPath) here instead?

	std::filesystem::path logDir = std::filesystem::absolute(argv[0]).parent_path() / "EngineDriverLogs";
	Dbg::Log::initLogging(logDir);
	CommonUtil::Perf::CodeTimer mainFunc("mainFunc");
	RandomClass inst;
	notifMgr.createNotifSource<float>("MyRandomFloat");
	inst.regNotif();

	float myNewVal = 5.0f;
	notifMgr.notify("MyRandomFloat", myNewVal);

	//Notifier<ConfigNotifiee, int> exampleNotifier("ExampleNotifier");
	//ConfigNotifiee exampleNotifiee("ExampleNotifiee");
	//exampleNotifier.addNotifiee(&exampleNotifiee);
	//int x = 5;
	//exampleNotifier.notifyAll(x);
	mainFunc.end();
}
