
(*
 *  execute "coolc bad.cl" to see the error messages that the coolc parser
 *  generates
 *
 *  execute "myparser bad.cl" to see the error messages that your parser
 *  generates
 *)


(* no error *)
class A {
};

(* error:  b is not a type identifier *)
Class b inherits A {
};

(* error:  a is not a type identifier *)
Class C inherits a {
};

(* error:  keyword inherits is misspelled *)
Class D inherts A {
};

(* error:  closing brace is missing *)
Class E inherits A {
;

(* test cases for errors in features in classes *)
(* error : assignment operator missing in feature 2
		   operator missing in expression of function 3
*)
Class F{
	x : Int;
	y : Int  1;
	
	func() : Type {
		x <- 1
	};

	func2(data1 : Int, data2 : Int) : Type2 {
		{
			x <- data1;
			y <- data2;
		}
	};

	func3(data : Int) : Type2 {
		x <- data  2
	};
}; 

(* error : errors in formal list and the expression in the func
		   and wrong type decleration in attribute z
*)
Class G{
	x : Int;
	y : Int <- 1;	
	
	func(data  Int) : Type2 {
		x <- data  2
	};

	z : int <- 2;
};





