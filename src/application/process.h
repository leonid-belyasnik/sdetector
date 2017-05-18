/**
* \file		process.h
* \brief	Events of main widget.
*
* \author	Leonid Belyasnik (leonid.belyasnik@gmail.com)
* \date		08/05/2017
*/

#ifndef PROCESS_H
#define PROCESS_H

#include <QEvent>

namespace T1 {

	enum class EvMode : uint8_t
	{
		Start = 0,
		Stop,
		Abort,
		Max,
		Inc,
		Dec,
		Status,
		Malware,
		Error
	};

	class ProcessEvent : public QEvent {
		EvMode m_bEvMode;
		int m_nValue;
		QString m_sFileName;
		QString m_sGUID;
		QString m_sMsg;
	private:
		ProcessEvent() = delete;
	public:
		enum { ProcessType = User + 1 };

		ProcessEvent(EvMode _mode) : QEvent((Type)ProcessType), m_bEvMode(_mode)
		{
		}

		void setValue(int n)
		{
			m_nValue = n;
		}

		void setStatus(const QString& stat_msg)
		{
			m_sMsg = stat_msg;
		}

		void setMalware(const QString& file_name, const QString& guid)
		{
			m_sFileName = file_name;
			m_sGUID = guid;
		}

		int value() const
		{
			return m_nValue;
		}

		const QString& status() const
		{
			return m_sMsg;
		}

		const QString& file() const
		{
			return m_sFileName;
		}

		const QString& guid() const
		{
			return m_sGUID;
		}

		EvMode mode() const
		{
			return m_bEvMode;
		}
	};

} // T1

#endif // PROCESS_H