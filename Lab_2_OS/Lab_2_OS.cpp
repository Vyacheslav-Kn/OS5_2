#include "pch.h"
#include <conio.h>
#include <iostream>
#include <fstream>
#include <thread>
#include <Windows.h>
#include <csignal>
#include <memory>
#include <string>
#include <mutex>  
#include "SmartPointer.h"
using namespace my_experimental;
using namespace std;

CONST int NUMBER_OF_ITERATIONS = 10;
CONST string PATH_TO_LOG_FILE = "log.txt";
CONST string PATH_TO_FUNCTION_RESULT_FILE = "test.txt";

SYSTEMTIME st;
ofstream fout;
ofstream logout;
HANDLE eventForFirstThread, eventForSecondThread, eventForThirdThread;

class Point {
	int x;
	int y;
	string timeOfGettingY;
	string timeOfWritingToLogFile;

public:
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

	string getStartTime() {
		return timeOfGettingY;
	}

	void setStartTime(string time) {
		timeOfGettingY = time;
	}

	string getEndTime() {
		return timeOfWritingToLogFile;
	}

	void setEndTime(string time) {
		timeOfWritingToLogFile = time;
	}
};

	
void QuadraticFunc(shared_pointer<Point> ptr) {
	for (int index = 0; index < NUMBER_OF_ITERATIONS; index++) {
		WaitForSingleObject(eventForFirstThread, INFINITE);
		cout << "Thread 1" << endl;
		int x = index;
		int y = pow(x, 2);
		ptr->setX(x);
		ptr->setY(y);
		//cout << x << " " << y << endl;
		GetLocalTime(&st);
		string startTime = to_string(int(st.wHour)) + ":" + to_string(int(st.wMinute)) + ":"
			+ to_string(int(st.wSecond)) + ":" + to_string(int(st.wMilliseconds));
		ptr->setStartTime(startTime);
		ResetEvent(eventForFirstThread);
		SetEvent(eventForSecondThread);
	}
}

void WriteResultToFileFunc(shared_pointer<Point> ptr) {
	for (int index = 0; index < NUMBER_OF_ITERATIONS; index++) {
		WaitForSingleObject(eventForSecondThread, INFINITE);
		cout << "Thread 2" << endl;
		GetLocalTime(&st);
		string endTime = to_string(int(st.wHour)) + ":" + to_string(int(st.wMinute)) + ":"
			+ to_string(int(st.wSecond)) + ":" + to_string(int(st.wMilliseconds));
		//cout << " x = " << ptr->getX() << " y = " << ptr->getY() << endl;
		fout << " x = " << ptr->getX() << " y = " << ptr->getY() << endl;
		ptr->setEndTime(endTime);
		ResetEvent(eventForSecondThread);
		SetEvent(eventForThirdThread);
	}
}

void WriteToLogFileFunc(shared_pointer<Point> ptr) {
	for (int index = 0; index < NUMBER_OF_ITERATIONS; index++) {
		WaitForSingleObject(eventForThirdThread, INFINITE);
		cout << "Thread 3" << endl;
		//cout << "START TIME: " << ptr->getStartTime() << endl;
		//cout << "END TIME: " << ptr->getEndTime() << endl;
		GetLocalTime(&st);
		string currentTime = to_string(int(st.wHour)) + ":" + to_string(int(st.wMinute)) + ":"
			+ to_string(int(st.wSecond)) + ":" + to_string(int(st.wMilliseconds));
		logout << currentTime << " [INFO] Start time " << ptr->getStartTime() << endl;
		logout << currentTime << " [INFO] End time " << ptr->getEndTime() << endl;
		cout << "----------------------------------------" << endl;
		ResetEvent(eventForThirdThread);
		SetEvent(eventForFirstThread);
	}
}

int main() {
	shared_pointer<Point> ptr(new Point());
	fout.open(PATH_TO_FUNCTION_RESULT_FILE, ios::trunc);
	logout.open(PATH_TO_LOG_FILE, ios::trunc);
	eventForSecondThread = CreateEvent(NULL, TRUE, TRUE, TEXT("event1"));
	eventForThirdThread = CreateEvent(NULL, TRUE, TRUE, TEXT("event2"));
	eventForFirstThread = CreateEvent(NULL, TRUE, TRUE, TEXT("event3"));
	SetEvent(eventForFirstThread);	
	std::thread th1(QuadraticFunc, ptr);
	std::thread th2(WriteResultToFileFunc, ptr);
	std::thread th3(WriteToLogFileFunc, ptr);
	th1.join();
	th2.join();
	th3.join();
	CloseHandle(eventForSecondThread);
	CloseHandle(eventForThirdThread);
	CloseHandle(eventForFirstThread);
	th1.~thread();
	th2.~thread();
	th3.~thread();
	ptr.~shared_pointer();
	fout.close();
	logout.close();
	system("pause");
}

