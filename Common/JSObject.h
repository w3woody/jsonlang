//
//  JSObject.h
//  AnalyzeJSON
//
//  Created by William Woody on 8/15/14.
//  Copyright (c) 2014 William Woody. All rights reserved.
//

#ifndef AnalyzeJSON_JSObject_h
#define AnalyzeJSON_JSObject_h

#include <stdlib.h>

/************************************************************************/
/*																		*/
/*	Basic Object Definitions											*/
/*																		*/
/************************************************************************/

/*	JSObject
 *
 *		Reference counting object
 */

class JSObject
{
	public:
							JSObject();
		virtual				~JSObject();

		void				addRef()
								{
									++refCount;
								}
		void				release()
								{
									--refCount;
									if (refCount == 0) {
										delete this;
									}
								}

	private:
		uint32_t			refCount;
};

/*	ref
 *
 *		Reference holder; a template which automatically increases and
 *	decreases the reference count to the objects that this holds
 */

template <class T> class ref
{
	public:
		ref()
			{
				v = NULL;
			}

		ref(T *ptr)
			{
				v = ptr;
				if (v) v->addRef();
			}
		ref(const ref &ref)
			{
				v = ref.v;
				if (v) v->addRef();
			}
		~ref()
			{
				if (v) v->release();
			}

		ref &operator = (const ref& ptr)
			{
				reset(ptr.v);
				return *this;
			}

		ref &operator = (T *ptr)
			{
				reset(ptr);
				return *this;
			}

		operator T*()
			{
				return v;
			}

		void reset(T* ptr = 0)
			{
				if (ptr) ptr->addRef();
				if (v) v->release();
				v = ptr;
			}

		T* operator ->() const
			{
				return v;
			}
		T& operator *() const
			{
				return *v;
			}

		T* get() const
			{
				return v;
			}

		int operator !() const			{ return (v == 0); }
		int operator == (const ref& ptr) const { return (v == ptr.v); }
		int operator != (const ref& ptr) const { return (v != ptr.v); }
		int operator == (const T* a) const { return (v == a); }
		int operator != (const T* a) const { return (v != a); }

	private:
		T *v;
};


#endif
