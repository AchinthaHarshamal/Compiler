class A{};
class B inherits A{};
class C inherits A{};
class D inherits C{};


class Main{
	
	a : Int <- 5;	
	b : Int <- 5;
	s : Bool <- true;
	a1 : A;
	b1 : B;
	c1 : C;
	d1 : D;	

	main():A {
		If a then b1 <- new B else d1 <- new D fi 
	};
};

class E inherits A{
	a:Int <- 2;

	f():Int{
		a+b
	};
};
