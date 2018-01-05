#include "typecheck.hpp"

// Defines the function used to throw type errors. The possible
// type errors are defined as an enumeration in the header file.
void typeError(TypeErrorCode code) {
  switch (code) {
    case undefined_variable:
      std::cerr << "Undefined variable." << std::endl;
      break;
    case undefined_method:
      std::cerr << "Method does not exist." << std::endl;
      break;
    case undefined_class:
      std::cerr << "Class does not exist." << std::endl;
      break;
    case undefined_member:
      std::cerr << "Class member does not exist." << std::endl;
      break;
    case not_object:
      std::cerr << "Variable is not an object." << std::endl;
      break;
    case expression_type_mismatch:
      std::cerr << "Expression types do not match." << std::endl;
      break;
    case argument_number_mismatch:
      std::cerr << "Method called with incorrect number of arguments." << std::endl;
      break;
    case argument_type_mismatch:
      std::cerr << "Method called with argument of incorrect type." << std::endl;
      break;
    case while_predicate_type_mismatch:
      std::cerr << "Predicate of while loop is not boolean." << std::endl;
      break;
    case do_while_predicate_type_mismatch:
      std::cerr << "Predicate of do while loop is not boolean." << std::endl;
      break;
    case if_predicate_type_mismatch:
      std::cerr << "Predicate of if statement is not boolean." << std::endl;
      break;
    case assignment_type_mismatch:
      std::cerr << "Left and right hand sides of assignment types mismatch." << std::endl;
      break;
    case return_type_mismatch:
      std::cerr << "Return statement type does not match declared return type." << std::endl;
      break;
    case constructor_returns_type:
      std::cerr << "Class constructor returns a value." << std::endl;
      break;
    case no_main_class:
      std::cerr << "The \"Main\" class was not found." << std::endl;
      break;
    case main_class_members_present:
      std::cerr << "The \"Main\" class has members." << std::endl;
      break;
    case no_main_method:
      std::cerr << "The \"Main\" class does not have a \"main\" method." << std::endl;
      break;
    case main_method_incorrect_signature:
      std::cerr << "The \"main\" method of the \"Main\" class has an incorrect signature." << std::endl;
      break;
  }
  exit(1);
}

// TypeCheck Visitor Functions: These are the functions you will
// complete to build the symbol table and type check the program.
// Not all functions must have code, many may be left empty.


void TypeCheck::visitProgramNode(ProgramNode* node) 
{
    classTable = new ClassTable();
    node->visit_children(this);
    if (classTable->count("Main") == 0) {
        typeError(no_main_class);
    }
}

void TypeCheck::visitClassNode(ClassNode* node)
{
	ClassInfo classInfo;
	
	if (node->identifier_2) {
        if (classTable->count(node->identifier_2->name) == 0 ) {
            typeError(undefined_class);
        }
        classInfo.superClassName = node->identifier_2->name;
    } 
	else classInfo.superClassName = "";

    currentMemberOffset = 0;
    currentLocalOffset = 0;
    currentClassName = node->identifier_1->name;
    currentVariableTable = new VariableTable();
    for (std::list<DeclarationNode*>::iterator iter = node->declaration_list->begin(); iter != node->declaration_list->end(); ++iter) {
        (*iter)->accept(this);
    }
    classInfo.members = currentVariableTable;
    classInfo.membersSize = currentVariableTable->size()*4;
	
    if (currentClassName == "Main" && currentVariableTable->size() > 0) {
        typeError(main_class_members_present);
	}

	(*classTable)[currentClassName] = classInfo;
	
	currentMethodTable = new MethodTable();
    for (std::list<MethodNode*>::iterator iter = node->method_list->begin(); iter != node->method_list->end(); ++iter) {
        (*iter)->accept(this);
    }
    classInfo.methods = currentMethodTable;
    
    if (currentClassName == "Main" && currentMethodTable->count("main") == 0) {
        typeError(no_main_method);
	}
    
    (*classTable)[currentClassName] = classInfo;
}

void TypeCheck::visitMethodNode(MethodNode* node) 
{
    MethodInfo methodInfo;
    std::list<CompoundType> *parameters = new std::list<CompoundType>();
	
    currentLocalOffset = -4;
    currentParameterOffset = 12;
	currentVariableTable = new VariableTable();
    for (std::list<ParameterNode*>::iterator iter = node->parameter_list->begin(); iter != node->parameter_list->end(); ++iter) {
		CompoundType parameterType;
		VariableInfo variableInfo;
		
        (*iter)->accept(this);
        parameterType.baseType = (*iter)->type->basetype;
        parameterType.objectClassName = (*iter)->type->objectClassName;
        
        variableInfo.type = parameterType;
		
        variableInfo.offset = currentParameterOffset;
		currentParameterOffset += 4;
		
        variableInfo.size = 4;
		
        parameters->push_back(parameterType);
        (*currentVariableTable)[(*iter)->identifier->name] = variableInfo;
    }
	
    node->type->accept(this);
    node->methodbody->accept(this);
	
    methodInfo.returnType.baseType = node->type->basetype;
    methodInfo.returnType.objectClassName = node->type->objectClassName;
	methodInfo.parameters = parameters;
    methodInfo.variables = currentVariableTable;
    methodInfo.localsSize = currentLocalOffset*(-1)-4;
	
	if (node->type->basetype != bt_none && currentClassName == node->identifier->name) {
        typeError(constructor_returns_type);
	}
	
    if ((node->type->basetype != bt_none && node->methodbody->returnstatement == NULL)
	    || (node->type->basetype == bt_none && node->methodbody->returnstatement != NULL)) {
        typeError(return_type_mismatch);
    }
    
    if ((node->type->basetype == bt_object && node->methodbody->basetype == bt_object) 
        && (node->methodbody->objectClassName != node->type->objectClassName)) {
            typeError(return_type_mismatch);
    }
    
	if (node->type->basetype != bt_none && (node->type->objectClassName != node->methodbody->objectClassName || node->type->basetype != node->methodbody->basetype)) {
        typeError(return_type_mismatch);
    }
	
	if (currentClassName == "Main" && node->identifier->name == "main" && (parameters->size() > 0 || node->type->basetype != bt_none)) {
        typeError(main_method_incorrect_signature);
    }
	
    (*currentMethodTable)[node->identifier->name] = methodInfo;
}

void TypeCheck::visitMethodBodyNode(MethodBodyNode* node) 
{
    node->visit_children(this);
    if (node->returnstatement) {
        node->basetype = node->returnstatement->basetype;
        node->objectClassName = node->returnstatement->objectClassName;
    }
}

void TypeCheck::visitParameterNode(ParameterNode* node) 
{
	node->type->accept(this);
}

void TypeCheck::visitDeclarationNode(DeclarationNode* node) 
{
    node->type->accept(this);
    if (node->type->basetype == bt_object && classTable->count(node->type->objectClassName) == 0) {
        typeError(undefined_class);
    }
    
    for (std::list<IdentifierNode*>::iterator iter = node->identifier_list->begin(); iter != node->identifier_list->end(); ++iter) {
        VariableInfo variableInfo;
		
        variableInfo.type.baseType = node->type->basetype;
		variableInfo.type.objectClassName = node->type->objectClassName;
		
        variableInfo.size = 4;
		
        (*iter)->basetype = node->type->basetype;
		
        if (currentLocalOffset != 0) {
			variableInfo.offset = currentLocalOffset;
            currentLocalOffset -= 4;
        } 
		else {
            variableInfo.offset = currentMemberOffset;
            currentMemberOffset += 4;
        }
		
        (*currentVariableTable)[(*iter)->name] = variableInfo;
    }
}

void TypeCheck::visitReturnStatementNode(ReturnStatementNode* node) 
{
    node->expression->accept(this);
    node->basetype = node->expression->basetype;
    node->objectClassName = node->expression ->objectClassName;
}

void TypeCheck::visitAssignmentNode(AssignmentNode* node) 
{
    node->expression->accept(this);
	
    CompoundType memberType;
	// flag to help determine if var is defined
    memberType.objectClassName = "-1";
    
    if (currentVariableTable->count(node->identifier_1->name) > 0) {
        memberType = (*currentVariableTable)[node->identifier_1->name].type;
    } 
	else if ((*classTable)[currentClassName].members->count(node->identifier_1->name) > 0) {
        memberType = (*((*classTable)[currentClassName].members))[node->identifier_1->name].type;
    }
	else {
        ClassInfo curClassInfo;
        curClassInfo = (*classTable)[currentClassName];
        std::string superClassName = curClassInfo.superClassName;
    
        while (superClassName != "") {
            curClassInfo = (*classTable)[superClassName];
            if (curClassInfo.members->count(node->identifier_1->name) > 0) {
                memberType = (*(curClassInfo.members))[node->identifier_1->name].type;
                break;
            }
            superClassName = curClassInfo.superClassName;
        }
    } 
    
    if (!node->identifier_2) {
        if (memberType.objectClassName != "-1") {
            node->basetype = memberType.baseType;
            node->objectClassName = memberType.objectClassName;
        } 
		else typeError(undefined_variable);
    } 
	else {
        if (memberType.objectClassName == "-1") typeError(undefined_variable);
        if (memberType.baseType != bt_object) typeError(not_object);
		
        if ((*classTable)[memberType.objectClassName].members->count(node->identifier_2->name) > 0) {
            node->basetype = (*((*classTable)[memberType.objectClassName].members))[node->identifier_2->name].type.baseType;
            node->objectClassName = (*((*classTable)[memberType.objectClassName].members))[node->identifier_2->name].type.objectClassName;
        } 
		else typeError(undefined_member);
    }
    
    if (node->objectClassName != node->expression->objectClassName || node->basetype != node->expression->basetype) {
        typeError(assignment_type_mismatch);
    }
}

void TypeCheck::visitCallNode(CallNode* node) 
{
    node->visit_children(this);
    node->basetype = node->methodcall->basetype;
    node->objectClassName = node->methodcall->objectClassName;
}

void TypeCheck::visitIfElseNode(IfElseNode* node) 
{
    node->visit_children(this);
    if (node->expression->basetype != bt_boolean) {
        typeError(if_predicate_type_mismatch);
    }
}

void TypeCheck::visitWhileNode(WhileNode* node) 
{
    node->visit_children(this);
    if (node->expression->basetype != bt_boolean) {
        typeError(while_predicate_type_mismatch);
    }
}

void TypeCheck::visitDoWhileNode(DoWhileNode* node) 
{
    node->visit_children(this);
    if (node->expression->basetype != bt_boolean) {
        typeError(do_while_predicate_type_mismatch);
    }
}

void TypeCheck::visitPrintNode(PrintNode* node) 
{
    node->visit_children(this);
}

void TypeCheck::visitPlusNode(PlusNode* node) 
{
    node->visit_children(this);
    if (node->expression_1->basetype != bt_integer || node->expression_2->basetype != bt_integer) {
        typeError(expression_type_mismatch);
    }
    node->basetype = bt_integer;
}

void TypeCheck::visitMinusNode(MinusNode* node) 
{
    node->visit_children(this);
    if (node->expression_1->basetype != bt_integer || node->expression_2->basetype != bt_integer) {
        typeError(expression_type_mismatch);
    }
    node->basetype = bt_integer;
}

void TypeCheck::visitTimesNode(TimesNode* node) 
{
    node->visit_children(this);
    if (node->expression_1->basetype != bt_integer || node->expression_2->basetype != bt_integer) {
        typeError(expression_type_mismatch);
    }
    node->basetype = bt_integer;
}

void TypeCheck::visitDivideNode(DivideNode* node) 
{
    node->visit_children(this);
    if (node->expression_1->basetype != bt_integer || node->expression_2->basetype != bt_integer) {
        typeError(expression_type_mismatch);
    }
    node->basetype = bt_integer;
}

void TypeCheck::visitGreaterNode(GreaterNode* node) 
{
    node->visit_children(this);
    if (node->expression_1->basetype != bt_integer || node->expression_2->basetype != bt_integer) {
        typeError(expression_type_mismatch);
    }
    node->basetype = bt_boolean;
}

void TypeCheck::visitGreaterEqualNode(GreaterEqualNode* node) 
{
    node->visit_children(this);
    if (node->expression_1->basetype != bt_integer || node->expression_2->basetype != bt_integer) {
        typeError(expression_type_mismatch);
    }
    node->basetype = bt_boolean;
}

void TypeCheck::visitEqualNode(EqualNode* node) 
{
    node->visit_children(this);
    if (node->expression_1->basetype != node->expression_2->basetype) {
        typeError(expression_type_mismatch);
    }
	if (node->expression_1->basetype == bt_none || node->expression_1->basetype == bt_object) {
        typeError(expression_type_mismatch);
    }
    node->basetype = bt_boolean;
}

void TypeCheck::visitAndNode(AndNode* node) 
{
    node->visit_children(this);
    if (node->expression_1->basetype != bt_boolean || node->expression_2->basetype != bt_boolean) {
        typeError(expression_type_mismatch);
    }
    node->basetype = bt_boolean;
}

void TypeCheck::visitOrNode(OrNode* node) 
{
    node->visit_children(this);
    if (node->expression_1->basetype != bt_boolean || node->expression_2->basetype != bt_boolean) {
        typeError(expression_type_mismatch);
    }
    node->basetype = bt_boolean;
}

void TypeCheck::visitNotNode(NotNode* node) 
{
    node->expression->accept(this);
    if (node->expression->basetype != bt_boolean) {
        typeError(expression_type_mismatch);
    }
    node->basetype = bt_boolean;
}

void TypeCheck::visitNegationNode(NegationNode* node) 
{
    node->expression->accept(this);
    if (node->expression->basetype != bt_integer) {
        typeError(expression_type_mismatch);
    }
    node->basetype = bt_integer;
}

void TypeCheck::visitMethodCallNode(MethodCallNode* node) 
{
    node->visit_children(this);
	
    MethodInfo methodInfo;
	// flag to help determine if method is defined
    methodInfo.localsSize = -1;
	
    if (!node->identifier_2) {
        if (currentMethodTable->count(node->identifier_1->name) > 0) {
            methodInfo = (*currentMethodTable)[node->identifier_1->name];
        }
        
        ClassInfo curClassInfo;
        curClassInfo = (*classTable)[currentClassName];
        std::string superClassName = curClassInfo.superClassName;
        
        while (superClassName != "") {
            curClassInfo = (*classTable)[superClassName];
            if (curClassInfo.methods->count(node->identifier_1->name) > 0) {
                methodInfo = (*(curClassInfo.methods))[node->identifier_1->name];
                break;
            }
            superClassName = curClassInfo.superClassName;
        }
        
    } 
	else {
        CompoundType returnType;
        returnType.objectClassName = " ";
        
        if (currentVariableTable->count(node->identifier_1->name) > 0) {
            returnType = (*currentVariableTable)[node->identifier_1->name].type;
        }
        else if ((*classTable)[currentClassName].members->count(node->identifier_1->name) > 0) {
            returnType = (*((*classTable)[currentClassName].members))[node->identifier_1->name].type;
        } 
		else {
            ClassInfo curClassInfo;
            curClassInfo = (*classTable)[currentClassName];
            std::string superClassName = curClassInfo.superClassName;
            
            while (superClassName != "") {
                curClassInfo = (*classTable)[superClassName];
                if (curClassInfo.members->count(node->identifier_1->name) > 0) {
                    returnType = (*(curClassInfo.members))[node->identifier_1->name].type;
                    break;
                }
                superClassName = curClassInfo.superClassName;
            }
        }

        if (returnType.baseType != bt_object || returnType.objectClassName == " ") {
            typeError(not_object);
        }
		
        if ((*classTable)[returnType.objectClassName].methods->count(node->identifier_2->name) > 0) {
            methodInfo = (*((*classTable)[returnType.objectClassName].methods))[node->identifier_2->name];
        } 
		else {
            ClassInfo curClassInfo;
            curClassInfo = (*classTable)[returnType.objectClassName];
            std::string superClassName = curClassInfo.superClassName;
            std::string objectName = node->identifier_2->name;
            
            while (superClassName != "") {
                curClassInfo = (*classTable)[superClassName];
                if (curClassInfo.methods->count(objectName) > 0) {
                    methodInfo = (*(curClassInfo.methods))[objectName];
                    break;
                }
                superClassName = curClassInfo.superClassName;
            }
        }

    }
    
    if (methodInfo.localsSize == -1) {
        typeError(undefined_method);
        return;
    }
	
    if (methodInfo.parameters->size() != node->expression_list->size()) {
        typeError(argument_number_mismatch);
        return;
    }
    
    std::list<CompoundType>::iterator parameterTypes = methodInfo.parameters->begin();
    for (std::list<ExpressionNode*>::iterator expressionTypes = node->expression_list->begin(); expressionTypes != node->expression_list->end(); ++expressionTypes) {
        if ((*expressionTypes)->objectClassName != (*parameterTypes).objectClassName || (*expressionTypes)->basetype != (*parameterTypes).baseType) {
            typeError(argument_type_mismatch);
        }
        parameterTypes++;
    }
    
    node->basetype = methodInfo.returnType.baseType;
    node->objectClassName = methodInfo.returnType.objectClassName;
}

void TypeCheck::visitMemberAccessNode(MemberAccessNode* node) 
{
    CompoundType memberType;
	// flag to help determine if member exists
    memberType.objectClassName = " ";
    
    if (currentVariableTable->count(node->identifier_1->name)>0){
        memberType = (*currentVariableTable)[node->identifier_1->name].type;
    }
    if ((*classTable)[currentClassName].members->count(node->identifier_1->name) > 0) {
        memberType = (*((*classTable)[currentClassName].members))[node->identifier_1->name].type;
    } 
	else {
        ClassInfo curClassInfo;
        curClassInfo = (*classTable)[currentClassName];
        std::string superClassName = curClassInfo.superClassName;
        while (superClassName != "") {
            curClassInfo = (*classTable)[superClassName];
            if (curClassInfo.members->count(node->identifier_1->name) > 0) {
                memberType = (*(curClassInfo.members))[node->identifier_1->name].type;
                break;
            }
            superClassName = curClassInfo.superClassName;
        }
    }
    
    if (memberType.objectClassName == " " || memberType.baseType != bt_object) {
        typeError(not_object);
    }
    
    if ((*classTable)[memberType.objectClassName].members->count(node->identifier_2->name) > 0) {
        memberType = (*((*classTable)[memberType.objectClassName].members))[node->identifier_2->name].type;
    } 
	else {
        ClassInfo curClassInfo;
        curClassInfo.membersSize = -1;
        curClassInfo = (*classTable)[memberType.objectClassName];
        std::string superClassName = curClassInfo.superClassName;
        std::string objectName = node->identifier_2->name;
        memberType.objectClassName = " ";
        
        while (superClassName != "") {
            curClassInfo = (*classTable)[superClassName];
            if (curClassInfo.members->count(objectName) > 0) {
                memberType = (*(curClassInfo.members))[objectName].type;
                break;
            }
            superClassName = curClassInfo.superClassName;
        }
        if (memberType.objectClassName == " ") {
            typeError(undefined_member);
        }
    }
	
    node->basetype = memberType.baseType;
    node->objectClassName = memberType.objectClassName;
}

void TypeCheck::visitVariableNode(VariableNode* node) 
{
    node->visit_children(this);
	
    CompoundType variableType;
	// flag to help determine if var exists
    variableType.objectClassName = " ";
    
    if (currentVariableTable->count(node->identifier->name) > 0) {
        variableType = (*currentVariableTable)[node->identifier->name].type;
    }
	
    if ((*classTable)[currentClassName].members->count(node->identifier->name) > 0) {
        variableType = (*((*classTable)[currentClassName].members))[node->identifier->name].type;
    } 
	else {
        ClassInfo curClassInfo;
        curClassInfo = (*classTable)[currentClassName];
        std::string superClassName = curClassInfo.superClassName;
        while (superClassName != "") {
            curClassInfo = (*classTable)[superClassName];
            if (curClassInfo.members->count(node->identifier->name) > 0) {
                variableType = (*(curClassInfo.members))[node->identifier->name].type;
                break;
            }
            superClassName = curClassInfo.superClassName;
        }
    }

    if (variableType.objectClassName != " ") {
        node->basetype = variableType.baseType;
        node->objectClassName = variableType.objectClassName;
    }
	else typeError(undefined_variable);
}

void TypeCheck::visitIntegerLiteralNode(IntegerLiteralNode* node) 
{
    node->basetype = bt_integer;
}

void TypeCheck::visitBooleanLiteralNode(BooleanLiteralNode* node) 
{
    node->basetype = bt_boolean;
}

void TypeCheck::visitNewNode(NewNode* node) 
{
    node->visit_children(this);
    
    if (classTable->count(node->identifier->name) == 0) typeError(undefined_class);
    if ((*classTable)[node->identifier->name].methods->count(node->identifier->name) == 0) typeError(undefined_method);
    
    MethodInfo methodInfo = (*((*classTable)[node->identifier->name].methods))[node->identifier->name];
    if (methodInfo.parameters->size() != node->expression_list->size()) {
        typeError(argument_number_mismatch);
    }
    
    std::list<CompoundType>::iterator parameterTypes = methodInfo.parameters->begin();
    for (std::list<ExpressionNode*>::iterator expressionTypes = node->expression_list->begin(); expressionTypes != node->expression_list->end(); ++expressionTypes){
        if ((*expressionTypes)->basetype != (*parameterTypes).baseType || (*expressionTypes)->objectClassName != (*parameterTypes).objectClassName) {
            typeError(argument_type_mismatch);
        }
        parameterTypes++;
    }
    
    node->basetype = bt_object;
    node->objectClassName = node->identifier->name;
}

void TypeCheck::visitIntegerTypeNode(IntegerTypeNode* node) 
{
    node->basetype = bt_integer;
}

void TypeCheck::visitBooleanTypeNode(BooleanTypeNode* node) 
{
    node->basetype = bt_boolean;
}

void TypeCheck::visitObjectTypeNode(ObjectTypeNode* node) 
{
    node->basetype = bt_object;
    node->objectClassName = node->identifier->name;
}

void TypeCheck::visitNoneNode(NoneNode* node) 
{
    node->basetype = bt_none;
}

void TypeCheck::visitIdentifierNode(IdentifierNode* node) {}

void TypeCheck::visitIntegerNode(IntegerNode* node) {}


// The following functions are used to print the Symbol Table.
// They do not need to be modified at all.

std::string genIndent(int indent) {
  std::string string = std::string("");
  for (int i = 0; i < indent; i++)
    string += std::string(" ");
  return string;
}

std::string string(CompoundType type) {
  switch (type.baseType) {
    case bt_integer:
      return std::string("Integer");
    case bt_boolean:
      return std::string("Boolean");
    case bt_none:
      return std::string("None");
    case bt_object:
      return std::string("Object(") + type.objectClassName + std::string(")");
    default:
      return std::string("");
  }
}


void print(VariableTable variableTable, int indent) {
  std::cout << genIndent(indent) << "VariableTable {";
  if (variableTable.size() == 0) {
    std::cout << "}";
    return;
  }
  std::cout << std::endl;
  for (VariableTable::iterator it = variableTable.begin(); it != variableTable.end(); it++) {
    std::cout << genIndent(indent + 2) << it->first << " -> {" << string(it->second.type);
    std::cout << ", " << it->second.offset << ", " << it->second.size << "}";
    if (it != --variableTable.end())
      std::cout << ",";
    std::cout << std::endl;
  }
  std::cout << genIndent(indent) << "}";
}

void print(MethodTable methodTable, int indent) {
  std::cout << genIndent(indent) << "MethodTable {";
  if (methodTable.size() == 0) {
    std::cout << "}";
    return;
  }
  std::cout << std::endl;
  for (MethodTable::iterator it = methodTable.begin(); it != methodTable.end(); it++) {
    std::cout << genIndent(indent + 2) << it->first << " -> {" << std::endl;
    std::cout << genIndent(indent + 4) << string(it->second.returnType) << "," << std::endl;
    std::cout << genIndent(indent + 4) << it->second.localsSize << "," << std::endl;
    print(*it->second.variables, indent + 4);
    std::cout <<std::endl;
    std::cout << genIndent(indent + 2) << "}";
    if (it != --methodTable.end())
      std::cout << ",";
    std::cout << std::endl;
  }
  std::cout << genIndent(indent) << "}";
}

void print(ClassTable classTable, int indent) {
  std::cout << genIndent(indent) << "ClassTable {" << std::endl;
  for (ClassTable::iterator it = classTable.begin(); it != classTable.end(); it++) {
    std::cout << genIndent(indent + 2) << it->first << " -> {" << std::endl;
    if (it->second.superClassName != "")
      std::cout << genIndent(indent + 4) << it->second.superClassName << "," << std::endl;
    print(*it->second.members, indent + 4);
    std::cout << "," << std::endl;
    print(*it->second.methods, indent + 4);
    std::cout <<std::endl;
    std::cout << genIndent(indent + 2) << "}";
    if (it != --classTable.end())
      std::cout << ",";
    std::cout << std::endl;
  }
  std::cout << genIndent(indent) << "}" << std::endl;
}

void print(ClassTable classTable) {
  print(classTable, 0);
}