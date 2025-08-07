#pragma once
#include <unordered_map> 
#include <unordered_set>

#include <string>
#include <iostream>

#include <boost/function.hpp>
#include <boost/any.hpp>
#include <boost/container/small_vector.hpp>
#include <boost/container/flat_map.hpp>
#include <boost/bimap.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/member.hpp>

#include <type_traits>
#include <typeindex>

#include "doobius/dbg/custom_assert.h"

namespace Doobius {
	namespace Notification {
		namespace cont = boost::container;
		namespace mi = boost::multi_index;
		constexpr int g_numNotifCbsOk = 15;
		constexpr int g_numNotifSrcOk = 5;

		class NotificationRegistry {
		private:
			/**
			 * \brief A callback wrapper containing void<const boost::any&> and a name you can associate with the callback.
			 * Intended for use with the NotificationRegistry. Each callback \b must return void.
			 *
			 */
			struct NotificationCallback {
				std::string notifieeName;
				boost::function<void(const boost::any&)> cb;
			};

			using CbId = unsigned int;
			using ChannelId = unsigned int;
			const CbId m_nullCbId = 0;
			const ChannelId m_nullChlId = 0;

			using CbStr = std::string;
			using ChannelStr = std::string;
			using NotificationCallbackRegistry = cont::small_flat_map<CbId, NotificationCallback, g_numNotifCbsOk>;
			using CallbackIdMapping = boost::bimap<CbId, CbStr>;
			using ChannelIdMapping = boost::bimap<ChannelId, ChannelStr>;

			struct ChannelSubscription {
				CbId cbId;
				ChannelId chlId;

				ChannelSubscription(CbId _cbId, ChannelId _chlId) : cbId(_cbId), chlId(_chlId) {}
				bool operator<(const ChannelSubscription& rhs) const {
					return std::less<CbId>()(cbId, rhs.cbId) || (cbId == rhs.cbId && std::less<ChannelId>()(chlId, rhs.chlId));
				}
			};
			struct ChannelTag {};
			struct CbTag {};

			using SubscriptionMapping = mi::multi_index_container <
				ChannelSubscription,
				mi::indexed_by <
				mi::ordered_unique<
				mi::identity<ChannelSubscription>
				>,
				mi::ordered_non_unique<
				mi::tag<ChannelTag>,
				mi::member<ChannelSubscription, ChannelId, &ChannelSubscription::chlId>
				>,
				mi::ordered_non_unique<
				mi::tag<CbTag>,
				mi::member<ChannelSubscription, CbId, &ChannelSubscription::cbId>
				>
				>
			>;

			using CbIter = SubscriptionMapping::index_iterator<CbTag>::type;
			using ChannelIter = SubscriptionMapping::index_iterator<ChannelTag>::type;

			std::string m_nameOfNotifReg;
			SubscriptionMapping m_registrations;
			NotificationCallbackRegistry m_callbackReg;
			CallbackIdMapping m_cbIdMap;
			ChannelIdMapping m_chlIdMap;
			CbId m_cbIdCounter;
			ChannelId m_chlIdCounter;

			void unsubCallbackFromChannel(CbId cbId, ChannelId chlId);
		public:
			enum class UpdateStatus {
				UPDATE_OK,
				UPDATE_EMPTY,
				UPDATE_CHANNEL_MISSING,
				UPDATE_CALLBACK_MISSING
			};

			NotificationRegistry();
			NotificationRegistry(const std::string& nameOfNotifReg);

			void createNotificationChannel(const ChannelStr& channelName);

			template<typename T, typename CbType>
			void registerCallback(CbType&& cb, const CbStr& cbName);

			template<typename T, typename CbType, typename BindingClass>
			void registerBoundCallback(BindingClass* classInst, CbType&& cb, const CbStr& cbName);

			UpdateStatus registerCallbackToChannel(const CbStr& cbName, const ChannelStr& chlName);

			template<typename T>
			UpdateStatus updateChannel(const ChannelStr& chlName, const T& notifData);

			UpdateStatus unsubCallbackFromChannel(const CbStr& cbName, const ChannelStr& chlName);

			UpdateStatus unsubCallbackFromAllChannels(const CbStr& cbName);

			UpdateStatus unsubAllCallbacksFromChannel(const ChannelStr& chlName);

			void destroyChannel(const ChannelStr& chlName);

			void removeCallback(const CbStr& cbName);

			// For debugging:

			int getNumCbsListeningTo(const ChannelStr& chlName) const;
			int getNumChannelsListenedBy(const CbStr& cbName) const;
			int getNumCbsRegistered() const;
			int getNumChannelsRegistered() const;
		};

		inline std::ostream& operator<<(std::ostream& os, NotificationRegistry::UpdateStatus status) {
			switch (status) {
			case NotificationRegistry::UpdateStatus::UPDATE_OK:              return os << "UPDATE_OK";
			case NotificationRegistry::UpdateStatus::UPDATE_EMPTY:           return os << "UPDATE_EMPTY";
			case NotificationRegistry::UpdateStatus::UPDATE_CHANNEL_MISSING: return os << "UPDATE_CHANNEL_MISSING";
			case NotificationRegistry::UpdateStatus::UPDATE_CALLBACK_MISSING:return os << "UPDATE_CALLBACK_MISSING";
			default:                                   return os << "UNKNOWN_STATUS";
			}
		}

		template<typename T, typename CbType>
		inline void NotificationRegistry::registerCallback(CbType&& cb, const CbStr& cbName)
		{
			DOOBIUS_FMT_DASSERT(m_cbIdMap.right.find(cbName) == m_cbIdMap.right.end(), "Found callback %1% already registered in notification registry %2%", cbName % m_nameOfNotifReg);

			m_cbIdMap.insert(CallbackIdMapping::value_type(++m_cbIdCounter, cbName));
			DOOBIUS_CLOG(trace) << "(" << cbName << " <-> " << m_cbIdCounter << ") in " << m_nameOfNotifReg;

			DOOBIUS_CLOG(info) << "Adding callback " << cbName << " for the first time to callback registry of " << m_nameOfNotifReg;
			m_callbackReg[m_cbIdCounter] =
			{
				cbName,
				[cb = std::forward<CbType>(cb)](const boost::any& genericArg) {
					cb(boost::any_cast<const T&>(genericArg));
				}
			};

			DOOBIUS_CLOG(trace) << cbName << " is now registered in " << m_nameOfNotifReg;
		}

		template<typename T, typename CbType, typename BindingClass>
		inline void NotificationRegistry::registerBoundCallback(BindingClass* classInst, CbType&& cb, const CbStr& cbName) {
			DOOBIUS_FMT_DASSERT(m_cbIdMap.right.find(cbName) == m_cbIdMap.right.end(), "Found callback %1% already registered in notification registry %2%", cbName % m_nameOfNotifReg);

			m_cbIdMap.insert(CallbackIdMapping::value_type(++m_cbIdCounter, cbName));
			DOOBIUS_CLOG(trace) << "(" << cbName << " <-> " << m_cbIdCounter << ") in " << m_nameOfNotifReg;

			DOOBIUS_CLOG(info) << "Adding callback " << cbName << " for the first time to callback registry of " << m_nameOfNotifReg;
			m_callbackReg[m_cbIdCounter] =
			{
				cbName,
				[classInst, cb = std::forward<CbType>(cb)](const boost::any& genericArg) {
					(classInst->*cb)(boost::any_cast<const T&>(genericArg));
				}
			};

			DOOBIUS_CLOG(trace) << cbName << " is now registered in " << m_nameOfNotifReg;
		}

		template<typename T>
		inline NotificationRegistry::UpdateStatus NotificationRegistry::updateChannel(const ChannelStr& chlName, const T& notifData)
		{
			if (m_chlIdMap.right.find(chlName) == m_chlIdMap.right.end()) {
				DOOBIUS_CLOG(warning) << chlName << " channel has either been deleted previously and all its callbacks de-registered or never existed from/in " << m_nameOfNotifReg;
				return UpdateStatus::UPDATE_CHANNEL_MISSING;
			}
			ChannelId chlId = m_chlIdMap.right.at(chlName);
			size_t count = m_registrations.get<ChannelTag>().count(chlId);
			if (count == 0) {
				DOOBIUS_CLOG(warning) << chlName << " channel has no callbacks listening in yet updateChannel() was called with it";
				return UpdateStatus::UPDATE_EMPTY;
			}

			const std::pair<ChannelIter, ChannelIter>& pit = m_registrations.get<ChannelTag>().equal_range(chlId);

#if defined(_DEBUG)
			DOOBIUS_CLOG(trace) << chlName << "'s callback(s):";
			for (ChannelIter it = pit.first; it != pit.second; ++it) {
				DOOBIUS_CLOG(trace) << '\t' << m_cbIdMap.left.at(it->cbId);
			}
#endif
			for (ChannelIter it = pit.first; it != pit.second; ++it) {
				DOOBIUS_FMT_DASSERT(m_callbackReg.find(it->cbId) != m_callbackReg.end(), "Couldn't find %1% CbId in the callback registry inside %2%", it->cbId % m_nameOfNotifReg);
				m_callbackReg.at(it->cbId).cb(notifData);
			}

			return UpdateStatus::UPDATE_OK;
		}
	};
};