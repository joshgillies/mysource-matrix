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
* $Id: FinderTree.java,v 1.4 2006/12/05 05:26:36 bcaldwell Exp $
*
*/

 /*
  * :tabSize=4:indentSize=4:noTabs=false:
  * :folding=explicit:collapseFolds=1:
  */

package net.squiz.matrix.matrixtree;

import net.squiz.cuetree.*;
import net.squiz.matrix.core.*;
import net.squiz.matrix.ui.*;
import net.squiz.matrix.assetmap.*;

import javax.swing.tree.*;
import javax.swing.event.*;
import javax.swing.*;

import java.io.IOException;
import java.util.*;
import java.net.*;

import java.awt.*;
import java.awt.event.*;
import java.awt.image.*;
import java.awt.geom.*;
import java.awt.dnd.*;
import java.awt.datatransfer.*;

/**
 * The FinderTree class is the tree used when in finder mode.
 * @author Nathan de Vries <ndvries@squiz.net>
 */
public class FinderTree extends MatrixTree {

	public FinderTree() {
		super();
	}

	public FinderTree(TreeModel model) {
		super(model);
	}

	protected MenuHandler getMenuHandler() {
		return new FinderMenuHandler();
	}

	protected DragHandler getDragHandler() {
		return null;
	}

	protected DropHandler getDropHandler() {
		return null;
	}

	protected CueGestureHandler getCueGestureHandler() {
		return null;
	}

	protected class FinderMenuHandler extends MenuHandler {

		private ActionListener addMenuListener;

		/**
		 * Constructs menu handler
		 * @return the menu handler
		 */
		public FinderMenuHandler() {
			addMenuListener = MatrixMenus.getMatrixTreeAddMenuListener(FinderTree.this);
		}

		/**
		 * Event listener method that is called when the mouse is clicked
		 * @param evt the MouseEvent
		 */
		public void mouseClicked(MouseEvent evt) {

			if (!GUIUtilities.isRightMouseButton(evt))
				return;

			JPopupMenu menu = null;

			if (getPathForLocation(evt.getX(), evt.getY()) == null) {
				return;
			} else {
				TreePath[] selectedPaths = getSelectionPathsForLocation(evt.getX(), evt.getY());

				if (selectedPaths.length == 1) {
					setSelectionPaths(selectedPaths);
					MatrixTreeNode node = getSelectionNode();
					if (MatrixTreeBus.typeIsRestricted(node.getAsset().getType()) && isInAssetFinderMode()) {
						menu = MatrixMenus.getUseMeMenu(node);
					}
				}
			}
			if (menu != null)
				menu.show(FinderTree.this, evt.getX(), evt.getY());
		}


	}//end class MenuHandler

}//end class FinderTree
