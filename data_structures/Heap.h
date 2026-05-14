// By: Gonçalo Leão
// Based on MutablePriorityQueue.h

#ifndef DA_TP_CLASSES_HEAP
#define DA_TP_CLASSES_HEAP

#include <vector>
using namespace std;

class Heap {
public:
    Heap();
    Heap(vector<int> v);
	void insert(int x);
	int extractMin();
	bool empty();
private:
    vector<int> elems;
    void heapifyUp(unsigned int i);
    void heapifyDown(unsigned int i);
};

#endif /* DA_TP_CLASSES_HEAP */
