#include <Windows.h>

namespace SmartPointer
{
	template <typename T>
	class SafeSmartPointer
	{
	private:
		T* data;            
		int numberOfReferences;
		const LPCWSTR mutexName = L"Mutex_Name";

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

	public:
		HANDLE mutex;
		SafeSmartPointer() : data(NULL), numberOfReferences(0), mutex(NULL) {} 

		SafeSmartPointer(T* value){
			numberOfReferences = 0;
			mutex = NULL;
			if (value != NULL){
				mutex = CreateMutex(NULL, FALSE, mutexName);
				data = getPointer(value);
			}
		}

		void CloseAccessToAnotherThreads() {
			WaitForSingleObject(mutex, INFINITE);
		}

		void OpenAccessToAnotherThreads() {
			ReleaseMutex(mutex);
		}

		T& operator* (){
			return *data;
		}

		T* operator-> (){
			return data;
		}

		~SafeSmartPointer() {
			delete data;
			ReleaseMutex(mutex);
			CloseHandle(mutex);
		}
		
	};
}