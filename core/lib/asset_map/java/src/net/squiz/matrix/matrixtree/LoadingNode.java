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
* $Id: LoadingNode.java,v 1.6 2006/12/05 05:26:36 bcaldwell Exp $
*
*/

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
