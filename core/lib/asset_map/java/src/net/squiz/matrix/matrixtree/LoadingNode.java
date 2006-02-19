
package net.squiz.matrix.matrixtree;

import net.squiz.matrix.core.*;

public class LoadingNode extends MatrixTreeNode {

	public LoadingNode() {
		super(null, "", 1, "", "", "", 0);
		Asset loadingAsset = new LoadingAsset();
		setUserObject(loadingAsset);
	}

	private class LoadingAsset extends Asset {
		public LoadingAsset() {
			super("0");
			name = Matrix.translate("asset_map_loading_node");
		}
	}
}
