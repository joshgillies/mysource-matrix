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
* $Id: SimpleAssetMap.java,v 1.3 2006/12/05 05:26:35 bcaldwell Exp $
*
*/

package net.squiz.matrix.assetmap;

import net.squiz.matrix.ui.*;
import net.squiz.matrix.core.*;
import net.squiz.matrix.matrixtree.*;
import javax.swing.*;

/**
 * The simple mode applet class
 * @author Nathan de Vries <ndvries@squiz.net>
 */
public class SimpleAssetMap extends AssetMap {

	private javax.swing.Timer timer;

	public void start() {
		initAssetMap();
	}

	public void stop() {}

	protected JComponent createApplet() {
		return new FinderView();
	}

	public void processAssetLocator(String params) {
		// we need to create 2 arrays
		String[] info = params.split("~");
		String[] assetIds = info[0].split("\\|");
		String[] sort_orders = info[1].split("\\|");

		// use the simple version for simple asset map
		MatrixTreeBus.startSimpleAssetLocator(assetIds, sort_orders);
	}

}
