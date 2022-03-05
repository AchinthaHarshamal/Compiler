class A {
};

Class B inherits A {
};


(* test cases for features *)
Class D{
	x : Int;
	y : Int <- 1;
	
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
		x <- data + 2
	};
}; 


(* test cases for blocks *)
Class E{
	func() : Int{
		{
			p <- 1;
			q <- 1 + 2;
			r <- 6 * 2;
			s <- true;
		}
	};
};

(* test cases for let expression*)
Class F{
	func():Int{
		let a:Int <- 1  in a+1
	};

};

(* variables without init*)
Class G{
	
	h : Int <- 9 ;
	
	func():Int{
		let a : Int , b : Int , c : Type3 ,c : Type3   in 1
	};
	h : Int <- 4;

};

(* both initialized variables and not initialized vars *)
Class H{

	func():Int{
		let a:Int <- 5 , b:Int , c:Type3 <- 8 in a+2
	};

};


(* test cases for three dispatchers*)
Class I{
	func() : Int {
		{
			x <- b.a(2+2,1*2);
			y <- b@A.a(x+y,q*t);
			z <- a(x+y,z+t);
		}
	};
};

(* test cases for case expressions*)
Class J{
	func() : Int {
		{
			x <- (
				case a of
					x : Int => true;
					y : Int => false;
				esac
			);
		}
	};
};

(* test cases for if else expressions*)
Class K{
	func() : Int {
		if a then false else true fi
	};

	func2() : Int {
		if a then false else if b then x<-1 else x<-2 fi fi
	};
};

(* test cases for if else expressions*)
Class K{
	func() : Int {
		if a then false else true fi
	};

	func2() : Int {
		if a then false else if b then x<-1 else x<-2 fi fi
	};
};

(* test cases for while loops expressions*)
Class K{
	func() : Int {
		while true loop x<- x+1 pool
	};
};
