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
* $Id: InitialisationEvent.java,v 1.3 2012/08/30 01:09:20 ewang Exp $
*
*/

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
