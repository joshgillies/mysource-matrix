
package net.squiz.matrix.inspector;

import net.squiz.matrix.matrixtree.MatrixTreeNode;

public interface Filter {
	public void addCondition(Object condition);
	public void removeCondition(Object condition);
	public boolean allowsNode(MatrixTreeNode node);
}
