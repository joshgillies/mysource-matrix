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
* $Id: CueTree.java,v 1.2 2005/04/06 02:10:01 ndvries Exp $
* $Name: not supported by cvs2svn $
*/

package net.squiz.cuetree;

import javax.swing.*;
import javax.swing.tree.*;
import java.awt.*;
import java.awt.image.*;
import java.awt.event.*;
import javax.swing.plaf.*;
import javax.swing.event.*;
import javax.swing.plaf.basic.BasicTreeUI;

import net.squiz.matrix.ui.*;

/**
 * A CueTree provides a means for moving nodes within a tree. The cue tree
 * does not handle the moving operations, but mearly provides an interface for
 * moving nodes within the tree.
 * @author Marc McIntyre <mmcintyre@squiz.net>
 */
public class CueTree extends JTree {

	/** The mode that indicate that the tree supports dragging */
	public static final int DRAG_MODE = 1;
	/** The mode that indicate that the tree supports clicing to move nodes */
	public static final int CLICK_MODE = 2;
	/** Mode that indicates that the tree is adding a node */

	/** Mode that indicates that we are adding a node */
	public static final int ADD_REQUEST_MODE = 1;
	/** Mode that indicates that the tree is moving a node */
	public static final int MOVE_REQUEST_MODE = 2;
	/** the cursor shown when the mouse hovers over a node that cannot be moved */
	public static final String INVALID_CURSOR = "Invalid.32x32";
	/** A non-null void rectangle */
	public static final Rectangle VOID_RECTANGLE = new Rectangle(0, 0, 0, 0);

	/** TRUE if we are in cue mode */
	protected boolean inCueMode = false;
	/** the offset that a cue line is drawn above or below the selected path*/
	protected int cueLineOffset = 5;
	/* the dirty bounds that the cue was drawn */
	protected Rectangle dirtyCueBounds = VOID_RECTANGLE;
	/* the dirty bounds that the cue was drawn */
	protected Rectangle dirtyGhostBounds = VOID_RECTANGLE;

	private Image ghostedNode = null;
	private TreePath currentPath = null;
	private TreePath[] sourcePaths = null;
	private boolean showsGhostedNode = false;
	/* stroke used for highliting paths */
	private Stroke highlightStroke;
	/* stroke used for the cue line */
	private Stroke cueLineStroke;
	private Color cueLineColor = Color.BLACK;

	/*
	 * if TRUE, then the last path that the cue line was drawn for will
	 * be the new parent for the moving/adding node. If FALSE, then the node
	 * will be added/moved the the exact location as the cue and on the same
	 * branch as the currentPath
	 */
	private boolean lastPathWasParent = false;

	/*
	 * TRUE if the cue line is abover the top most path in the tree. used
	 * so we can move items to the top of the tree
	 */
	private boolean aboveTopPath = false;

	private boolean moveEnabled = true;
	private int requestMode = MOVE_REQUEST_MODE;
	private CueGestureHandler cueGestureHandler = null;
	private Cursor moveCursor = new Cursor(Cursor.HAND_CURSOR);
	private Cursor noMoveCursor;
	private boolean triggersPath = true;

	/**
	 * Creates a <code>CueTree</code> and adds some sample data
	 */
	public CueTree() {
		super(getDefaultTreeModel());
		init();
	}

	/**
	 * Constructs a <code>CueTree</code> and sets the tree model.
	 * @param model the tree model
	 */
	public CueTree(TreeModel model) {
		super(model);
		init();
	}

	/**
	 * inititalises the CueRequestHandler and the invalid cursor
	 */
	private void init() {
		cueGestureHandler = getCueGestureHandler();
		setInvalidCursor();
		setUI(new CueTreeUI());
		setMoveEnabled(true);
	}

	/**
	 * Sets the invaid cursor that is used when rolling over nodes
	 * that cannot be moved
	 */
	private void setInvalidCursor() {
		try {
			noMoveCursor = Cursor.getSystemCustomCursor(INVALID_CURSOR);
		} catch (Exception e) {
			System.err.println("No invalid cursor " + INVALID_CURSOR + " found");
			e.printStackTrace();
			noMoveCursor = Cursor.getDefaultCursor();
		}
	}

	/**
	 * Adds a listener for Cue Events
	 * @param cl the listener
	 * @see #removeCueGestureListener(CueGestureListener)
	 */
	public void addCueGestureListener(CueGestureListener cgl) {
		listenerList.add(CueGestureListener.class, cgl);
	}

	/**
	 * Removes a listener from Cue Events
	 * @param cl the listener
	 * @see #addCueGestureListener(CueGestureListener)
	 */
	public void removeCueGestureListener(CueGestureListener cgl) {
		listenerList.remove(CueGestureListener.class, cgl);
	}

	/**
	 * Returns the CueRequestHandler that will be used to handle cue requests
	 * @return the CueRequestHandler
	 */
	protected CueGestureHandler getCueGestureHandler() {
		return new CueGestureHandler();
	}

	/**
	 * Sets the showsGhostedNode property. If TRUE during a drag
	 * sequence the node in question is shown as a ghosted image
	 * and follows the mouse.
	 *
	 * @param value If TRUE the node will be shown as a ghosted image
	 * during a drag sequence
	 * @return void
	 * @see #getShowsGhostedNode()
	 */
	public void setShowsGhostedNode(boolean b) {
		showsGhostedNode = b;
	}

	/**
	 * Returns the ghosted node property
	 * @return the ghosted node property
	 * @see #setShowsGhostedNode(boolean)
	 */
	public boolean getShowsGhostedNode() {
		return showsGhostedNode;
	}

	/**
	 * Sets if moves are enabled on this tree. The tree has move enabled
	 * by default
	 * @param enabled if TRUE move opreations will be enabled
	 */
	public void setMoveEnabled(boolean b) {
		moveEnabled = b;
		if (b == false) {
			removeMouseListener(cueGestureHandler);
			removeMouseMotionListener(cueGestureHandler);
		} else {
			addMouseListener(cueGestureHandler);
			addMouseMotionListener(cueGestureHandler);
		}
	}

	/**
	 * Returns TRUE if move opeations are enabled.
	 * @return TRUE if move operation are enabled
	 */
	public boolean isMoveEnabled() {
		return moveEnabled;
	}

	// TODO: (MM) remove this if its obsolete
	/**
	 * Returns the sourcePath of the cue, or null if not in cue mode.
	 * @return the source path of the cue
	 */
	public TreePath getCuePath() {
		return inCueMode ? sourcePaths[0] : null;
	}

	/**
	 * Sets the stroke used for the cue line
	 * @param stroke the stroke to use for the cue line
	 * @see #getCueLineStroke()
	 * @see #setHighlightPathStroke(Stroke)
	 * @see #getHighlightPathStroke()
	 */
	public void setCueLineStroke(Stroke stroke) {
		cueLineStroke = stroke;
	}

	/**
	 * Gets the stroke used for the cue line
	 * @return the stroke to use for the cue line
	 * @see #setCueLineStroke(Stroke)
	 * @see #setHighlightPathStroke(Stroke)
	 * @see #getHighlightPathStroke()
	 */
	public Stroke getCueLineStroke() {
		return cueLineStroke;
	}

	/**
	 * Sets the color of the cue line that is drawn in move operations.
	 * @param color the color of the cue line
	 * @see #getCueLineColor()
	 */
	public void setCueLineColor(Color color) {
		cueLineColor = color;
	}

	/**
	 * Returns the color of the cue line.
	 * @return the color of the cue line
	 * @see #setCueLineColor(Color)
	 */
	public Color getCueLineColor() {
		return cueLineColor;
	}

	/**
	 * Sets the highlight path stroke that is used to indicate that a move
	 * will move the node under the highlighted parent
	 * @param stroke the stroke to use for the cue line
	 * @see #setCueLineStroke(Stroke)
	 * @see #getCueLineStroke()
	 * @see #getHighlightPathStroke()
	 */
	public void setHighlightPathStroke(Stroke stroke) {
		highlightStroke = stroke;
	}

	/**
	 * Gets the highlight path stroke that is used to indicate that a move
	 * will move the node under the highlighted parent
	 * @return the stroke to use for the cue line
	 * @see #setCueLineStroke(Stroke)
	 * @see #getCueLineStroke()
	 * @see #setHighlightPathStroke(Stroke)
	 */
	public Stroke getHighlightPathStroke() {
		return highlightStroke;
	}

	/**
	 * Sets the cursor to indicate that a move operation will occur
	 * @param cursor the cursor
	 * @see setNoMoveCursor(cursor)
	 */
	public void setMoveCursor(Cursor cursor) {
		moveCursor = cursor;
	}

	/**
	 * Sets the cursor to indicate that this node cannot be moved
	 * @param cursor the cursor
	 * @see setMoveCursor(cursor)
	 */
	public void setNoMoveCursor(Cursor cursor) {
		noMoveCursor = cursor;
	}

	/**
	 * Sets offset a which the cue line is drawn above/below the mouse pointer
	 * The default is 5. A negative integer will draw the cue line below the mouse
	 * pointer.
	 *
	 * @param offset the offset at which to draw the cue line from the mouse pointer
	 */
	public void setCueLineOffset(int offset) {
		cueLineOffset = offset;
	}

	/**
	 * Returns the renderable component for the specified path
	 * @param path the <code>TreePath</code> to the wanted renderable component
	 * @return the renderable component for the specified path
	 */
	protected Component getComponentForPath(TreePath path) {
		if (path == null)
			throw new IllegalArgumentException("path is null");
		TreeCellRenderer renderer = getCellRenderer();
		Object node = path.getLastPathComponent();

		TreeUI ui = getUI();
		if (ui != null) {
			int row = ui.getRowForPath(this, path);
			int lsr = getLeadSelectionRow();
			boolean hasFocus = isFocusOwner()
				&& (lsr == row);

			return renderer.getTreeCellRendererComponent(
				this,
				node,
				isPathSelected(path),
				isExpanded(path),
				getModel().isLeaf(node),
				row,
				hasFocus
			);
		}
		return null;
	}

	/**
	 * Returns a union of the path bounds of all the paths in the specified
	 * TreePath array
	 * @param paths the paths of the wanted bounds
	 * @return the union of the bounds of the specified TreePaths
	 * @see JTree#getPathBounds(TreePath)
	 */
	public Rectangle getPathBounds(TreePath[] paths) {
		if (paths.length == 1)
			return getPathBounds(paths[0]);

		Rectangle bounds = new Rectangle();
		// create a rectangle that is the union of all the specified paths
		for (int i = 0; i < paths.length; i++) {
			if (paths[i] != null)
				bounds.add(getPathBounds(paths[i]));
		}
		return bounds;
	}

	/**
	 * Returns the Image for the component at the specified TreePath
	 * @param path the TreePath for the wanted component Image
	 * @return the image representation of the component at the give TreePath
	 * @see #getGhostedNode(TreePath[])
	 */
	public Image getGhostedNode(TreePath path) {
		Component c = getComponentForPath(path);
		if (c == null)
			throw new NullPointerException("Cannot create Ghosted Node: " +
				" component is null");

		Rectangle bounds = getPathBounds(path);
		// need to set the size of the component
		c.setSize(bounds.width, bounds.height);

		BufferedImage image = new BufferedImage(
			bounds.width,
			bounds.height,
			BufferedImage.TYPE_INT_ARGB_PRE
		);

		Graphics2D g2d = (Graphics2D) image.createGraphics();
		g2d.setComposite(AlphaComposite.getInstance(AlphaComposite.SRC, 0.3f));
		c.paint(g2d);
		g2d.dispose();

		return image;
	}

	/**
	 * Returns a ghosted image for the specfied TreePaths
	 * @param paths the paths of the wanted ghosted node
	 * @return the ghosted node of the specifed paths
	 * @see #getGhostedNode(TreePath)
	 */
	public Image getGhostedNode(TreePath[] paths) {
		Rectangle bounds = getPathBounds(paths);
		BufferedImage image = new BufferedImage(
			bounds.width,
			bounds.height,
			BufferedImage.TYPE_INT_ARGB_PRE
		);
		Graphics2D g2d = (Graphics2D) image.createGraphics();

		// paint all the paths to the image in the location
		// that they exist in the tree
		for (int i = 0; i < paths.length; i++) {
			Image pathImage = getGhostedNode(paths[i]);
			Rectangle pathBounds = getPathBounds(paths[i]);
			g2d.drawImage(pathImage, pathBounds.x, pathBounds.y, null);
		}
		g2d.dispose();

		return image;
	}

	/**
	 * Paints the ghosted node with the given TreePath at the specified location
	 * @param x the x co-ordinate
	 * @param y the y co-ordinate
	 * @param path the TreePath of the node to paint
	 */
	protected void paintGhostedNode(int x, int y, TreePath path) {
		if (ghostedNode == null)
			ghostedNode = getGhostedNode(path);
		Graphics g = getGraphics();
		g.drawImage(ghostedNode, x, y, this);
		Rectangle bounds = getPathBounds(path);
		dirtyGhostBounds.setRect(x, y, bounds.width, bounds.height);
	}

	/**
	 * Returns the bounds of the Icon that belongs to the
	 * cell of the node at the specified <code>TreePath</code>
	 * If the component returned by the cellrenderer is not an
	 * Instance of <code>JLabel</code>, null is returned
	 *
	 * @param path the <code>TreePath</code>of the wanted cell to whom
	 * the <code>Icon</code> belongs
	 * @return the Icons bounds
	 * @see nodeIconContainsPoint(Point)
	 * @see nodeIconContainsPoint(TreePath, Point)
	 */
	public Rectangle getNodeIconBounds(TreePath path) {
		if (path == null)
			throw new IllegalArgumentException("path is null");

		Component c = getComponentForPath(path);

		if (c == null)
			return null;
		if (!(c instanceof JLabel))
			return null;

		// get the current bounds of the path so we can use
		// its x and y co-ordinates
		Rectangle bounds = getPathBounds(path);
		Icon icon = ((JLabel) c).getIcon();
		if (icon == null)
			return null;

		bounds.setSize(icon.getIconWidth(), icon.getIconHeight());

		return bounds;
	}

	/**
	 * Returns TRUE if the specified point is within the specified path's
	 * icon's bounds
	 *
	 * @param path the path to the node
	 * @param point the point to check
	 * @return TRUE if the point is within the icon's bounds
	 * @see #nodeIconContainsPoint(Point)
	 * @see #getNodeIconBounds(TreePath)
	 */
	protected boolean nodeIconContainsPoint(TreePath path, Point point) {
		if (path == null)
			return false;
		Rectangle bounds = getNodeIconBounds(path);
		if (bounds == null)
			return false;

		return (bounds.contains(point));
	}

	/**
	 * Returns true if the specified point is within the bounds of a node's icon
	 * @param point the point to check
	 * @return TRUE if the specified point is within the bouse of a node's icon
	 * @see #nodeIconContainsPoint(TreePath, Point)
	 * @see #getNodeIconBounds(TreePath)
	 */
	protected boolean nodeIconContainsPoint(Point point) {
		TreePath path = getPathForLocation(point.x, point.y);
		if (path == null)
			return false;

		return nodeIconContainsPoint(path, point);
	}

	/**
	 * initiates that a new node will be added to the tree
	 * use the specified TreePath to specify information about the new
	 * node that will be added. eg.
	 *
	 * <pre>
	 *   TreePath path = new TreePath(new TreeNode(userObject));
	 * </pre>
	 *
	 * @param path the treepath that the fireAddStarted event will use
	 * as the sourcePath
	 */
	public void initiateAddMode(TreePath[] paths) {
		if (inCueMode)
			return;
		requestMode = ADD_REQUEST_MODE;
		fireAddGestureRecognized(paths, null, -1, new Point(0, 0));
		startCueMode(paths);
	}

	/**
	 * Explicitly starts cue mode with the specified paths in the move
	 * operation.
	 * @param paths the paths in the move operation
	 * @see #stopCueMode()
	 */
	public void startCueMode(TreePath[] paths) {
		if (inCueMode)
			return;
		// set the cursor back to the default cursor,
		// as the cursor will still be a hand from clicking the icon
		setCursor(Cursor.getDefaultCursor());
		sourcePaths = paths;
		inCueMode = true;
	}

	/**
	 * Explicitly stops Cue mode operations
	 * @see #startCueMode(TreePath[])
	 */
	public void stopCueMode() {
		paintImmediately(dirtyCueBounds);
		inCueMode = false;
		ghostedNode = null;
	}

	/**
	 * Sets whether a path is triggered when the move operation has completed.
	 * This is enabled by default.
	 * @param b if TRUE the path will be triggered when the move operation has
	 * completed
	 */
	public void setTriggersPath(boolean b) {
		triggersPath = b;
	}

	/**
	 * Draws a cue line to indicate where the node will be placed. The
	 * cue line will be drawn underneath the specified <code>TreePath</code>
	 *
	 * @param path the <code>TreePath</code> where the cue line will be drawn
	 * underneath
	 * @param pathIsNewParent if TRUE the specified path is the new parent
	 * for this node.
	 */
	protected void drawCueLine(TreePath path, boolean pathIsNewParent, boolean aboveTopPath) {

		paintImmediately(dirtyCueBounds);
		Rectangle bounds = getPathBounds(path);
		Graphics2D g2d = (Graphics2D) getGraphics();

		int x1 = 0, x2 = 0, y = 0;

		if (pathIsNewParent) {
			dirtyCueBounds = highlightPath(path, UIManager.getColor("CueLine.stroke"));
			x1 = bounds.x + bounds.width;
			y  = bounds.y + (bounds.height / 2);
		} else {
			if (isExpanded(path) && getModel().isLeaf(path.getLastPathComponent())) {
				// if the path given is not the new parent then we
				// want to draw a cue line so that it's between the first
				// node on this branch and the parent node

				// get the first child on the branch so we can use its x co-ords
				Object child = getModel().getChild(path.getLastPathComponent(), 0);
				Rectangle childBounds = getPathBounds(path.pathByAddingChild(child));
				x1 = childBounds.x;
			} else {
				dirtyCueBounds = VOID_RECTANGLE;
				x1 = bounds.x;
			}
			if (!aboveTopPath)
				y = bounds.y + bounds.height;
		}

		x2 = getX() + getWidth();

		int lineWidth = 1;
		if (cueLineStroke != null) {
			g2d.setStroke(cueLineStroke);
			if (highlightStroke instanceof BasicStroke)
				lineWidth = (int) ((BasicStroke) highlightStroke).getLineWidth();
		}

		// create a union of the current bounds of the cue
		// and the line that we are about to draw
		dirtyCueBounds.add(new Rectangle(x1, y, Math.abs(x2 - x1), lineWidth));

		g2d.setColor(UIManager.getColor("CueLine.stroke"));
		g2d.drawLine(x1, y, x2, y);
	}

	/**
	 * draws a cue line for the specifed path. The cue line may depict that the
	 * new position will be a child of the path specifed based on the specified
	 * mouse Y co-ordinate.
	 *
	 * @param path the treePath to draw the cue line
	 * @param mouseY the mouse Y co-ordinate
	 */
	protected void drawCueLine(TreePath path, int mouseY) {
		drawCueLine(path, mouseY, false);
	}

	private void drawCueLine(TreePath path, int mouseY, boolean forceRedraw) {
		boolean prevPathWasParent = lastPathWasParent;

		if (mouseY < 3) {
			lastPathWasParent = false;
			aboveTopPath = true;
		} else {
			aboveTopPath = false;
			Rectangle bounds = getPathBounds(path);
			lastPathWasParent
				= !((bounds.y + bounds.height - (bounds.height / 2)) < mouseY);

			// we only want to re-paint the line if the line itself has moved
			// and we are not painting a ghosted node
			if (!showsGhostedNode && (path == currentPath && lastPathWasParent == prevPathWasParent)) {
				if (!forceRedraw)
					return;
			}
		}
		currentPath = path;
		drawCueLine(path, lastPathWasParent, aboveTopPath);
	}

	/**
	 * Highlights the specified path by drawing a rectangle around its bounds
	 * in the specified colour. The bounds of the painted rectangle are returned
	 * so the highlighted area can be erased.
	 *
	 * @param path the path of the node to be highlighted
	 * @param color the color of the highlighting rectangled
	 * @return the bounds of the painted rectangle
	 */
	protected Rectangle highlightPath(TreePath path, Color color) {
		Rectangle bounds = getPathBounds(path);
		Graphics2D g2d = (Graphics2D) getGraphics();

		if (color != null)
			g2d.setColor(color);

		int lineWidth = 1;
		if (highlightStroke != null) {
			g2d.setStroke(highlightStroke);
			if (highlightStroke instanceof BasicStroke)
				lineWidth = (int) ((BasicStroke) highlightStroke).getLineWidth();
		}

		g2d.drawRect(bounds.x, bounds.y, bounds.width, bounds.height);
		// grow by lineWidth as the current bounds are on the boundary
		// of the highlight
		bounds.grow(0, lineWidth);

		return bounds;
	}

	/* Listener Methods */

	/**
	 * Fires an moveGestureRecognized event to <code>CueGestureListener</code>s to
	 * indicate that a move request has begin
	 * @param sourcePath the source path to the node which is moving
	 * @param parentPath the parent path to the node which is moving
	 * @param index the index of the node that is moving
	 * @param p the mouse point where the event occured
	 */
	public void fireMoveGestureRecognized(
		TreePath sourcePath,
		TreePath parentPath,
		int index,
		Point p) {
			TreePath[] paths = new TreePath[] { sourcePath };
			fireMoveGestureRecognized(paths, parentPath, index, p);
	}


	public void fireMoveGestureRecognized(
		TreePath[] sourcePaths,
		TreePath parentPath,
		int index,
		Point p) {

			// Guaranteed to return a non-null array
			Object[] listeners = listenerList.getListenerList();
			CueEvent evt = null;

			// Process the listeners last to first, notifying
			// those that are interested in this event
			for (int i = listeners.length - 2; i >= 0; i -= 2) {
				if (listeners[i] == CueGestureListener.class) {
					// Lazily create the event:
					if (evt == null)
						evt = new CueEvent(this, sourcePaths, parentPath, index, p);
					if (sourcePaths.length == 1) {
						((CueGestureListener) listeners[i + 1]).
							moveGestureRecognized(evt);
					} else {
						((CueGestureListener) listeners[i + 1]).
							multipleMoveGestureRecognized(evt);
					}
				}
			}
	}

	public void fireMoveGestureCompleted(
		TreePath sourcePath,
		TreePath parentPath,
		int index,
		Point p) {
			TreePath[] paths = new TreePath[] { sourcePath };
			fireMoveGestureCompleted(paths, parentPath, index, p);
	}

	/**
	 * @param sourcePath the source path to the node which is moving
	 * @param parentPath the parent path to the node which is moving
	 * @param index the index of the node that is moving
	 * @param p the mouse point where the event occured
	 */
	public void fireMoveGestureCompleted(
		TreePath[] sourcePaths,
		TreePath parentPath,
		int index,
		Point p) {

			// Guaranteed to return a non-null array
			Object[] listeners = listenerList.getListenerList();
			CueEvent evt = null;

			// Process the listeners last to first, notifying
			// those that are interested in this event
			for (int i = listeners.length - 2; i >= 0; i -= 2) {
				if (listeners[i] == CueGestureListener.class) {
					// Lazily create the event:
					if (evt == null)
						evt = new CueEvent(this, sourcePaths, parentPath, index, p);

					if (sourcePaths.length == 1) {
						((CueGestureListener) listeners[i + 1]).
							moveGestureCompleted(evt);
					} else {
						((CueGestureListener) listeners[i + 1]).
							multipleMoveGestureCompleted(evt);
					}
				}
			}
	}

	public void fireAddGestureRecognized(
		TreePath sourcePath,
		TreePath parentPath,
		int index,
		Point p) {
			TreePath[] paths = new TreePath[] { sourcePath };
			fireAddGestureRecognized(paths, parentPath, index, p);
	}

	/**
	 * Fires an addGestureRecognized event to <code>CueGestureListener</code>s to
	 * indicate that a new node has requested to be added
	 *
	 * @param sourcePath the source path of the node to be added, use this
	 * as information carrier about the new node
	 * @param parentPath the parent path to the node to be added
	 * @param index the index of the the add started
	 * @param p the mouse point where the event occured
	 */
	public void fireAddGestureRecognized(
		TreePath[] sourcePaths,
		TreePath parentPath,
		int index,
		Point p) {
			// Guaranteed to return a non-null array
			Object[] listeners = listenerList.getListenerList();
			CueEvent evt = null;

			// Process the listeners last to first, notifying
			// those that are interested in this event
			for (int i = listeners.length - 2; i >= 0; i -= 2) {
				if (listeners[i] == CueGestureListener.class) {
					// Lazily create the event:
					if (evt == null)
						evt = new CueEvent(this, sourcePaths, parentPath, index, p);

					if (sourcePaths.length == 1) {
						((CueGestureListener) listeners[i + 1]).
							addGestureRecognized(evt);
					} else {
						((CueGestureListener) listeners[i + 1]).
							multipleAddGestureRecognized(evt);
					}
				}
			}
	}

	public void fireAddGestureCompleted(
		TreePath sourcePath,
		TreePath parentPath,
		int index,
		Point p) {
			TreePath[] paths = new TreePath[] { sourcePath };
			fireAddGestureCompleted(paths, parentPath, index, p);
	}

	/**
	 * Fires an addGestureCompleted event to <code>CueGestureListener</code>s to
	 * indicate that the operation for a  new node has requested to be added has
	 * completed
	 *
	 * @param sourcePath the source path of the node where the add completed
	 * @param parentPath the parent path to the node where the add completed
	 * @param index the index of the the add completed
	 * @param p the mouse point where the event occured
	 */
	public void fireAddGestureCompleted(
		TreePath[] sourcePaths,
		TreePath parentPath,
		int index,
		Point p) {
			// Guaranteed to return a non-null array
			Object[] listeners = listenerList.getListenerList();
			CueEvent evt = null;

			// Process the listeners last to first, notifying
			// those that are interested in this event
			for (int i = listeners.length - 2; i >= 0; i -= 2) {
				if (listeners[i] == CueGestureListener.class) {
					// Lazily create the event:
					if (evt == null)
						evt = new CueEvent(this, sourcePaths, parentPath, index, p);

					if (sourcePaths.length == 1) {
						((CueGestureListener) listeners[i + 1]).
							addGestureCompleted(evt);
					} else {
						((CueGestureListener) listeners[i + 1]).
							multipleAddGestureCompleted(evt);
					}
				}
			}
	}

	/**
	 * A <code>CueGestureHandler</code> handles mouse events for the
	 * <code>CueTree</code>. When a mouse event occurs that initiates the cue
	 * the <code>CueGestureHandler</code> invokes startCueMode().
	 * @author Marc McIntyre <mmcintyre@squiz.net>
	 */
	protected class CueGestureHandler extends MouseAdapter
		implements MouseMotionListener {

		/**
		 * Event Listener method that is called when the mouse is dragged
		 * @param evt the MouseEvent
		 */
		public void mouseDragged(MouseEvent evt) {}

		/**
		 * Event Listener method that is called when the mouse is pressed
		 * @param evt the mouse event
		 */
		public void mousePressed(MouseEvent evt) {

			// TODO: (MM) need to make use of GUIUtilities.isRightMouseButton
			if (SwingUtilities.isRightMouseButton(evt))
				return;

			// Use this listener stub in favour of mouseClicked() or mouseReleased()
			// as there is an instance where if a particluar node is expanded, and
			// a scrollbar is required, when that node is collapsed, the tree would
			// go into cueMode because the icon of the node above was pressed

			if (((CueTreeUI) getUI()).isLocationInExpandControl(evt.getX(), evt.getY()))
				return;

			if (inCueMode) {
				if (sourcePaths.length == 1)
					handleSingleSource(evt.getPoint());
				else
					handleMultipleSources(evt.getPoint());
				return;
			}

			TreePath path = getPathForLocation(evt.getX(), evt.getY());
			if (path == null)
				return;

			if (pointTriggersMove(evt.getPoint())) {
				if (canMoveNode(path.getLastPathComponent())) {
					int index =
						getModel().getIndexOfChild(
							path.getParentPath().getLastPathComponent(),
							path.getLastPathComponent()
						);

					TreePath[] paths = new TreePath[] { path };
					fireMoveGestureRecognized(paths, path.getParentPath(), index, evt.getPoint());
					// if the icon was pressed, then we are in move mode
					requestMode = MOVE_REQUEST_MODE;
					startCueMode(paths);
				}
			}
		}

		protected void handleSingleSource(Point initPoint) {
			TreePath parentPath = null;
			TreePath sourcePath = sourcePaths[0];

			// -1 indicates that the the parent wasn't expanded. It is up to
			// the CueGestureListener to determine where in the tree the node is to
			// be added/moved
			int index = -1;

			if (!lastPathWasParent) {
				// if the last path wasn't the new parent and the current path
				// was expanded, then the new position is the first child of
				// current path
				if (isExpanded(currentPath)) {
					parentPath = currentPath;
					index = 0;
				} else {
					// if we are on the same branch...
					index = 1;
					if (currentPath.getParentPath() == sourcePath.getParentPath()) {

						int newIndex = getModel().getIndexOfChild(
							currentPath.getParentPath().getLastPathComponent(),
							currentPath.getLastPathComponent()
						);

						int oldIndex = getModel().getIndexOfChild(
							sourcePath.getParentPath().getLastPathComponent(),
							sourcePath.getLastPathComponent()
						);

						// if the old index is the same as the new index
						// and we are still on the same branch, then
						// do nothing and stop cue mode
						if (oldIndex == newIndex) {
							stopCueMode();
							return;
						}
					}

					index += getModel().getIndexOfChild(
							currentPath.getParentPath().getLastPathComponent(),
							currentPath.getLastPathComponent()
						);
					parentPath = currentPath.getParentPath();
				}
			} else {
				parentPath = currentPath;
			}

			// if the path was above the top path in the tree then the index
			// is 0
			if (aboveTopPath) {
				index = 0;
			}
	//		if (triggersPath)
	//			triggerPath(currentPath, initPoint.y, 5);
			if (requestMode == MOVE_REQUEST_MODE) {
				fireMoveGestureCompleted(sourcePath, parentPath, index, initPoint);
			} else {
				fireAddGestureCompleted(sourcePath, parentPath, index, initPoint);
			}

			stopCueMode();
		}

		protected void handleMultipleSources(Point initPoint) {
			TreePath parentPath = null;

			// -1 indicates that the the parent wasn't expanded. It is up to
			// the CueGestureListener to determine where in the tree the node is to
			// be added/moved
			int index = -1;

			if (!lastPathWasParent) {
				// if the last path wasn't the new parent and the current path
				// was expanded, then the new position is the first child of
				// current path
				if (isExpanded(currentPath)) {
					parentPath = currentPath;
					index = 0;
				} else {
					// if we are on the same branch...
					if (currentPath.getParentPath() == sourcePaths[0].getParentPath()) {

						int newIndex = getModel().getIndexOfChild(
							currentPath.getParentPath().getLastPathComponent(),
							currentPath.getLastPathComponent()
						);

						int oldIndex = getModel().getIndexOfChild(
							sourcePaths[0].getParentPath().getLastPathComponent(),
							sourcePaths[0].getLastPathComponent()
						);

						// if the old index is the same as the new index
						// and we are still on the same branch, then
						// do nothing and stop cue mode
						if (oldIndex == newIndex) {
							stopCueMode();
							return;
						}
						// if the node is moving down in the branch,
						// then we need to compensate by 1 because we
						// need to remove the node first, so the index
						// will be 1 less
						if (oldIndex < newIndex)
							index = 0;
						else
						if (!aboveTopPath)
							index = 1;
					} else {
						index = 1;
					}

					index += getModel().getIndexOfChild(
							currentPath.getParentPath().getLastPathComponent(),
							currentPath.getLastPathComponent()
						);

					parentPath = currentPath.getParentPath();
				}
			} else {
				parentPath = currentPath;
			}
	//		if (triggersPath)
	//			triggerPath(currentPath, initPoint.y, 5);

			if (requestMode == MOVE_REQUEST_MODE) {
				sourcePaths = filterMultipleNodes(sourcePaths);
				fireMoveGestureCompleted(sourcePaths, parentPath, index, initPoint);
			} else {
				fireAddGestureCompleted(sourcePaths, parentPath, index, initPoint);
			}
			stopCueMode();
		}

		/**
		 * Filters out unwanted nodes from the multiple move selection and returns
		 * the wanted nodes.
		 * @param sourcePaths the sourcePaths that are current in the multiple
		 * move operation.
		 * @return the filtered sourcePaths
		 */
		protected TreePath[] filterMultipleNodes(TreePath[] sourcePaths) {
			return sourcePaths;
		}

		/**
		 * Returns true if the specified point will trigger a move operation
		 * when the mouse if pressed on that point
		 * @param p the point to check
		 */
		protected boolean pointTriggersMove(Point p) {
			return nodeIconContainsPoint(p);
		}

		/**
		 * Event Listener method that is called during mouse movement operations.
		 * @param evt the mouse event
		 */
		public void mouseMoved(MouseEvent evt) {

			if (!inCueMode) {
				if (pointTriggersMove(evt.getPoint())) {
					TreePath path = getPathForLocation(evt.getX(), evt.getY());
					// if we can move the mode, set the mouse cursor to
					// the move cursor, else set it to the cant move cursor
					if (canMoveNode(path.getLastPathComponent())) {
						setCursor(moveCursor);
					} else {
						setCursor(noMoveCursor);
						return;
					}
				} else {
					setCursor(Cursor.getDefaultCursor());
				}
			} else {
				TreePath path = getClosestPathForLocation(evt.getX(), evt.getY() - cueLineOffset);

				// if we are showing the ghosted node, paint it onto
				// the location where the mouse currently is before we
				// paint the cue line
				if (showsGhostedNode)
					paintGhostedNode(evt.getX() + 5, evt.getY() - 5, path);
				drawCueLine(path, evt.getY());
			}
		}

		/**
		 * Oscilates the cue line on and off for the specified path as a means
		 * of indicating that the path has been selected for a move operation.
		 * @param path the path to trigger
		 * @param mouseY the mouse Y co-ordinate
		 * @param triggerCount the number of times to trigger the path
		 */
		protected void triggerPath(
			final TreePath path,
			final int mouseY,
			final int triggerCount) {
				final Timer t = new Timer(50, null);
				ActionListener listener = new ActionListener() {
					private boolean triggered = false;
					private int triggeredCount = 1;
					public void actionPerformed(ActionEvent evt) {
						if (!triggered)
							drawCueLine(path, mouseY, true);
						else
							paintImmediately(dirtyCueBounds);
						if (triggeredCount++ == triggerCount) {
							paintImmediately(dirtyCueBounds);
							t.stop();
						}
						triggered = (triggered) ? false : true;
					}
				};
				t.addActionListener(listener);
				t.start();
		}

	}//end CueRequestHandler

	protected class CueTreeUI extends BasicTreeUI {
		public boolean isLocationInExpandControl(int mouseX, int mouseY) {
			if ((getPathForLocation(mouseX, mouseY) != null))
				return false;
			TreePath path = getClosestPathForLocation(CueTree.this, mouseX, mouseY);
			return isLocationInExpandControl(path, mouseX, mouseY);
		}
	}

	/**
	 * Returns TRUE if the node can be moved
	 * @param node the node the check
	 * @return TRUE if the node can be moved
	 */
	protected boolean canMoveNode(Object node) {
		return true;
	}

	public static void main(String[] args) {
		JFrame f = new JFrame();
		CueTree cueTree = new CueTree();

		f.getContentPane().add(new JScrollPane(cueTree));
		f.setSize(300, 500);
		f.show();
	}
}
