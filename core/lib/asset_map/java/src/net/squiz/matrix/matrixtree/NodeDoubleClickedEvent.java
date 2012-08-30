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
* $Id: NodeDoubleClickedEvent.java,v 1.3 2012/08/30 01:09:21 ewang Exp $
*
*/

package net.squiz.matrix.matrixtree;

import java.util.EventObject;
import javax.swing.tree.TreePath;
import java.awt.Point;

/**
 * CueEvent is used to notify interested parties that the
 * cue has started or completed a request for a move or add for
 * a tree node.
 *
 * @author Nathan de Vries <ndvries@squiz.net>
 */
public class NodeDoubleClickedEvent extends EventObject {

	private TreePath clickedPath;
	private Point point;

	/**
	 * Constructs a NodeDoubleClickedEvent object.
	 *
	 * @param clickedPath  the path where the event began
	 * @param point  the point where the event began
	 */
	public NodeDoubleClickedEvent(
		Object source,
		TreePath clickedPath,
		Point point) {
			super(source);
			this.clickedPath = clickedPath;
			this.point = point;
	}

	/**
	 * Returns the path where the event occured
	 *
	 * @return the path where the event occured
	 */
	public TreePath getClickedPath() {
		return clickedPath;
	}

	/**
	 * Returns the point where the event occured
	 *
	 * @return the point where the event occured
	 */
	public Point getPoint() {
		return point;
	}

	/**
	 * Returns the x co-ordinate where the event occured
	 *
	 * @return the x co-ordinate where the event occured
	 */
	public int getX() {
		return point.x;
	}

	/**
	 * Returns the y co-ordinate where the event occured
	 *
	 * @return the y co-ordinate where the event occured
	 */
	public int getY() {
		return point.y;
	}
}
