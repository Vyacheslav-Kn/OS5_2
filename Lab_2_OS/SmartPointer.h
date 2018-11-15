#include <Windows.h>

namespace SmartPointer
{
	struct ref_count_data{
		unsigned int use_count;
	};

	class reference_count
	{
		private:
		ref_count_data ref_counts;

		public:
		HANDLE mutex;
		reference_count()
		{
			ref_counts.use_count = 1;
			mutex = CreateMutex(NULL, FALSE, NULL);
		}

		void* get_shared_ref(void* pdata)
		{
			WaitForSingleObject(mutex, INFINITE);

			if (ref_counts.use_count){
				ref_counts.use_count++;
				ReleaseMutex(mutex);
				return pdata;
			}
			else{
				ReleaseMutex(mutex);
				return NULL;
			}
		}

		ref_count_data release_shared_ref(){
			//WaitForSingleObject(mutex, INFINITE);
			//ref_counts.use_count--;
			//ReleaseMutex(mutex);
			return ref_counts;
		}

		~reference_count() {
			//ReleaseMutex(mutex);
			//CloseHandle(mutex);
		}
	};

	template <typename T>
	class SafeSmartPointer
	{
	private:
		T* pdata;            

	public:
		SafeSmartPointer() : pdata(NULL), rc(NULL) {} 
		reference_count* rc; 

		SafeSmartPointer(T* pvalue) : pdata(pvalue){
			if (NULL != pdata){
				rc = new reference_count();
			}
		}
		
		/*SafeSmartPointer(const SafeSmartPointer<T>& sp) : pdata(NULL), rc(sp.rc){
			if (NULL != rc){
				pdata = static_cast<T*>(rc->get_shared_ref(sp.pdata));

				if (NULL == pdata){
					rc = NULL;
				}
			}
		}*/

		~SafeSmartPointer()
		{
			if (NULL != rc){
				//CloseHandle(rc->mutex);
				//ref_count_data updated_counts = rc->release_shared_ref();
				//rc->~reference_count();
				//delete rc;
				//rc = NULL;
				//delete pdata;
			}
		}

		T& operator* () const{
			WaitForSingleObject(rc->mutex, INFINITE);
			ReleaseMutex(rc->mutex);
			return *pdata;
		}

		T* operator-> () const{		
			WaitForSingleObject(rc->mutex, INFINITE);
			ReleaseMutex(rc->mutex);
			return pdata;
		}
		
	};
}