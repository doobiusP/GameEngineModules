#pragma once
#include <type_traits>
#include <unordered_set>
#include <string>

#include "notification_types.h"
#include "dbg/logging.h"

namespace CommonUtil {
	namespace Notification {

		class NotifieeCommon {
		public:
			std::string notifieeName;
			NotifieeCommon(const char* name) : notifieeName(name)
			{}
			virtual ~NotifieeCommon() = default;
		};

		template <typename NotifyingData>
		class INotifiee : virtual public NotifieeCommon {
		protected:
			INotifiee() = default;
			virtual void onNotify(const NotifyingData& data, const std::string& notifierName) = 0;
			virtual ~INotifiee() = default;
		};

		/*
		* This class should be directly instantiated. A module that needs to transmit notifications to observers just needs
		* to create a member variable of this type with the specific kind of observer class and the accompanying data structure
		* passed as template parameters. While it is possible to inherit this class, doing so is not as flexible compared to
		* direct instantiation.
		*/
		template <typename SourceNotifiee, typename NotifyingData>
		class Notifier {
			static_assert(std::is_base_of_v<INotifiee<NotifyingData>, SourceNotifiee>, 
						  "SourceNotifiee must inherit from INotifiee");
		private:
			std::string m_sourceName;
			std::unordered_set<SourceNotifiee*> m_notifieeList;
		public:
			Notifier(const char* sourceName) : m_sourceName(sourceName)
			{
				LOG(INFO) << "Source of notifications called " << m_sourceName << " now created\n";
			}

			void addNotifiee(SourceNotifiee* notifiee) {
				m_notifieeList.insert(notifiee);
				LOG(INFO) << m_sourceName << " added " << notifiee->notifieeName << " to its notification list\n";
			}

			void removeNotifiee(SourceNotifiee* notifiee) {
				m_notifieeList.erase(notifiee);
				LOG(INFO) << m_sourceName << " removed " << notifiee->notifieeName << " from its notification list\n";
			}

			void notifyAll(const NotifyingData& data) {
				for (auto notifiee : m_notifieeList) {
					notifiee->onNotify(data, m_sourceName);
				}
			}
		};

		// Concrete Observers
		class ConfigNotifiee : public INotifiee<json>, public INotifiee<int> {
		public:
			void onNotify(const json& config, const std::string& notifierName) override;
			void onNotify(const int& val, const std::string& notifierName) override;
			ConfigNotifiee(const char* moduleName);
			~ConfigNotifiee() = default;
		};
	};
};