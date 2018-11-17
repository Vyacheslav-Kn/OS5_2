#include <Windows.h>

namespace SmartPointer
{
	template <typename T>
	class SafeSmartPointer
	{
	private:
		T* data;            
		int numberOfReferences;
		HANDLE mutex;

		T* getPointer(T* data)
		{
			WaitForSingleObject(mutex, INFINITE);

			if (numberOfReferences == 0) {
				numberOfReferences++;
				ReleaseMutex(mutex);
				return data;
			}
			else {
				ReleaseMutex(mutex);
				return NULL;
			}
		}

		void refreshMutexToSignalState() {
			CloseHandle(mutex);
			mutex = CreateMutex(NULL, TRUE, NULL);
		}

	public:
		SafeSmartPointer() : data(NULL), numberOfReferences(0), mutex(NULL) {} 

		SafeSmartPointer(T* value){
			numberOfReferences = 0;
			mutex = NULL;
			if (value != NULL){
				mutex = CreateMutex(NULL, TRUE, NULL);
				data = getPointer(value);
			}
		}

		T& operator* (){
			refreshMutexToSignalState();
			WaitForSingleObject(mutex, INFINITE);

			if (numberOfReferences == 1) {
				ReleaseMutex(mutex);
				return *data;
			}

			ReleaseMutex(mutex);
			return NULL;
		}

		T* operator-> (){
			refreshMutexToSignalState();
			WaitForSingleObject(mutex, INFINITE);

			if (numberOfReferences == 1) {
				ReleaseMutex(mutex);
				return data;
			}

			ReleaseMutex(mutex);
			return NULL;
		}

		~SafeSmartPointer() {
			delete data;
			ReleaseMutex(mutex);
			CloseHandle(mutex);
		}
		
	};
}