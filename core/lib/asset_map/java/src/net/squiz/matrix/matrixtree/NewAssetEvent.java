/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: NewAssetEvent.java,v 1.3 2012/08/30 01:09:21 ewang Exp $
*
*/

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
