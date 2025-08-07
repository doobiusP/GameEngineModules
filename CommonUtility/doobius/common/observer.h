#pragma once
#include <unordered_map> 
#include <unordered_set>

#include <string>

#include "doobius/dbg/custom_assert.h"

namespace Doobius {
	namespace Notification {
		// ============================== Direct Notifications (Classic Observer Pattern Impl.) ============================== //

		class DirectNotifieeCommon {
		public:
			std::string notifieeName;
			DirectNotifieeCommon(const char* name) : notifieeName(name)
			{
			}
			virtual ~DirectNotifieeCommon() = default;
		};

		template <typename DirectNotifDType>
		class IDirectNotifiee : virtual public DirectNotifieeCommon {
		protected:
			IDirectNotifiee() = default;
			virtual void onNotify(const DirectNotifDType& data, const std::string& notifierName) = 0;
			virtual ~IDirectNotifiee() = default;
		};

		/**
		* \brief This class should be directly instantiated. A module that needs to transmit notifications to observers just needs
		* to create a member variable of this type with the specific kind of observer class and the accompanying data structure
		* passed as template parameters. While it is possible to inherit this class, doing so is not as flexible compared to
		* direct instantiation.
		*/
		template <typename SourceNotifiee, typename DirectNotifDType>
		class DirectNotifier {
			BOOST_STATIC_ASSERT_MSG(std::is_base_of_v<IDirectNotifiee<DirectNotifDType>, SourceNotifiee>,
				"SourceNotifiee must inherit (very specifically) from IDirectNotifiee<DirectNotifDType>");
		private:
			std::string m_sourceName;
			std::unordered_set<SourceNotifiee*> m_notifieeList;
		public:
			DirectNotifier(const char* sourceName) : m_sourceName(sourceName)
			{
				DOOBIUS_CLOG(trace) << "Direct source of notifications called " << m_sourceName << " now created";
			}

			void addNotifiee(SourceNotifiee* notifiee) {
				m_notifieeList.insert(notifiee);
				DOOBIUS_CLOG(trace) << m_sourceName << " added " << notifiee->notifieeName << " to its notification list";
			}

			void removeNotifiee(SourceNotifiee* notifiee) {
				m_notifieeList.erase(notifiee);
				DOOBIUS_CLOG(trace) << m_sourceName << " removed " << notifiee->notifieeName << " from its notification list";
			}

			void notifyAll(const DirectNotifDType& data) {
				for (auto notifiee : m_notifieeList) {
					notifiee->onNotify(data, m_sourceName);
				}
			}
		};
	};
};