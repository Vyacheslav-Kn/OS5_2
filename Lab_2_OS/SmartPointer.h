#include <Windows.h>

namespace SmartPointer
{
	template <typename T>
	class SafeSmartPointer
	{
	private:
		T* data;
		HANDLE mutex;
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
		SafeSmartPointer() : data(NULL), numberOfReferences(0), mutex(NULL) {}

		SafeSmartPointer(T* value) {
			numberOfReferences = 0;
			mutex = NULL;
			if (value != NULL) {
				mutex = CreateMutex(NULL, FALSE, mutexName);
				data = getPointer(value);
			}
		}

		T& operator* () {
			WaitForSingleObject(mutex, INFINITE);
			ReleaseMutex(mutex);
			return *data;
		}

		T* operator-> () {
			WaitForSingleObject(mutex, INFINITE);
			ReleaseMutex(mutex);
			return data;
		}

		~SafeSmartPointer() {
			delete data;
			ReleaseMutex(mutex);
			CloseHandle(mutex);
		}

	};
}