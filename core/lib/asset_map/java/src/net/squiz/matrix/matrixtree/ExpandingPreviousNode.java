
package net.squiz.matrix.matrixtree;

import net.squiz.matrix.core.*;

public class ExpandingPreviousNode extends ExpandingNode {

	public ExpandingPreviousNode(int parentTotalAssets, int currentAssetsCount, int viewedAssetsCount) {
		setParentTotalAssets(parentTotalAssets);
		setCurrentAssetsCount(currentAssetsCount);
		setViewedAssetsCount(viewedAssetsCount);

		int to = (getCurrentLoc()+AssetManager.getLimit());
		if (to > parentTotalAssets) {
			to = parentTotalAssets;
		}
		Asset expandingAsset = new ExpandingAsset(getName());
		setUserObject(expandingAsset);
		setCueModeName(Matrix.translate("asset_map_expanding_node_move_to_previous_set"));
	}

	public int getStartLoc(int evtX, double boundsX) {
		int res = evtX - (int)boundsX;
		if (res >=0 && res <=12) {
			// first img clicked, we will get previous set of nodes
			if (getViewedAssetsCount() <= AssetManager.getLimit()) {
				return 0;
			} else {
				return getViewedAssetsCount() - AssetManager.getLimit();
			}
		} else if (res >=13 && res <= 35) {
			// second img clicked, get the first set of nodes
			return 0;
		} else {
			return -1;
		}
	}

}
