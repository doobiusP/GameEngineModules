#include "doobius/common/notif_registry.h"

namespace Doobius {
	namespace Notification {
		void NotificationRegistry::unsubCallbackFromChannel(CbId cbId, ChannelId chlId)
		{
			DOOBIUS_FMT_DASSERT(m_chlIdMap.left.find(chlId) != m_chlIdMap.left.end(), "Did not find channel %1% in notification registry %2%", chlId % m_nameOfNotifReg);
			DOOBIUS_FMT_DASSERT(m_cbIdMap.left.find(cbId) != m_cbIdMap.left.end(), "Did not find callback %1% in notification registry %2%", cbId % m_nameOfNotifReg);

			m_registrations.erase(ChannelSubscription(cbId, chlId));
			const CbStr& cbName = m_cbIdMap.left.at(cbId);
			const ChannelStr& chlName = m_chlIdMap.left.at(chlId);

			DOOBIUS_CLOG(info) << cbName << " stopped listening to " << chlName << " in " << m_nameOfNotifReg;
		}

		NotificationRegistry::NotificationRegistry() : m_cbIdCounter{ m_nullCbId }, m_chlIdCounter{ m_nullChlId }, m_nameOfNotifReg{ "UnknownNotifReg" }
		{
			DOOBIUS_CLOG(info) << m_nameOfNotifReg << " notification registry was created";
		}

		NotificationRegistry::NotificationRegistry(const std::string& nameOfNotifReg) : m_cbIdCounter{ m_nullCbId }, m_chlIdCounter{ m_nullChlId }, m_nameOfNotifReg{ nameOfNotifReg }
		{
			DOOBIUS_CLOG(info) << m_nameOfNotifReg << " notification registry was created";
		}

		void NotificationRegistry::createNotificationChannel(const ChannelStr& channelName)
		{
			DOOBIUS_FMT_DASSERT(m_chlIdMap.right.find(channelName) == m_chlIdMap.right.end(), "Found channel %1% already registered in notification registry %2%", channelName % m_nameOfNotifReg);
			m_chlIdMap.insert(ChannelIdMapping::value_type(++m_chlIdCounter, channelName));
			DOOBIUS_CLOG(trace) << channelName << " <-> " << m_chlIdCounter << " : " << m_nameOfNotifReg;
		}


		NotificationRegistry::UpdateStatus NotificationRegistry::registerCallbackToChannel(const CbStr& cbName, const ChannelStr& chlName)
		{
			if (m_cbIdMap.right.find(cbName) == m_cbIdMap.right.end()) {
				DOOBIUS_CLOG(warning) << "Did not find callback " << cbName << " in notification registry " << m_nameOfNotifReg;
				return UpdateStatus::UPDATE_CALLBACK_MISSING;
			}
			if (m_chlIdMap.right.find(chlName) == m_chlIdMap.right.end()) {
				DOOBIUS_CLOG(warning) << "Did not find channel " << chlName << " in notification registry " << m_nameOfNotifReg;
				return UpdateStatus::UPDATE_CHANNEL_MISSING;
			}

			CbId cbId = m_cbIdMap.right.at(cbName);
			ChannelId chlId = m_chlIdMap.right.at(chlName);
			m_registrations.insert(ChannelSubscription(cbId, chlId));

			DOOBIUS_CLOG(info) << cbName << " is now listening to " << chlName << " in " << m_nameOfNotifReg;
			return UpdateStatus::UPDATE_OK;
		}

		NotificationRegistry::UpdateStatus NotificationRegistry::unsubCallbackFromChannel(const CbStr& cbName, const ChannelStr& chlName)
		{
			if (m_cbIdMap.right.find(cbName) == m_cbIdMap.right.end()) {
				DOOBIUS_CLOG(warning) << "Did not find callback " << cbName << " in notification registry " << m_nameOfNotifReg;
				return UpdateStatus::UPDATE_CALLBACK_MISSING;
			}
			if (m_chlIdMap.right.find(chlName) == m_chlIdMap.right.end()) {
				DOOBIUS_CLOG(warning) << "Did not find channel " << chlName << " in notification registry " << m_nameOfNotifReg;
				return UpdateStatus::UPDATE_CHANNEL_MISSING;
			}

			CbId cbId = m_cbIdMap.right.at(cbName);
			ChannelId chlId = m_chlIdMap.right.at(chlName);

			m_registrations.erase(ChannelSubscription(cbId, chlId));
			DOOBIUS_CLOG(info) << cbName << " stopped listening to " << chlName << " in " << m_nameOfNotifReg;

			return UpdateStatus::UPDATE_OK;
		}

		NotificationRegistry::UpdateStatus NotificationRegistry::unsubCallbackFromAllChannels(const CbStr& cbName)
		{
			if (m_cbIdMap.right.find(cbName) == m_cbIdMap.right.end()) {
				DOOBIUS_CLOG(warning) << "Did not find callback " << cbName << " in notification registry " << m_nameOfNotifReg;
				return UpdateStatus::UPDATE_CALLBACK_MISSING;
			}

			CbId cbId = m_cbIdMap.right.at(cbName);
			const std::pair<CbIter, CbIter>& pit = m_registrations.get<CbTag>().equal_range(cbId);
			std::vector<ChannelId> tempChannelIdsToRemove;
			for (CbIter it = pit.first; it != pit.second; ++it) {
				tempChannelIdsToRemove.push_back(it->chlId);
			}

			for (auto chlId : tempChannelIdsToRemove) {
				unsubCallbackFromChannel(cbId, chlId);
			}

			return UpdateStatus::UPDATE_OK;
		}

		NotificationRegistry::UpdateStatus NotificationRegistry::unsubAllCallbacksFromChannel(const ChannelStr& chlName)
		{
			if (m_chlIdMap.right.find(chlName) == m_chlIdMap.right.end()) {
				DOOBIUS_CLOG(warning) << "Did not find channel " << chlName << " in notification registry " << m_nameOfNotifReg;
				return UpdateStatus::UPDATE_CHANNEL_MISSING;
			}

			ChannelId chlId = m_chlIdMap.right.at(chlName);
			const std::pair<ChannelIter, ChannelIter>& pit = m_registrations.get<ChannelTag>().equal_range(chlId);
			std::vector<CbId> tempCbIdsToRemove;

			for (ChannelIter it = pit.first; it != pit.second; ++it) {
				tempCbIdsToRemove.push_back(it->cbId);
			}

			for (auto cbId : tempCbIdsToRemove) {
				unsubCallbackFromChannel(cbId, chlId);
			}
			return UpdateStatus::UPDATE_OK;
		}

		void NotificationRegistry::destroyChannel(const ChannelStr& chlName)
		{
			UpdateStatus unsubRes = unsubAllCallbacksFromChannel(chlName);
			DOOBIUS_FMT_DASSERT(unsubRes == UpdateStatus::UPDATE_OK, "Did not find channel %1% in notification registry %2%", chlName % m_nameOfNotifReg);
			m_chlIdMap.right.erase(chlName);

			DOOBIUS_CLOG(info) << "Channel " << chlName << " destroyed";
		}

		void NotificationRegistry::removeCallback(const CbStr& cbName)
		{
			UpdateStatus unsubRes = unsubCallbackFromAllChannels(cbName);
			DOOBIUS_FMT_DASSERT(unsubRes == UpdateStatus::UPDATE_OK, "Did not find channel %1% in notification registry %2%", cbName % m_nameOfNotifReg);

			CbId cbId = m_cbIdMap.right.at(cbName);
			DOOBIUS_FMT_VERIFY(m_callbackReg.erase(cbId), "Failed to remove callback %1% from callback registry in %2%", cbName % m_nameOfNotifReg);
			m_cbIdMap.right.erase(cbName);

			DOOBIUS_CLOG(info) << "Callback " << cbName << " destroyed";
		}

		int NotificationRegistry::getNumCbsListeningTo(const ChannelStr& chlName) const
		{
			if (m_chlIdMap.right.find(chlName) == m_chlIdMap.right.end()) {
				DOOBIUS_CLOG(warning) << "Did not find channel " << chlName << " in notification registry " << m_nameOfNotifReg;
				return 0;
			}

			ChannelId chlId = m_chlIdMap.right.at(chlName);
			int count = m_registrations.get<ChannelTag>().count(chlId);
			return count;
		}

		int NotificationRegistry::getNumChannelsListenedBy(const CbStr& cbName) const
		{
			if (m_cbIdMap.right.find(cbName) == m_cbIdMap.right.end()) {
				DOOBIUS_CLOG(warning) << "Did not find callback " << cbName << " in notification registry " << m_nameOfNotifReg;
				return 0;
			}

			CbId cbId = m_cbIdMap.right.at(cbName);
			int count = m_registrations.get<CbTag>().count(cbId);
			return count;

		}

		int NotificationRegistry::getNumCbsRegistered() const
		{
			return m_cbIdMap.size();
		}

		int NotificationRegistry::getNumChannelsRegistered() const
		{
			return m_chlIdMap.size();
		}
	}
}