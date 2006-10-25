/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: SimpleAssetMap.java,v 1.2 2006/10/25 00:55:55 rong Exp $
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
