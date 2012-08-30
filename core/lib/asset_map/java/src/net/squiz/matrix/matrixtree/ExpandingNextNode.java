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
* $Id: ExpandingNextNode.java,v 1.4 2012/08/30 01:09:21 ewang Exp $
*
*/

package net.squiz.matrix.matrixtree;

import net.squiz.matrix.core.*;

public class ExpandingNextNode extends ExpandingNode {

	public ExpandingNextNode(int parentTotalAssets, int currentAssetsCount, int viewedAssetsCount) {
		setParentTotalAssets(parentTotalAssets);
		setCurrentAssetsCount(currentAssetsCount);
		setViewedAssetsCount(viewedAssetsCount);
		Asset expandingAsset = new ExpandingAsset(getName());
		setUserObject(expandingAsset);
		setCueModeName(Matrix.translate("asset_map_expanding_node_move_to_next_set"));
	}

	public int getStartLoc(int evtX, double boundsX) {
		int res = evtX - (int)boundsX;
		if (res >=0 && res <=12) {
			// first img clicked, we will get next set of nodes
			return getViewedAssetsCount() + AssetManager.getLimit();
		} else if (res >=13 && res <=35) {
			// second img clicked, get the last set of nodes
			if (getParentTotalAssets() < 0) {
				return -1;
			}
			int loc = getParentTotalAssets() - (getParentTotalAssets()%AssetManager.getLimit());
			if (loc == getParentTotalAssets())
				loc -= AssetManager.getLimit();
			return loc;
		} else {
			return -1;
		}
	}

}
