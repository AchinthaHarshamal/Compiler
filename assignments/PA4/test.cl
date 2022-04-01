class Main{
	
	a : Int <- 5;	
	b : Int <- 5;
	s : Bool;	

	main():Bool {
		{
			b <- a*a;
			a <- b-a;
			s <- true;
		}
	};
};
