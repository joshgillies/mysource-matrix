/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: Selection.java,v 1.2 2006/12/05 05:26:37 bcaldwell Exp $
*
*/

package net.squiz.matrix.matrixtree;

import java.util.*;
import javax.swing.tree.*;

public class Selection {

	private static List nodes = new ArrayList();

	// cannot instantiate
	private Selection() {}

	public static void setNodes(MatrixTreeNode[] newNodes) {
		// dont remove the current selection if the specified nodes
		// is null; keep the current selection
		if (newNodes == null)
			return;
		removeAllNodes();
		for (int i = 0; i < newNodes.length; i++) {
			nodes.add(newNodes[i]);
		}
	}

	/**
	 * @see #setNodes(MatrixTreeNode[])
	 */
	public static MatrixTreeNode[] getNodes() {
		if (nodes != null)
			return (MatrixTreeNode[]) nodes.toArray(new MatrixTreeNode[nodes.size()]);
		return null;
	}

	/**
	 * @see #removeNodes(TreePath[])
	 * @see #removeNodes(MatrixTreeNode[])
	 * @see #removeAllNodes()
	 */
	public static void removeNode(MatrixTreeNode node) {
		nodes.remove(node);
	}

	/**
	 * @see #removeNode(MatrixTreeNode)
	 * @see #removeNodes(MatrixTreeNode[])
	 * @see #removeAllNodes()
	 */
	public static void removeNodes(TreePath[] paths) {
		for (int i = 0; i < paths.length; i++) {
			nodes.remove(paths[i].getLastPathComponent());
		}
	}

	/**
	 * @see #removeNode(MatrixTreeNode)
	 * @see #removeNodes(TreePath[])
	 * @see #removeAllNodes()
	 */
	public static void removeNodes(MatrixTreeNode[] removeNodes) {
		for (int i = 0; i < removeNodes.length; i++) {
			nodes.remove(removeNodes[i]);
		}
	}

	/**
	 * @see #removeNode(MatrixTreeNode)
	 * @see #removeNodes(TreePath[])
	 * @see #removeNode(MatrixTreeNode)
	 */
	public static void removeAllNodes() {
		nodes.clear();
	}

	public static void addNode(MatrixTreeNode node) {
		nodes.add(node);
	}
}
