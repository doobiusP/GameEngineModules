#include "doobius/core/root.h"

namespace Doobius {
	Root::Root() : m_setup{ false }, rootNotifReg{ "RootNotifReg" } {
		BOOST_LOG_NAMED_SCOPE("RootInit");
		DOOBIUS_CLOG(info) << "Doobius root startup complete";
	}

	Root::~Root()
	{
		BOOST_LOG_NAMED_SCOPE("RootEnd");
		DOOBIUS_CLOG(info) << "Doobius root shutdown complete";
	}

	void Root::init(const DoobiusRootConfig& rootConfig)
	{
		BOOST_LOG_NAMED_SCOPE("RootInit");
		if (m_setup) {
			DOOBIUS_CLOG(warning) << "Root has already been initialized before";
			return;
		}

		m_setup = true;
	}

	Root& Root::get() {
		static Root _root;
		return _root;
	}
}
