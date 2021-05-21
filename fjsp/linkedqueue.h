#ifndef LINKEDQUEUE_H
#define LINKEDQUEUE_H
#include <iostream>
template<class Type>
struct LNode {
	Type data;
	LNode<Type>* next;
};
template<class Type>
class LinkedQueue
{
private:
	LNode<Type>* front, * rear;
public:
	LinkedQueue();
	~LinkedQueue();
	bool AddElement(Type);
	Type DeleteElement();
	bool IsEmpty();
	void Print();
};
template<class Type>
LinkedQueue<Type>::LinkedQueue()
{
	front = new LNode<Type>;
	rear = front;
	front->next = NULL;
	//front->data = (Type)-111;
}
template<class Type>
LinkedQueue<Type>::~LinkedQueue()
{
	LNode<Type>* temp = front;
	while (temp)
	{
		LNode<Type>* tempvalue = temp;
		temp = temp->next;
		delete tempvalue;
	}
}
template<class Type>
bool LinkedQueue<Type>::AddElement(Type x)
{
	LNode<Type>* temp = new LNode<Type>;
	temp->data = x;
	temp->next = NULL;
	rear->next = temp;
	rear = temp;
	return true;
}
template<class Type>
Type LinkedQueue<Type>::DeleteElement()
{
	if (front == rear)
		return Type(-111);
	LNode<Type> * temp = front->next;
	front->next = front->next->next;
	Type x = temp->data;
	if (temp == rear)
		rear = front;
	delete temp;
	return x;
}
template<class Type>
bool LinkedQueue<Type>::IsEmpty()
{
	return (front == rear ? true : false);
}
template<class Type>
void LinkedQueue<Type>::Print()
{
	LNode<Type>* temp = front->next;
	while (temp)
	{
		std::cout << temp->data << " ";
		temp = temp->next;
	}
	std::cout << std::endl;
}
#endif