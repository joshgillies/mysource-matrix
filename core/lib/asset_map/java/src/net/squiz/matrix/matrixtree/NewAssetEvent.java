
package net.squiz.matrix.matrixtree;

import net.squiz.matrix.core.*;
import java.util.EventObject;
import javax.swing.tree.TreeNode;

public class NewAssetEvent extends EventObject {

	private String typeCode;
	private MatrixTreeNode parent;
	private int index;

	public NewAssetEvent(Object source,
		String typeCode,
		MatrixTreeNode parent,
		int index) {
			super(source);
			this.typeCode = typeCode;
			this.parent = parent;
			this.index = index;
	}

	public String getTypeCode() {
		return typeCode;
	}

	public MatrixTreeNode getParentNode() {
		return parent;
	}

	public int getIndex() {
		return index;
	}
}
