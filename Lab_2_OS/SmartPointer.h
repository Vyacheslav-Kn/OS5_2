#pragma once

#ifndef SMART_POINTER_HEADER
#define SMART_POINTER_HEADER

#include <cstddef>
#include <mutex>

using namespace std;

namespace SmartPointer
{
	struct ref_count_data{
		unsigned int use_count;
	};

	class reference_count
	{
		private:
		ref_count_data ref_counts;
		mutable mutex ref_count_mutex;

		public:
		reference_count()
		{
			ref_counts.use_count = 1;
		}

		void* get_shared_ref(void* pdata)
		{
			lock_guard<mutex> lock(ref_count_mutex);

			if (ref_counts.use_count){
				ref_counts.use_count++;
				return pdata;
			}
			else{
				return NULL;
			}
		}

		ref_count_data release_shared_ref(){
			lock_guard<mutex> lock(ref_count_mutex);
			ref_counts.use_count--;
			return ref_counts;
		}

		unsigned int get_use_count() const
		{
			lock_guard<mutex> lock(ref_count_mutex);
			return ref_counts.use_count;
		}
	};

	template <typename T>
	class SafeSmartPointer
	{
	private:
		T* pdata;            // pointer
		reference_count* rc; // Reference count

	public:
		SafeSmartPointer() : pdata(NULL), rc(NULL) {} //default constructor

		SafeSmartPointer(T* pvalue) : pdata(pvalue), rc(NULL){
			if (NULL != pdata){
				rc = new reference_count();
			}
		}

		//Copy constructor
		SafeSmartPointer(const SafeSmartPointer<T>& sp) : pdata(NULL), rc(sp.rc){
			if (NULL != rc){
				pdata = static_cast<T*>(rc->get_shared_ref(sp.pdata));

				if (NULL == pdata){
					rc = NULL;
				}
			}
		}

		~SafeSmartPointer()
		{
			if (NULL != rc){
				ref_count_data updated_counts = rc->release_shared_ref();

				if (0 == updated_counts.use_count){
					delete pdata;
				}
			}
		}

		T& operator* () const{
			return *pdata;
		}

		T* operator-> () const{
			return pdata;
		}
		
	};
}

#endif