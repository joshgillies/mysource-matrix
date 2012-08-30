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
* $Id: TransferListener.java,v 1.3 2012/08/30 01:09:21 ewang Exp $
*
*/

package net.squiz.matrix.inspector;

import java.util.EventListener;

/**
 * TransferListener defines the interface for an object that listens to
 * copy or move events in an associated JTable.
 *
 * @author Nathan de Vries <ndvries@squiz.net>
 */
public interface TransferListener extends EventListener {

	/**
	 * A transfer gesture has been recognised, and the listening class is
	 * notifying this listener in order for it to initiate the action for
	 * the user
	 */
	public void transferGestureRecognized(TransferEvent e);
}
