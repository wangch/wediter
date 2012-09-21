/*
* singleton.h
*/

#ifndef W_SINGLETON_H_
#define W_SINGLETON_H_

#include <Windows.h>

namespace w {

	namespace noncopyable_
	{
		class noncopyable
		{
		protected:
			noncopyable() {}
			~noncopyable() {}
		private:
			noncopyable(const noncopyable&);
			const noncopyable& operator=(const noncopyable&);
		};
	}

	typedef noncopyable_::noncopyable noncopyable;


	typedef ::CRITICAL_SECTION critical_section;

	class mutex
	{
	private:

		critical_section cs_;

		mutex(mutex const &);
		mutex & operator=(mutex const &);

	public:

		mutex()
		{
			InitializeCriticalSection(&cs_);
		}

		~mutex()
		{
			DeleteCriticalSection(&cs_);
		}

		void lock()
		{
			EnterCriticalSection(&cs_);
		}
		// TryEnterCriticalSection only exists on Windows NT 4.0 and later
#if (defined(_WIN32_WINNT) && (_WIN32_WINNT >= 0x0400))
		bool try_lock()
		{
			return TryEnterCriticalSection(&cs_) != 0;
		}
#else
		bool try_lock()
		{
			return false;
		}
#endif
		void unlock()
		{
			LeaveCriticalSection(&cs_);
		}
	};

	struct null_lock : public noncopyable
	{
		null_lock(mutex& m) {}
	};

	class lock : public noncopyable
	{
	public:
		lock(mutex& m): mutex_(m)
		{
			mutex_.lock();
		}
		~lock()
		{
			mutex_.unlock();
		}
	private:
		mutex &mutex_;
	};

	typedef lock mt_lock;

	template<typename TYPE, typename LOCK = null_lock>
	class singleton
	{
		typedef LOCK scoped_lock;
	protected:
		singleton(void){}
		virtual ~singleton(void){}
		singleton(const singleton&);
		singleton& operator=(const singleton&);

	public:
		static TYPE* instance(void)
		{
			if(instance_ == 0)
			{
				scoped_lock lock(mutex_);
				if(instance_ == 0)
					instance_ = new TYPE();

				// for in atexit call ~TYPE()
				//::atexit(destroy);
			}

			return instance_;
		}

		static void destroy(void)
		{
			delete instance_;
		}

	private:
		static TYPE* instance_;
		static mutex mutex_;
	};

	template<typename TYPE, typename LOCK>
	TYPE* singleton<TYPE, LOCK>::instance_ = 0;

	template<typename TYPE, typename LOCK>
	mutex singleton<TYPE, LOCK>::mutex_;
}

#endif // W_SINGLETON_H_