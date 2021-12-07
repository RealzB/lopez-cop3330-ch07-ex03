/*
 *  UCF COP3330 Fall 2021 Assignment 6 Solution
 *  Copyright 2021 Brandon Lopez
 */

#include "std_lib_facilities.h"


//Provide an assignment operator, =, so that you can change the value of a variable after you introduce it using let. Discuss why that can be useful and how it can be a source of problems

//It can be useful to reassign a value if you are working with a variable that you use multiple times but sometimes have to change the value of. It is not useful if you are working with a variable that you dont want changing and you have some code that keeps changing the value against your desire for it to remain the same. In this circumstance you might use a constant instead.

//use let <variable name> = value
//in order to set a variable equal to something
//use format "x + y =" to receive an output of the answer value 

class Token {
public:
  char kind;     //token type
  double value;   
  string name;   
  Token(char ch)             : kind(ch), value(0)   {}
  Token(char ch, double val) : kind(ch), value(val) {}
  Token(char ch, string n)   : kind(ch), name(n)    {}
};

//

class Token_stream {
public: 
  Token_stream();  //read cin
  Token get();     
  void putback(Token t);  
  void ignore(char c);   
private:
  bool full;    
  Token buffer;   
};

//

const char number = '8';   
const char quit   = 'q';   
const char print  = ';';   
const char name   = 'a';   
const char let    = 'L';    
const char con    = 'C';   
const string declare = "let";		
const string constant = "const";




//empty buffer
Token_stream::Token_stream()
:full(false), buffer(0){} 


//

void Token_stream::putback(Token t)
{
  if (full) error("putback() into a full buffer");
  buffer = t;       // t into buffer
  full = true;      // set buffer full
}

//

Token Token_stream::get() // read from cin
{
  if (full) {   //check for token
    full=false;
    return buffer;
  }  

  char ch;
  cin >> ch;     

  switch (ch) {
  case quit:
  case print:
  case '(':
  case ')':
  case '+':
  case '-':
  case '*':
  case '/': 
  case '%':
  case '=':
    return Token(ch); 
  case '.':            
  case '0': case '1': case '2': case '3': case '4':
  case '5': case '6': case '7': case '8': case '9':    
  {
    cin.putback(ch);
    double val;
    cin >> val;     
    return Token(number,val);
  }
  default:
    if (isalpha(ch)) {	//starts with letter
      string s;
      s += ch;
      while (cin.get(ch) && (isalpha(ch) || isdigit(ch) || ch=='_'))//while its these it'll continue to add the characters
        s+=ch;	
      cin.putback(ch);
      if (s == declare) 
        return Token(let); 
      if (s == constant) 
        return Token(con); 
      return Token(name,s);
    }
    error("Bad token");
  }
}

//

void Token_stream::ignore(char c)//input token type
{
  if (full && c==buffer.kind) {
    full = false;
    return;
  }
  full = false;

  char ch = 0;
  while (cin>>ch)
    if (ch==c) return;
}

//

Token_stream ts;     

//

class Variable {
public:
  string name;
  double value;
bool var;	
  Variable (string n, double v, bool va = true) :name(n), value(v), var(va) { }
};

//

vector<Variable> var_table;

//

double get_value(string s)  // return value of variables
{
  for (int i = 0; i<var_table.size(); ++i)
      if (var_table[i].name == s) 
        return var_table[i].value;
  error("get: undefined variable ", s);
}

//

void set_value(string s, double d)  // set from s to d
{
  for (int i = 0; i<var_table.size(); ++i)
      if (var_table[i].name == s) {
    if (var_table[i].var==false) error(s," is a constant");
      var_table[i].value = d;
      return;
      }
  error("set: undefined variable ", s);
}

//

bool is_declared(string var)  // check if in a table
{
  for (int i = 0; i<var_table.size(); ++i)
    if (var_table[i].name == var) return true;
  return false;
}

//

double define_name(string s, double val, bool var=true)  // add to table
{
  if (is_declared(s)) 
    error(s," declared twice");
  var_table.push_back(Variable(s,val,var));
  return val;
}

//

double expression(); //called from primary

//


double primary() //numbers and parenthesis
{
  Token t = ts.get();
  switch (t.kind) {
  case '(':         
  {
    double d = expression();
    t = ts.get();
    if (t.kind != ')') error("')' expected");
    return d;
  }
  case number:    
    return t.value;  
  case name:
  {
    Token next = ts.get();
    if (next.kind == '=') {	
      double d = expression();
      set_value(t.name,d);
      return d;
    }
    else {
      ts.putback(next);	
      return get_value(t.name); 
    }
  }
  case '-':
    return - primary();
  case '+':
    return primary();
  default:
    error("primary expected");
  }
}

//


double term() // multiplication, division, and mod cases
{
  double left = primary();
  Token t = ts.get(); 

  while(true) {
    switch (t.kind)  {
    case '*':
      left *= primary();
      t = ts.get();
      break;
    case '/':
    {    
      double d = primary();
      if (d == 0) error("divide by zero");
      left /= d; 
      t = ts.get();
      break;
    }
    case '%':
    {    
      int i1 = narrow_cast<int>(left);
      int i2 = narrow_cast<int>(term());
      if (i2 == 0) error("%: divide by zero");
      left = i1%i2; 
      t = ts.get();
      break;
    }
    default: 
      ts.putback(t);      
      return left;
    }
  }
}

//


double expression() // add and sub
{
  double left = term();     
  Token t = ts.get();     

  while(true) {    
    switch(t.kind) {
    case '+':
      left += term();    
      t = ts.get();
      break;
    case '-':
      left -= term();   
      t = ts.get();
      break;
    default: 
      ts.putback(t);     
      return left;    //returns answer if there isn't any more operations to be done
    }
  }
}

//

double declaration(Token k)
{
  Token t = ts.get();
  if (t.kind != name) error ("name expected in declaration");
  string var_name = t.name;

  Token t2 = ts.get();
  if (t2.kind != '=') error("= missing in declaration of ", var_name);

  double d = expression();
  define_name(var_name,d,k.kind==let);
  return d;
}

//

double statement()
{
  Token t = ts.get();
  switch (t.kind) {
  case let:
	  case con:
    return declaration(t.kind);
  default:
    ts.putback(t);
    return expression();
  }
}

//

void clean_up_mess()
{ 
  ts.ignore(print);
}

//

const string prompt  = "> "; //requesting input when seen in console
const string result  = "= "; //showing a result when seen in console

void calculate()
{
  while (cin)
    try {
    cout << prompt;
    Token t = ts.get();
    while (t.kind == print) 
      t=ts.get();   
    if (t.kind == quit) 
      return;    
    ts.putback(t);
    cout << result << statement() << endl;
  }
  catch (exception& e) { 
    cerr << e.what() << endl;     
    clean_up_mess();
  }
}

//

int main()
try {
  //Provide named constants that you really can’t change the value of
  define_name("pi",3.14,false);
  define_name("e",2.718,false);

  calculate();


  return 0;
}
catch (exception& e) {
  cerr << e.what() << endl;
  return 1;
}
catch (...) {
  cerr << "exception \n";
  return 2;
}


