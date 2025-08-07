#pragma once
#include "doobius/dbg/custom_assert.h"
#include "doobius/common/notif_registry.h"

#define DOOBIUS_ROOT() Doobius::Root::get()

namespace Doobius {
	using namespace Doobius::Notification;
	class Root {
	private:
		Root();
		~Root();

		bool m_setup;
	public:
		struct DoobiusRootConfig {
		};

		NotificationRegistry rootNotifReg;

		Root(const Root&) = delete;
		Root(Root&&) = delete;
		Root& operator=(const Root&) = delete;
		Root& operator=(Root&&) = delete;

		void init(const DoobiusRootConfig& rootConfig);
		static Root& get();
	};
}
