
package net.squiz.matrix.matrixtree;

import net.squiz.matrix.assetmap.*;
import java.util.EventObject;
import javax.swing.tree.TreeNode;

public class NewLinkEvent extends EventObject {
	
	public static final String LINK_TYPE_MOVE     = "move asset";
	public static final String LINK_TYPE_NEW_LINK = "new link";
	public static final String LINK_TYPE_CLONE    = "clone";
	
	private String type;
	private MatrixTreeNode[] sourceNodes;
	private MatrixTreeNode parentNode;
	private int index, prevIndex;
	private String[] parentIds;
	
	public NewLinkEvent(
		Object source,
		String type,
		MatrixTreeNode[] sourceNodes,
		MatrixTreeNode parentNode,
		int index,
		int prevIndex,
		String[] parentIds) {
			
			super(source);
			if (type != LINK_TYPE_MOVE &&
				type != LINK_TYPE_NEW_LINK &&
				type != LINK_TYPE_CLONE) {
					throw new IllegalArgumentException("type must be one of " +
						"LINK_TYPE_MOVE or LINK_TYPE_NEW_LINK or LINK_TYPE_CLONE");
			}
			
			this.type = type;
			this.sourceNodes = sourceNodes;
			this.parentNode = parentNode;
			this.index = index;
			this.prevIndex = prevIndex;
			this.parentIds = parentIds;
	}
	
	public String getType() {
		return type;
	}
	
	public MatrixTreeNode[] getSourceNodes() {
		return sourceNodes;
	}
	
	public MatrixTreeNode getParentNode() {
		return parentNode;
	}
	
	public int getIndex() {
		return index;
	}

	public int getPrevIndex() {
		return prevIndex;
	}

	public String[] getParentIds() {
		return parentIds;
	}
}
