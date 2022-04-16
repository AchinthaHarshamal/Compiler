///////////////////////////////////////
//	Authors :						 //
//		Achintha Harshamal E/17/090	 //
//		Esara Sithumal	E/17/176	 //
///////////////////////////////////////


#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "algorithm"
#include <vector>
#include <stack>
#include "semant.h"
#include "utilities.h"


extern int semant_debug;            /* debug flag */
extern char *curr_filename;         /* will be used in cgen */


//////////////////////////////////////////////////////////////////////
//
// Symbols
//
// For convenience, a large number of symbols are predefined here.
// These symbols include the primitive type and method names, as well
// as fixed names used by the runtime system.
//
//////////////////////////////////////////////////////////////////////
static Symbol 
    arg,
    arg2,
    Bool,
    concat,
    cool_abort,
    copy,
    Int,
    in_int,
    in_string,
    IO,
    length,
    Main,
    main_meth,
    No_class,
    No_type,
    Object,
    out_int,
    out_string,
    prim_slot,
    self,
    SELF_TYPE,
    Str,
    str_field,
    substr,
    type_name,
    val;
//
// Initializing the predefined symbols.
//
static void initialize_constants(void)
{
    arg         = idtable.add_string("arg");
    arg2        = idtable.add_string("arg2");
    Bool        = idtable.add_string("Bool");
    concat      = idtable.add_string("concat");
    cool_abort  = idtable.add_string("abort");
    copy        = idtable.add_string("copy");
    Int         = idtable.add_string("Int");
    in_int      = idtable.add_string("in_int");
    in_string   = idtable.add_string("in_string");
    IO          = idtable.add_string("IO");
    length      = idtable.add_string("length");
    Main        = idtable.add_string("Main");
    main_meth   = idtable.add_string("main");
    //   _no_class is a symbol that can't be the name of any 
    //   user-defined class.
    No_class    = idtable.add_string("_no_class");
    No_type     = idtable.add_string("_no_type");
    Object      = idtable.add_string("Object");
    out_int     = idtable.add_string("out_int");
    out_string  = idtable.add_string("out_string");
    prim_slot   = idtable.add_string("_prim_slot");
    self        = idtable.add_string("self");
    SELF_TYPE   = idtable.add_string("SELF_TYPE");
    Str         = idtable.add_string("String");
    str_field   = idtable.add_string("_str_field");
    substr      = idtable.add_string("substr");
    type_name   = idtable.add_string("type_name");
    val         = idtable.add_string("_val");
}


/* the WHOLE AST will be build in this constructor */
ClassTable::ClassTable(Classes classes) : semant_errors(0) , error_stream(cerr) {

    class_symtable.enterscope();	//start a scope 
	//add basic classes of cool to table inside the scope that is previously created    
	install_basic_classes();	

    Symbol class_name;
    c_node current_class;
 
    // First pass, build class symbol table
	// checks for the redefinitions of basic classes and multiple class definitions
    for ( int i = classes->first(); classes->more(i); i = classes->next(i) ) {

        current_class = (c_node)classes->nth(i);
        class_name = current_class->get_name();

        if(class_name == SELF_TYPE || class_name == Object || class_name == Int || 
                class_name == Str || class_name == IO){
            ostream& os =  semant_error(current_class);
            os << "Redifinition of basic class " << class_name << "." << endl;
            continue;
        }

        else  if( class_symtable.lookup(class_name) != NULL ){
            //  check the situation of class multiply defined. 
            ostream& os =  semant_error(current_class);
            os << "Class " << class_name << " was previously defined." << endl;
            continue;
        } 

		// if class is well defined add to class symbol table
        class_symtable.addid(class_name,current_class);
    }

    // first pass, checks valid feature definitions in classes
	// and add features to feature tables of classes
	// Here, only checked for correctly defined method/attribute names and type names
    for( int i = classes->first(); classes->more(i); i = classes->next(i) ){
        current_class = (c_node)classes->nth(i);
        semant_class(current_class);
    }

    //check Main class is defined, this must be done after first pass
	// because the class and the method must be added to tables prior to check
    if ( class_symtable.probe(Main) == NULL){
        ostream& os =  semant_error();
        os << "Class main is not defined." << endl;
    }
    else {
        c_node main_class = (c_node)class_symtable.probe(Main);
        Table main_table = main_class->featureTable;
        // if main class is defined, look for the main method
        if ( main_table.probe(main_meth) == NULL ){ 
            ostream& os =  semant_error(main_class);
            os << "no 'main' method in class Main." << endl;
        }

    }

    // second pass, in this pass all the expressions are type checked
    for( int i = classes->first(); classes->more(i); i = classes->next(i) ){
        current_class = (c_node)classes->nth(i);
        semant_class_attr(current_class);
    }

	// exit from the scope
    class_symtable.exitscope();
}

// first pass, this function checks for the valid feature definitions
void ClassTable::semant_class(c_node current_class){
	// each class has it own feature table
    Table current_table = current_class->featureTable;
    current_table.enterscope(); //start a scope in feature table

    Symbol class_name = current_class->get_name();
    Symbol parent_name;
	
	//check for valid inheritance in classes
    if ( class_name != Object ){
        parent_name = current_class->get_parent();
		// error for inheriting from basic class
        if ( parent_name == Bool || parent_name == SELF_TYPE || parent_name == Str){
            ostream& os =  semant_error(current_class);
            os << "Class " << class_name << " cannot inherit from class " << parent_name << "." << endl;
        }
		// error for inheriting from undefined class
        else if ( class_symtable.lookup(parent_name) == NULL ){
            ostream& os =  semant_error(current_class);
            os << "Class " << class_name << " inherits from an undefined class " << parent_name << "." << endl;
        }
    }
	
	//check attributes and methods in class
    Features features = current_class->get_features();

    for( int i = features->first(); features->more(i); i = features->next(i) ){
        Feature feature = features->nth(i);
        if ( feature->get_type() == attrType ){
			//if current feature is a attribute, type check for attribute  
            semant_attr( current_class, (attr_class*)feature );       
        }
        else if ( feature-> get_type() == methodType ){
			//if current feature is a method, type check for method 
            semant_method( current_class, (method_class*)feature );       
        }
    } 


}

// First pass,this function checks for valid attribute definitions(names,type declerations are checked,expressions are not checked)
void ClassTable::semant_attr(c_node current_class,attr_class* attr){
    Symbol attr_name = attr->get_name();
    Table current_table = current_class->featureTable;
    Symbol attr_type = attr->get_type_decl();
	// error for attribute declared with undefined types
    if( class_symtable.lookup(attr_type) == NULL){
        ostream& os = semant_error(current_class);
        os << "attribute " << attr_name << " declared with undefined type " << attr_type << endl;
    }
	// cannot use self as attribute name
    if ( attr_name == self ){
        ostream& os = semant_error(current_class);
        os << "Cannot assign to 'self'." << endl;
    }
    // error for multiple definitions of attribute names
    else if ( current_table.probe(attr_name) ){
        ostream& os = semant_error(current_class);
        os << "attribute " << attr_name << " is multiply defined " << endl;
    }
	// if well defined add atribute to feature table of current class
    current_table.addid(attr_name,attr);

}

// First pass,this function checks for valid method definitions(names,type declerations are checked,expressions are not checked)
void ClassTable::semant_method(c_node current_class,method_class* method){
    Symbol method_name = method->get_name();
    Table current_table = current_class->featureTable;
    Symbol ret_type = method->get_return_type();
    Formals formals = method->get_formals();
    Expression expr = method->get_expr();
	// if return type is not in class symbol table
    if( class_symtable.lookup(ret_type) == NULL){
        ostream& os = semant_error(current_class);
        os << "method " << method_name << " return undefined type " << ret_type << endl;
    }
	// check for multiple definitions
    if ( current_table.probe(method_name) ){
        ostream& os = semant_error(current_class);
        os << "method " << method_name << " is multiply defined " << endl;
    }
	
	// if well defined, add the method to feature table
    current_table.addid(method_name,method);
}



// second pass,this function typechecks for features in a class
void ClassTable::semant_class_attr(c_node current_class){

    Table current_table = current_class->featureTable;

    Symbol class_name = current_class->get_name();
    Symbol parent_name;
    Features features = current_class->get_features();
	// get the feature of the class from feature table
    for( int i = features->first(); features->more(i); i = features->next(i) ){
        Feature feature = features->nth(i);
		// type check attributes        
		if ( feature->get_type() == attrType ){
            semant_attr_expr( current_class, (attr_class*)feature );         
        }
		// type check methods
        else if ( feature-> get_type() == methodType ){
            semant_method_expr( current_class, (method_class*)feature );       
        }
    } 

}


// second pass, check typecheck attribute initializaton 
void ClassTable::semant_attr_expr(c_node current_class,attr_class* attr){
    Symbol attr_name = attr->get_name();
    Table current_table = current_class->featureTable;
    Symbol attr_type = attr->get_type_decl();
    Expression init = attr->get_init();
    
	semant_expr(current_class, init);	//typecheck the initilization expression of the attribute
	
	/*this is_subclass fumction checks the type compatibility of attribute type and type of the expression*/
    if ( is_subclass(attr_type , init->type,current_class->get_name()) == false ){
        ostream& os = semant_error(current_class);
        os << "expression type " << init->type <<" must conform to attribution type " << attr_type << "." << endl; 
    }
    
}

// second pass, typecheck the formals, return type and body of a method
void ClassTable::semant_method_expr(c_node current_class,method_class* method){
    Formals formals = method->get_formals();
    Symbol ret_type = method->get_return_type();
    Table current_table = current_class->featureTable;
    
	current_table.enterscope(); // inside a method is a new scope

	// typecheck the formal list of the method
    for( int i = formals->first(); formals->more(i); i = formals->next(i) ){
        Formal f = formals->nth(i);
        semant_formal(current_class,f);
    }
	
	// type check the body of the method
    Expression expr = method->get_expr();

	if(ret_type == SELF_TYPE){
		ret_type = current_class->get_name();	
	}

    semant_expr(current_class,expr);

	/*this is_subclass function checks the type compatibility of return type and type of the expression*/
    if ( is_subclass(ret_type , expr->type,current_class->get_name()) == false ){
        ostream& os = semant_error(current_class);
        os << "expression type " << expr->type <<" must conform to return type " << ret_type << "." << endl; 
    }

	current_table.exitscope(); // close the scope at the end of the function
    
}


// typechecks the a formal of a method
void ClassTable::semant_formal(c_node current_class,Formal f){
    Table current_table = current_class->featureTable;
    formal_class * formal = (formal_class*) f;
    // formal name cannot be self
    if (formal->get_name() == self){
        ostream& os = semant_error(current_class);
        os << "'self' as a formal identifier." <<endl; 
    
    }
	// cannot be defined multiple times
    else if(current_table.probe(formal->get_name() ) ){
        ostream& os = semant_error(current_class);
        os << "formal " << formal->get_name() << "was defined previously." <<endl; 
    }
    // type of the formal cannot be SELF_TYPE
    if(formal->get_type_decl() == SELF_TYPE){
        ostream& os = semant_error(current_class);
        os << "SELF_TYPE as a formal type.\n";
    }
	// type must be a class defined in class symbol table
    else if(class_symtable.lookup(formal->get_type_decl()) == NULL ) {
        ostream& os = semant_error(current_class);
        os << "formal " << formal->get_name() << "has undefined type " << formal->get_type_decl() << "." << endl; 
    }

	// if well defined add the formal to feature table of the class inside current scope
    current_table.addid(formal->get_name(), formal);
}

// this function type checks for all the expression types in Cool language
// a switch case is used do apply various typechecks for correspondic expression types
void ClassTable::semant_expr(c_node current_class,Expression expr){
	
    expr->type = No_type;

    switch (expr->get_type()){
        case assignType:
            {
                assign_class* classptr = (assign_class*) expr;
                Table current_table = current_class->featureTable;
                Feature feature = (Feature)current_table.lookup(classptr->get_name()); 
				// the variable must be defined before assigning a value                
				if(feature == NULL || feature->get_type() != attrType){
                    ostream& os = semant_error(current_class);
                    os << classptr->get_name() << " : identifier not defined.\n";
                }
                else {
                    semant_expr(current_class,classptr->get_expr());

					// check for the type compatibility of expression and the declared type of variable
                    if(is_subclass(get_feature_type(feature),classptr->get_expr()->type,current_class->get_name())){
                        expr->type = classptr->get_expr()->type;
                    }
                    else {
                        ostream& os = semant_error(current_class);
                        os << "expression return type " << classptr->get_expr()->type << " not conform to identifier " << classptr->get_name() << "'s type " << get_feature_type(feature) << ".\n";
                    }
                }
                break;
            }
        case static_dispatchType:
            {
                static_dispatch_class* dispatch = (static_dispatch_class*) expr;
				Expression e0 = dispatch->get_expr();
				Expressions ens = dispatch->get_actual();
				Symbol method_name = dispatch->get_name();
				Symbol t = dispatch->get_type_name();

				semant_expr(current_class,e0);
				Symbol t0  = e0->type;
				
				if(!is_subclass(t, t0,current_class->get_name())){
					ostream& os = semant_error(current_class);
                    os <<"Expression type " << t0 << " does not conform to declared static dispatch type " << t << "." << endl;
				}		

				// find the method 
				c_node t_class = (c_node)class_symtable.probe(t);
				method_class *method = find_method(t_class ,method_name);
				// if method is not defined => error
				if(!method){
					 ostream& os = semant_error(current_class);
                     os << "Dispatch to undefined method " << dispatch->get_name() << "."<<endl;
					 expr->type = Object ;
					 return ;
				}

				bool less_formals = false ;
				Formals formals = method->get_formals();
				int i ;
				// checks whether formals are correctly passed
				for(i = ens->first() ; ens->more(i); i = ens->next(i)){
					Expression en = ens->nth(i);
					semant_expr(current_class,en);
										
					if(formals->more(i)){
						formal_class* f = (formal_class*)formals->nth(i);
						Symbol type_formal = f->get_type_decl();
						// checks type compatibility of formal definition and passed expression
						if(!is_subclass(type_formal , en->type,current_class->get_name())){
							ostream& os = semant_error(current_class);
                     		os << "In call of method " << dispatch->get_name() << ", type " << en->type <<
                   			" of parameter " << f->get_name() << " does not conform to "
                    		"declared type " << type_formal << "." << endl;
					 	
						}
					}else{
						less_formals = true;
					}


				}
				
				//if formals are not correctly passed
				if(less_formals || formals->more(i)){
					ostream& os = semant_error(current_class);
                    os << "Method " << dispatch->get_name() << " called with wrong number of arguments." << endl;
				}
			
				// set the return type of dispatcher
				Symbol m_type = method->get_return_type();
				if(m_type == SELF_TYPE){
					expr->type = t0;
				}
				else{
					expr->type = m_type ;
		
				}
				
				
				
                break;
            }
        case dispatchType:
            {
				// this is the typecheck for dynamic dispatcher, the logic is same as before (static dispatcher)
                dispatch_class* dispatch = (dispatch_class*) expr;
				Expression e0 = dispatch->get_expr();
				Expressions ens = dispatch->get_actual();
				Symbol method_name = dispatch->get_name();
				
				semant_expr(current_class,e0);
				Symbol t0  = e0->type;
				Symbol t0d = t0 ;
				if(t0 == SELF_TYPE){
					t0d = current_class->get_name();		
		
				}
				c_node e0d_class = (c_node)class_symtable.probe(t0d);
				method_class *method = find_method(e0d_class , dispatch->get_name());

				if(!method){
					 ostream& os = semant_error(current_class);
                     os << "Dispatch to undefined method " << dispatch->get_name() << "."<<endl;
					 expr->type = Object ;
					 return ;
				}

				bool less_formals = false ;
				Formals formals = method->get_formals();
				int i ; 
				for(i = ens->first() ; ens->more(i); i = ens->next(i)){
					Expression en = ens->nth(i);
					semant_expr(current_class,en);

					
					if(formals->more(i)){
						formal_class* f = (formal_class*)formals->nth(i);
						Symbol type_formal = f->get_type_decl();

						if(!is_subclass(type_formal , en->type,current_class->get_name())){
							ostream& os = semant_error(current_class);
                     		os << "In call of method " << dispatch->get_name() << ", type " << en->type <<
                   			" of parameter " << f->get_name() << " does not conform to "
                    		"declared type " << type_formal << "." << endl;
					 	
						}
					}else{
						less_formals = true;
					}


				}
				
				if(less_formals || formals->more(i)){
					ostream& os = semant_error(current_class);
                    os << "Method " << dispatch->get_name() << " called with wrong number of arguments." << endl;
				}

				Symbol m_type = method->get_return_type();
				if(m_type == SELF_TYPE){
					expr->type = t0;
				}
				else{
					expr->type = m_type ;
		
				}
				
				
				
                break;
            }
        case condType:
            {
				//typecheck for if-else conditions				
				cond_class * classptr = (cond_class*)expr;
				
				Expression e1 = classptr->get_pred();
				Expression e2 = classptr->get_then_exp();
				Expression e3 = classptr->get_else_exp();
				
				//check the type of the predicate, it must be bool
				semant_expr(current_class,e1);
				if(e1->type!=Bool){
					ostream& os = semant_error(current_class);
                 	os <<   "Predicate of 'if' does not have type Bool" << ".\n";
				}
				
				//type check the two expressions (expression is if then clock and else block)
				semant_expr(current_class,e2);
  				semant_expr(current_class,e3);
				
				//type of the condition expression is LUB of two expression types
				expr->type = get_union(e2->type,e3->type);

                break;
            }
        case loopType:
            {
				//type check for while loop
				loop_class* loop = (loop_class*) expr ;
				Expression pred = loop->get_pred();
				Expression body = loop->get_body();
				
				//typecheck the predicate, it must be bool
				semant_expr(current_class,pred);
				Symbol pred_type = pred->type;
				if(pred_type!=Bool){
					ostream& os = semant_error(current_class);
                 	os <<   "Loop condition does not have type Bool" << ".\n";
				}

				semant_expr(current_class,body);
				expr->type = Object;				
                break;
            }
        case caseType:
            {
				typcase_class* typcase = (typcase_class*) expr ;
				Cases cases = typcase->get_cases();
				Table current_table = current_class->featureTable;
				Expression e0 = typcase->get_expr();
				
				semant_expr(current_class,e0);
				
				//this vector is used to hold previously used types in branches
				// there cannot be branches with same type
				std::vector<Symbol> used; 
		
				Symbol type, prev_type ;
   				//typecheck for the branches of the case
				for(int i = cases->first() ; cases->more(i) ; i = cases->next(i) ){
					prev_type = type;
 
					branch_class* c = (branch_class*)cases->nth(i);
					Symbol type_decl = c->get_type_decl();
					// if type is used previously => error
					if (std::find(used.begin(), used.end(), type_decl) == used.end() ) {
            			used.push_back(type_decl);
        			} else {
            			ostream& os = semant_error(current_class);
                        os <<   "Duplicate branch " << type_decl << " in case statement." << ".\n";
           				return;
        			}
					
					current_table.enterscope();

					current_table.addid(c->get_name(),c);
					// typecheck the body of the branch
					semant_expr(current_class,c->get_expr());
                    
                    type = c->get_expr()->type;
					// type of the case is union of the types of branches
					if(i>0){
						type = get_union(type,prev_type);
					}                    
					current_table.exitscope();					
				}
				expr->type = type;
                break;
            }
        case blockType:
            {
				block_class* classptr = (block_class*)expr;
				Expressions exprs = classptr->get_body();
				
				Symbol last_type;
				// typecheck each expression of the block
				for(int i = exprs->first();exprs->more(i);i = exprs->next(i)){
					Expression nth = exprs->nth(i);					
					semant_expr(current_class,nth);
					last_type = nth->type;
				}				
				// type of the block is the type of last expression in the block
				expr->type = last_type;
                break;
            }
        case letType:
            {
				let_class* classptr = (let_class*)expr;
				Table current_table = current_class->featureTable;

				Expression init = classptr->get_init();
				Symbol t0 = classptr->get_type_decl();

				if(t0==SELF_TYPE){
					t0 = current_class->get_name();
				}				

				Symbol identifier = classptr->get_identifier();
				Expression body = classptr->get_body();				

				semant_expr(current_class,init);				
				Symbol t1 = init->type;
				
				Symbol type = Object;

				if(t1!=No_type){
					//type cheking for let with init					
					if (!is_subclass(t0, t1,current_class->get_name())) {
        				ostream& os = semant_error(current_class);
                    	os << "Inferred type " << t1 << " of initialization of " << classptr->get_identifier() << " does not confirm to identifier's declared type "<<t0<<"."<< endl;
    				}
				}
				
				current_table.enterscope();

				if(identifier==self){
					ostream& os = semant_error(current_class);
                  	os << "'self' cannot be bound in a 'let' expression."<< endl;
				}
				else{
					current_table.addid(identifier,classptr);
				}				
				
				semant_expr(current_class,body);
				type = body->type;					
					
				current_table.exitscope();
				
				expr->type = type;
                break;
            }
		// typechecks for arithmatic operations and booleanoperations follows same format
		// the oparnds must have type Int
        case plusType:
            {
                plus_class* classptr = (plus_class*)expr;
                semant_expr(current_class,classptr->get_e1());
                semant_expr(current_class,classptr->get_e2());
                if( classptr->get_e1()->type == Int && classptr->get_e2()->type == Int){
                    expr->type = Int;
                }
                else {
                    ostream& os = semant_error(current_class);
                    os << "non-Int arguments: " << classptr->get_e1()->type << " + " << classptr->get_e2()->type << "."<<endl;
                }
                break;
            }
        case subType:
            {
                sub_class* classptr = (sub_class*)expr;
                semant_expr(current_class,classptr->get_e1());
                semant_expr(current_class,classptr->get_e2());
                if( classptr->get_e1()->type == Int && classptr->get_e2()->type == Int){
                    expr->type = Int;
                }
                else {
                    ostream& os = semant_error(current_class);
                    os << "non-Int arguments: " << classptr->get_e1()->type << " - " << classptr->get_e2()->type << "."<<endl;
                }
                break;
            }
        case mulType:
            {
                mul_class* classptr = (mul_class*)expr;
                semant_expr(current_class,classptr->get_e1());
                semant_expr(current_class,classptr->get_e2());
                if( classptr->get_e1()->type == Int && classptr->get_e2()->type == Int){
                    expr->type = Int;
                }
                else {
                    ostream& os = semant_error(current_class);
                    os << "non-Int arguments: " << classptr->get_e1()->type << " * " << classptr->get_e2()->type << "."<<endl;
                }
                break;
            }
        case divType:
            {
                divide_class* classptr = (divide_class*)expr;
                semant_expr(current_class,classptr->get_e1());
                semant_expr(current_class,classptr->get_e2());
                if( classptr->get_e1()->type == Int && classptr->get_e2()->type == Int){
                    expr->type = Int;
                }
                else {
                    ostream& os = semant_error(current_class);
                    os << "non-Int arguments: " << classptr->get_e1()->type << " + " << classptr->get_e2()->type << "." << endl;
                }
                break;
            }
        case negType:
            {
                neg_class* classptr = (neg_class*) expr;
                semant_expr(current_class,classptr->get_e1());
                if( classptr->get_e1()->type == Int){
                    expr->type = Int;
                }
                else {
                    ostream& os = semant_error(current_class);
                    os << "non-Int arguments: " << classptr->get_e1()->type <<  "." << endl;
                }
                break;
            }
        case ltType:
            {
                lt_class* classptr = (lt_class*) expr;
                semant_expr(current_class,classptr->get_e1());
                semant_expr(current_class,classptr->get_e2());
                if( classptr->get_e1()->type == Int && classptr->get_e2()->type == Int){
                    expr->type = Bool;
                }
                else {
                    ostream& os = semant_error(current_class);
                    os << "non-Int arguments: " << classptr->get_e1()->type <<  " = " << classptr->get_e2()->type << "." << endl;
                }
 
                break;
            }
        case eqType:
            {
                eq_class* classptr = (eq_class*) expr;
                semant_expr(current_class,classptr->get_e1());
                semant_expr(current_class,classptr->get_e2());
                if( classptr->get_e1()->type == Int && classptr->get_e2()->type == Int){
                    expr->type = Bool;
                }
                else {
                    ostream& os = semant_error(current_class);
                    os << "non-Int arguments: " << classptr->get_e1()->type <<  " = " << classptr->get_e2()->type << "." << endl;
                }
 
                break;
            }
        case leqType:
            {
                leq_class* classptr = (leq_class*) expr;
                semant_expr(current_class,classptr->get_e1());
                semant_expr(current_class,classptr->get_e2()); 
                if( classptr->get_e1()->type == Int && classptr->get_e2()->type == Int){
                    expr->type = Bool;
                }
                else {
                    ostream& os = semant_error(current_class);
                    os << "non-Int arguments: " << classptr->get_e1()->type <<  " <= " << classptr->get_e2()->type << "." << endl;
                }
                break;
            }
        case compType:
            {
                comp_class* classptr = (comp_class*) expr;
                semant_expr(current_class,classptr->get_e1());
                if( classptr->get_e1()->type == Int){
                    expr->type = Int;
                }
                else {
                    ostream& os = semant_error(current_class);
                    os << "non-Int arguments: " << classptr->get_e1()->type <<  "." << endl;
                }

                break;
            }
        case int_constType:
            {
                expr->type = Int;
                break;
            }
        case bool_constType:
            {
                expr->type = Bool;
                break;
            }
        case string_constType:
            {
                expr->type = Str;
                break;
            }
        case newType:
            {
                new__class* classptr = (new__class*)expr;
                Symbol type_name = classptr->get_type_name();
                if (type_name == SELF_TYPE){
                    expr->type = current_class->get_name();
                }
                else {
                    expr->type = type_name;
                }
                break;
            }
        case isvoidType:
            {
                expr->type = Bool;
                break;
            }
        case no_exprType:
            {
                expr->type = No_type;
                break;
            }
        case objectType:
            {
                object_class* classptr = (object_class*) expr;
                Table current_table = current_class->featureTable;
                Symbol name = classptr->get_name();
				// if the object name is self the type of expression is SELF TYPE
                if(name==self){
					expr->type = SELF_TYPE;
				}
				else {
					// the object must previously defined in the scope
					Feature feature = (Feature)current_table.lookup(name);
		
					if( feature == NULL || feature->get_type() == methodType){
		                ostream& os = semant_error(current_class);
		                os << "Undeclared identifier " << name << endl;
		            }
		            else{
		         		expr->type = get_feature_type(feature);
		            }
				}
                break;
            }

        default:
            break;
    }

}



/*
 *  subroutines that helps implement the semantic analysis
 *
 *  is_subclass(Symbol parent, Symbol child,Symbol current_class) : check if inheritance exists
 * 
 *  get_union Symbol type1,Symbol type2) : find the least upper bound between two input types
 *
 *  get_feature_type(c_node class,Feature feature):find the type of the feature
 */

bool ClassTable::is_subclass(Symbol parent, Symbol child,Symbol current_class) {
    if (child == SELF_TYPE) {
        if (parent == SELF_TYPE) {
            return true;
        }
		child = current_class;
    }

	if ( parent == child || parent == Object || child == No_type ){
        return true;
    }

    while (child != No_class){
        c_node c = (c_node) class_symtable.lookup(child);
        if (c == NULL){
            return false;
        }
        child = c->get_parent();
        if (child == parent){
            return true;
        }
    }
    return false;
}

Symbol ClassTable::get_union(Symbol curr_type, Symbol prev_type){

	std::stack<Symbol> curr_stack, prev_stack;
    
	class__class* curr = static_cast<class__class*>(class_symtable.lookup(curr_type));

    while (curr != NULL) {
        curr_stack.push(curr->get_name());
        curr = static_cast<class__class*>(class_symtable.lookup(curr->get_parent()));
    }

    curr = static_cast<class__class*>(class_symtable.lookup(prev_type));

    while (curr != NULL) {
        prev_stack.push(curr->get_name());
        curr = static_cast<class__class*>(class_symtable.lookup(curr->get_parent()));
    }

    Symbol curr_head = curr_stack.top();
    curr_stack.pop();

    Symbol prev_head = prev_stack.top();
    prev_stack.pop();

    Symbol common_type = curr_head;

    while (prev_head == curr_head && !curr_stack.empty() && !prev_stack.empty()) {
         
		curr_head = curr_stack.top();
        curr_stack.pop();

        prev_head = prev_stack.top();
        prev_stack.pop();

        if (curr_head == prev_head) {
        	common_type = curr_head;
        }
    }
    return common_type;
	
}

Symbol ClassTable::get_feature_type(Feature feature){    
	if(feature->get_type() == attrType){
        attr_class* attr = (attr_class*) feature;
        return attr->get_type_decl();
    }
	else if(feature->get_type()==formalType){
		formal_class* formal = (formal_class*) feature;
		return formal->get_type_decl();
	}
    else {
        method_class* method = (method_class*) feature;
        return method->get_return_type();
    }  
}

method_class* ClassTable::find_method(c_node current_class , Symbol method_name){
	if(current_class->get_name() == Object){
		return NULL;
	}
	Table fTable = current_class->featureTable;
	Feature_class* f = (Feature_class*)fTable.probe(method_name) ;
	if( f !=NULL && f-> get_type() == methodType){
		return (method_class*)f;
	}else{
		c_node parent = (c_node)class_symtable.probe(current_class->get_parent());
		return find_method(parent,method_name);
	}	
}
/*
 * subroutines end
 */


void ClassTable::install_basic_classes() {

    // The tree package uses these globals to annotate the classes built below.
    // curr_lineno  = 0;
    Symbol filename = stringtable.add_string("<basic class>");

    // The following demonstrates how to create dummy parse trees to
    // refer to basic Cool classes.  There's no need for method
    // bodies -- these are already built into the runtime system.

    // IMPORTANT: The results of the following expressions are
    // stored in local variables.  You will want to do something
    // with those variables at the end of this method to make this
    // code meaningful.

    // 
    // The Object class has no parent class. Its methods are
    //        abort() : Object    aborts the program
    //        type_name() : Str   returns a string representation of class name
    //        copy() : SELF_TYPE  returns a copy of the object
    //
    // There is no need for method bodies in the basic classes---these
    // are already built in to the runtime system.

    Class_ Object_class =
        class_(Object, 
                No_class,
                append_Features(
                    append_Features(
                        single_Features(method(cool_abort, nil_Formals(), Object, no_expr())),
                        single_Features(method(type_name, nil_Formals(), Str, no_expr()))),
                    single_Features(method(copy, nil_Formals(), SELF_TYPE, no_expr()))),
                filename);

    // 
    // The IO class inherits from Object. Its methods are
    //        out_string(Str) : SELF_TYPE       writes a string to the output
    //        out_int(Int) : SELF_TYPE            "    an int    "  "     "
    //        in_string() : Str                 reads a string from the input
    //        in_int() : Int                      "   an int     "  "     "
    //
    Class_ IO_class = 
        class_(IO, 
                Object,
                append_Features(
                    append_Features(
                        append_Features(
                            single_Features(method(out_string, single_Formals(formal(arg, Str)),
                                    SELF_TYPE, no_expr())),
                            single_Features(method(out_int, single_Formals(formal(arg, Int)),
                                    SELF_TYPE, no_expr()))),
                        single_Features(method(in_string, nil_Formals(), Str, no_expr()))),
                    single_Features(method(in_int, nil_Formals(), Int, no_expr()))),
                filename);  

    //
    // The Int class has no methods and only a single attribute, the
    // "val" for the integer. 
    //
    Class_ Int_class =
        class_(Int, 
                Object,
                single_Features(attr(val, prim_slot, no_expr())),
                filename);

    //
    // Bool also has only the "val" slot.
    //
    Class_ Bool_class =
        class_(Bool, Object, single_Features(attr(val, prim_slot, no_expr())),filename);

    //
    // The class Str has a number of slots and operations:
    //       val                                  the length of the string
    //       str_field                            the string itself
    //       length() : Int                       returns length of the string
    //       concat(arg: Str) : Str               performs string concatenation
    //       substr(arg: Int, arg2: Int): Str     substring selection
    //       
    Class_ Str_class =
        class_(Str, 
                Object,
                append_Features(
                    append_Features(
                        append_Features(
                            append_Features(
                                single_Features(attr(val, Int, no_expr())),
                                single_Features(attr(str_field, prim_slot, no_expr()))),
                            single_Features(method(length, nil_Formals(), Int, no_expr()))),
                        single_Features(method(concat, 
                                single_Formals(formal(arg, Str)),
                                Str, 
                                no_expr()))),
                    single_Features(method(substr, 
                            append_Formals(single_Formals(formal(arg, Int)), 
                                single_Formals(formal(arg2, Int))),
                            Str, 
                            no_expr()))),
                filename);

    // add primitive class into class table
    class_symtable.addid(Object,(class__class*)Object_class); 
    class_symtable.addid(IO,(class__class*)IO_class); 
    class_symtable.addid(Int,(class__class*)Int_class); 
    class_symtable.addid(Bool,(class__class*)Bool_class); 
    class_symtable.addid(Str,(class__class*)Str_class); 

}

////////////////////////////////////////////////////////////////////
//
// semant_error is an overloaded function for reporting errors
// during semantic analysis.  There are three versions:
//
//    ostream& ClassTable::semant_error()                
//
//    ostream& ClassTable::semant_error(Class_ c)
//       print line number and filename for `c'
//
//    ostream& ClassTable::semant_error(Symbol filename, tree_node *t)  
//       print a line number and filename
//
///////////////////////////////////////////////////////////////////

ostream& ClassTable::semant_error(Class_ c)
{         

    return semant_error(c->get_filename(),c);
}    

ostream& ClassTable::semant_error(Symbol filename, tree_node *t)
{
    error_stream << filename << ":" << t->get_line_number() << ": ";
    return semant_error();
}

ostream& ClassTable::semant_error()                  
{                                                 
    semant_errors++;                            
    return error_stream;
} 



/*   This is the entry point to the semantic checker.
     Your checker should do the following two things:
     1) Check that the program is semantically correct
     2) Decorate the abstract syntax tree with type information
     by setting the `type' field in each Expression node.
     (see `tree.h')
     You are free to first do 1), make sure you catch all semantic
     errors. Part 2) can be done in a second stage, when you want
     to build mycoolc.
     */
void program_class::semant()
{
    initialize_constants();

    /* ClassTable constructor may do some semantic analysis */

    // the parameter 'classes' was received from the result of parser. 
    ClassTable *classtable = new ClassTable(classes);

    /* some semantic analysis code may go here */

    if (classtable->errors()) {
        cerr << "Compilation halted due to static semantic errors." << endl;
        exit(1);
    }
}
