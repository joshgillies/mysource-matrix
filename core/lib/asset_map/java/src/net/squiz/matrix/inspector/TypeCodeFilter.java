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
* $Id: TypeCodeFilter.java,v 1.2 2006/12/05 05:26:36 bcaldwell Exp $
*
*/

package net.squiz.matrix.inspector;

import net.squiz.matrix.matrixtree.MatrixTreeNode;
import java.util.*;

public class TypeCodeFilter implements Filter {

	private List conditions;

	public TypeCodeFilter() {
		conditions = new ArrayList();
	}

	public TypeCodeFilter(Object condition) {
		conditions = new ArrayList();
		addCondition(condition);
	}

	public void addCondition(Object condition) {
		if (!conditions.contains(condition))
			conditions.add(condition);
	}

	public void removeCondition(Object condition) {
			conditions.remove(condition);
	}

	public boolean allowsNode(MatrixTreeNode node) {
		String typeCode = node.getAsset().getType().getTypeCode();
		Iterator iterator = conditions.iterator();
		while (iterator.hasNext()) {
			String filterTypeCode = (String) iterator.next();
			if (typeCode.equals(filterTypeCode)) {
				return true;
			}
		}
		return false;
	}
}
