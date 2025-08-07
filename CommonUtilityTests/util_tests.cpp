#define BOOST_TEST_MODULE CommonUtilityTests
#define BOOST_ALL_DYN_LINK
#include "doobius/common/notif_registry.h"
#include <boost/test/unit_test.hpp>

namespace Notif = Doobius::Notification;
using NReg = Notif::NotificationRegistry;
using US = NReg::UpdateStatus;


struct LoggingFixture {
	LoggingFixture() {
		BOOST_LOG_NAMED_SCOPE("CommonUtilityLogFixture");
		std::filesystem::path testDir = "CommonUtilTestLogs";
		std::filesystem::path testDirFull = DOOBIUS_TEST_EXE_DIR / testDir;
		DOOBIUS_LOG_MNG().initLogging(testDirFull);
	}
	~LoggingFixture() = default;
};

BOOST_TEST_GLOBAL_FIXTURE(LoggingFixture);

class NotifRegTestClass {
private:
	int m_val;

	void updateVal(const int& cnt) {
		m_val += cnt;
	}

public:
	NotifRegTestClass() : m_val{ 0 } {}
	int getVal() const {
		return m_val;
	}

	void regCb(NReg& nReg) {
		nReg.registerBoundCallback<int>(this, &NotifRegTestClass::updateVal, "TestUpdateCb");
		nReg.registerCallbackToChannel("TestUpdateCb", "TestUpdateChannel");
	}
};

BOOST_AUTO_TEST_CASE(NotifRegTests)
{
	// BOOST_TEST(true);
	Doobius::Notification::NotificationRegistry nReg("NotifTestRegistry");

	int cb1Active = 0;
	int cb2Active = 0;
	int cb3Active = 0;

	nReg.createNotificationChannel("c1");
	nReg.createNotificationChannel("c2");
	nReg.createNotificationChannel("c3");

	BOOST_TEST(nReg.getNumChannelsRegistered() == 3);

	nReg.registerCallback<int>([&cb1Active](const int& cnt) { cb1Active += cnt; }, "cb1");
	nReg.registerCallback<int>([&cb2Active](const int& cnt) { cb2Active += cnt; }, "cb2");
	nReg.registerCallback<int>([&cb3Active](const int& cnt) { cb3Active += cnt; }, "cb3");

	BOOST_TEST(nReg.getNumCbsRegistered() == 3);

	nReg.registerCallbackToChannel("cb1", "c1");
	nReg.registerCallbackToChannel("cb1", "c2");
	nReg.registerCallbackToChannel("cb1", "c3");

	nReg.registerCallbackToChannel("cb2", "c1");
	nReg.registerCallbackToChannel("cb2", "c2");

	nReg.registerCallbackToChannel("cb3", "c1");

	BOOST_TEST(nReg.getNumCbsListeningTo("c1") == 3);
	BOOST_TEST(nReg.getNumCbsListeningTo("c2") == 2);
	BOOST_TEST(nReg.getNumCbsListeningTo("c3") == 1);

	BOOST_TEST(nReg.getNumChannelsListenedBy("cb1") == 3);
	BOOST_TEST(nReg.getNumChannelsListenedBy("cb2") == 2);
	BOOST_TEST(nReg.getNumChannelsListenedBy("cb3") == 1);

	int tmpCnt = 1;
	nReg.updateChannel("c1", tmpCnt);

	BOOST_TEST(cb1Active == 1);
	BOOST_TEST(cb2Active == 1);
	BOOST_TEST(cb3Active == 1);

	nReg.updateChannel("c2", tmpCnt);

	BOOST_TEST(cb1Active == 2);
	BOOST_TEST(cb2Active == 2);

	nReg.updateChannel("c3", tmpCnt);

	BOOST_TEST(cb1Active == 3);

	nReg.unsubCallbackFromChannel("cb1", "c1");
	BOOST_TEST(nReg.getNumCbsListeningTo("c1") == 2);
	BOOST_TEST(nReg.getNumChannelsListenedBy("cb1") == 2);

	nReg.unsubCallbackFromAllChannels("cb3");

	BOOST_TEST(nReg.getNumCbsListeningTo("c1") == 1);
	BOOST_TEST(nReg.getNumChannelsListenedBy("cb3") == 0);

	nReg.unsubAllCallbacksFromChannel("c2");

	BOOST_TEST(nReg.getNumCbsListeningTo("c2") == 0);
	BOOST_TEST(nReg.getNumChannelsListenedBy("cb1") == 1);
	BOOST_TEST(nReg.getNumChannelsListenedBy("cb2") == 1);

	nReg.destroyChannel("c3");
	auto updateRes = nReg.updateChannel("c3", tmpCnt);

	BOOST_TEST(updateRes == US::UPDATE_CHANNEL_MISSING);
	BOOST_TEST(nReg.getNumChannelsRegistered() == 2);

	nReg.removeCallback("cb1");

	BOOST_TEST(nReg.getNumCbsRegistered() == 2);

	// Bound callback check

	nReg.createNotificationChannel("TestUpdateChannel");
	NotifRegTestClass myTestInst;
	myTestInst.regCb(nReg);
	nReg.updateChannel("TestUpdateChannel", tmpCnt);

	BOOST_TEST(myTestInst.getVal() == 1);

}