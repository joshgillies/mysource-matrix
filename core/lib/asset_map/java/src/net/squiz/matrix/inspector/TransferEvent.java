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
* $Id: TransferEvent.java,v 1.2 2006/12/05 05:26:36 bcaldwell Exp $
*
*/

package net.squiz.matrix.inspector;

import java.util.EventObject;
import java.awt.Point;
import net.squiz.matrix.matrixtree.*;

/**
 * TransferEvent is used to notify interested parties that a copy
 * or move operation has occured
 *
 * @author Nathan de Vries <ndvries@squiz.net>
 */
public class TransferEvent extends EventObject {

	private int dragIndex;
	private int dropIndex;
	private MatrixTreeNode node;
	private MatrixTreeNode dropParent;

	/**
	 * Constructs a TransferEvent object.
	 *
	 * @param dragIndex  index of the node which was clicked to start the drag
	 * @param dropIndex  index of the node which was dropped onto
	 * @param node  the node that was dragged
	 * @param dropParent  the parent of the node at dropIndex
	 */
	public TransferEvent(	Object source,
							int dragIndex,
							int dropIndex,
							MatrixTreeNode node,
							MatrixTreeNode dropParent) {
			super(source);
			this.dragIndex = dragIndex;
			this.dropIndex = dropIndex;
			this.node = node;
			this.dropParent = dropParent;
	}

	/**
	 * Returns the index of the node that was dragged
	 *
	 * @return the dragged node's index
	 */
	public int getDragIndex() {
		return dragIndex;
	}

	/**
	 * Returns the index of the node that the drop operation took place over.
	 *
	 * @return the dropped node's index
	 */
	public int getDropIndex() {
		return dropIndex;
	}

	/**
	 * Returns the MatrixTreeNode that was dragged
	 *
	 * @return the dragged node
	 */
	public MatrixTreeNode getNode() {
		return node;
	}

	/**
	 * Returns the parent of the node at dropIndex
	 *
	 * @return the dropped node's parent
	 */
	public MatrixTreeNode getDropParent() {
		return dropParent;
	}
}
