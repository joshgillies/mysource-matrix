

package net.squiz.matrix.core;

import java.util.EventObject;
import net.squiz.matrix.matrixtree.MatrixTreeNode;

public class InitialisationEvent extends EventObject {

	private MatrixTreeNode root;
	
	public InitialisationEvent(Object source, MatrixTreeNode root) {
		super(source);
		this.root = root;
	}
	
	public MatrixTreeNode getRootNode() {
		return root;
	}
}
