#include <typeinfo>
#include <iostream>
#include <cxxabi.h>
#include <string>
#include "program.hpp"
using namespace std;

char* demangle(const char *demangle) {
    int status;
    return abi::__cxa_demangle(demangle, 0, 0, &status);
}

template<char... ops>struct program{};
template<char... chr>struct output{};
template<int... i>struct memory{};

template<typename T>
struct from_output;
template<char c,char... cs>
struct from_output<output<c,cs...> >{
    static string f(){
	string s;s.push_back(c);
	return s + from_output<output<cs...> >::f();
    }
};
template<>
struct from_output<output<> >{
    static string f(){
	return "";
    }
};


template<typename T1,typename T2>
struct Pair{
    typedef T1 fst;
    typedef T2 snd;
};

template<int i,typename Then,typename Else>
struct if_0{
    typedef typename Else::value value;
};
template<typename Then,typename Else>
struct if_0<0,Then,Else>{
    typedef typename Then::value value;
};

template<int i,typename T>struct cons;
template<int i,int... is>
struct cons<i,memory<is...> >{
    typedef memory<i,is...> value;
};

template<char c,typename T>struct snoc;
template<char c,char... cs>
struct snoc<c,output<cs...> >{
    typedef output<cs...,c> value;
};

template<int i> struct make_memory{
    typedef typename make_memory<i-1>::value tmp;
    typedef typename cons<0,tmp>::value value;
};
template<> struct make_memory<0>{
    typedef memory<> value;
};

//get_value<i,memory<...>>で、memory<...>のi番目(0-origin)の要素を取得する
//配列のアクセス風に書くと、memory<...>[i] な感じ
template<int idx,typename Memory>struct get_value;
template<int v,int... is>
struct get_value<0,memory<v,is...> >{
    static const int value = v;
};
template<int idx,int v,int... is>
struct get_value<idx,memory<v,is...> >{
    static const int value = get_value<idx-1,memory<is...> >::value;
};

//set_value<i,v,memory<...>>で、memory<...>のi番目(0-origin)に値vを代入する
//配列のアクセス風に書くと、memory<...>[i]=v な感じ
template<int idx,int v,typename memory>struct set_value;
template<int v,int v_,int... is>
struct set_value<0,v,memory<v_,is...> >{
    typedef memory<v,is...> value;
};
template<int idx,int v,int v_,int... is>
struct set_value<idx,v,memory<v_,is...> >{
    typedef typename cons<v_,typename set_value<idx-1,v,memory<is...> >::value>::value value;
};

template<typename Rest,typename Eated,int c>
struct search_forward;
template<char... Rest,char... Eated>
struct search_forward<program<']',Rest...>,program<Eated...>,0>{
    typedef Pair<program<Rest...>,program<']',Eated...> > value;
};
template<char... Rest,char... Eated,int c>
struct search_forward<program<']',Rest...>,program<Eated...>,c>{
    typedef typename search_forward<program<Rest...>,
				    program<']',Eated...>,
				    c-1>::value value;
};
template<char... Rest,char... Eated,int c>
struct search_forward<program<'[',Rest...>,program<Eated...>,c>{
    typedef typename search_forward<program<Rest...>,
				    program<'[',Eated...>,
				    c+1>::value value;
};
template<char... Rest,char... Eated,char op,int c>
struct search_forward<program<op,Rest...>,program<Eated...>,c>{
    typedef typename search_forward<program<Rest...>,
				    program<op,Eated...>,
				    c>::value value;
};

template<typename Rest,typename Eated,int c>
struct search_backward;
template<char... Rest,char... Eated>
struct search_backward<program<Rest...>,program<'[',Eated...>,0>{
    typedef Pair<program<'[',Rest...>,program<Eated...> > value;
};
template<char... Rest,char... Eated,int c>
struct search_backward<program<Rest...>,program<'[',Eated...>,c>{
    typedef typename search_backward<program<'[',Rest...>,
				     program<Eated...>,
				     c-1>::value value;
};
template<char... Rest,char... Eated,int c>
struct search_backward<program<Rest...>,program<']',Eated...>,c>{
    typedef typename search_backward<program<']',Rest...>,
				     program<Eated...>,
				     c+1>::value value;
};
template<char... Rest,char... Eated,char op,int c>
struct search_backward<program<Rest...>,program<op,Eated...>,c>{
    typedef typename search_backward<program<op,Rest...>,
				     program<Eated...>,
				     c>::value value;
};

template<typename Rest,typename Eated,int idx,typename Memory,typename Output>
struct exec;

template<int idx,typename Memory,typename Output,char... Eated>
struct exec<program<>,program<Eated...>,idx,Memory,Output>{
    typedef Pair<Output,Memory> value;
};

template<int idx,typename Memory,typename Output,char... Rest,char... Eated>
struct exec<program<'>',Rest...>,program<Eated...>,idx,Memory,Output>{
    typedef typename exec<program<Rest...>,program<'>',Eated...>,idx+1,Memory,Output>::value value;
};

template<int idx,typename Memory,typename Output,char... Rest,char... Eated>
struct exec<program<'<',Rest...>,program<Eated...>,idx,Memory,Output>{
    typedef typename exec<program<Rest...>,program<'<',Eated...>,idx-1,Memory,Output>::value value;
};

template<int idx,typename Memory,typename Output,char... Rest,char... Eated>
struct exec<program<'+',Rest...>,program<Eated...>,idx,Memory,Output>{
    typedef typename set_value<idx,get_value<idx,Memory>::value+1,Memory>::value NewMemory;
    typedef typename exec<program<Rest...>,program<'+',Eated...>,idx,NewMemory,Output>::value value;
};

template<int idx,typename Memory,typename Output,char... Rest,char... Eated>
struct exec<program<'-',Rest...>,program<Eated...>,idx,Memory,Output>{
    typedef typename set_value<idx,get_value<idx,Memory>::value-1,Memory>::value NewMemory;
    typedef typename exec<program<Rest...>,program<'-',Eated...>,idx,NewMemory,Output>::value value;
};

template<int idx,typename Memory,char... Rest,char... str,char... Eated>
struct exec<program<'.',Rest...>,program<Eated...>,idx,Memory,output<str...> >{
    typedef typename snoc<get_value<idx,Memory>::value,output<str...> >::value new_output;
    typedef typename exec<program<Rest...>,program<'.',Eated...>,idx,Memory,new_output>::value value;
};    

template<int idx,typename Memory,typename Output,char... Rest,char... Eated>
struct exec<program<'[',Rest...>,program<Eated...>,idx,Memory,Output>{
    static const int now = get_value<idx,Memory>::value;
    typedef typename search_forward<program<Rest...>,program<'[',Eated...>,0>::value result;
    typedef typename result::fst new_rest;
    typedef typename result::snd new_eated;
    typedef exec<new_rest,new_eated,idx,Memory,Output> Then;
    typedef exec<program<Rest...>,program<'[',Eated...>,idx,Memory,Output> Else;
    typedef typename if_0<now,Then,Else>::value value;
};

template<int idx,typename Memory,typename Output,char... Rest,char... Eated>
struct exec<program<']',Rest...>,program<Eated...>,idx,Memory,Output>{
    static const int now = get_value<idx,Memory>::value;
    typedef typename search_backward<program<']',Rest...>,program<Eated...>,0>::value result;
    typedef typename result::fst new_rest;
    typedef typename result::snd new_eated;
    typedef exec<program<Rest...>,program<']',Eated...>,idx,Memory,Output> Then;
    typedef exec<new_rest,new_eated,idx,Memory,Output> Else;
    typedef typename if_0<now,Then,Else>::value value;
};

//load program
typedef program<
#include "helloworld.hpp"
> whole_program;

int main()
{
    typedef exec<whole_program,program<>,0,make_memory<50>::value,output<> >::value ans;
    cout << demangle(typeid(ans).name()) << endl;
    cout << from_output<ans::fst>::f() << endl;
    return 0;
}
