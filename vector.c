/** Generic Vector data type (container)
 * Stores pointer to user provided elements of generic type (void*)
 * The Vector is heap allocated and can grow and shrink on demand.
 *
 * Vector indexing starts at 1
 *
 * Author: gilkolin at g-mail (Based on header file by Muhammad Z & Alexander Katz)
 *
 * Created: ===
 * bugs: destroy, (for_each?))
 */

#include <stdlib.h>
#include <stdio.h>
#include "Vector.h"
#include "Magic.h"

struct Vector{
		size_t m_nItems;
		size_t m_extendSize;
		size_t m_initialCapacity;
		size_t m_capacity;						/*problem member name "size" (VECTOR_SIZE)*/
		void** m_myArray;
		int m_magicNum;				/*void* */
};


#define YES_MAGIC				VECTOR_YES_MAGIC
#define NO_MAGIC				VECTOR_NO_MAGIC

#define IS_VALID(V)				((V) && (YES_MAGIC == ((V)->m_magicNum)))
#define IS_FULL(V)				((V)->m_capacity <= ((V)->m_nItems+1)*sizeof(void*))
#define STATIC_SIZE(V)			((V)->m_extendSize == 0)
#define IS_EMPTY(V)				(0 == (V)->m_nItems)
#define NEED_DECREASE_SIZE(V)	((((V)->m_nItems)*sizeof(void*) + (V)->m_extendSize) * 2 < (V)->m_capacity)	/*if feture new size smaller twise of size, need to reallocated down*/

#define INDEX_TOO_BIG(V,I)		((I) > (V)->m_nItems)
#define INDEX_TOO_SMALL(I)		((I) <= 0)

/*---------------------------side func decleration----------------------------------*/

static Vector_Result ResizeVector(Vector* _vector, size_t _sizeChange);

/*---------------------------API func----------------------------------*/


/* @brief Function dynamically creates a new vector ADT of a given capacity and extent */
Vector* Vector_Create(size_t _initialCapacity, size_t _extendSize)
{
	Vector* _vector;

	if (_initialCapacity < 0 || _extendSize < 0)
	{
		return NULL;
	}

	_vector = (Vector*)malloc(sizeof(Vector));
	if (_vector == NULL)
	{
		return NULL;
	}
	_vector->m_myArray = (void*)malloc(_initialCapacity + 1); 	/*vector start from 1*/
	if (_vector->m_myArray == NULL)
	{
		free(_vector);
		return NULL;
	}

	_vector->m_nItems = 0;
	_vector->m_extendSize = _extendSize;
	_vector->m_initialCapacity = _initialCapacity + 1;
	_vector->m_capacity = _initialCapacity + 1;
	_vector->m_magicNum = YES_MAGIC;

	return _vector;
}

/* @brief Dynamically deallocates a previously allocated vector, possibly releasing the elements*/
void Vector_Destroy(Vector* *_vector, void (*_elementDestroy)(void* _element))
{
	void* dataVector = NULL;
	
	if (!IS_VALID(*_vector))
	{
		return;		
	}

	(*_vector)->m_magicNum = NO_MAGIC;
	
	while ((*_vector)->m_nItems > 0)
	{
		Vector_Remove((*_vector), &dataVector);
		if (_elementDestroy)
		{
			_elementDestroy(dataVector);
		}
	}
	
	free((*_vector)->m_myArray);
	(*_vector)->m_myArray = NULL;
	free(*_vector);
	*_vector = NULL;
}



/* @brief Add an element (element) to the end of the vector*/
Vector_Result Vector_Append(Vector* _vector, void* _element)
{
	if (!IS_VALID(_vector))
	{
		return VECTOR_UNINITIALIZED_ERROR;		
	}

	if (IS_FULL(_vector))
	{
		if (STATIC_SIZE(_vector))
		{
			return VECTOR_OVERFLOW_ERROR;
		}
		if (VECTOR_SUCCESS != ResizeVector(_vector, _vector->m_extendSize))
		{
			return VECTOR_ALLOCATION_ERROR;
		}
	}

	_vector->m_myArray[_vector->m_nItems + 1] = _element;
	++_vector->m_nItems;

	return VECTOR_SUCCESS;
}

/* @brief Remove an element from the end of the vector.*/
Vector_Result Vector_Remove(Vector* _vector, void* *_pValue)
{
	if (!_vector || !_pValue)							/*need check magic but to do destroy too (after magic=NO)*/
	{
		return VECTOR_UNINITIALIZED_ERROR;		
	}
	if (IS_EMPTY(_vector))
	{
		return VECTOR_UNDERFLOW_ERROR;
	}

	*_pValue = _vector->m_myArray[_vector->m_nItems];
	_vector->m_nItems--;

	if (NEED_DECREASE_SIZE(_vector))
	{
		if (VECTOR_SUCCESS != ResizeVector(_vector, -1*(_vector->m_extendSize)))
		{
			return VECTOR_ALLOCATION_ERROR;
		}			
	}

	return VECTOR_SUCCESS;
}

/* @brief Retrieve a specified elements's value*/
Vector_Result Vector_Get(const Vector* _vector, size_t _index, void* *_pValue)
{
	if (!IS_VALID(_vector) || !_pValue)		/*not check?*/
	{
		return VECTOR_UNINITIALIZED_ERROR;		
	}
	
	if (IS_EMPTY(_vector) || INDEX_TOO_BIG(_vector, _index) || INDEX_TOO_SMALL(_index))
	{
		return VECTOR_INDEX_OUT_OF_BOUNDS_ERROR;		
	}

	*_pValue = _vector->m_myArray[_index];
	return VECTOR_SUCCESS;
}

 /* @brief Function changes a specified element's value*/
Vector_Result Vector_Set(Vector* _vector, size_t _index, void*  _value)
{
	if (!IS_VALID(_vector) || !_value)
	{
		return VECTOR_UNINITIALIZED_ERROR;		
	}

	if (IS_EMPTY(_vector) || INDEX_TOO_BIG(_vector, _index) || INDEX_TOO_SMALL(_index))
	{
		return VECTOR_INDEX_OUT_OF_BOUNDS_ERROR;
	}
	
	_vector->m_myArray[_index] = _value;
	return VECTOR_SUCCESS;
}

 /* @brief Function returns the actual number of elements currently in the vector.*/
size_t Vector_Size(const Vector* _vector)				/*problem member name "size"*/
{
	if (!IS_VALID(_vector))
	{
		return VECTOR_UNINITIALIZED_ERROR;		
	}
	
	return _vector->m_nItems;
}

 /* @brief Function gets the current capacity of the  vector.*/
size_t Vector_Capacity(const Vector* _vector)
{
	if (!IS_VALID(_vector))
	{
		return VECTOR_UNINITIALIZED_ERROR;		
	}
	
	return _vector->m_capacity;				/*problem member name "size"*/
}


 /* @brief Iterates over all elements in a vector, executing a user-specified action*/
size_t Vector_ForEach(const Vector* _vector, VectorElementAction _action, void* _context)
{
	int i;
	size_t count;
	int result;
	
	if (!IS_VALID(_vector))
	{
		return VECTOR_UNINITIALIZED_ERROR;		
	}
	
	for (i = 1; i <= _vector->m_nItems; ++i)
	{
		result = _action(_vector->m_myArray[i], i, _context);
		if (!result)
		{
			++count;
		}
	}
	return count;
}

/*---------------------------side func----------------------------------*/

static Vector_Result ResizeVector(Vector* _vector, size_t _sizeChange)
{
	void** temp;
	temp = (void**)realloc(_vector->m_myArray,sizeof(void*)*(_vector->m_capacity + _sizeChange));
	if (temp == NULL)
	{
		return VECTOR_ALLOCATION_ERROR;
	}
	
	_vector->m_myArray = temp;
	_vector->m_capacity += _sizeChange;
	
	return VECTOR_SUCCESS;
}



/*
void PrintVectorData(Vector* _vector)
{
	int i;
	if ((0 == _vector->m_nItems) || (_vector==NULL) || (_vector->m_magicNum!=MAGIC))
	{
		return;
	}
	printf("your data");
	printf("num of item is: %lu\n",_vector->m_nItems);
	printf("extension is: %lu\n",_vector->m_extendSize);
	printf("initial size is: %lu\n",_vector->m_initialCapacity);
	printf("current size is: %lu\n",_vector->m_capacity);
	if (_vector->m_capacity>0)
	{
		printf("your array is: ");
		for (i=0; i<_vector->m_nItems; i++)
		{
			printf("%d,",_vector->m_myArray[i+1]);
		}
		printf("\n\n");
	}
}


void PrintVector(Vector* _vector)
{
	int i;
	if ((0 == _vector->m_nItems) || (_vector==NULL) || (_vector->m_magicNum!=MAGIC))
	{
		return;
	}

	if (_vector->m_capacity>0)
	{
		printf("your array is: ");
		for (i=0; i<_vector->m_nItems; i++)
		{
			printf("%p,",_vector->m_myArray[i+1]);
		}
		printf("\n\n");
	}
}
*/


/*
Vector_Result VectorItemsNum(Vector* _vector,size_t* _numOfItems)
{
	if ((_vector==NULL) || (_vector->m_magicNum!=MAGIC))
	{
		return VECTOR_UNINITIALIZED_ERROR;
	}
	*_numOfItems = _vector->m_nItems;
	return VECTOR_SUCCESS;
}
*/
