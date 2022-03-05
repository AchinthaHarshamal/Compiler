
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
(* error : assignment operator clearmissing in feature 2
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




(* test cases for errors in blocks *)
(* error : operator missing in expression 2
	   and assignment operator missing in expression 4*)
Class H{
	func() : Int{
		{
			p <- 1;
			q <- 1  2;
			r <- 6 * 2;
			s  true;
		}
	};
};


(* test cases for errors in the let expression*)
(* an error in the expression of body*)
Class I{
	
	
	func():Int{
		let a:Int <- 1  in a+
	};

};

(*errors in variable list and the body*)
Class J{
	
	h : int - 9 ;
	
	func():Int{
		let a:Int < 1 , b:Int <- 2 , c:Type3 <- 3 ,c:Type3 < 4  in a+
	};
	h : Int  4;

};

(*error in a the variable list*)
Class K{
	
	x : INT <-5;
	func():Int{
		let a:Int < 5 , b:Int , c:Type3 <- 8 in a+2
	};

};




