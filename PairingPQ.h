

#ifndef PAIRINGPQ_H
#define PAIRINGPQ_H

#include "Eecs281PQ.h"
#include <deque>
#include <utility>

using namespace std;

// A specialized version of the 'priority_queue' ADT implemented as a pairing heap.
template<typename TYPE, typename COMP_FUNCTOR = std::less<TYPE>>
class PairingPQ : public Eecs281PQ<TYPE, COMP_FUNCTOR> {
    // This is a way to refer to the base class object.
    using BaseClass = Eecs281PQ<TYPE, COMP_FUNCTOR>;

public:
    // Each node within the pairing heap
    class Node {
        public:
            // TODO: After you add add one extra pointer (see below), be sure to
            // initialize it here.
            explicit Node(const TYPE &val)
        : elt{ val }, child{ nullptr }, sibling{ nullptr }, parent { nullptr }
            {}

            // Description: Allows access to the element at that Node's position.
			// There are two versions, getElt() and a dereference operator, use
			// whichever one seems more natural to you.
            // Runtime: O(1) - this has been provided for you.
            const TYPE &getElt() const { return elt; }
            const TYPE &operator*() const { return elt; }

            // The following line allows you to access any private data members of this
            // Node class from within the PairingPQ class. (ie: myNode.elt is a legal
            // statement in PairingPQ's add_node() function).
            friend PairingPQ;

        private:
            TYPE elt;
            Node *child;
            Node *sibling;
            Node *parent;
            // TODO: Add one extra pointer (parent or previous) as desired.
    }; // Node


    // Description: default Construct an empty priority_queue with an optional comparison functor.
    // Runtime: O(1)
    explicit PairingPQ(COMP_FUNCTOR comp = COMP_FUNCTOR()) :
        BaseClass{ comp } {
        // TODO: Implement this function.
        numNodes = 0;
        root = nullptr;
    } // PairingPQ()


    // Description: range based Construct a priority_queue out of an iterator range with an optional
    //              comparison functor.
    // Runtime: O(n) where n is number of elements in range.
    // TODO: when you implement this function, uncomment the parameter names.
    template<typename InputIterator>
    PairingPQ(InputIterator start, InputIterator end, COMP_FUNCTOR comp = COMP_FUNCTOR()) :
    BaseClass{ comp }, root { nullptr }, numNodes { 0 }{
        // TODO: Implement this function.
            while (start != end){
                this->push(*start);
                start++;
            }
    } // PairingPQ()


    // Description: Copy constructor.
    // Runtime: O(n)
    PairingPQ(const PairingPQ& other) :
        BaseClass{ other.compare } {
        // TODO: Implement this function.
            numNodes = 0;
            root = nullptr;
            deque<Node *> dq;
            dq.push_back(other.root);
            while (!dq.empty()){
                Node * temp = dq.front();
                if (temp->child != nullptr){
                    dq.push_back(temp->child);
                }
                if (temp->sibling != nullptr){
                    dq.push_back(temp->sibling);
                }
                push(temp->elt);
                dq.pop_front();
            }
    } // PairingPQ()


    // Description: Copy assignment operator.
    // Runtime: O(n)
    // TODO: when you implement this function, uncomment the parameter names.
    PairingPQ& operator=(const PairingPQ& rhs) {
        // TODO: Implement this function.
        
        PairingPQ temp(rhs);
        
        swap(numNodes, temp.numNodes);
        swap(root, temp.root);

        return *this;
    } // operator=()


    // Description: Destructor
    // Runtime: O(n)
    ~PairingPQ() {
        // TODO: Implement this function.
        if (root == nullptr){
            return;
        }
        deque<Node *> dq;
        dq.push_back(root);
        while (!dq.empty()){
            Node * temp = dq.front();
            if (temp->child != nullptr){
                dq.push_back(temp->child);
            }
            if (temp->sibling != nullptr){
                dq.push_back(temp->sibling);
            }
            delete temp;
            dq.pop_front();
        }
    } // ~PairingPQ()


    // Description: Assumes that all elements inside the priority_queue are out of order and
    //              'rebuilds' the priority_queue by fixing the priority_queue invariant.
    // Runtime: O(n)
    virtual void updatePriorities() {
        // TODO: Implement this function.
        if (root == nullptr){
            return;
        }
        deque<Node *> dq;
        dq.push_back(root);
        while (!dq.empty()){
            Node * temp = dq.front();
            dq.pop_front();
            if (temp->child != nullptr){
                dq.push_back(temp->child);
            }
            if (temp->sibling != nullptr){
                dq.push_back(temp->sibling);
            }
            temp->sibling = nullptr;
            temp->child = nullptr;
            //now add it to reconstructed heap
            if (temp->parent == nullptr){
                root = temp;
            }
            else {
                temp->parent = nullptr;
                root = meld(root, temp);
            }
        }
    } // updatePriorities()


    // Description: Add a new element to the priority_queue. This is almost done,
    //              in that you should implement push functionality in the addNode()
    //              function, and this function should call addNode().
    // Runtime: O(1)
    // TODO: when you implement this function, uncomment the parameter names.
    virtual void push(const TYPE & val) {
        // TODO: Implement this function.
        addNode(val);
    } // push()


    // Description: Remove the most extreme (defined by 'compare') element from
    //              the priority_queue.
    // Note: We will not run tests on your code that would require it to pop an
    // element when the priority_queue is empty. Though you are welcome to if you are
    // familiar with them, you do not need to use exceptions in this project.
    // Runtime: Amortized O(log(n))
    virtual void pop() {
        // TODO: Implement this function.
        
        if (root->child == nullptr){
            delete root;
            root = nullptr;
            numNodes--;
            return;
        }
        Node * temp = root->child;
        delete root; // delete what root points to
        numNodes--;
        
        
        if (temp == nullptr){
            root = nullptr;
        }
        
        deque<Node *> dq;
        while (temp->sibling != nullptr){
            dq.push_back(temp);
            temp = temp->sibling;
        }
        
        if (temp->sibling == nullptr){
            dq.push_back(temp);
        }
        
        while (dq.size() > 1){
            Node * first = dq.front();
            dq.pop_front();
            first->sibling = nullptr;
            first->parent = nullptr;
            Node * second = dq.front();
            dq.pop_front();
            second->sibling = nullptr;
            second->parent = nullptr;
            
            dq.push_back(meld(first, second));
        }
        
        root = dq.front();
        dq.pop_front();
        
    } // pop()


    // Description: Return the most extreme (defined by 'compare') element of
    //              the heap.  This should be a reference for speed.  It MUST be
    //              const because we cannot allow it to be modified, as that
    //              might make it no longer be the most extreme element.
    // Runtime: O(1)
    virtual const TYPE & top() const {
        // TODO: Implement this function
        return root->elt;
        // These lines are present only so that this provided file compiles.
        //static TYPE temp; // TODO: Delete this line
        //return temp;      // TODO: Delete or change this line
    } // top()


    // Description: Get the number of elements in the priority_queue.
    // Runtime: O(1)
    virtual std::size_t size() const {
        // TODO: Implement this function
        return size_t(numNodes); // TODO: Delete or change this line
    } // size()

    // Description: Return true if the priority_queue is empty.
    // Runtime: O(1)
    virtual bool empty() const {
        // TODO: Implement this function
        if (numNodes == 0){
            return true;
        }
        else {
            return false;
        }
        //return true; // TODO: Delete or change this line
    } // empty()


    // Description: Updates the priority of an element already in the priority_queue by
    //              replacing the element refered to by the Node with new_value.
    //              Must maintain priority_queue invariants.
    //
    // PRECONDITION: The new priority, given by 'new_value' must be more extreme
    //               (as defined by comp) than the old priority.
    //
    // Runtime: As discussed in reading material.
    // TODO: when you implement this function, uncomment the parameter names.
    void updateElt(Node* node, const TYPE & new_value) {
        // TODO: Implement this function
        
        node->elt = new_value;
        Node * temp = node->parent;
        
        if (root == node){
            return;
        }
        
        if (this->compare(temp->elt, node->elt)){
            //if node is leftmost in its teir
            if (temp->child == node){
                temp->child = node->sibling;
                node->sibling = nullptr;
                node->parent = nullptr;
                root = meld(root, node);
            }
            //if node is not leftmost in its teir
            else {
                Node * temp2 = temp->child;
                while (temp2->sibling != node){
                    temp2 = temp2->sibling;
                }
                temp2->sibling = node->sibling;
                node->sibling = nullptr;
                node->parent = nullptr;
                root = meld(root, node);
            }
        }
        
    } // updateElt()


    // Description: Add a new element to the priority_queue. Returns a Node* corresponding
    //              to the newly added element.
    // Runtime: O(1)
    // TODO: when you implement this function, uncomment the parameter names.
    // NOTE: Whenever you create a node, and thus return a Node *, you must be sure to
    //       never move or copy/delete that node in the future, until it is eliminated
    //       by the user calling pop().  Remember this when you implement updateElt() and
    //       updatePriorities().
    Node* addNode(const TYPE & val) {
        // TODO: Implement this function
        Node * newNode = new Node(val);
        
        if (root == nullptr){
            root = newNode;
        }
        
        else if (root != nullptr and newNode != nullptr){
            root = meld(root, newNode);
        }
        numNodes++;
        return newNode; // TODO: Delete or change this line
    } // addNode()


private:
    // TODO: Add any additional member functions or data you require here.
    // TODO: We recommend creating a 'meld' function (see the Pairing Heap papers).
    
    Node * meld(Node * pa, Node * pb){
        //must have .... pa->parent == nullptr;
        //pa->sibling == nullptr;
        //must have .... pb->parent == nullptr;
        //pb`->sibling == nullptr;
        if (this->compare(pa->elt, pb->elt)){
            pa->sibling = pb->child;
            pb->child = pa;
            pa->parent = pb;
            return pb;
        }
        else {
            pb->sibling = pa->child;
            pa->child = pb;
            pb->parent = pa;
            return pa;
        }
    }
    
    Node * root;
    int numNodes;
};


#endif // PAIRINGPQ_H
