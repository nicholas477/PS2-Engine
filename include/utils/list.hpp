#pragma once

#include "assert.hpp"

template <class ContainerType>
class TLinkedListIteratorBase
{
public:
	explicit TLinkedListIteratorBase(ContainerType* FirstLink)
	    : CurrentLink(FirstLink)
	{
	}

	/**
	 * Advances the iterator to the next element.
	 */
	void Next()
	{
		check(CurrentLink);
		CurrentLink = (ContainerType*)CurrentLink->GetNextLink();
	}

	TLinkedListIteratorBase& operator++()
	{
		Next();
		return *this;
	}

	TLinkedListIteratorBase operator++(int)
	{
		auto Tmp = *this;
		Next();
		return Tmp;
	}

	/** conversion to "bool" returning true if the iterator is valid. */
	explicit operator bool() const
	{
		return CurrentLink != nullptr;
	}

	bool operator==(const TLinkedListIteratorBase& Rhs) const { return CurrentLink == Rhs.CurrentLink; }
	bool operator!=(const TLinkedListIteratorBase& Rhs) const { return CurrentLink != Rhs.CurrentLink; }

protected:
	ContainerType* CurrentLink;
};

template <class ContainerType, class ElementType>
class TLinkedListIterator: public TLinkedListIteratorBase<ContainerType>
{
	typedef TLinkedListIteratorBase<ContainerType> Super;

public:
	explicit TLinkedListIterator(ContainerType* FirstLink)
	    : Super(FirstLink)
	{
	}

	// Accessors.
	ElementType& operator->() const
	{
		checkSlow(this->CurrentLink);
		return **(this->CurrentLink);
	}

	ElementType& operator*() const
	{
		checkSlow(this->CurrentLink);
		return **(this->CurrentLink);
	}
};

template <class ContainerType, class ElementType>
class TIntrusiveLinkedListIterator: public TLinkedListIteratorBase<ElementType>
{
	typedef TLinkedListIteratorBase<ElementType> Super;

public:
	explicit TIntrusiveLinkedListIterator(ElementType* FirstLink)
	    : Super(FirstLink)
	{
	}

	// Accessors.
	ElementType& operator->() const
	{
		check(this->CurrentLink);
		return *(this->CurrentLink);
	}

	ElementType& operator*() const
	{
		check(this->CurrentLink);
		return *(this->CurrentLink);
	}
};


/**
 * Base linked list class, used to implement methods shared by intrusive/non-intrusive linked lists
 */
template <class ContainerType, class ElementType, template <class, class> class IteratorType>
class TLinkedListBase
{
public:
	/**
	 * Used to iterate over the elements of a linked list.
	 */
	typedef IteratorType<ContainerType, ElementType> TIterator;
	typedef IteratorType<ContainerType, const ElementType> TConstIterator;


	/**
	 * Default constructor (empty list)
	 */
	TLinkedListBase()
	    : NextLink(NULL)
	    , PrevLink(NULL)
	{
	}

	/**
	 * Removes this element from the list in constant time.
	 *
	 * This function is safe to call even if the element is not linked.
	 */
	void Unlink()
	{
		if (NextLink)
		{
			NextLink->PrevLink = PrevLink;
		}
		if (PrevLink)
		{
			*PrevLink = NextLink;
		}
		// Make it safe to call Unlink again.
		NextLink = nullptr;
		PrevLink = nullptr;
	}


	/**
	 * Adds this element to a list, before the given element.
	 *
	 * @param Before	The link to insert this element before.
	 */
	void LinkBefore(ContainerType* Before)
	{
		check(Before != NULL);

		PrevLink         = Before->PrevLink;
		Before->PrevLink = &NextLink;

		NextLink = Before;

		if (PrevLink != NULL)
		{
			*PrevLink = (ContainerType*)this;
		}
	}

	/**
	 * Adds this element to the linked list, after the specified element
	 *
	 * @param After		The link to insert this element after.
	 */
	void LinkAfter(ContainerType* After)
	{
		check(After != NULL);

		PrevLink  = &After->NextLink;
		NextLink  = *PrevLink;
		*PrevLink = (ContainerType*)this;

		if (NextLink != NULL)
		{
			NextLink->PrevLink = &NextLink;
		}
	}

	/**
	 * Adds this element to the linked list, replacing the specified element.
	 * This is equivalent to calling LinkBefore(Replace); Replace->Unlink();
	 *
	 * @param Replace	Pointer to the element to be replaced
	 */
	void LinkReplace(ContainerType* Replace)
	{
		check(Replace != NULL);

		ContainerType**& ReplacePrev = Replace->PrevLink;
		ContainerType*& ReplaceNext  = Replace->NextLink;

		PrevLink = ReplacePrev;
		NextLink = ReplaceNext;

		if (PrevLink != NULL)
		{
			*PrevLink = (ContainerType*)this;
		}

		if (NextLink != NULL)
		{
			NextLink->PrevLink = &NextLink;
		}

		ReplacePrev = NULL;
		ReplaceNext = NULL;
	}


	/**
	 * Adds this element as the head of the linked list, linking the input Head pointer to this element,
	 * so that when the element is linked/unlinked, the Head linked list pointer will be correctly updated.
	 *
	 * If Head already has an element, this functions like LinkBefore.
	 *
	 * @param Head		Pointer to the head of the linked list - this pointer should be the main reference point for the linked list
	 */
	void LinkHead(ContainerType*& Head)
	{
		if (Head != NULL)
		{
			Head->PrevLink = &NextLink;
		}

		NextLink = Head;
		PrevLink = &Head;
		Head     = (ContainerType*)this;
	}


	/**
	 * Returns whether element is currently linked.
	 *
	 * @return true if currently linked, false otherwise
	 */
	bool IsLinked()
	{
		return PrevLink != nullptr;
	}

	ContainerType** GetPrevLink() const
	{
		return PrevLink;
	}

	ContainerType* GetNextLink() const
	{
		return NextLink;
	}

	ContainerType* Next()
	{
		return NextLink;
	}

private:
	/** The next link in the linked list */
	ContainerType* NextLink;

	/** Pointer to 'NextLink', within the previous link in the linked list */
	ContainerType** PrevLink;


	friend TIterator begin(ContainerType& List) { return TIterator(&List); }
	friend TConstIterator begin(const ContainerType& List) { return TConstIterator(const_cast<ContainerType*>(&List)); }
	friend TIterator end(ContainerType& List) { return TIterator(nullptr); }
	friend TConstIterator end(const ContainerType& List) { return TConstIterator(nullptr); }
};

/**
 * Encapsulates a link in a single linked list with constant access time.
 *
 * This linked list is non-intrusive, i.e. it stores a copy of the element passed to it (typically a pointer)
 */
template <class ElementType>
class TLinkedList: public TLinkedListBase<TLinkedList<ElementType>, ElementType, TLinkedListIterator>
{
	typedef TLinkedListBase<TLinkedList<ElementType>, ElementType, TLinkedListIterator> Super;

public:
	/** Default constructor (empty list). */
	TLinkedList()
	    : Super()
	{
	}

	/**
	 * Creates a new linked list with a single element.
	 *
	 * @param InElement The element to add to the list.
	 */
	explicit TLinkedList(const ElementType& InElement)
	    : Super()
	    , Element(InElement)
	{
	}


	// Accessors.
	ElementType* operator->()
	{
		return &Element;
	}
	const ElementType* operator->() const
	{
		return &Element;
	}
	ElementType& operator*()
	{
		return Element;
	}
	const ElementType& operator*() const
	{
		return Element;
	}


private:
	ElementType Element;
};

/**
 * Encapsulates a link in a single linked list with constant access time.
 * Structs/classes must inherit this, to use it, e.g: struct FMyStruct : public TIntrusiveLinkedList<FMyStruct>
 *
 * This linked list is intrusive, i.e. the element is a subclass of this link, so that each link IS the element.
 *
 * Never reference TIntrusiveLinkedList outside of the above class/struct inheritance, only ever refer to the struct, e.g:
 *	FMyStruct* MyLinkedList = NULL;
 *
 *	FMyStruct* StructLink = new FMyStruct();
 *	StructLink->LinkHead(MyLinkedList);
 *
 *	for (FMyStruct::TIterator It(MyLinkedList); It; It.Next())
 *	{
 *		...
 *	}
 */
template <class ElementType>
class TIntrusiveLinkedList: public TLinkedListBase<ElementType, ElementType, TIntrusiveLinkedListIterator>
{
	typedef TLinkedListBase<ElementType, ElementType, TIntrusiveLinkedListIterator> Super;

public:
	/** Default constructor (empty list). */
	TIntrusiveLinkedList()
	    : Super()
	{
	}
};