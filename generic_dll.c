/**
 *  @file generic dll.c
 *  @brief Create a Generic Double Linked List data type
 *
 *  @details Implemented as double linked list using head and tail sentinels.
 *  Nodes are allocated from heap and store pointer to user provided element.
 *
 *  @author gilkolin at g-mail (Based on header file by Muhammad)
 *
 *  @bug No known bugs.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "List.h"
#include "List_itr.h"
#include "List_functions.h"
#include "Magic.h"


typedef struct Node Node;
struct Node
{
	void*		m_data;
	Node*		m_next;
	Node*		m_prev;
};


struct List
{
	size_t		m_magicNum;
	Node		m_head;
	Node		m_tail;
};


#define LIST_HEAD(L)			(&(L)->m_head)
#define LIST_TAIL(L)			(&(L)->m_tail)
#define LIST_FIRST(L)			(LIST_HEAD(L)->m_next)
#define LIST_LAST(L)			(LIST_TAIL(L)->m_prev)
#define LIST_IS_EMPTY(L)		(LIST_FIRST(L) == LIST_TAIL(L))

#define IS_LIST_VALID(L) 		((L) && (YES_MAGIC == ((L)->m_magicNum)))

#define ITR_IS_TAIL(I)			(!((Node*)I)->m_next)
#define ITR_IS_HEAD(I)			(!((Node*)I)->m_prev)
#define ITR_IS_EDGE(I)			(ITR_IS_TAIL(I) || ITR_IS_HEAD(I))

# define YES_MAGIC				DLL_YES_MAGIC
# define NO_MAGIC				DLL_NO_MAGIC

# define FALSE 0
# define TRUE !FALSE 


/*-----------------------------------side func--------------------------------------*/

static Node* NodeCreate (Node* _bfr, void* _data);
static void* NodeRemove (Node* _which);

/*-----------------------------------API func---------------------------------------*/


List* List_Create(void)
{
	List* _pList;

	_pList = (List*)malloc(sizeof(List));
	
	if (!_pList) 
	{
		return NULL;
	}
	
	_pList->m_head.m_next = &_pList->m_tail;
	_pList->m_head.m_prev = NULL;
	_pList->m_tail.m_next = NULL;
	_pList->m_tail.m_prev = &_pList->m_head;
	_pList->m_magicNum = YES_MAGIC;

	return _pList;
}



void List_Destroy(List** _pList, void (*_elementDestroy)(void* _item))
{	
	if (_pList && IS_LIST_VALID(*_pList))
	{
		(*_pList)->m_magicNum = NO_MAGIC;

		while (!LIST_IS_EMPTY(*_pList))
		{
			void * data = NodeRemove(LIST_FIRST(*_pList));
			if (_elementDestroy)
			{
				_elementDestroy(data);
			}
		}

		free(*_pList);
		*_pList = NULL;
	}
}


List_Result List_PushHead(List* _list, void* _item)
{
	Node* _newNode = NULL;
	
	if (!IS_LIST_VALID(_list)) 
	{
			return LIST_UNINITIALIZED_ERROR;
	}

	_newNode = NodeCreate (LIST_FIRST(_list), _item);
	return (_newNode) ? LIST_SUCCESS : LIST_ALLOCATION_ERROR;
}


List_Result List_PushTail(List* _list, void* _item)
{
	Node* _newNode = NULL;
	
	if (!IS_LIST_VALID(_list)) 
	{
		return LIST_UNINITIALIZED_ERROR;
	}

	_newNode = NodeCreate (LIST_TAIL(_list), _item);
	return (_newNode) ? LIST_SUCCESS : LIST_ALLOCATION_ERROR;
}

List_Result List_PopHead(List* _list, void* *_pItem)
{
	if (!IS_LIST_VALID(_list) || (!_pItem))
	{
		return LIST_UNINITIALIZED_ERROR;
	}
	if (LIST_IS_EMPTY(_list))
	{
		return LIST_UNDERFLOW_ERROR;
	}

	*_pItem = NodeRemove(LIST_FIRST(_list));
	return LIST_SUCCESS;
}


List_Result List_PopTail(List* _list, void* *_pItem)
{
	if (!IS_LIST_VALID(_list) || (!_pItem))
	{
		return LIST_UNINITIALIZED_ERROR;
	}
	if (LIST_IS_EMPTY(_list))
	{
		return LIST_UNDERFLOW_ERROR;
	}
	
	*_pItem = NodeRemove(LIST_LAST(_list));
	return LIST_SUCCESS;
}


size_t List_Size(const List* _list)
{	
	const Node* pNode = NULL;
	size_t numOfItems = 0;

	if (!IS_LIST_VALID(_list))
	{
		return 0;
	}
	
	for (pNode=LIST_FIRST(_list); pNode != LIST_TAIL(_list); pNode = pNode->m_next)
	{
		++numOfItems;
	}
	return numOfItems;
}

int List_IsEmpty(const List* _list)
{
	return (!IS_LIST_VALID(_list) || (LIST_FIRST(_list) == LIST_TAIL(_list))) ? TRUE : FALSE;
}


void List_Print(const List* _list)
{
	Node* curr;
	
	if (!IS_LIST_VALID(_list))
	{
		return;
	}
	
	printf("List is ={");
	for(curr = LIST_FIRST(_list);
		curr != LIST_TAIL(_list);
		curr = curr->m_next)
	{
		printf("%d ", *((int*)curr->m_data));
	}
	printf("}\n");
}

/*-----------------------------Iterator func--------------------------*/

ListItr ListItr_Begin(const List* _list)
{
	if (!IS_LIST_VALID(_list))
	{
		return NULL;
	}
	return (ListItr)_list->m_head.m_next;
}

ListItr ListItr_End(const List* _list)
{
	return (!IS_LIST_VALID(_list)) ? NULL : (ListItr)LIST_TAIL(_list);
}

int ListItr_Equals(const ListItr _a, const ListItr _b)														
{	
	return (_a && _b) ? ((Node*)_a == (Node*)_b) : FALSE;
}

ListItr ListItr_Next(ListItr _itr)
{
	if (!_itr)
	{
		return NULL;
	}
	if ((ListItr)((Node*)_itr)->m_next == NULL)
	{
		return (ListItr)((Node*)_itr);
	}
	return (ListItr)((Node*)_itr)->m_next;
}

ListItr ListItr_Prev(ListItr _itr)
{
	if (!_itr)
	{
		return NULL;
	}
	if ((ListItr)((Node*)_itr)->m_prev == NULL || ((ListItr)((Node*)_itr)->m_prev->m_prev == NULL && (ListItr)((Node*)_itr)->m_next == NULL))	/*macro head || tail+empty*/
	{
		return (ListItr)((Node*)_itr);
	}
	return (ListItr)((Node*)_itr)->m_prev;
}

void* ListItr_Get(ListItr _itr)
{
	Node* node = (Node*)_itr;
	
/*	return (!_itr || ITR_IS_EDGE(_itr)) ? NULL : ((Node*)_itr)->m_data;*/
	if (!node || ITR_IS_TAIL(_itr))
	{
		return NULL;
	}
	return node->m_data;
}

void* ListItr_Set(ListItr _itr, void* _element)
{
	void* dataToReturn = NULL;
	
	if (!_itr || !ITR_IS_EDGE(_itr))
	{
		dataToReturn = ((Node*)_itr)->m_data;
		((Node*)_itr)->m_data = _element;
	}
	return dataToReturn;
}

ListItr ListItr_InsertBefore(ListItr _itr, void* _element)
{
	if (!_itr || ITR_IS_HEAD((Node*)_itr))
	{
		return NULL;
	}
	return (ListItr)NodeCreate((Node*)_itr, _element);
}

void* ListItr_Remove(ListItr _itr)
{
	if (!_itr || ITR_IS_EDGE(_itr))
	{
		return NULL;
	}
	return NodeRemove((Node*)_itr);
}

ListItr ListItr_FindFirst(ListItr _begin, ListItr _end, PredicateFunction _predicate, void* _context)
{
	ListItr i=NULL;
	
	if((_begin) && (_end) && (_predicate))
	{
		for(i = _begin; !(ListItr_Equals(i, _end)); i = ListItr_Next(i))
		{
			if(_predicate(ListItr_Get(i), _context))
			{
				return i;
			}
		}
	}
	return _end;		/*return i*/
}

size_t ListItr_CountIf(ListItr _begin, ListItr _end, PredicateFunction _predicate, void* _context)								
{
	ListItr i = NULL;
	size_t count = 0;
	
	if((_begin) && (_end) && (_predicate))
	{
		for(i = _begin; !(ListItr_Equals(i, _end)); i = ListItr_Next(i))
		{
			if( _predicate( ListItr_Get(i) , _context) )
			{
				++count;
			}
		}
	}
	return count;
}

ListItr ListItr_ForEach(ListItr _begin, ListItr _end, ListActionFunction _action, void* _context)
{
	ListItr i = NULL;
	
	if((_begin) && (_end) && (_action))
	{
		for(i = _begin; !(ListItr_Equals(i, _end)); i = ListItr_Next(i))
		{
			if(!_action(ListItr_Get(i), _context))
			{
				break;
			}
		}
	}
	return i;	
}

/*----------------------------------side func---------------------------------------*/

static Node* NodeCreate (Node* _bfr, void* _data)
{
	Node* newNode;

	newNode = (Node*)malloc(sizeof(Node));
	if (!newNode) 
	{
		return NULL;
	}
	
	newNode->m_next = _bfr;
	newNode->m_prev = _bfr->m_prev;
	_bfr->m_prev = newNode;
	newNode->m_prev->m_next = newNode;		/*macro*/
	
	newNode->m_data = _data;
	return newNode;
}

static void* NodeRemove (Node* _which)
{
	void* item = _which->m_data;

	_which->m_prev->m_next = _which->m_next;
	_which->m_next->m_prev = _which->m_prev;

	free(_which);
	return item;
}

/*
int main()
{return 0;}
*/
