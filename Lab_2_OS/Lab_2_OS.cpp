#include "pch.h"
#include <iostream>
#include <string>
#include <Windows.h>
#include <process.h>
#include "SmartPointer.h"

using namespace SmartPointer;
using namespace std;

const int NUMBER_OF_ITERATIONS = 5;
const LPCWSTR PATH_TO_LOG_FILE = L"log.txt";
const LPCWSTR PATH_TO_FUNCTION_RESULT_FILE = L"test.txt";

SYSTEMTIME st;
DWORD dwBytesWritten;
HANDLE valuesFile, logFile, thread1, thread2, thread3;
string pointStartTime, pointEndTime;

class Point {

public:
	int x;
	int y;

	Point() {}

	Point(int x, int y) {
		this->x = x;
		this->y = y;
	}

	int getX() {
		return x;
	}

	int getY() {
		return y;
	}

	void setX(int x) {
		this->x = x;
	}

	void setY(int y) {
		this->y = y;
	}
};

void QuadraticFunc(SafeSmartPointer<Point> ptr) {
	for (int index = 0; index < NUMBER_OF_ITERATIONS; index++) {
		cout << "Thread 1" << endl;
		int y = pow(index, 2);
		GetLocalTime(&st);
		ptr->setX(index);
		ptr->setY(y);
		//cout << x << " " << y << endl;

		string startTime = to_string(int(st.wHour)) + ":" + to_string(int(st.wMinute)) + ":"
			+ to_string(int(st.wSecond)) + ":" + to_string(int(st.wMilliseconds));
		pointStartTime = startTime;

		string currentTime = to_string(int(st.wHour)) + ":" + to_string(int(st.wMinute)) + ":"
			+ to_string(int(st.wSecond)) + ":" + to_string(int(st.wMilliseconds));

		string logFirstThreadMessage = currentTime + " [INFO] Data is calculate:  x =  " + to_string(ptr->getX()) +
			", y = " + to_string(ptr->getY()) + "\n";
		WriteFile(logFile, logFirstThreadMessage.c_str(), logFirstThreadMessage.size(), &dwBytesWritten, NULL);
		
		ResumeThread(thread2);

		SuspendThread(thread1);
	}
}

void WriteResultToFileFunc(SafeSmartPointer<Point> ptr) {
	for (int index = 0; index < NUMBER_OF_ITERATIONS; index++) {
		cout << "Thread 2" << endl;
		GetLocalTime(&st);
		string endTime = to_string(int(st.wHour)) + ":" + to_string(int(st.wMinute)) + ":"
			+ to_string(int(st.wSecond)) + ":" + to_string(int(st.wMilliseconds));
		pointEndTime = endTime;

		//cout << " x = " << ptr->getX() << " y = " << ptr->getY() << endl;

		string currentTime = to_string(int(st.wHour)) + ":" + to_string(int(st.wMinute)) + ":"
			+ to_string(int(st.wSecond)) + ":" + to_string(int(st.wMilliseconds));

		string valuesMessage = "x = " + to_string(ptr->getX()) + " y = " + to_string(ptr->getY()) + "\n";
		WriteFile(valuesFile, valuesMessage.c_str(), valuesMessage.size(), &dwBytesWritten, NULL);

		string logSecondThreadMessage = currentTime + " [INFO] Data is written to file:  x =  " + to_string(ptr->getX()) +
			", y = " + to_string(ptr->getY()) + "\n";
		WriteFile(logFile, logSecondThreadMessage.c_str(), logSecondThreadMessage.size(), &dwBytesWritten, NULL);

		ResumeThread(thread3);

		SuspendThread(thread2);
	}

}

void WriteToLogFileFunc(SafeSmartPointer<Point> ptr) {
	for (int index = 0; index < NUMBER_OF_ITERATIONS; index++) {
		cout << "Thread 3" << endl;
		//cout << "START TIME: " << pointStartTime << endl;
		//cout << "END TIME: " << pointEndTime << endl;
		GetLocalTime(&st);
		string currentTime = to_string(int(st.wHour)) + ":" + to_string(int(st.wMinute)) + ":"
			+ to_string(int(st.wSecond)) + ":" + to_string(int(st.wMilliseconds));

		string log = currentTime + " [INFO] Start time " + pointStartTime +
			"\n" + currentTime + " [INFO] End time " + pointEndTime + "\n";
		WriteFile(logFile, log.c_str(), log.size(), &dwBytesWritten, NULL);
		cout << "----------------------------------------" << endl;
		
		ResumeThread(thread1);

		SuspendThread(thread3);
	}
}

int main() {
	SafeSmartPointer<Point> ptr(new Point());
	thread2 = (HANDLE)_beginthread((_beginthread_proc_type)WriteResultToFileFunc, 0, (void*)&ptr);
	SuspendThread(thread2);
	thread3 = (HANDLE)_beginthread((_beginthread_proc_type)WriteToLogFileFunc, 0, (void*)&ptr);
	SuspendThread(thread3);
	valuesFile = CreateFile(PATH_TO_FUNCTION_RESULT_FILE, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	logFile = CreateFile(PATH_TO_LOG_FILE, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

	thread1 = (HANDLE)_beginthread((_beginthread_proc_type)QuadraticFunc, 0, (void*)&ptr);
	WaitForSingleObject(thread1, INFINITE);
	CloseHandle(thread2);
	CloseHandle(thread3);
	CloseHandle(valuesFile);
	CloseHandle(logFile);
	system("pause");
}