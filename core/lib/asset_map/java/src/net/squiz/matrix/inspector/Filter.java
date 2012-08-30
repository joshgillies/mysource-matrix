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
* $Id: Filter.java,v 1.3 2012/08/30 01:09:20 ewang Exp $
*
*/

package net.squiz.matrix.inspector;

import net.squiz.matrix.matrixtree.MatrixTreeNode;

public interface Filter {
	public void addCondition(Object condition);
	public void removeCondition(Object condition);
	public boolean allowsNode(MatrixTreeNode node);
}
