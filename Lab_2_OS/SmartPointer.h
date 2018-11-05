#pragma once

#ifndef SMART_POINTER_HEADER
#define SMART_POINTER_HEADER

#include <cstddef>
#include <mutex>

using namespace std;

namespace my_experimental
{
	struct ref_count_data{
		unsigned int use_count;
		unsigned int weak_count;
	};

	class reference_count
	{
		public:
		ref_count_data ref_counts;
		mutable mutex ref_count_mutex;
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
	class shared_pointer
	{
	private:
		T* pdata;            // pointer

		//Copy constructor for dynamic_pointer_cast and weak_pointer lock
		shared_pointer(T* pvalue_arg, reference_count* rc_arg) : pdata(NULL), rc(rc_arg) {
			if (NULL == pvalue_arg) {
				rc = NULL;
			}
			else {
				pdata = static_cast<T*>(rc->get_shared_ref(pvalue_arg));
				if (NULL == pdata) {
					rc = NULL;
				}
			}
		}

	public:
		reference_count* rc; // Reference count
		shared_pointer() : pdata(NULL), rc(NULL) {} //default constructor

		shared_pointer(T* pvalue) : pdata(pvalue), rc(NULL){
			if (NULL != pdata){
				rc = new reference_count();
			}
		}

		//Copy constructor
		shared_pointer(const shared_pointer<T>& sp) : pdata(NULL), rc(sp.rc){
			if (NULL != rc){
				pdata = static_cast<T*>(rc->get_shared_ref(sp.pdata));

				if (NULL == pdata){
					rc = NULL;
				}
			}
		}

		~shared_pointer()
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

		void increment_reference_count()
		{
			if (NULL != rc)
			{
				lock_guard<mutex> rc_lock(rc->ref_count_mutex);
				rc->ref_counts.use_count++;
			}
		}

		void decrement_reference_count_and_delete_if_needed()
		{
			if (NULL != rc)
			{
				bool delete_pdata = false;
				bool delete_rc = false;

				{ //lock
					lock_guard<mutex> rc_lock(rc->ref_count_mutex);

					rc->ref_counts.use_count--;

					if (0 == rc->ref_counts.use_count)
					{
						rc->ref_counts.weak_count--;
						delete_pdata = true;
					}

					if (0 == rc->ref_counts.weak_count)
					{
						delete_rc = true;
					}
				} //unlock

				if (delete_pdata)
				{
					//best not to call unknown code from locked context so delete here
					//for example the destructor could end up doing something with a
					//weak_pointer to pdata (that would be poorly written code but we know nothing
					//about the destructor we are calling so best not to call from locked context)
					delete pdata;
				}

				if (delete_rc)
				{
					//rc contains the mutex that we were locking so we must delete outside of locked context
					//this is safe as we know that no one else holds a reference to rc once we get here
					delete rc;
				}
			}
		}

		shared_pointer<T>& operator = (const shared_pointer<T>& sp)
		{
			if (this != &sp) // Avoid self assignment
			{
				decrement_reference_count_and_delete_if_needed();

				pdata = sp.pdata;
				rc = sp.rc;

				increment_reference_count();
			}

			return *this;
		}

		template <class Y, class Z> friend shared_pointer<Y> dynamic_pointer_cast(const shared_pointer<Z>& sp);
	};
	template <class T, class U> shared_pointer<T> dynamic_pointer_cast(const shared_pointer<U>& sp){
		return shared_pointer<T>(dynamic_cast<T*>(sp.pdata), sp.rc);
	}
}

#endif