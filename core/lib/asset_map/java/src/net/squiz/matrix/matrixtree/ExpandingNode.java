
package net.squiz.matrix.matrixtree;

import net.squiz.matrix.core.*;

public class ExpandingNode extends MatrixTreeNode {

	private int parentTotalAssets,currentAssetsCount,viewedAssetsCount = 0;
	private int clicks = 0;
	private int names = 4;
	private int initStrWidth = 0;

	public ExpandingNode() {
		super(null, "", 1, "", "");
	}

	public void setParentTotalAssets(int parentTotalAssets) {
		this.parentTotalAssets = parentTotalAssets;
	}

	public void setCurrentAssetsCount(int currentAssetsCount) {
		this.currentAssetsCount = currentAssetsCount;
	}

	public void setViewedAssetsCount(int viewedAssetsCount) {
		this.viewedAssetsCount = viewedAssetsCount;
	}

	public int getParentTotalAssets() {
		return parentTotalAssets;
	}

	public int getCurrentAssetsCount() {
		return currentAssetsCount;
	}

	public int getViewedAssetsCount() {
		return viewedAssetsCount;
	}


	public int getCurrentLoc() {
		int currentLoc = 0;
		currentLoc = viewedAssetsCount;

		return currentLoc;
	}

	public int getInitStrWidth() {
		return initStrWidth;
	}

	public void setInitStrWidth(int width) {
		if (width > initStrWidth) {
			initStrWidth = width;
		}
	}

	public String getName() {
		int to = getCurrentLoc()+AssetManager.getLimit();
		if ((to > getParentTotalAssets()) && getParentTotalAssets() > 0) {
			to = getParentTotalAssets();
		}
		int total = 0;
		if (getParentTotalAssets() > 0) {
			total = getParentTotalAssets();
		}

		if (clicks == 0 || names < clicks) {
			clicks = 1;
		}

		String name = "";
		if (clicks == 1) {
			String totalStr = "N/A";
			String fromStr = ""+(getCurrentLoc()+1);
			String toStr = ""+to;
			if (total > 0) {
				totalStr = ""+total;
			}

			Object[] transArgs = {
						fromStr,
						toStr,
						totalStr
					};

			name = Matrix.translate("asset_map_expanding_node_one",transArgs);
		} if (clicks == 2) {
			String fromStr = ""+((int)((getCurrentLoc()+1)/AssetManager.getLimit())+1);
			String toStr = "N/A";
			if (total > 0) {
				toStr = ""+((int)(total/AssetManager.getLimit())+1);
			}
			Object[] transArgs = {
						fromStr,
						toStr,
					};
			name = Matrix.translate("asset_map_expanding_node_two",transArgs);
		} else if (clicks == 3) {
			String totalStr = "N/A";
			if (total > 0) {
				totalStr = ""+total;
			}
			Object[] transArgs = {
						totalStr
					};
			name = Matrix.translate("asset_map_expanding_node_three",transArgs);
		} else if (clicks == 4) {
			int remaining = (total-getCurrentLoc()-AssetManager.getLimit());
			String remainingStr = "0";
			if (remaining > 0) {
				remainingStr = ""+remaining;
			}
			Object[] transArgs = {
						remainingStr
					};
			name = Matrix.translate("asset_map_expanding_node_four",transArgs);
		}
		return name;
	}

	public void switchName(int evtX, double boundsX) {
		int res = evtX - (int)boundsX;
		if (res > 35) {
			clicks++;
			((ExpandingAsset)getUserObject()).setName(getName());
		}
	}

	public class ExpandingAsset extends Asset {

		public ExpandingAsset(String assetName) {
			super("0");
			name = assetName;
		}
	}
}
