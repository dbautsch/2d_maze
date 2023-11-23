#ifndef STACK_H_INCLUDED
#define STACK_H_INCLUDED

#include <cstdlib>

template <typename T>
class Stack
{
	public:
		struct StackElem
		{
			T val;
			StackElem* prev;
		};

		Stack()
		{
			top = NULL;
		}

		~Stack()
		{
			if (top != NULL)
			{
				StackElem* helper;

				while (top->prev != NULL)
				{
					helper = top->prev;
					delete top;
					top = helper;
				}
			}
		}

		void Push(T val)
		{
			StackElem* newElem = new StackElem;
			newElem->val = val;
			newElem->prev = top;
			top = newElem;
		}

		T Pop()
		{
			if (top == NULL)
				return 0;

			T val = top->val;
			StackElem* helper = top->prev;
			delete top;
			top = helper;

			return val;
		}

		bool HasElements()
		{
		    return top != NULL;
		}

	private:
		StackElem* top;
};

#endif
