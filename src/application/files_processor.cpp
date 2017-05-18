#include "files_processor.h"
#include "client.h"
#include <QtWidgets>
#include <sstream>
#include "common.h"

using namespace T1;
using namespace CITOOL;

FilesProcessor::FilesProcessor(QObject* parent) : pView(parent), processed_count(0)
{	
}

FilesProcessor::~FilesProcessor()
{
	if (is_running())
		abort();

	if (!is_terminate())
	{
		terminate();
		notify_process();
	}

	if (th_finished.joinable())
		th_finished.join();

}

void FilesProcessor::start_guard_finish()
{
	if (th_finished.joinable())
	{
		terminate();
		notify_process();
		if (th_finished.joinable())
			th_finished.join();

		unterminate();
	}

	std::thread fn([this] {
		for (;;)
		{
			{
				std::unique_lock<std::mutex> lock(this->cv_mutex);
				this->cv_finished.wait(lock, [this] { return (this->is_terminate() || ((a_busy <= 0) && (this->count() == 0))); });
			}
			if (this->is_terminate())
			{
				return;
			}
			if (this->is_running())
			{
				this->stop();
				return;
			}
		}
	});

	std::swap(th_finished, fn);
}

void FilesProcessor::find_recursion(const QString &path, const QString &pattern, QStringList *result)
{
	QDir currentDir(path);
	const QString prefix = path + QLatin1Char('/');

	foreach(const QString &match, currentDir.entryList(QStringList(pattern), QDir::Files | QDir::NoSymLinks))
		result->append(prefix + match);

	foreach(const QString &dir, currentDir.entryList(QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot))
		find_recursion(prefix + dir, pattern, result);
}

void FilesProcessor::view_event(EvMode mode, int value)
{
	ProcessEvent* pe = new ProcessEvent(mode);
	pe->setValue(value);
	QApplication::postEvent(pView, pe);
}

void FilesProcessor::view_event(const QString &msg)
{
	ProcessEvent* pe = new ProcessEvent(EvMode::Status);
	pe->setStatus(msg);
	QApplication::postEvent(pView, pe);
}

void FilesProcessor::malware_event(const QString &file, const QString &guid, size_t pos)
{
	ProcessEvent* pe = new ProcessEvent(EvMode::Malware);
	pe->setMalware(file, guid);
	pe->setValue((int)pos);
	{
		std::unique_lock<std::mutex> lock(ev_mutex);
		QApplication::postEvent(pView, pe);
	}
}

void FilesProcessor::error_event(const QString &errmsg)
{
	ProcessEvent* pe = new ProcessEvent(EvMode::Error);
	pe->setStatus(errmsg);
	{
		std::unique_lock<std::mutex> lock(ev_mutex);
		QApplication::postEvent(pView, pe);
	}
}

void FilesProcessor::start()
{
	start_guard_finish();
	start_pool();
}

void FilesProcessor::stop()
{
	CITOOL::UsePool::stop();
	cv_finished.notify_one();
	STOP(processed_count)
}

void FilesProcessor::abort()
{
	CITOOL::UsePool::abort();
	a_busy = 0;
	cv_finished.notify_one();
	ABORT(processed_count)
}

bool FilesProcessor::run(const QString &path)
{
	if (is_running())
	{
		STATUS("Already running ...")
		return false;
	}

	STATUS("Scan directory for files ...")

	QStringList files;
	find_recursion(path, QStringLiteral("*"), &files);
	if (files.isEmpty())
	{
		STATUS("Files are not found")
		return false;
	}
	processed_count = 0;
	START(files.count())

	unterminate();
	set_stop(false);

	for (int i = 0; i < files.size(); ++i)
	{
		std::string fname = files.at(i).toStdString();
		enqueue(bind(&FilesProcessor::process, std::move(fname), this));
	}
	a_busy = 0;
	start();

	return true;
}

void FilesProcessor::notify_process()
{
	cv_finished.notify_one();
}

void FilesProcessor::process(std::string fpath, FilesProcessor* parent)
{
	++parent->a_busy;
	auto this_id = std::this_thread::get_id();
	std::stringstream ss;
	ss << this_id;
	std::string stid = ss.str();

	Socket::Init();
	try
	{
		ConnectSocket connect_socket;
		connect_socket.Connect(SVRHOST, SVRPORT);

		if (!connect_socket.Hello())
			throw SocketException(-1, "Hello fail !!!");
		
		if (connect_socket.SendFile(fpath.c_str()) > 0)
		{
			ConfirmationHeader header;
			memset(&header, 0, sizeof(ConfirmationHeader));
			while (connect_socket.ReadConfirmation(header) > 0)
			{
				if (header.operation != OP_ANSWER)
				{
					if (header.operation == OP_ERR)
						throw SocketException(-3, "Server returned ERROR !!!");
					else
						throw SocketException(-4, "Server not answered !!!");
				}

				if (header.size == 0)
				{
					break;
				}

				const char* cguid = reinterpret_cast<const char*>(&header.data);
				if (!cguid)
					throw SocketException(-5, "Incorrect GUID");

				parent->malware_event(QString::fromStdString(fpath), QString::fromStdString(cguid), header.size);

				memset(&header, 0, sizeof(ConfirmationHeader));
			}
		}
		else
		{
			throw SocketException(-2, "SendConfirmation fail !!!");
		}
	}
	catch (SocketException& ex)
	{
		QString errMsg = QString("ERROR [%1]: %2 (%3)").arg(QString::fromStdString(stid)).arg(QString::fromStdString(ex.err_msg)).arg(QString::fromStdString(fpath));
		parent->error_event(errMsg);
	}
	catch (...)
	{
		parent->error_event(QString("ERROR [%1]: in task for file: %2").arg(QString::fromStdString(stid)).arg(QString::fromStdString(fpath)));
	}

	Socket::Quit();

	parent->view_event(EvMode::Inc, 0);
	parent->processed_count++;
	--parent->a_busy;
	parent->notify_process();
}

int FilesProcessor::processed()
{
	return processed_count;
}