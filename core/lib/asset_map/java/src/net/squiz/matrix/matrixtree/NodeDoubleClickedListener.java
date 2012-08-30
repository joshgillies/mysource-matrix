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
* $Id: NodeDoubleClickedListener.java,v 1.3 2012/08/30 01:09:21 ewang Exp $
*
*/

package net.squiz.matrix.matrixtree;

import java.util.EventListener;

/**
 * CueListener defines the interface for an object that listens to
 * double clicks on nodes in associated tree MatrixTree.
 *
 * @author Nathan de Vries <ndvries@squiz.net>
 */
public interface NodeDoubleClickedListener extends EventListener {

	/**
	 * Tells listeners that a node has been double clicked.
	 */
	public void nodeDoubleClicked(NodeDoubleClickedEvent e);
}
