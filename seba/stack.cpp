#include <stdio.h>
#include <assert.h>

template <class T>
class stack {

public:
    stack();
    ~stack();
    void push(T n);
    T pop();

private:
    T data[10];
    int top;

};

template <class T>
stack<T>::stack(){
    top = 0;    
    printf("constr \n");
}

template <class T>
stack<T>::~stack(){
    printf("destr \n");
}

template <class T>
void stack<T>::push(T n){
    assert(top<10);
    data[top] = n;
    top++;    
}

template <class T>
T stack<T>::pop(){
    top--;
    return data[top];    
}

stack<int> otromas; //variable global

int main(){
//    stack<char> sc;
//    sc.push('c');
//    sc.push('d');
//    printf("%c\n",sc.pop());

//    stack<stack<char>> ss;
    stack<char> *otro = new stack<char>(); // equiv al malloc
 
    stack<int> s;
    s.push(10);
    s.push(20);
    printf("%d\n",s.pop());
    printf("%d\n",s.pop());
//    delete otro; //equiv al free
}
