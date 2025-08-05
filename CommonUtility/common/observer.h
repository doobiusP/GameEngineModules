#pragma once
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <boost/function.hpp>
#include <boost/any.hpp>
#include <boost/container/small_vector.hpp>
#include <boost/container/flat_map.hpp>
#include <typeindex>

#include "notification_types.h"
#include "dbg/custom_assert.h"


namespace CommonUtil {
	namespace Notification {
		constexpr int g_numNotifCbsOk = 5;
		constexpr int g_numNotifSrcOk = 5;

		/**
		 * \brief A callback wrapper containing void<const boost::any&> and a name you can associate with the callback.
		 * Intended for use with the NotificationManager. Each callback \b must return void.
		 *
		 */
		struct NotificationCallback {
			std::string notifieeName;
			boost::function<void(const boost::any&)> cb;
		};


		/**
		 * \brief A collection of NotificationCallbacks with type information of what kind of parameter they accept
		 *
		 */
		struct NotificationBundle {
			boost::container::small_vector<NotificationCallback, g_numNotifCbsOk> cbs;
			std::type_index notifType;

			NotificationBundle() : notifType(typeid(void)) {}
			NotificationBundle(std::type_index type) : notifType(type) {}
		};

		/**
		 * \brief A manager class for dealing with notification transmission and receival in a decoupled manner.
		 * Register a "notification source" / object-of-interest (OOI) and NotificationCallbacks to that OOI.
		 * Updates to an OOI immediately executes callbacks subscribed to that OOI.
		 *
		 */
		class NotificationManager {
		private:
			using NotifRegistry = boost::container::small_flat_map<std::string, NotificationBundle, g_numNotifSrcOk>;
			// using NotifRegistry = std::unordered_map<std::string, NotificationBundle>;
			using _Notifiee = std::string;
			using _Notifier = std::string;
			NotifRegistry m_notifRegistry;
			std::unordered_map<_Notifiee, _Notifier> m_registrants;


			//template <typename NotifDatatype, typename CbType>
			//static NotificationCallback makeNotifCallback(const std::string& cbName, CbType&& cb) {
			//	return {
			//		cbName,
			//		[anyWrapperCb = std::forward<CbType>(cb)](const boost::any& genericArg) {
			//			anyWrapperCb(boost::any_cast<NotifDatatype>(genericArg));
			//		}
			//	}
			//}

		public:
			/**
			 * \brief Returns a NotificationCallback containing a function object appropriate for registering with the
			 * NotificationManager.
			 *
			 * \param name - Name of callback
			 * \param userCb - Callback object that accepts const T& for template type T
			 * \return NotificationCallback for use with NotificationManager
			 */
			template <typename T>
			static NotificationCallback makeCallback(const std::string& name, boost::function<void(const T&)> userCb) {
				return {
					name,
					[userCb](const boost::any& val) {
						userCb(boost::any_cast<T>(val));
					}
				};
			}

			template <typename T>
			void createNotifSource(const std::string& notifSourceName) {
				BOOST_ASSERT_MSG(!m_notifRegistry.contains(notifSourceName), "Can't register notification source that still exists in the registry.");
				m_notifRegistry.emplace(
					notifSourceName,
					NotificationBundle(typeid(T)) // default construct bundle
				);
				DOOBIUS_CLOG(trace) << "Created notification source called " << notifSourceName;
			}

			//template <typename T>
			//void registerNotifCallback(const std::string& notifieeName, const std::string& notifSourceName, boost::function<void(const T&)> userCb)
			//{
			//	BOOST_ASSERT_MSG(m_notifRegistry.contains(notifSourceName), "Can't register a NotifCb to a notif source that isnt registered");
			//	BOOST_ASSERT_MSG(m_notifRegistry[notifSourceName].notifType == typeid(T), "Cant register a NotifCb that doesnt handle the same type as the notification source");
			//	NotificationCallback = makeCallback(notifieeName, userCb);

			//	m_notifRegistry[notifSourceName].cbs.push_back({
			//		notifCb.notifieeName,
			//		[notifCb](const boost::any& val) {
			//			notifCb.cb(boost::any_cast<T>(val));
			//		}
			//		});

			//	m_registrants.insert({ notifCb.notifieeName, notifSourceName });
			//	DOOBIUS_CLOG(trace) << "Successfully pushed " << notifCb.notifieeName << "'s callback to " << notifSourceName;
			//}

			template <typename T>
			void registerCbForNotif(const std::string& notifSourceName, NotificationCallback& notifCb)
			{
				BOOST_ASSERT_MSG(m_notifRegistry.contains(notifSourceName), "Can't register a NotifCb to a notif source that isnt registered");
				BOOST_ASSERT_MSG(m_notifRegistry[notifSourceName].notifType == typeid(T), "Cant register a NotifCb that doesnt handle the same type as the notification source");
				m_notifRegistry[notifSourceName].cbs.push_back({
					notifCb.notifieeName,
					[notifCb](const boost::any& val) {
						notifCb.cb(boost::any_cast<T>(val));
					}
					});

				m_registrants.insert({ notifCb.notifieeName, notifSourceName });
				DOOBIUS_CLOG(trace) << "Successfully pushed " << notifCb.notifieeName << "'s callback to " << notifSourceName;
			}

			template <typename T>
			void notify(const std::string& notifSourceName, const T& val) {
				BOOST_ASSERT_MSG(m_notifRegistry.contains(notifSourceName), "Can't push updates to a notif source that isnt registered");
				BOOST_ASSERT_MSG(m_notifRegistry[notifSourceName].notifType == typeid(T), "Cant notify a different type than the notification source");

				boost::any anyVal(val);
				for (auto& notifiee : m_notifRegistry[notifSourceName].cbs) {
					notifiee.cb(val);
				}
			}

			// TODO: Deregister function here

		};

		class NotifieeCommon {
		public:
			std::string notifieeName;
			NotifieeCommon(const char* name) : notifieeName(name)
			{
			}
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
			BOOST_STATIC_ASSERT_MSG(std::is_base_of_v<INotifiee<NotifyingData>, SourceNotifiee>,
				"SourceNotifiee must inherit from (very specifically) INotifiee<NotifyingData>");
		private:
			std::string m_sourceName;
			std::unordered_set<SourceNotifiee*> m_notifieeList;
		public:
			Notifier(const char* sourceName) : m_sourceName(sourceName)
			{
				DOOBIUS_CLOG(trace) << "Source of notifications called " << m_sourceName << " now created";
			}

			void addNotifiee(SourceNotifiee* notifiee) {
				m_notifieeList.insert(notifiee);
				DOOBIUS_CLOG(trace) << m_sourceName << " added " << notifiee->notifieeName << " to its notification list";
			}

			void removeNotifiee(SourceNotifiee* notifiee) {
				m_notifieeList.erase(notifiee);
				DOOBIUS_CLOG(trace) << m_sourceName << " removed " << notifiee->notifieeName << " from its notification list";
			}

			void notifyAll(const NotifyingData& data) {
				for (auto notifiee : m_notifieeList) {
					notifiee->onNotify(data, m_sourceName);
				}
			}
		};

		// Concrete Observers
		class ConfigNotifiee : public INotifiee<json::value>, public INotifiee<int> {
		public:
			void onNotify(const json::value& config, const std::string& notifierName) override;
			void onNotify(const int& val, const std::string& notifierName) override;
			ConfigNotifiee(const char* moduleName);
			~ConfigNotifiee() = default;
		};
	};
};