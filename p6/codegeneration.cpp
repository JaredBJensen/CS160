#include "codegeneration.hpp"

// CodeGenerator Visitor Functions: These are the functions
// you will complete to generate the x86 assembly code. Not
// all functions must have code, many may be left empty.

void CodeGenerator::visitProgramNode(ProgramNode* node) {
    std::cout << ".data" << std::endl;
    std::cout << "printstr: .asciz \"%d\\n\"" << std::endl;
    std::cout << ".text" << std::endl;
    std::cout << ".globl Main_main" << std::endl;
	node->visit_children(this);
}

void CodeGenerator::visitClassNode(ClassNode* node) {
    currentClassName = node->identifier_1->name;
	currentClassInfo = (*classTable)[currentClassName];
	node->visit_children(this);
}

void CodeGenerator::visitMethodNode(MethodNode* node) {
    currentMethodName = node->identifier->name;
	currentMethodInfo = (*(currentClassInfo.methods))[currentMethodName];
    std::cout << " " << currentClassName << "_" << currentMethodName << ":" << std::endl;
	node->visit_children(this);
}

void CodeGenerator::visitMethodBodyNode(MethodBodyNode* node) {
    std::cout << "push %ebp" << std::endl;
    std::cout << "mov %esp, %ebp" << std::endl;
    std::cout << "sub $" << currentMethodInfo.localsSize << ", %esp" << std::endl;
    node->visit_children(this);
    std::cout << "leave" << std::endl;
    std::cout << "ret" << std::endl;
}

void CodeGenerator::visitParameterNode(ParameterNode* node) {}

void CodeGenerator::visitDeclarationNode(DeclarationNode* node) {}

void CodeGenerator::visitReturnStatementNode(ReturnStatementNode* node) {
    node->visit_children(this);
	std::cout << "pop %eax" << std::endl;
}

void CodeGenerator::visitAssignmentNode(AssignmentNode* node) {
    
}

void CodeGenerator::visitCallNode(CallNode* node) {
    node->visit_children(this);
}

void CodeGenerator::visitIfElseNode(IfElseNode* node) {
    
}

void CodeGenerator::visitWhileNode(WhileNode* node) {
    
}

void CodeGenerator::visitPrintNode(PrintNode* node) {
    node->visit_children(this);
	std::cout << "push $printstr" << std::endl;
	std::cout << "call printf" << std::endl;
	std::cout << "add $4, %esp" << std::endl;
}

void CodeGenerator::visitDoWhileNode(DoWhileNode* node) {
    
}

void CodeGenerator::visitPlusNode(PlusNode* node) {
    node->visit_children(this);
    std::cout << "pop %edx" << std::endl;
    std::cout << "pop %eax" << std::endl;
    std::cout << "add %edx, %eax" << std::endl;
    std::cout << "push %eax" << std::endl;
}

void CodeGenerator::visitMinusNode(MinusNode* node) {
    node->visit_children(this);
    std::cout << "pop %edx" << std::endl;
    std::cout << "pop %eax" << std::endl;
    std::cout << "sub %edx, %eax" << std::endl;
    std::cout << "push %eax" << std::endl;
}

void CodeGenerator::visitTimesNode(TimesNode* node) {
    node->visit_children(this);
    std::cout << "pop %edx" << std::endl;
    std::cout << "pop %eax" << std::endl;
    std::cout << "imul %edx, %eax" << std::endl;
    std::cout << "push %eax" << std::endl;
}

void CodeGenerator::visitDivideNode(DivideNode* node) {
    node->visit_children(this);
    std::cout << "pop %edx" << std::endl;
    std::cout << "pop %eax" << std::endl;
    std::cout << "cdq" << std::endl;
    std::cout << "idiv %ebx" << std::endl;
    std::cout << "push %eax" << std::endl;
}

void CodeGenerator::visitGreaterNode(GreaterNode* node) {
    int label = nextLabel();
	node->visit_children(this);
	std::cout << "pop %edx" << std::endl;
	std::cout << "pop %eax" << std::endl;
	std::cout << "cmp %edx, %eax" << std::endl;
	std::cout << "jg greater" << label << std::endl;
	std::cout << "push $0" << std::endl;
	std::cout << "jmp end" << label << std::endl;
	std::cout << "greater" << label << std::endl;
	std::cout << "push $1" << std::endl;
	std::cout << "end" << label << std::endl;
}

void CodeGenerator::visitGreaterEqualNode(GreaterEqualNode* node) {
    int label = nextLabel();
	node->visit_children(this);
	std::cout << "pop %edx" << std::endl;
	std::cout << "pop %eax" << std::endl;
	std::cout << "cmp %edx, %eax" << std::endl;
	std::cout << "jge ge" << label << std::endl;
	std::cout << "push $0" << std::endl;
	std::cout << "jmp end" << label << std::endl;
	std::cout << "ge" << label << std::endl;
	std::cout << "push $1" << std::endl;
	std::cout << "end" << label << std::endl;
}

void CodeGenerator::visitEqualNode(EqualNode* node) {
    int label = nextLabel();
	node->visit_children(this);
	std::cout << "pop %edx" << std::endl;
	std::cout << "pop %eax" << std::endl;
	std::cout << "cmp %edx, %eax" << std::endl;
	std::cout << "je equal" << label << std::endl;
	std::cout << "push $0" << std::endl;
	std::cout << "jmp end" << label << std::endl;
	std::cout << "equal" << label << std::endl;
	std::cout << "push $1" << std::endl;
	std::cout << "end" << label << std::endl;
}

void CodeGenerator::visitAndNode(AndNode* node) {
    node->visit_children(this);
	std::cout << "pop %edx" << std::endl;
	std::cout << "pop %eax" << std::endl;
	std::cout << "and %edx, %eax" << std::endl;
	std::cout << "push %eax" << std::endl;
}

void CodeGenerator::visitOrNode(OrNode* node) {
    node->visit_children(this);
	std::cout << "pop %edx" << std::endl;
	std::cout << "pop %eax" << std::endl;
	std::cout << "or %edx, %eax" << std::endl;
	std::cout << "push %eax" << std::endl;
}

void CodeGenerator::visitNotNode(NotNode* node) {
    node->visit_children(this);
	std::cout << "pop %eax" << std::endl;
	std::cout << "xor $1, %eax" << std::endl;
	std::cout << "push %eax" << std::endl;
}

void CodeGenerator::visitNegationNode(NegationNode* node) {
    node->visit_children(this);
	std::cout << "pop %eax" << std::endl;
	std::cout << "neg %eax" << std::endl;
	std::cout << "push %eax" << std::endl;
}

void CodeGenerator::visitMethodCallNode(MethodCallNode* node) {
    
}

void CodeGenerator::visitMemberAccessNode(MemberAccessNode* node) {
    
}

void CodeGenerator::visitVariableNode(VariableNode* node) {
    
}

void CodeGenerator::visitIntegerLiteralNode(IntegerLiteralNode* node) {
    std::cout << "push $" << node->integer->value << std::endl;
}

void CodeGenerator::visitBooleanLiteralNode(BooleanLiteralNode* node) {
    std::cout << "push $" << node->integer->value << std::endl;
}

void CodeGenerator::visitNewNode(NewNode* node) {
    
}

void CodeGenerator::visitIntegerTypeNode(IntegerTypeNode* node) {}

void CodeGenerator::visitBooleanTypeNode(BooleanTypeNode* node) {}

void CodeGenerator::visitObjectTypeNode(ObjectTypeNode* node) {}

void CodeGenerator::visitNoneNode(NoneNode* node) {}

void CodeGenerator::visitIdentifierNode(IdentifierNode* node) {}

void CodeGenerator::visitIntegerNode(IntegerNode* node) {}
