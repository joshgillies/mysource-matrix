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
* $Id: SelectionTool.java,v 1.4 2006/12/05 05:26:37 bcaldwell Exp $
*
*/

package net.squiz.matrix.ui;

import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.tree.*;
import javax.swing.table.*;
import java.util.*;
import javax.swing.plaf.*;
import net.squiz.matrix.inspector.*;

/**
 * @author Marc McIntyre <mmcintyre@squiz.net>
 */
public class SelectionTool extends MouseAdapter implements MouseMotionListener {

	private Component comp;
	private SelectionHandler selHandler;

	/** The point where the drag originated */
	private Point initDragPoint = null;
	/** the current mouse x position */
	private int mouseX;
	/** the current mouse y position */
	private int mouseY;
	/** the bounds of the selection tool */
	private Rectangle selectionBounds = new Rectangle(0, 0, 0, 0);
	/** A list of TreePaths of the nodes in the current selection */
	private java.util.List currSelection = new ArrayList();

	/* TRUE if we are dragging the selection tool */
	private boolean dragging = false;
	/* a list of nodes to be removed from the selection */
	private java.util.List removeSelections = new ArrayList();
	/* a list of nodes to be added to the selection */
	private java.util.List addSelections = new ArrayList();

	public SelectionTool(JTree comp) {
		this.comp = comp;
		selHandler = new TreeSelection(comp);
	}

	public SelectionTool(JTable comp) {
		this.comp = comp;
		selHandler = new TableSelection(comp);
	}

	public SelectionTool(SelectionHandler handler, Component comp) {
		selHandler = handler;
		this.comp = comp;
	}

	public Point getInitialDragPoint() {
		return initDragPoint;
	}

	/**
	 * Returns true if the selection tool is currently dragging
	 * @return TRUE if the selection tool is currently dragging
	 */
	public boolean isDragging() {
		return dragging;
	}

	/**
	 * Event listener method that is called when the mouse is moved
	 * @param evt the MouseEvent
	 */
	public void mouseMoved(MouseEvent evt) {}

	/**
	 * Event listener method that is called when the mouse is released
	 * @param evt the MouseEvent
	 */
	public void mouseReleased(MouseEvent evt) {
		dragging = false;
		initDragPoint = null;
		currSelection.clear();
		removeSelections.clear();
		addSelections.clear();
		comp.repaint();
	}

	/**
	 * Event Listener method that is called when the mouse is dragged
	 * @param evt the MouseEvent
	 */
	public void mouseDragged(MouseEvent evt) {
		mouseX = evt.getX();
		mouseY = evt.getY();

		if (initDragPoint == null) {
			if (!selHandler.canSelect(evt.getPoint()))
				return;

			initDragPoint = evt.getPoint();
			dragging = true;
			// remove any nodes that are currently selected
			selHandler.clearSelection();
		}
		selHandler.updateSelection(evt.getPoint());
		comp.repaint();
	}

	/**
	 * Paints the rectangular tool to indicate what nodes will be in
	 * the current selection. This method calls drawSelectionImage to
	 * do the actual painting
	 * @param g2d the graphics object to paint to
	 */
	public void paintSelectionTool(Graphics2D g2d) {

		if (initDragPoint == null)
			return;

		int x = 0, y = 0, width = 0, height = 0;

		// this is the best way to visualise this:
		// the slashes point up to the way the rectangle should be drawn
		// where the x and y co-ordinates are the first slashes origin
		// in each quadrant

		/* width -->  | width -->   |
		----------------------------- -
		|          /  |         /   | ^
		|      /      |     /       | | height
		|  / x,y      | / x,y       |
		|---------------------------| * initDragPoint.y
		|         /   |        /    |
		|      /      |     /       | ^
		|  / x,y      | / x,y       | | height
		|---------------------------|
					  * initDragPoint.x
		*/

		if (initDragPoint.x < mouseX) {
			x     = initDragPoint.x;
			width = mouseX - initDragPoint.x;
		} else {
			x     = mouseX;
			width = initDragPoint.x -mouseX;
		}

		if (initDragPoint.y > mouseY) {
			y      = mouseY;
			height = initDragPoint.y - mouseY;
		} else {
			y      = initDragPoint.y;
			height = mouseY - initDragPoint.y;
		}
		drawSelectionImage(g2d, x, y, width, height);
		selectionBounds.setBounds(x, y, width, height);
	}

	/**
	 * Paints the selection image
	 * @param g2d the graphics set to draw to
	 * @param x the x co-ordinate to start drawing in
	 * @param y the x co-ordinate to start drawing in
	 * @param width the width of the selection tool
	 * @param height the height of the selection tool
	 */
	protected void drawSelectionImage(
		Graphics2D g2d,
		int x,
		int y,
		int width,
		int height) {
			g2d.setColor(UIManager.getColor("SelectionTool.background"));
			g2d.setComposite(AlphaComposite.getInstance(AlphaComposite.SRC_OVER, 0.3f));
			g2d.fillRect(x, y, width, height);
			g2d.setColor(UIManager.getColor("SelectionTool.bordercolor"));
			g2d.drawRect(x, y, width - 1, height - 1);
	}

	class TreeSelection implements SelectionHandler {

		private JTree tree;

		public TreeSelection(JTree tree) {
			this.tree = tree;
		}

		/**
		 * Returns an array of TreePaths for the specifed List of TreePaths
		 * @param paths the List of TreePaths
		 * @return the array of TreePaths
		 */
		private TreePath[] pathsToArray(java.util.List paths) {
			return (TreePath[]) paths.toArray(new TreePath[paths.size()]);
		}

		public boolean canSelect(Point point) {
			// check to see if the point if within the bounds of a node.
			// if it is, we don't want to start a selection operation, because
			// we want to let the operation cascade down to the drag and drop
			// methods
			if (tree.getPathForLocation(point.x, point.y) != null)
				return false;
			return true;
		}

		public void clearSelection() {
			tree.clearSelection();
		}

		public void updateSelection(Point point) {
			TreePath mousePath = tree.getClosestPathForLocation(
			initDragPoint.x, initDragPoint.y);
			TreePath initPath = tree.getClosestPathForLocation(point.x, point.y);

			// get the rows between the initial drag point and the current mouse
			// point a check each of the tree path's bounds to see if it
			// intersects the rectangle drawing tool

			TreePath[] rows = getPathBetweenRows(
				tree.getRowForPath(mousePath), tree.getRowForPath(initPath));

			currSelection.clear();

			for (int i = 0; i < rows.length; i++) {
				if (rows[i] == null)
					continue;

					if (selectionBounds.intersects(tree.getPathBounds(rows[i]))) {
						currSelection.add(rows[i]);
					}
			}
			if (currSelection != null && !currSelection.isEmpty())
				intersectSelection(currSelection);
			// see the paintComponent method of this class to see how
			// the selection tool is drawn
		}

		private TreePath[] getPathBetweenRows(int index0, int index1) {
			int newMinIndex, newMaxIndex;
			TreeUI ui = tree.getUI();

			newMinIndex = Math.min(index0, index1);
			newMaxIndex = Math.max(index0, index1);

			if(tree != null) {
				TreePath[] selection = new TreePath[newMaxIndex - newMinIndex + 1];

				for (int counter = newMinIndex; counter <= newMaxIndex; counter++) {
					selection[counter - newMinIndex] = ui.getPathForRow(tree, counter);
				}

				return selection;
			}

			return null;
		}

		/**
		 * Intersects the current selection with the new specified selection
		 * and adds any treepaths that are not in the current selection to it,
		 * and removes any treepaths in the current selection that are not in
		 * the new specified selection.
		 *
		 * @param selPaths the new selection treepaths
		 */
		protected void intersectSelection(java.util.List selPaths) {
			TreePath[] currSelPaths = tree.getSelectionPaths();

			// if there are currently no paths selected, then just selected
			// the specified selection paths
			if (currSelPaths == null) {
				tree.addSelectionPaths(pathsToArray(selPaths));
				return;
			}

			addSelections.clear();
			removeSelections.clear();
			boolean found = false;
			TreePath[] selPathsArr = pathsToArray(selPaths);

			for (int i = 0; i < selPathsArr.length; i++) {
				found = false;
				for (int j = 0; j < currSelPaths.length; j++) {
					// check to see if new selection path is allready selection
					if (selPathsArr[i] == currSelPaths[j]) {
						found = true;
						break;
					}
				}
				if (!found)
					addSelections.add(selPathsArr[i]);
			}
			for (int i = 0; i < currSelPaths.length; i++) {
				found = false;
				for (int j = 0; j < selPathsArr.length; j++) {
					// check to see if old selection
					// is not in the new selection
					if (currSelPaths[i] == selPathsArr[j]) {
						found = true;
						break;
					}
				}
				if (!found)
					removeSelections.add(currSelPaths[i]);
			}
			// add any new selections that are not currently selected
			if (!addSelections.isEmpty())
				tree.addSelectionPaths(pathsToArray(addSelections));
			// remove any new selections that are current selected
			if (!removeSelections.isEmpty())
				tree.removeSelectionPaths(pathsToArray(removeSelections));
		}
	}

	class TableSelection implements SelectionHandler {

		private JTable table;

		public TableSelection(JTable table) {
			this.table = table;
		}

		public boolean canSelect(Point point) {
			if ( (table.rowAtPoint(point) == -1 ) || (table.columnAtPoint(point) == -1) )
				return true;
			if (table.getValueAt( table.rowAtPoint(point), table.columnAtPoint(point) ) == null)
				return true;

			// MM: need to move mouseInsideCellComponent into this
			// inner class
			InspectorGadget tempTable = (InspectorGadget) table;
			if (!tempTable.mouseInsideCellComponent(point))
				return true;
			return false;
		}

		public void updateSelection(Point point) {
			int currentRow = table.rowAtPoint(point);
			int currentColumn = table.columnAtPoint(point);
			int startRow = table.rowAtPoint(initDragPoint);
			int startColumn = table.columnAtPoint(initDragPoint);

			if (startRow == -1)
				startRow = table.getRowCount() - 1;

			table.changeSelection(currentRow,currentColumn,false,false);
			table.changeSelection(startRow,startColumn,false,true);
		}

		public void clearSelection() {
			table.clearSelection();
		}
	}
}
