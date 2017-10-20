#include "sum.hpp"

void SumVisitor::visitNode(Node* node) {
	this->sum += node->value;
	node->visit_children(this);
}
