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
* $Id: CueGestureListener.java,v 1.2 2005/05/13 02:14:58 ndvries Exp $
* $Name: not supported by cvs2svn $
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
