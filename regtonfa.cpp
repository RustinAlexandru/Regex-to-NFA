// regtonfa.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <conio.h>
#include <string>
#include <iostream>
#include <stack>
#include <vector>


using namespace std;

std::stack <char> stiva;
 
int nstate;

enum 
{
	final = 256,
	split = 257
};

typedef struct State State;
struct State
{
	int c;
	State* out1;
	State* out2;
	int lastlist;
};

State finalstate = { final };

typedef struct Frag Frag;
typedef union Ptrlist Ptrlist;

union Ptrlist
{
	Ptrlist* next;
	State* s;
};

struct Frag
{
	State* start;
	Ptrlist* out;
};


State* state(int c, State* out1, State* out2)
{
	State* s;
	nstate++;
	s = new State;
	s->lastlist = 0;
	s->c = c;
	s->out1 = out1;
	s->out2 = out2;
	return s;
}

Frag frag(State* start, Ptrlist* out1)
{
	Frag n = { start, out1 };
	return n;

}
Ptrlist* list1(State** outp)
{
	Ptrlist* l;
	l = (Ptrlist*)outp;
	l->next = NULL;
	return l;

}

void patch(Ptrlist* l, State* s)
{
	Ptrlist* next;
	for (; l; l = next)
	{
		next = l->next;
		l->s = s;
	}

}
Ptrlist* append(Ptrlist* l1, Ptrlist* l2)
{

	Ptrlist* p;
	p = l1;
	while (l1->next)
		l1 = l1->next;
	l1->next = l2;
	return p;
}

typedef struct List List;
struct List
{
	State **s;
	int n;
};
List l1, l2;

static int listid;

void addstate(List*, State*);
void step(List*, int, List*);

List* startlist(State* start, List *l)
{
	l->n = 0;
	listid++;
	addstate(l, start);
	return l;
}

int isfinal (List*l)
{

	for (int i = 0; i < l->n; i++)
	{
		if (l->s[i] == &finalstate)
			return 1;
	}
	return 0;
}

void addstate(List* l, State* s)
{
	if (s == NULL || s->lastlist == listid)
		return;
	s->lastlist = listid;
	if (s->c == split)
	{
		addstate(l, s->out1);
		addstate(l, s->out2);
		return;
	}
	l->s[l->n++] = s;
}

void step(List* clist, int c, List* nlist)
{
	State* s;
	listid++;
	nlist->n = 0;
	for (int i = 0; i < clist->n; i++)
	{
		s = clist->s[i];
		if (s->c = c)
			addstate(nlist, s->out1);

	}
}

int Final(State* start, char* s)
{
	int c;
	std::vector<char>::size_type i = 0;
	List* clist, *nlist, *t;
	clist = startlist(start, &l1);
	nlist = &l2;
	for (; *s;s++)
	{
		c = *s;
		step(clist, c, nlist);
		t = clist; clist = nlist; nlist = t;
	}
	return isfinal(clist);
}


int isOperator(char string[],int i)
{
	switch (string[i])
	{
	case '*':
	case '.':
	case '|':
	case '(':
	case ')': return true;
	default:
		return false;
	}

}

int precedence(char x)
{
	switch (x)
	{
		case'|':return 1;
		case'.':return 2;
		case'*': return 3;
		default:return 0;
	}
}

std::vector <char> re2post(char string[])
{
	
	std::vector<char> postfix;
	std::vector<char>::size_type a = 0;
	for (int i = 0; string[i]!=0; i++) {
		if (isOperator(string,i)) {
			while (!stiva.empty() && precedence(stiva.top()) <= precedence(string[i])) {
				postfix.push_back(stiva.top());
				a++;
				stiva.pop();
			}
			stiva.push(string[i]);
		}
		else if (string[i] == '(') {
			stiva.push(string[i]);
		}
		else if (string[i] == ')') {
			while (!stiva.empty() && stiva.top() != '(') {
				postfix.push_back(stiva.top());
				a++;
				stiva.pop();
			}
			if (!stiva.empty())
		stiva.pop();
		}
		else {
			postfix.push_back(string[i]);
			a++;
		}
	}
	while (!stiva.empty())
	{
		postfix.push_back(stiva.top());
		a++;
		stiva.pop();
	}
	return postfix;
}

State* post2nfa(std::vector<char> postfix)
{
	//std::stack <Frag> stack;
	std::vector<char>::size_type i = 0;
	Frag stack[100], *stackp, e1, e2, e;
	State* s;
	stackp = stack;
#define push(s) *stackp++=s
#define pop() *--stackp
	for (i = 0; i < postfix.size(); i++)
	{
		switch (postfix[i])
		{
		default:
			s = state(postfix[i], NULL, NULL);
			push(frag(s, list1(&s->out1)));
			break;
		case'.':
		
				   e2 = pop();
				   e1 = pop();
				   patch(e1.out, e2.start);
				   push(frag(e1.start, e2.out));
				   break;
		case'|':
				   e2 = pop();
				   e1 = pop();
				   s = state(split, e1.start, e2.start);
				   push(frag(s, append(e1.out, e2.out)));
				   break;
		case'*':
			e = pop();
			s = state(split, e.start, NULL);
			patch(e.out, s);
			push(frag(s, list1(&s->out2)));
			break;
		case'+':
			e = pop();
			s = state(split,e.start,NULL);
			patch(e.out, s);
			push(frag(e.start, list1(&s->out2)));
			break;
		}
		e = pop();
		patch(e.out, &finalstate);
		return e.start;
	}
}
void print(std::vector<char> &v)
{
	std::vector<char>::iterator it;
	for (it = v.begin(); it != v.end(); ++it)
		std::cout << (*it) << "";
}
int _tmain(int argc, _TCHAR* argv[])
{
	State* start;
	std::vector < char> post;
	char* str;
	str = "aa";
	post = re2post("a.a");
	print(post);
	
_getch();
	return 0;
}

