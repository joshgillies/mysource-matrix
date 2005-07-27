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
* $Id: CueEvent.java,v 1.3 2005/07/27 10:45:22 brobertson Exp $
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
	private int index;
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
