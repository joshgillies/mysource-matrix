
package net.squiz.matrix.matrixtree;

import java.util.EventListener;

public interface NewAssetListener extends EventListener {
	public void requestForNewAsset(NewAssetEvent evt);
}
