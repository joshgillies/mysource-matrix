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
* $Id: ExpandingNode.java,v 1.6 2012/08/30 01:09:21 ewang Exp $
*
*/

package net.squiz.matrix.matrixtree;

import net.squiz.matrix.core.*;

public class ExpandingNode extends MatrixTreeNode {

	private int parentTotalAssets,currentAssetsCount,viewedAssetsCount = 0;
	private int clicks = 0;
	private int names = 4;
	private int initStrWidth = 0;
	private int firstNameLength = 0;
	private String cueModeName = "";

	public ExpandingNode() {
		super(null, "", 1, "", "", "", 0);
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

	private void setfirstNameLength(int length) {
		firstNameLength = length;
	}

	private int getFirstNameLength() {
		return firstNameLength;
	}

	public void setClicks(int clicks) {
		this.clicks = clicks;
	}

	public int getClicks() {
		return clicks;
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

		if (getClicks() == 0 || names < getClicks()) {
			setClicks(1);
		}

		String name = "";
		if (getClicks() == 1) {
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
			setfirstNameLength(name.length());
		} if (getClicks() == 2) {
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
		} else if (getClicks() == 3) {
			String totalStr = "N/A";
			if (total > 0) {
				totalStr = ""+total;
			}
			Object[] transArgs = {
						totalStr
					};
			name = Matrix.translate("asset_map_expanding_node_three",transArgs);
		} else if (getClicks() == 4) {
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
		if (getFirstNameLength() > name.length()) {
			int diff = getFirstNameLength() - name.length();
			for (int i=0; i< diff+4;i++) {
				name += " ";
			}
		}

		return name;
	}


	public void setCueModeName(String name) {
		cueModeName = name;
	}

	public String getCueModeName() {
		return cueModeName;
	}

	public String getAssetName() {
		return ((ExpandingAsset)getUserObject()).getName();
	}

	public boolean usingCueModeName() {
		return getAssetName().equals(getCueModeName());
	}

	public void useCueModeName() {
		setName(getCueModeName());
	}

	public void setName(String name) {
		((ExpandingAsset)getUserObject()).setName(name);
	}

	public void switchName() {
		setClicks(0);
		setName(getName());
	}

	public void switchName(int evtX, double boundsX) {
		int res = evtX - (int)boundsX;
		if (res > 35) {
			setClicks(getClicks()+1);
			setName(getName());
		}
	}

	public class ExpandingAsset extends Asset {

		String name;
		public ExpandingAsset(String assetName) {
			super("0");
			name = assetName;
		}

		public String getName() {
			return name;
		}

		public void setName(String name) {
			this.name = name;
		}
	}
}
