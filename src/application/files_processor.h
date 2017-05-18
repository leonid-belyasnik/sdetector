/**
* \file		files_processor.h
* \brief	File handler.
*
* \author	Leonid Belyasnik (leonid.belyasnik@gmail.com)
* \date		08/05/2017
*/

#ifndef FilesProcessor_H
#define FilesProcessor_H

#include <QtCore>
#include "usepool.h"
#include "process.h"

/**
* \defgroup	VIEWEVENTS Wrappers for events.
* \brief	Events to main widget.
*
*  @{
*/
#define STATUS(m)			view_event(QString(m));
#define START(n)			view_event(EvMode::Start, n);
#define STOP(n)				view_event(EvMode::Stop, n);
#define ABORT(n)			view_event(EvMode::Abort, n);
#define PROCESS(w)			view_event(EvMode::Inc, 0);
#define SETMAX(n)			view_event(EvMode::Max, n);
/** @} */

/**
*	namespace T1
*	\brief Test 1
*/
namespace T1 {
	/**
	* \brief	File handler class.
	*/
	class FilesProcessor : public CITOOL::UsePool
	{
		QObject* pView;							///< Pointer to main widget.
		std::atomic_int processed_count{ 0 };	///< Number of processed.	
		std::condition_variable cv_finished;	///< Completion state variable.
		std::thread th_finished;				///< Completion thread.
		std::mutex cv_mutex;					///< Completion mutex.
		std::mutex ev_mutex;					///< Event mutex.
		std::atomic_int a_busy{ -1 };			///< Count busy tasks.
	private:
		/**
		* Static function of processing a file for tasks in the pool.
		*
		* \param [in]	fpath	Path to file.
		* \param [in]	parent	Pointer to this.
		*/
		static void process(std::string fpath, FilesProcessor* parent);
		/**
		* Do not use default constructor.
		*/
		FilesProcessor() = delete;
		/**
		* Start guard thread of finished process.
		*/
		void start_guard_finish();
	public:
		/**
		* Constructor.
		*
		* \param [in]	parent	Pointer to main Vidget.
		*/
		FilesProcessor(QObject* parent);
		/**
		* Destructor.
		*/
		~FilesProcessor();

		/**
		* Implement virtual function start.
		*/
		void start();
		/**
		* Overload function stop.
		*/
		void stop();
		/**
		* Overload function abort.
		*/
		void abort();
		/**
		* Notify to guard thread about changes.
		*/
		void notify_process();
		/**
		* Starts processing files.
		*
		* \param [in]	path	Path to root scan directory.
		*
		* \return	Successful launch.
		*/
		bool run(const QString &path);
		/**
		* Event reported progress.
		*
		* \param [in]	mode	Event mode.
		* \param [in]	value	Value of progress.
		*/
		void view_event(EvMode mode, int value);
		/**
		* Event reported about change status.
		*
		* \param [in]	msg	Message.
		*/
		void view_event(const QString &msg);
		/**
		* Event reported about found malware.
		*
		* \param [in]	file	File path.
		* \param [in]	guid	GUID malware.
		* \param [in]	pos	Position in file.
		*/
		void malware_event(const QString &file, const QString &guid, size_t pos);
		/**
		* Event reported about error.
		*
		* \param [in]	errmsg	Message.
		*/
		void error_event(const QString &errmsg);
		/**
		* Processed total.
		*
		* \return	The number processed at the moment.
		*/
		int processed();
		/**
		* Searching files in the directory.
		*
		* \param [in]	path	Path to directory.
		* \param [in]	pattern	Search pattern.
		* \param [in,out]	result	Pointer to list for result.
		*/
		static void find_recursion(const QString &path, const QString &pattern, QStringList *result);
	};

} // T1

#endif // FilesProcessor_H