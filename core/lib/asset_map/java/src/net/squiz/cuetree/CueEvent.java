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
* $Id: CueEvent.java,v 1.6 2012/08/30 01:09:20 ewang Exp $
*
*/

package net.squiz.cuetree;

import java.util.EventObject;
import javax.swing.tree.TreePath;
import java.awt.Point;

/**
 * CueEvent is used to notify interested parties that the
 * cue has started or completed a request for a move or add for
 * a tree node.
 *
 * @author Marc McIntyre <mmcintyre@squiz.net>
 */
public class CueEvent extends EventObject {

	private TreePath parentPath;
	private TreePath[] sourcePaths;
	private int index, prevIndex;
	private Point point;

	/**
	 * Constructs a CueEvent object.
	 *
	 * @param source the source tree where the event was fired
	 * @param sourcePath the path where the event began
	 * @param parentPath the parent path where the event began
	 * @param index the index within the parent where the event begain
	 * @param p the point where the event began
	 */
	public CueEvent(
		Object source,
		TreePath[] sourcePaths,
		TreePath parentPath,
		int index,
		Point point) {
			super(source);
			this.parentPath = parentPath;
			this.sourcePaths = sourcePaths;
			this.index = index;
			this.point = point;
	}

	public CueEvent(
		Object source,
		TreePath[] sourcePaths,
		TreePath parentPath,
		int index,
		int prevIndex,
		Point point) {
			super(source);
			this.parentPath = parentPath;
			this.sourcePaths = sourcePaths;
			this.index = index;
			this.prevIndex = prevIndex;
			this.point = point;
	}

	/**
	 * Returns the source path where the event occured
	 * @return the source path where the event occured
	 */
	public TreePath[] getSourcePaths() {
		return sourcePaths;
	}

	public TreePath getSourcePath() {
		return sourcePaths[0];
	}

	/**
	 * Returns the index where the event occured
	 * @return the index where the event occured
	 */
	public int getIndex() {
		return index;
	}

	/**
	 * Returns the prev index where the event occured
	 * @return the prev index where the event occured
	 */
	public int getPrevIndex() {
		return prevIndex;
	}

	/**
	 * Returns the source path where the event occured
	 * @return the source path where the event occured
	 */
	public TreePath getParentPath() {
		return parentPath;
	}

	/**
	 * Returns the point where the event occured
	 * @return the point where the event occured
	 */
	public Point getPoint() {
		return point;
	}

	/**
	 * Returns the x co-ordinate where the event occured
	 * @return the x co-ordinate where the event occured
	 */
	public int getX() {
		return point.x;
	}

	/**
	 * Returns the y co-ordinate where the event occured
	 * @return the y co-ordinate where the event occured
	 */
	public int getY() {
		return point.y;
	}
}
