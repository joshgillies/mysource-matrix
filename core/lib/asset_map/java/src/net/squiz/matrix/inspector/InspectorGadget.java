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
* $Id: InspectorGadget.java,v 1.7 2007/05/08 02:28:37 rong Exp $
*
*/

/**
 * :tabSize=4:indentSize=4:noTabs=false:
 * :folding=explicit:collapseFolds=1:
 */

package net.squiz.matrix.inspector;

import net.squiz.matrix.core.*;
import net.squiz.matrix.matrixtree.*;
import net.squiz.matrix.ui.*;

import java.util.*;
import org.w3c.dom.*;
import java.io.IOException;

import java.awt.*;
import java.awt.dnd.*;
import java.awt.datatransfer.*;
import java.awt.event.*;
import java.awt.image.*;
import java.awt.geom.*;

import javax.swing.*;
import javax.swing.event.*;
import javax.swing.table.*;
import javax.swing.tree.*;
import javax.swing.plaf.*;


/**
 * The InspectorGadget class is the Explorer style table in the Matrix asset map.
 *
 * @author Nathan de Vries <ndvries@squiz.net>
 */
public class InspectorGadget 	extends 	JTable
								implements 	MouseListener,
											ComponentListener,
											NodeDoubleClickedListener,
											TransferListener,
											InitialisationListener,
											Autoscroll,
											Scrollable {

	private int columns;
	private java.util.List filters = new Vector();
	private MatrixTree tree;
	private InspectorNavigator navigator;
	private TreePath currentPath;

	private SelectionTool selTool;
	private DragHandler dragHandler;
	private DropHandler dropHandler;
	private MenuHandler menuHandler;
	private DragSource dragSource = null;

	private int lastRowOver = -1;
	private static final int AUTOSCROLL_MARGIN = 12;

	private static final int CUE_LEFT = 0;
	private static final int CUE_RIGHT = 1;

	protected BufferedImage dblBuffer = null;

	//{{{ Public Methods

	/**
	* Returns an InspectorGadget which is a table representation of the tree
	* passed as a parameter.
	*
	* @param model  the table model to construct the table with.
	* @param tree  the tree that the table represents.
	*/
	public InspectorGadget(TableModel model, MatrixTree tree) {
		super(model);
	//	filters.add(new TypeCodeFilter("page_standard"));
		addMouseListener(this);
		setCellEditor(null);
		setTableHeader(null);

		this.tree = tree;
		// we want to listen to the tree model updates
		tree.getModel().addTreeModelListener(getInspectorTreeModelListener());

		//currentPath = tree.getPathToRoot(AssetManager.getRootFolderNode());

		navigator = new InspectorNavigator(this);
		selTool = new SelectionTool((JTable)this);
		dragHandler = getDragHandler();
		dropHandler = getDropHandler();
		menuHandler = getMenuHandler();

		addMouseListener(selTool);
		addMouseListener(menuHandler);
		addMouseMotionListener(selTool);

		dragSource = DragSource.getDefaultDragSource();

		DragGestureRecognizer dgr =
			dragSource.createDefaultDragGestureRecognizer(
			this,                             //DragSource
			DnDConstants.ACTION_COPY_OR_MOVE, //specifies valid actions
			dragHandler                      //DragGestureListener
		);

		dgr.setSourceActions(dgr.getSourceActions() & ~InputEvent.BUTTON3_MASK);
		DropTarget dropTarget = new DropTarget(this, dropHandler);

		AssetManager.addInitialisationListener(this);

		setUI(new MatrixTableUI());

		// create a mouse motion listener to update the ViewPort when we
		// do a drag operation where the drag extends greater than the tree size
		/*MouseMotionListener mmListener = new MouseMotionAdapter() {
			public void mouseDragged(MouseEvent evt) {
				Point pt = evt.getPoint();

				int row = rowAtPoint(pt);

				if (row == getRowCount() - 1)
					return;

				Rectangle bounds = getBounds();

				if (pt.y + bounds.y <= AUTOSCROLL_MARGIN) {
					if (row > 0) --row;
				}
				else {
					if (row < getRowCount() - 1) ++row;
				}
				scrollRectToVisible(getCellRect(row, 0, true));

				//Rectangle r = new Rectangle(evt.getX(), evt.getY(), 6, 6);
				//scrollRectToVisible(r);
			}
		};*/
		setAutoscrolls(false);
		//addMouseMotionListener(mmListener);
	}

	/**
	 * Returns the InspectorNavigator which provides navigational tools.
	 *
	 * @return the inspector navigator
	 */
	public InspectorNavigator getNavigator() {
		return navigator;
	}

	/**
	 * Returns the MatrixTree that this table is linked to.
	 *
	 * @return the tree.
	 */
	public MatrixTree getTree() {
		return tree;
	}

	public void autoscroll(Point pt) {
		int row = rowAtPoint(pt);
		Rectangle bounds = getBounds();

		if (pt.y + bounds.y <= AUTOSCROLL_MARGIN) {
			if (row > 0) --row;
		}
		else {
			if (row < getRowCount() - 1) ++row;
		}
		scrollRectToVisible(getCellRect(row, 0, true));
	}

	public Insets getAutoscrollInsets() {
		Rectangle outer = getBounds();
		Rectangle inner = getParent().getBounds();
		return new Insets(	inner.y - outer.y + AUTOSCROLL_MARGIN,
							inner.x - outer.x + AUTOSCROLL_MARGIN,
							outer.height - inner.height - inner.y + outer.y + AUTOSCROLL_MARGIN,
							outer.width - inner.width - inner.x + outer.x + AUTOSCROLL_MARGIN);
	}

	/**
	 * Adds a TransferListener to listen for transfer events
	 *
	 * @param l the TransferListener to add
	 */
	public void addTransferListener(TransferListener l) {
		listenerList.add(TransferListener.class, l);
	}

	/**
	 * Removes a TransferListener
	 *
	 * @param l the TransferListener to remove
	 */
	public void removeTransferListener(TransferListener l) {
		listenerList.remove(TransferListener.class, l);
	}

	/**
	 * Returns the first node that is currently selected in table
	 *
	 * @return the first node that is selected in the table, or null
	 *         if there are no nodes currently selected
	 * @see #getSelectionNodes()
	 */
	public NodePosition getSelectionNode() {
		int row = getSelectedRow();
		int col = getSelectedColumn();

		if (isCellSelected(row,col)) {
			return new NodePosition(row,col);
		}
		return null;
	}

	/**
	 * Returns the nodes in the current selection.
	 *
	 * @return the nodes in the current selection, or null if there are
	 *         no nodes in the current selection
	 * @see #getSelectionNode()
	 */
	public NodePosition[] getSelectionNodes() {
		// Get the min and max ranges of selected cells
		int rowIndexStart = getSelectedRow();
		int rowIndexEnd = getSelectionModel().getMaxSelectionIndex();
		int colIndexStart = getSelectedColumn();
		int colIndexEnd = getColumnModel().getSelectionModel().getMaxSelectionIndex();

		// MM: what happens if there is no selection?

		// Check each cell in the range
		ArrayList nodes = new ArrayList();

		for (int row = 0; row <= getRowCount(); row++) {
			for (int col = 0; col <= getColumnCount(); col++) {
				if (isCellSelected(row, col)) {
					nodes.add( new NodePosition(row,col) );
				}
			}
		}

		return (NodePosition[]) nodes.toArray(new NodePosition[nodes.size()]);
	}

	/**
	 * Returns the cell position of the top most selected node.
	 *
	 * @return the cell position
	 */
	public NodePosition getTopMostSelection(NodePosition[] positions) {
		NodePosition topMost = positions[0];

		for (int i = 0; i < positions.length; i++){
			if (positions[i].getRow() < topMost.getRow())
				topMost = positions[i];
		}
		return topMost;
	}

	/**
	 * Returns the cell position of the bottom most selected node.
	 *
	 * @return the cell position
	 */
	public NodePosition getBottomMostSelection(NodePosition[] positions) {
		NodePosition bottomMost = positions[0];

		for (int i = 0; i < positions.length; i++){
			if (positions[i].getRow() > bottomMost.getRow())
				bottomMost = positions[i];
		}
		return bottomMost;
	}

	/**
	 * Returns the cell position of the left most selected node.
	 *
	 * @return the cell position
	 */
	public NodePosition getLeftMostSelection(NodePosition[] positions) {
		NodePosition leftMost = positions[0];

		for (int i = 0; i < positions.length; i++){
			if (positions[i].getColumn() < leftMost.getColumn())
				leftMost = positions[i];
		}
		return leftMost;
	}

	/**
	 * Returns the cell position of the right most selected node.
	 *
	 * @return the cell position
	 */
	public NodePosition getRightMostSelection(NodePosition[] positions) {
		NodePosition rightMost = positions[0];

		for (int i = 0; i < positions.length; i++){
			if (positions[i].getColumn() > rightMost.getColumn())
				rightMost = positions[i];
		}
		return rightMost;
	}

	/**
	 * Returns the paths of all selected values.
	 *
	 * @return an array of TreePath objects indicating the selected nodes, or
	 *         null if nothing is currently selected
	 */
	public TreePath[] getSelectionPaths() {
		NodePosition[] nodes = getSelectionNodes();
		TreePath[] paths = new TreePath[nodes.length];

		for (int i = 0; i < nodes.length; i++) {
			MatrixTreeNode node = (MatrixTreeNode) getValueAt(nodes[i].getRow(), nodes[i].getColumn());
			paths[i] = tree.getPathToRoot(node);
		}
		return paths;
	}

	public TreePath getSelectionPath() {
		NodePosition pos = getSelectionNode();
		MatrixTreeNode node = (MatrixTreeNode) getValueAt(pos.getRow(), pos.getColumn());
		return tree.getPathToRoot(node);
	}

	/**
	 * Returns the Rectangle that the selected nodes are drawn into.
	 *
	 * @param positions  the NodePositions of each selected node
	 * @return the Rectangle the selected nodes are drawn in
	 */
	public Rectangle getSelectionBounds(NodePosition[] positions)
	{
		NodePosition topMost    = getTopMostSelection(positions);
		NodePosition bottomMost = getBottomMostSelection(positions);
		NodePosition leftMost   = getLeftMostSelection(positions);
		NodePosition rightMost  = getRightMostSelection(positions);

		int topX = (int) getCellRect(leftMost.getRow(),leftMost.getColumn(),true).getX();
		int topY = (int) getCellRect(topMost.getRow(),topMost.getColumn(),true).getY();

		int bottomX = (int) getCellRect(rightMost.getRow(),rightMost.getColumn(),true).getX();
		bottomX += (int) getCellRect(rightMost.getRow(),rightMost.getColumn(),true).getWidth();

		int bottomY = (int) getCellRect(bottomMost.getRow(),bottomMost.getColumn(),true).getY();
		bottomY += (int) getCellRect(bottomMost.getRow(),bottomMost.getColumn(),true).getHeight();

		return new Rectangle(topX, topY, bottomX - topX, bottomY - topY);
	}

	/**
	 * Returns the NodePosition of the last !null cell in the table
	 *
	 * @return the NodePosition
	 */
	public NodePosition getLastDraggableNode() {
		for (int i = getColumnCount() - 1; i >= 0; i--) {
			if (getValueAt(getRowCount() - 1, i) != null)
				return new NodePosition(getRowCount() - 1, i);
		}
		return null;
	}

	/**
	 * Fills the table with all the nodes in one level of a TreePath
	 *
	 * @param path  the path of the nodes to display in the table
	 */
	public void populateInspector(TreePath path) {
		MatrixTreeNode parent = (MatrixTreeNode) path.getLastPathComponent();
		Enumeration children = parent.children();
		int childCount = parent.getChildCount();

		columns = Math.round(getWidth() / 70);

		// MM: fix this
		if (columns == 0)
			columns = 3;

		DefaultTableModel model = new InspectorTableModel(0,columns);

		MatrixTreeNode[] row = new MatrixTreeNode[columns];

		int allowedNodes = 0;
		int i = 0;
		boolean rowAdded = false;

		while (children.hasMoreElements()) {
			MatrixTreeNode child = (MatrixTreeNode) children.nextElement();

			if (filtersAllowNode(child)) {
				row[i] = child;
				i++;
				allowedNodes++;
				rowAdded = false;
			}

			if (i == columns) {
				rowAdded = true;
				model.addRow(row);
				row = new MatrixTreeNode[columns];
				i = 0;
			}
		}
		if (!rowAdded && i > 0)
			model.addRow(row);

		setModel(model);
	}

	/**
	 * Strips the nodes from a table and recreates the model according to the
	 * number of columns, usually after the table is resized
	 */
	public void redrawInspector() {

		if (Math.round(getWidth()) / 70 == columns)
			return;

		columns = Math.round(getWidth()) / 70;

		DefaultTableModel oldModel = (DefaultTableModel) getModel();
		DefaultTableModel model = new InspectorTableModel(0,columns);

		MatrixTreeNode[] nodes = new MatrixTreeNode[oldModel.getRowCount() * oldModel.getColumnCount()];
		MatrixTreeNode[] row = new MatrixTreeNode[columns];

		int a = 0;
		for (int b = 0; b < oldModel.getRowCount(); b++) {
			for (int c = 0; c <  oldModel.getColumnCount(); c++) {
				MatrixTreeNode node = (MatrixTreeNode) oldModel.getValueAt(b,c);
				if (node != null) {
					nodes[a] = node;
					a++;
				}
			}
		}

		int i = 0;
		boolean rowAdded = false;
		for (int j = 0; j < nodes.length; j++) {
			// Add all the nodes from nodes[] to the new table.
			MatrixTreeNode node = (MatrixTreeNode) nodes[j];

			row[i] = node;
			i++;
			rowAdded = false;

			if (i == columns) {
				rowAdded = true;
				model.addRow(row);
				row = new MatrixTreeNode[columns];
				i = 0;
			}
		}
		if (!rowAdded && i > 0)
			model.addRow(row);
		setModel(model);
	}

	/* MouseListener methods */

	/**
	 * Invoked when the mouse enters a component
	 *
	 * @param evt  an event which indicates that a mouse action occurred in a
	 *             component
	 */
	public void mouseEntered(MouseEvent evt) {}

	/**
	 * Invoked when the mouse exits a component
	 *
	 * @param evt  an event which indicates that a mouse action occurred in a
	 *             component
	 */
	public void mouseExited(MouseEvent evt) {}

	/**
	 * Invoked when a mouse button has been released on a component
	 *
	 * @param evt  an event which indicates that a mouse action occurred in a
	 *             component
	 */
	public void mouseReleased(MouseEvent evt) {}

	/**
	 * Invoked when a mouse button has been pressed on a component
	 *
	 * @param evt  an event which indicates that a mouse action occurred in a
	 *             component
	 */
	public void mousePressed(MouseEvent evt) {}

	/**
	 * Invoked when the mouse button has been clicked (pressed and released)
	 * on a component
	 *
	 * @param evt  an event which indicates that a mouse action occurred in a
	 *             component
	 */
	public void mouseClicked(MouseEvent evt) {

		// we dont want to drill down the asset on right mouse clicks
		if (GUIUtilities.isRightMouseButton(evt))
			return;

		if (evt.getClickCount() != 2)
			return;

		if (getSelectedColumn() == -1)
			return;

		final MatrixTreeNode node = (MatrixTreeNode) getValueAt(getSelectedRow(), getSelectedColumn());

		if (node.getAsset().getNumKids() == 0)
			return;

		// The selected cell is null (remainder cell on the end of a row), so return
		if (node == null) {
			return;
		}

		if (!node.getAsset().childrenLoaded()) {
			MatrixStatusBar.setStatus("Loading children...");
			MatrixSwingWorker worker = new MatrixSwingWorker() {
				public Object construct() {
					try {
						AssetManager.refreshAsset(node, "");
					} catch (IOException ioe) {
						ioe.printStackTrace();
					}
					MatrixStatusBar.setStatusAndClear("Success!", 1000);
					TreePath path = tree.getPathToRoot(node);
					populateInspector(path);
					navigator.setBackPath(path);
					return null;
				}
			};
			worker.start();
		} else {
			TreePath path = tree.getPathToRoot(node);
			populateInspector(path);
			navigator.setBackPath(path);
		}
	}

	/* ComponentListener methods */

	/**
	 * Invoked when the component has been made invisible
	 *
	 * @param e  a low-level event which indicates that a component moved,
	 *           changed size, or changed visibility
	 */
	public void componentHidden(ComponentEvent e) {}

	/**
	 * Invoked when the component's position changes
	 *
	 * @param e  a low-level event which indicates that a component moved,
	 *           changed size, or changed visibility
	 */
	public void componentMoved(ComponentEvent e) {}

	/**
	 * Invoked when the component's position changes
	 *
	 * @param e  a low-level event which indicates that a component moved,
	 *           changed size, or changed visibility
	 */
	public void componentShown(ComponentEvent e) {}

	/**
	 * Invoked when the component's position changes
	 *
	 * @param e  a low-level event which indicates that a component moved,
	 *           changed size, or changed visibility
	 */
	public void componentResized(ComponentEvent e) {
		redrawInspector();
	}

	/**
	 * Fires an event about the nodes that were dragged, and where they were
	 * dropped
	 *
	 * @param dragIndex  index of the node which was clicked to start the drag
	 * @param dropIndex  index of the node which was dropped onto
	 * @param node  the node that was dragged
	 * @param dropParent  the parent of the node at dropIndex
	 */
	public void fireTransfer(	int dragIndex,
								int dropIndex,
								MatrixTreeNode node,
								MatrixTreeNode dropParent) {
		// Guaranteed to return a non-null array
		Object[] listeners = listenerList.getListenerList();
		TransferEvent evt = null;

		// Process the listeners last to first, notifying
		// those that are interested in this event
		for (int i = listeners.length - 2; i >= 0; i -= 2) {
			if (listeners[i] == TransferListener.class) {
				// Lazily create the event:
				if (evt == null) {
					evt = new TransferEvent(this, dragIndex, dropIndex, node, dropParent);
				}
				((TransferListener) listeners[i + 1]).transferGestureRecognized(evt);
			}
		}
	}

	/**
	 * NodeDoubleClickedListener event that is fired when a node in a tree is
	 * double clicked
	 *
	 * @param e  the CueEvent
	 * @see NodeDoubleClickedListener#nodeDoubleClicked(NodeDoubleClickedEvent)
	 */
	public void nodeDoubleClicked(NodeDoubleClickedEvent e) {
		populateInspector(e.getClickedPath());
		navigator.setBackPath(e.getClickedPath());
		currentPath = e.getClickedPath();
	}

	/**
	 * TransferListener event that is fired when a valid node (or nodes) is
	 * dropped in an InspectorGadget component
	 *
	 * @param e  the TransferEvent
	 * @see TransferListener#transferGestureRecognized(TransferEvent)
	 */
	public void transferGestureRecognized(TransferEvent evt) {

		//MM: we need to move all this stuff with the communication into one
		// generic place so its transparent to the tree and inspector
		tree.fireCreateLink(NewLinkEvent.LINK_TYPE_MOVE, new MatrixTreeNode [] { evt.getNode() }, evt.getDropParent(), evt.getDropIndex());
	}

	/**
	 * InitialisationListener event that is fired when the tree has completely finished
	 * loading
	 *
	 * @param e  the InitialisationEvent
	 * @see InitialisationListener#initialisationComplete(InitialisationEvent)
	 */
	public void initialisationComplete(InitialisationEvent e) {
		currentPath = tree.getPathToRoot(e.getRootNode());
		navigator.setCurrentPath(currentPath);
		populateInspector(currentPath);
	}

	/**
	 * Checks whether the location specified by point is within the bounds of a cell's
	 * label or icon (JLabel)
	 *
	 * @param point  the point to check for
	 * @return TRUE if the point is within a component
	 * @return FALSE if the point isn't within a component
	 */
	 public boolean mouseInsideCellComponent(Point point) {
		int row = rowAtPoint(point);
		int col = columnAtPoint(point);
		TableCellRenderer renderer = getCellRenderer(row,col);
		JLabel label = (JLabel) renderer.getTableCellRendererComponent(this,getValueAt(row,col),true,true,row,col);

		if (label == null)
			return false;

		Dimension dim = label.getPreferredSize();

		Rectangle cellRect = getCellRect(row,col,true);

		// ICON STUFF //
		int iconX = (int)( ( cellRect.getWidth() - label.getIcon().getIconWidth() ) / 2 + cellRect.getX() );
		int iconY = (int)( ( cellRect.getHeight() - dim.getHeight() ) / 2 + cellRect.getY() );

		Rectangle iconRect = new Rectangle(iconX, iconY, label.getIcon().getIconWidth(), label.getIcon().getIconHeight());

		if (iconRect.contains(point))
			return true;
		// END ICON STUFF //


		// TEXT STUFF //
		int textWidth;
		int textHeight;

		if (dim.getWidth() > cellRect.getWidth())
			textWidth = (int) cellRect.getWidth();
		else
			textWidth = (int) dim.getWidth();

		// Override the above with a quick fix so that textWidth is the width of the cell
		//textWidth = (int) cellRect.getWidth();

		textHeight = (int)( dim.getHeight() - label.getIcon().getIconHeight() + label.getIconTextGap() );

		int textX = (int)( ( cellRect.getWidth() - textWidth ) / 2 + cellRect.getX() );
		int textY = (int)( iconRect.getY() + iconRect.getHeight() - label.getIconTextGap() );

		Rectangle textRect = new Rectangle(textX, textY, textWidth, textHeight);

		if (textRect.contains(point))
			return true;
		// END TEXT STUFF //

		return false;
	 }

	/**
	 * Overrides the paintComponent() method in JTable to perform double
	 * buffering operations
	 *
	 * @param g  the graphics set to paint to.
	 */
	public void paintComponent(Graphics g) {
		Graphics2D g2 = (Graphics2D) g;

		// this gets executed once
		if (dblBuffer == null) {
			initBufferImage();
		}
		g2.drawImage(dblBuffer, null, 0, 0);
		super.paintComponent(g);

		if (selTool.isDragging())
			selTool.paintSelectionTool(g2);
		else if (dropHandler.isDropping()) {
			dropHandler.paintDropImage(g2);
			dropHandler.paintCueLine(g2);
		}
	}

	//}}}

	//{{{ Protected Methods

	/**
	 * Checks if the filter allows this node to be displayed.
	 *
	 * @param node the MatrixTreeNode to check
	 * @return TRUE if the filters don't block the node, FALSE otherwise.
	 */
	protected boolean filtersAllowNode(MatrixTreeNode node) {
		Iterator filterIterator = filters.iterator();
		while (filterIterator.hasNext()) {
			Filter filter = (Filter) filterIterator.next();
			if (!filter.allowsNode(node))
				return false;
		}
		return true;
	}

	/**
	 * Returns the drag handler that handles drag operations.
	 *
	 * @return the drag handler
	 */
	protected DragHandler getDragHandler() {
		return new DragHandler();
	}

	/**
	 * Returns the Drop handler that handles Drop operations.
	 *
	 * @return the drop handler
	 */
	protected DropHandler getDropHandler() {
		return new DropHandler();
	}

	protected MenuHandler getMenuHandler() {
		return new MenuHandler();
	}

	protected InspectorTreeModelListener getInspectorTreeModelListener() {
		return new InspectorTreeModelListener();
	}

	/**
	 * Returns the Image for the component at the specified NodePosition
	 *
	 * @param position  the NodePosition for the wanted component Image
	 * @return the image representation of the component at the given
	           NodePosition
	 * @see #getGhostedNode(NodePosition[])
	 */
	protected Image getGhostedNode(NodePosition position) {
		Component c = getComponentForPosition(position);
		if (c == null)
			return null;

		Rectangle bounds = getCellRect(position.getRow(), position.getColumn(), true);
		// need to set the size of the component
		c.setSize(bounds.width, bounds.height);

		BufferedImage image = new BufferedImage(
			bounds.width,
			bounds.height,
			BufferedImage.TYPE_INT_ARGB_PRE
		);

		Graphics2D g2d = (Graphics2D) image.createGraphics();
		g2d.setComposite(AlphaComposite.getInstance(AlphaComposite.SRC, 0.9f));
		c.paint(g2d);
		g2d.dispose();

		return image;
	}

	/**
	 * Returns a ghosted image for the specfied NodePosition
	 *
	 * @param positions  the positions of the wanted ghosted node
	 * @return the ghosted node of the specifed positions
	 * @see #getGhostedNode(NodePosition)
	 */
	protected Image getGhostedNode(NodePosition[] positions) {

		Rectangle bounds = getSelectionBounds(positions);

		BufferedImage image = new BufferedImage((int) bounds.getWidth(), (int) bounds.getHeight(), BufferedImage.TYPE_INT_ARGB_PRE);

		Graphics2D g2d = (Graphics2D) image.createGraphics();

		int widthOffset = 0;
		int heightOffset = 0;

		NodePosition topMost = getTopMostSelection(positions);
		NodePosition leftMost = getLeftMostSelection(positions);

		if (leftMost.getColumn() != 0)
			widthOffset = leftMost.getColumn() * ((int) getCellRect(leftMost.getRow(), leftMost.getColumn(),true).getWidth());

		if (topMost.getRow() != 0)
			heightOffset = topMost.getRow() * ((int) getCellRect(topMost.getRow(), topMost.getColumn(),true).getHeight());

		for (int i = 0; i < positions.length; i++) {
			if (getValueAt(positions[i].getRow(), positions[i].getColumn()) != null) {
				Image nodeImage = getGhostedNode(positions[i]);
				Rectangle nodeBounds = getCellRect( positions[i].getRow(), positions[i].getColumn(), true );
				g2d.drawImage(nodeImage, nodeBounds.x - widthOffset, nodeBounds.y - heightOffset, null);
			}
		}

		/*////IMAGE TESTER////
		ImageIcon imageIcon = new ImageIcon(image);
		JFrame testFrame = new JFrame();
		JLabel label = new JLabel( imageIcon );
		testFrame.getContentPane().add(label);
		testFrame.setSize(400,400);
		testFrame.setVisible(true);
		g2d.setColor(Color.BLUE);
		g2d.setComposite(AlphaComposite.getInstance(AlphaComposite.SRC_OVER, 0.5f));
		g2d.fillRect(0,0, imageIcon.getIconWidth(), imageIcon.getIconHeight());
		g2d.setColor(Color.BLUE.darker());
		g2d.drawRect(0,0, imageIcon.getIconWidth() - 1, imageIcon.getIconHeight() - 1);
		////IMAGE TESTER////*/

		g2d.dispose();

		return image;
	}

	/**
	 * Returns a drag image for the specifed positions. If there is multiple
	 * positions, the drag image will reflect the cell layout offsets in the
	 * table
	 *
	 * @param positions  the positions for the wanted drag image
	 * @return the drag image that reflects the specfied positions
	 */
	protected Image getDragImageForPosition(NodePosition[] positions) {
		if (positions == null)
			throw new IllegalArgumentException("positions is null");
		Image ghostedImage = (positions.length == 1)
			? getGhostedNode(positions[0])
			: getGhostedNode(positions);

		return ghostedImage;
	}

	/**
	 * Returns the renderable component for the specified position
	 *
	 * @param position  the <code>NodePosition</code> to the wanted renderable
	 *                  component
	 * @return the renderable component for the specified position
	 */
	protected Component getComponentForPosition(NodePosition position) {
		if (position == null)
			throw new IllegalArgumentException("position is null");

		int row = position.getRow();
		int col = position.getColumn();

		TableCellRenderer renderer = getCellRenderer(row,col);
		Object node = getValueAt(row,col);

		TableUI ui = getUI();
		if (ui != null) {

			return renderer.getTableCellRendererComponent(
				InspectorGadget.this,
				node,
				true,
				isFocusOwner(),
				row,
				col
			);
		}
		return null;
	}

	//}}}

	//{{{ Package Private Methods

	//}}}

	//{{{ Private Methods

	/**
	 * Initialises the double buffering image for offscreen painting
	 */
	private void initBufferImage() {
		int w = getWidth();
		int h = getHeight();
		dblBuffer = (BufferedImage) createImage(w, h);
		Graphics2D gc = dblBuffer.createGraphics();
		gc.setColor(getBackground());
		gc.fillRect(0, 0, w, h);
	}

	//}}}

	//{{{ Protected Inner Classes

	/**
	 * Class that handles drag operations that occur in the inspector.
	 *
	 * @author Nathan de Vries <ndvries@squiz.net>
	 */
	protected class DragHandler 	extends 	DragSourceAdapter
									implements 	DragGestureListener {

		protected TreePath[] dragPaths;
		protected Point dragOffset = new Point(5, 5);


		/* DragGestureListener methods */

		/**
		 * Event method from DragGestureListener that is invoked when a drag
		 * operation is recognized
		 *
		 * @param dge  the DragGestureEvent
		 */
		public void dragGestureRecognized(DragGestureEvent dge) {
			NodePosition pos = getSelectionNode();
			if ( (pos == null) || (!mouseInsideCellComponent(dge.getDragOrigin())) ) {
				return;
			}

			MatrixTreeNode dragNode = (MatrixTreeNode) getValueAt(pos.getRow(), pos.getColumn());
			dragPaths = getSelectionPaths();

			if (dragNode != null) {
				MatrixTreeTransferable transferable = new MatrixTreeTransferable(dragPaths);

				BufferedImage dragImage = (BufferedImage) getDragImageForPosition(getSelectionNodes());

				// Calculate the offset between the mouse and drag image, so that
				// the image can be drawn in the same spot in a different component
				Point topLeft = new Point(getSelectionBounds(getSelectionNodes()).getLocation());
				Point origin = dge.getDragOrigin();
				dragOffset.setLocation(origin.getX() - topLeft.getX(), origin.getY() - topLeft.getY());

				dge.startDrag(
					new Cursor(Cursor.DEFAULT_CURSOR),
					dragImage,
					dragOffset,
					transferable,
					this
				);

				DragImageExchange.setDragImage(dragImage, dragOffset);
			}
		}
	}//end class DragHandler

	/**
	 * The Drop Handler class handles drop operations that occur within the
	 * MatrixTree. Currently, only MatrixTreeTransferable.TREE_NODE_FLAVOUR flavours
	 * are accepted as successful drop transferables.
	 *
	 * @author Nathan de Vries <ndvries@squiz.net>
	 */
	protected class DropHandler implements DropTargetListener {

		protected Point initMousePt;
		protected Point lastMousePt;
		protected BufferedImage dragImage;
		protected Point mouseOffset = new Point(5,5);
		private boolean isDropping = false;

		/**
		 * Event listener method that is called when the mouse is dragged
		 * into the bounds of the InspectorGadget
		 *
		 * @param dtde  the DropTargetDragEvent
		 */
		public void dragEnter(DropTargetDragEvent dtde) {
			dragImage = DragImageExchange.getDragImage();
			mouseOffset = DragImageExchange.getMouseOffset();
			isDropping = true;
		}

		/**
		 * Events Listener method that is called when the drop action changes
		 *
		 * @param dtde  the DropTargetDragEvent
		 */
		public void dropActionChanged(DropTargetDragEvent dtde){}

		/**
		 * Event listener method that is called when the mouse is dragged
		 * ouside the bounds of the InspectorGadget
		 *
		 * @param dte  the DropTargetEvent
		 */
		public void dragExit(DropTargetEvent dte) {
			isDropping = false;
			dragImage = null;
			repaint();
		}

		/**
		 * Event listener method that is called repeatedly when the mouse
		 * is within the bounds of the InspectorGadget
		 *
		 * @param dtde  the DropTargetDragEvent
		 */
		public void dragOver(DropTargetDragEvent dtde) {
			if (lastMousePt != null && lastMousePt.equals(dtde.getLocation()))
				return;
			if (initMousePt == null) {
				initMousePt = dtde.getLocation();
				SwingUtilities.convertPointFromScreen(initMousePt, InspectorGadget.this);
			}
			lastMousePt = dtde.getLocation();
			repaint();
		}

		/**
		 * Event Listener method that is called when the mouse is released
		 * during a drop operation
		 *
		 * @param dtde  the DropTargetDropEvent
		 */
		public void drop(DropTargetDropEvent dtde) {

			Transferable transfer = dtde.getTransferable();
			java.util.List paths = null;
			try {
				paths = (java.util.List) transfer.getTransferData(
					MatrixTreeTransferable.TREE_NODE_FLAVOUR);
			} catch (UnsupportedFlavorException ufe) {
				ufe.printStackTrace();
			} catch (IOException ioe) {
				ioe.printStackTrace();
			}

			Point dropLocation = dtde.getLocation();

			int mouseX = (int) dtde.getLocation().getX();

			int dropRow = rowAtPoint(dropLocation);
			int dropCol = columnAtPoint(dropLocation);

			if (dropRow == -1)
				dropRow = getRowCount() - 1;

			Rectangle cellBounds = getCellRect(dropRow, dropCol, true);

			if (mouseX > (cellBounds.x + cellBounds.width / 2))
				if (dropCol < getColumnCount() - 1)
					dropCol++;

			int dropIndex = ( dropRow * getColumnCount() ) + dropCol;

			if (getValueAt(dropRow,dropCol) == null) {
				NodePosition pos = getLastDraggableNode();
				dropCol = pos.getColumn();
				dropIndex = ( dropRow * getColumnCount() ) + dropCol;
			}

			/*// just a quick hack so that we can get single moves working
			MatrixTreeNode node = (MatrixTreeNode) ((TreePath) paths.get(0)).getLastPathComponent();

			int dragIndex = node.getParent().getIndex(node);
			int dropIndex = ( dropRow * getColumnCount() ) + dropCol;

			MatrixTreeNode dropTarget = (MatrixTreeNode) getValueAt(dropRow, dropCol);
			MatrixTreeNode dropParent = (MatrixTreeNode) dropTarget.getParent();

			//fire an event to say that a move is occuring (tree can listen to this)
			fireTransfer(dragIndex, dropIndex, node, dropParent);*/

			for (int i = 0; i < paths.size(); i++) {
				MatrixTreeNode node = (MatrixTreeNode) ((TreePath) paths.get(i)).getLastPathComponent();
				int dragIndex = node.getParent().getIndex(node);
				MatrixTreeNode dropTarget = (MatrixTreeNode) getValueAt(dropRow, dropCol);
				MatrixTreeNode dropParent = (MatrixTreeNode) dropTarget.getParent();
				fireTransfer(dragIndex, dropIndex, node, dropParent);

				//Shift the drop index to the right to prevent reverse pasting
				dropIndex++;
			}

			//TreePath path = tree.getPathToRoot((MatrixTreeNode) getValueAt(1,1));
		//	populateInspector(currentPath);

			DragImageExchange.completeExchange();
			lastMousePt = null;
			initMousePt = null;
			isDropping = false;
			repaint();
		}

		/**
		 * Returns TRUE if we are currently performing a drop operation
		 *
		 * @return TRUE if we are currently performing a drop operation
		 */
		 protected boolean isDropping() {
			return isDropping;
		}

		/**
		 * Paints the drag image so the image appears in the same location
		 * under the mouse where the drag operation first started.
		 *
		 * @param g2d  the graphics to paint the drag image to
		 */
		protected void paintDropImage(Graphics2D g2d) {
			int x = lastMousePt.x - mouseOffset.x;
			int y = lastMousePt.y - mouseOffset.y;

			g2d.drawImage(dragImage, x, y, InspectorGadget.this);
		}

		/**
		 * Paints a cue line to indicate where the node(s) will be placed in
		 * relation to where the mouse has been dragged to.
		 *
		 * @param g2d  the graphics to paint the drag image to
		 */
		protected void paintCueLine(Graphics2D g2d) {

			int row = rowAtPoint(lastMousePt);
			int col = columnAtPoint(lastMousePt);

			if (col == -1)
				return;

			// Simple checking for legitimate placement of cueline
			if (row == -1)
				row = getRowCount() - 1;

			if (getValueAt(row,col) == null) {
				NodePosition pos = getLastDraggableNode();
				col = pos.getColumn();
			}
			// End checking

			Rectangle cellBounds = getCellRect(row, col, true);
			Rectangle bounds = getBounds();

			int mouseX = (int) lastMousePt.getX();
			int mouseY = (int) lastMousePt.getY();

			int leftRight;

			if (mouseX < (cellBounds.x + cellBounds.width / 2))
				leftRight = CUE_LEFT;
			else
				leftRight = CUE_RIGHT;

			int x = 0, y = 0, height = 0;

			x = leftRight == CUE_LEFT ? cellBounds.x : cellBounds.x + cellBounds.width;
			y = cellBounds.y;
			height = cellBounds.height;

			g2d.setColor(UIManager.getColor("CueLine.stroke"));

			g2d.setStroke(new BasicStroke(1));
			g2d.drawLine(x, y, x, y + height);
			g2d.drawLine(x - 2, y, x + 2, y);
			g2d.drawLine(x - 2, y + height, x + 2, y + height);
		}
	}//end class DropHandler

	protected class MenuHandler extends MouseAdapter {
		public void mouseClicked(MouseEvent evt) {
			if (!GUIUtilities.isRightMouseButton(evt))
				return;
			JPopupMenu menu = null;

			// if the click occured where there was no node, get a menu
			// for void space
		//	if (getPathForLocation(evt.getX(), evt.getY()) == null) {
		//		menu = getMenuForVoidSpace();
		//	} else {
				int currentRow = rowAtPoint(evt.getPoint());
				int currentColumn = columnAtPoint(evt.getPoint());

				if (currentRow == -1 || currentColumn == -1)
					return;

				changeSelection(currentRow, currentColumn, false, false);
				TreePath[] selectedPaths = getSelectionPaths();

				menu = (selectedPaths.length == 1)
					? getMenuForSingleSelection()
					: getMenuForMultipleSelection();
		//	}
			if (menu != null)
				menu.show(InspectorGadget.this, evt.getX(), evt.getY());
		}

		protected JPopupMenu getMenuForVoidSpace() {
			return MatrixMenus.getPopupAddMenu(null);
		}
		protected JPopupMenu getMenuForSingleSelection() {
			return MatrixMenus.getPopupScreenMenu((MatrixTreeNode) getSelectionPath().getLastPathComponent());
		}
		protected JPopupMenu getMenuForMultipleSelection() {
			return null;
		}
		protected JMenuItem[] getAncillaryMenuItems() {
			return null;
		}
	}

	protected class InspectorTreeModelListener implements TreeModelListener {

		public void treeNodesChanged(TreeModelEvent e) {
			populateInspector(currentPath);
		}

		public void treeNodesInserted(TreeModelEvent e) {
			populateInspector(currentPath);
		}

		public void treeNodesRemoved(TreeModelEvent e) {
			populateInspector(currentPath);
		}

		public void treeStructureChanged(TreeModelEvent e) {}
	}

	//}}}

	//{{{ Inner Classes

	/**
	 * MatrixTableUI modifys the BasicTableUI so that it is more suitable to
	 * drag operations that may occur in the InspectorGadget.
	 *
	 * @author Nathan de Vries <ndvries@squiz.net>
	 */
	class MatrixTableUI extends javax.swing.plaf.basic.BasicTableUI {

		/**
		 * Constructs the MatrixMouseListener to handle mouse events
		 *
		 * @return a new instance of the MatrixMouseHandler
		 */
		protected MouseInputListener createMouseInputListener() {
			return new MatrixMouseHandler();
		}

		/**
		 * The MatrixMouseHandler class tweaks the MouseHandler class in
		 * BasicTableUI so that it is more suitable to Drag Operations.
		 *
		 * @author Nathan de Vries <ndvries@squiz.net>
		 */
		public class MatrixMouseHandler 	extends 	MouseInputHandler
											implements 	MouseMotionListener {

			boolean isDragging = false;

			/**
			 * Event listener method called when a drag operation occurs
			 *
			 * @param evt  the MouseEvent
			 */
			public void mouseDragged(MouseEvent evt) {
				isDragging = true;
			}

			/**
			 * Event Listener method that is called when the mouse is moved
			 *
			 * @param evt  the MouseEvent
			 */
			public void mouseMoved(MouseEvent evt) {}

			/**
			 * Event Listener method that is called when the mouse is pressed
			 *
			 * @param evt  the MouseEvent
			 */
			public void mousePressed(MouseEvent evt) {}

			/**
			 * Event listener method that is called when the mouse is released
			 *
			 * @param evt  the MouseEvent
			 */
			public void mouseReleased(MouseEvent evt) {
				if (isDragging) {
					isDragging = false;
					return;
				}
				int mouseX = evt.getX();
				int mouseY = evt.getY();

				if ((rowAtPoint(evt.getPoint()) == -1)
						&& !GUIUtilities.isRightMouseButton(evt))
					clearSelection();
				else if (!mouseInsideCellComponent(evt.getPoint()))
					clearSelection();
				else
					super.mouseReleased(evt);
			}
		}//end class MatrixMouseListener
	}//end class MatrixTableUI

	//}}}

	//{{{ Main (Testing)

/*	public static void main(String[] args) {
		Matrix.setProperty("url.iconurl", "__lib/web/images/icons");
		Matrix.setProperty("url.typecodeurl", "__data/asset_types");
		Matrix.setProperty("url.notaccessibleicon", "asset_map/not_accessible.png");
		Matrix.setProperty("url.assetmapiconurl", "__lib/web/images/icons/asset_map");
		Matrix.setProperty("url.baseurl", "http://beta.squiz.net/marc/");
		Matrix.setProperty("url.execurl", "http://beta.squiz.net/marc/?SQ_ACTION=asset_map_request");

		tree = MatrixTreeBus.createTree(MatrixTree.LOADING_NODE);

		final InspectorGadget inspector = new InspectorGadget(new InspectorTableModel(0, 4), tree);
		inspector.setDefaultRenderer(inspector.getColumnClass(0), new InspectorCellRenderer());
		inspector.setShowHorizontalLines(false);
		inspector.setShowVerticalLines(false);
		inspector.setRowHeight(35);
		inspector.setRowSelectionAllowed(false);
		inspector.setCellSelectionEnabled(true);
		inspector.addComponentListener(inspector);
		inspector.addTransferListener(inspector);

		//tree.addTreeExpansionListener(inspector);
		//tree.addNodeDoubleClickedListener(inspector);

		// Hack to stop tree expansions when a node is doubleclicked
		//tree.setToggleClickCount(10);

		JSplitPane splitPane = new JSplitPane(JSplitPane.VERTICAL_SPLIT);
		//JPanel bottomPanel = new JPanel();
		JPanel bottomPanel = new JPanel(new BorderLayout());
		//bottomPanel.setLayout(new GridLayout(2, 1));

		splitPane.add(new JScrollPane(tree), JSplitPane.TOP);
		splitPane.add(bottomPanel, JSplitPane.BOTTOM);

		//bottomPanel.add(new InspectorNavigator(inspector));
		//bottomPanel.add(new JScrollPane(inspector));
		//navigator = new InspectorNavigator(inspector);
		//bottomPanel.add(BorderLayout.NORTH, navigator);
		bottomPanel.add(BorderLayout.CENTER, new JScrollPane(inspector));


		//pane.add(new JScrollPane(inspector), JSplitPane.BOTTOM);

		JFrame frame = new JFrame();
		frame.getContentPane().add(splitPane);
		Dimension screenSize = Toolkit.getDefaultToolkit().getScreenSize();
		frame.setSize(240, screenSize.height - 25);
		splitPane.setDividerLocation(screenSize.height - 400);

		frame.setVisible(true);
		Runnable runner = new Runnable() {
			public void run() {
				AssetManager.init();
				MatrixTreeNode root = AssetManager.getRootFolderNode();
				((DefaultTreeModel) tree.getModel()).setRoot(root);
				TreePath path = new TreePath(((DefaultTreeModel) tree.getModel()).getPathToRoot(root));
				currentPath = path;
				inspector.populateInspector(path);
				navigator.setBackPath(path);
			}
		};
		SwingUtilities.invokeLater(runner);
	}*/

}//end class InspectorGadget
