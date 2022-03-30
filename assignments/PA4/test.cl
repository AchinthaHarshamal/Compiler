class Main{
	
	a : Int <- 5;	
	b : Int <- 5;

	main():Int {
	  a <- 
		case true of
			x : Bool => x;
			y : Int =>	not x;
		esac
	};
};

class A{};

class B inherits A{};
