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
* $Id: CueGestureListener.java,v 1.4 2006/12/05 05:28:40 bcaldwell Exp $
*
*/

package net.squiz.cuetree;

import java.util.EventListener;

/**
 * CueListener defines the interface
 * for an object that listens to requests for moving and adding of nodes
 * to a CueTree.
 *
 * @author Marc McIntyre <mmcintyre@squiz.net>
 */
public interface CueGestureListener extends EventListener {

	/**
	 * Tells listeners that a request for a node to be move has begun.
	 */
	public void moveGestureRecognized(CueEvent evt);

	/**
	 * Tells listeners that a request for a node to be move has completed.
	 * If the parent where the node was not expanded, the index is -1, and
	 * therefore is up to the implementing CueListener to determine where the
	 * node should be moved to in the tree
	 */
	public void moveGestureCompleted(CueEvent evt);

	public void multipleMoveGestureRecognized(CueEvent evt);
	public void multipleMoveGestureCompleted(CueEvent evt);


	public void multipleAddGestureRecognized(CueEvent evt);
	public void multipleAddGestureCompleted(CueEvent evt);

	/**
	 * Tells listeners that a request for a node to be added has begun.
	 */
	public void addGestureRecognized(CueEvent evt);

	/**
	 * Tells listeners that a request for a node to be added has completed.
	 * If the parent where the node was not expanded, the index is -1, and
	 * therefore is up to the implementing CueListener to determine where the
	 * node should be added to in the tree
	 */
	public void addGestureCompleted(CueEvent evt);
}
