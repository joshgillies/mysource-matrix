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
* $Id: TransferListener.java,v 1.2 2006/12/05 05:26:36 bcaldwell Exp $
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
