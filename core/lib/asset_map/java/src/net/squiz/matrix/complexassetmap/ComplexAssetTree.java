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
* $Id: ComplexAssetTree.java,v 1.1 2004/06/29 06:53:06 mmcintyre Exp $
* $Name: not supported by cvs2svn $
*/

package net.squiz.matrix.complexassetmap;

import net.squiz.matrix.assetmap.*;
import java.awt.event.*;
import java.awt.*;
import java.net.*;
import javax.swing.*;
import javax.swing.event.*;
import javax.swing.tree.*;
import java.util.*;
import java.io.IOException;

/**
 * The Complex Asset Tree. Functionality that is extended over
 * the simple asset tree includes:
 * 
 * <ul>
 *  <li>Ability to move assets</li>
 *  <li>Right click menu on assets for their menu</li>
 *  <li>Right click menu on assets for their menu</li>
 *  <li>Teleport to root functionality</li>
 *  <li>Menu for moving/linking/cloning</li>
 * </ul>
 * 
 *
 * @author Marc McIntyre <mmcintyre@squiz.net>
 * @see AssetTreeModel	net.squiz.matrix.AssetTreeModel
 * @see MySource		net.squiz.matrix.MySource
 */
public class ComplexAssetTree extends AssetTree implements MouseMotionListener {
	
	/** if TRUE was are creating a link */
	private boolean createLinkMode = false;
	
	/** If TRUE we are creating an asset */
	private boolean createAssetMode = false;
	
	/** The new asset type code that we are creating */
	private String newAssetTypeCode;
	
	/** Where the last cue image was drawn */
	private Rectangle cueImageBounds = VOID_RECTANGLE;
	
	/** The colour of the cue line */
	public static final Color CUE_LINE_COLOUR = Color.red;
	
	/** The asset that we are moving if we are in create link mode */
	private AssetTreeNode mover;
	
	/** The tree path of the asset that we are moving */
	private TreePath moverPath;
	
	/** the new parent path to where we will create the new link */
	private TreePath newParentPath;
	
	/** the new asset index for the new link */
	private int newAssetIndex = 0;
	
	/** a constant indicating that we are moving this asset */
	public static final int ASSET_MOVE = 1;
	
	/** a constant indicating that we are create a new link*/
	public static final int ASSET_NEW_LINK = 2;
	
	/** a constant indicating that we are cloning this asset */
	public static final int ASSET_CLONE = 3;
	
	/** a rectangle with no space */
	public static final Rectangle VOID_RECTANGLE = new Rectangle(0,0,0,0);
	
	/** the distance from the parent where the cue line will be drawn */
	public static final int CUE_LINE_Y_FROM_PARENT = 3;
	
	/** the length of each size of the cue triangle */
	public static final int CUE_TRIANGLE_SIZE = 4;
	
	/** 
	 * the offset that indicates that the node that we are curerntly over
	 * is the parent of this branch 
	 */
	public static final int NODE_OVER_AS_PARENT_OFFSET = 8;
	
	/** the cursor that is shown when the mouse is over the asset's icon */
	public static final Cursor ICON_OVER_CURSOR = new Cursor(Cursor.HAND_CURSOR);
	
	/** the default cursor */
	public static final Cursor DEFAULT_CURSOR = new Cursor(Cursor.DEFAULT_CURSOR);
	
	/** TRUE if the node has been expanded */
	private boolean expandRequest = false;
	
	/** the dash used when an asset is being moved within a branch */
	private final static float dash1[] = {2.0f};
	
	/** the stroke used when an asset is being moved within a branch*/
	private final static BasicStroke dashedStoke = new BasicStroke(1.0f, 
            BasicStroke.CAP_BUTT, 
            BasicStroke.JOIN_MITER, 
            10.0f, dash1, 0.0f);

	
	/**
	 * Constructs an new <code>ComplexAssetTree</code>
	 * @param model the asset tree model
	 */
	public ComplexAssetTree(TreeModel model) {
		super(model);
		addMouseMotionListener(this);
	}

	/**
	* Shows the asset action menu for an asset at the specified coordinates.
	* 
	* @param asset The asset for which to print out
	* @param x The x-coordinate in the <code>AssetTree</code>'s 
	* coordinate-space
	* @param y The y-coordinate in the <code>AssetTree</code>'s 
	* coordinate-space
	*/
	private void showAssetMenu(final AssetTreeNode node, int x, int y) {
		
		final Asset asset = node.getAsset();
		JPopupMenu menu = new JPopupMenu(asset.getName());
		
		AssetType type = asset.getType();
		Iterator screens = type.getScreenNames();
		
		  /////////////
		 // SCREENS //
		/////////////
		
		ActionListener screenListener = new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				
				String command = e.getActionCommand();
				URL url = null;
				
				try {
					String assetPathStr 
						= MatrixToolkit.rawUrlEncode(getAssetPath(node), true);
					String linkPathStr  
						= MatrixToolkit.rawUrlEncode(getLinkPath(node), true);
					
					url = new URL(MySource.INSTANCE.getBaseURL() + 
							AssetMap.getApplet().getParameter("BACKEND_SUFFIX") + 
							"/?SQ_BACKEND_PAGE=main&backend_section=" +
							"am&am_section=edit_asset&assetid=" + asset.getId() + 
							"&sq_asset_path=" + assetPathStr + "&sq_link_path=" +
							linkPathStr + "&asset_ei_screen=" + command);
					
				} catch (MalformedURLException mue) {
					System.out.println("Could not get Screen URL: " 
						+ mue.getMessage());
				}
				
				ComplexAssetMap.getUrl(url);
			}
		};
		
		
		while (screens.hasNext()) {
			AssetTypeScreen screen = (AssetTypeScreen) screens.next();
			JMenuItem item = new JMenuItem(screen.screenName);
			item.addActionListener(screenListener);
			item.setActionCommand(screen.codeName);
			item.setFont(MENU_FONT);
			item.setIcon(AssetManager.INSTANCE.getAssetType("thumbnail").getIcon());
			
			menu.add(item);
		}
		
		menu.addSeparator();
		
		  ////////////
		 // EXTRAS //
		////////////
		
		final JMenuItem previewItem = new JMenuItem("Preview");
		final JMenuItem teleportItem = new JMenuItem("Teleport");
		final JMenuItem refreshItem = new JMenuItem("Refresh");
		
		ActionListener extrasListener = new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				if (e.getSource().equals(previewItem)) {
					try {
						ComplexAssetMap.getUrl(new URL(node.getPreviewURL()), "_blank");
					} catch (MalformedURLException mue) {
						System.out.println("Could not get Preview URL: " + mue.getMessage());
					}
				} else if (e.getSource().equals(teleportItem)) {
					teleportToRoot(node);
				} else if (e.getSource().equals(refreshItem)) {
					System.out.println("calling reload from TREE");
					AssetManager.INSTANCE.reloadAsset(node.getAsset().getId());
				}
			}
		};
		
		if (!(node.getPreviewURL().equals(""))) 
			previewItem.addActionListener(extrasListener);
		else 
			previewItem.setEnabled(false);
		
		if (!(node.isLeaf()) && (node != getModel().getRoot()))
			teleportItem.addActionListener(extrasListener);
		else
			teleportItem.setEnabled(false);
		
		refreshItem.addActionListener(extrasListener);
		
		previewItem.setFont(MENU_FONT);
		teleportItem.setFont(MENU_FONT);
		refreshItem.setFont(MENU_FONT);
		
		previewItem.setIcon(AssetManager.INSTANCE.getAssetType("thumbnail").getIcon());
		teleportItem.setIcon(AssetManager.INSTANCE.getAssetType("thumbnail").getIcon());
		refreshItem.setIcon(AssetManager.INSTANCE.getAssetType("thumbnail").getIcon());
		
		menu.add(teleportItem);
		menu.add(previewItem);
		menu.add(refreshItem);
	
		menu.show(this, x, y);
	}
	
	/**
	 * Shows the create menu, which consists of a ok and cancel option
	 * 
	 * @param parent the parent where we are creating under
	 * @param typeCode the typecode of the asset type that we are creating
	 * @param index the index where the asset is to be created
	 * @param x the x pos where to show the menu
	 * @param y the y pos where to show the menu
	 */
	public void showCreateMenu(
			final AssetTreeNode parent,
			final String typeCode,
			final int index,
			int x,
			int y) {
	
		JPopupMenu createMenu = new JPopupMenu();
		
		Action createAction = new AbstractAction() {
			public void actionPerformed(ActionEvent e) {
				AssetManager.INSTANCE.addAsset(parent, typeCode, index);
			}
		};
		Action cancelAction = new AbstractAction() {
			public void actionPerformed(ActionEvent e) {
			}
		};
		
		createAction.putValue(Action.NAME, "Create Here");
		cancelAction.putValue(Action.NAME, "Cancel");
		
		JMenuItem createItem = new JMenuItem(createAction);
		JMenuItem cancelItem = new JMenuItem(cancelAction);
		
		createMenu.add(createItem);
		createMenu.add(cancelItem);
		
		createMenu.show(this, x, y);
		
	}
	
	/**
	 * Set the specified node as the root node of the tree
	 * 
	 * @param node the node to set as the root node
	 */
	public void teleportToRoot(AssetTreeNode node) {
		if (!(node.getAsset().childrenLoaded())) {
			try {
				AssetManager.INSTANCE.loadChildAssets(node);
			} catch(IOException ioe) {
				throwVisibleError("Teleport Failed", 
						"Could not teleport " + node.getAsset().getName() 
						+ " to the root position: " + ioe.getMessage());
			}
		}
		((DefaultTreeModel) getModel()).setRoot((TreeNode) node);
		setRootVisible(true);
	}
	
	/**
	 * Restores the root node back to the root folder
	 */
	public void restoreRoot() {
		((DefaultTreeModel) getModel()).setRoot(AssetManager.INSTANCE.getRootNode());
		setRootVisible(false);
	}

	/**
	 * Shows the new link menu at the specified co-ordinates
	 * 
	 * @param mover the node that is being moved
	 * @param parent the parent node to create the link under
	 * @param index the index where to create the link
	 * @param x the x pos where to show the menu
	 * @param y the y pos where to show the menu
	 */
	public void showNewLinkMenu(
		final AssetTreeNode mover, 
		final AssetTreeNode parent, 
		final int index,
		int x, 
		int y) {
		
		JPopupMenu menu = new JPopupMenu();
		menu.setFont(MENU_FONT);
		
		final JMenuItem moveMenuItem    = new JMenuItem("Move \"" 
				+ mover.getAsset().getName() + "\" here");
		final JMenuItem newLinkMenuItem = new JMenuItem("New Link here");
		final JMenuItem cloneMenuItem   = new JMenuItem("Clone here");
		final JMenuItem cancelMenuItem  = new JMenuItem("Cancel");
	
		ActionListener listener = new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				String type = null;
				if (e.getSource().equals(moveMenuItem)) {
					type = AssetManager.LINK_TYPE_MOVE;
				} else if (e.getSource().equals(newLinkMenuItem)) {
					type = AssetManager.LINK_TYPE_NEW_LINK;
				} else if (e.getSource().equals(cloneMenuItem)) {
					type = AssetManager.LINK_TYPE_CLONE;
				} else if (e.getSource().equals(cancelMenuItem)) {
					if (createLinkMode)
						clearCueImage();
				}
				
				if (type != null)
					AssetManager.INSTANCE.createLink(type, mover, parent, index);
			}
		};
		
		moveMenuItem.setFont(MENU_FONT);
		newLinkMenuItem.setFont(MENU_FONT);
		cloneMenuItem.setFont(MENU_FONT);
		cancelMenuItem.setFont(MENU_FONT);
		
		moveMenuItem.addActionListener(listener);
		newLinkMenuItem.addActionListener(listener);
		cloneMenuItem.addActionListener(listener);
		cancelMenuItem.addActionListener(listener);
		
		menu.add(moveMenuItem);
		menu.add(newLinkMenuItem);
		menu.add(cloneMenuItem);
		menu.addSeparator();
		menu.add(cancelMenuItem);
		
		menu.show(this, x, y);
	}
	
	/**
	 * Draws a cueLine under the specified path.
	 * 
	 * @param path the path to draw the cue line for
	 * @param pathIsNewParent indicates that the current position
	 * 		is in the threshold that will display an arrow, but does
	 * 		not necessary indicate that the node will not be added to 
	 * 		the path's children if </code>pathIsNewParent</code>
	 * 	 	is <code>False</code>
	 * @see #setCueLineColour
	 * @see #setCueLineThickness
	 */
	private void drawCueLineForPath(TreePath path, boolean pathIsNewParent) {
		if (path == null)
			throw new IllegalArgumentException("path is null");
		
		Rectangle pathBounds = getPathBounds(path);
		Rectangle mapBounds = getBounds();
		
		Graphics2D g2D = (Graphics2D) getGraphics();
		g2D.setColor(CUE_LINE_COLOUR);
		
		// the co-ordinates at where the line should start
		int xoffset = 0, yoffset = 0;
		
		if (pathIsNewParent) {
			setNewAssetPosition(path, true, false);
			xoffset = pathBounds.x + pathBounds.width + 3;
			yoffset = pathBounds.y + (pathBounds.height / 2);
			
			// draw an arrow head at the end of the cue line
			// to indicate that the current position will under this node
			
			Polygon arrow = new Polygon();
			arrow.addPoint(xoffset, yoffset + CUE_TRIANGLE_SIZE);
			arrow.addPoint(xoffset - CUE_TRIANGLE_SIZE, yoffset);
			arrow.addPoint(xoffset, yoffset - CUE_TRIANGLE_SIZE);
			
			g2D.fillPolygon(arrow);
			
			cueImageBounds.setRect(xoffset, yoffset, getWidth(), 1);
			cueImageBounds = cueImageBounds.union(arrow.getBounds());
		
		} else {
		
			// if the mode that we are currently at is expanded, 
			// then it makes more sense that we want to put this
			// under that node (a child) at the first position. So the 
			// x co-ordinate of the line will only start at the same
			// position of the current first child node of this parent
			
			if (isExpanded(path)) {
				setNewAssetPosition(path, true, true);
				AssetTreeNode node = (AssetTreeNode) 
						((AssetTreeNode) path.getLastPathComponent()).getChildAt(0);
				
				xoffset = getPathBounds(path.pathByAddingChild(node)).x - 13;
			} else {
				setNewAssetPosition(path, false, false);
				xoffset = pathBounds.x - 13;
			}
			
			yoffset = pathBounds.y + pathBounds.height;
			
			// if we are not pointing to a root of a branch then
			// we are going to draw a dashed line
			
			g2D.setStroke(dashedStoke);
			
			cueImageBounds.setRect(xoffset, yoffset, getWidth(), 1);
		}

		g2D.drawLine(xoffset, yoffset, mapBounds.width - mapBounds.x, yoffset);
		g2D.dispose();
	}
	
	/**
	 * Returns the last bounding rectangle where the cue line was drawn.
	 * useful for repainting over that area
	 * 
	 * @return the rectangle countaining the bounds of the last cue line
	 */
	private Rectangle getLastCueLineBounds() {
		return (cueImageBounds != VOID_RECTANGLE) 
			? cueImageBounds 
			: VOID_RECTANGLE; 
	}
	
	/**
	 * Draws a cueline for the spcecifed location based of the position 
	 * of the node at or near that location. If the exact location 
	 * @param x
	 * @param y
	 * @param exactPath
	 */
	private void drawCueLineForLocation(int x, int y, boolean exactPath) {
		TreePath path = null;
		if (exactPath)
			path = getPathForLocation(x, y);
		else 
			path = getClosestPathForLocation(x, y);
		if (path != null) {
			Rectangle bounds = getPathBounds(path);
			boolean pathIsNewParent = false;
			if (!(bounds.y + bounds.height - NODE_OVER_AS_PARENT_OFFSET < y))
				pathIsNewParent = true;
			drawCueLineForPath(path, pathIsNewParent);
		}
	}
	
	/**
	 * Set the position of where the new asset will be created
	 * 
	 * @param path the path to the parent
	 * @param isParent if position is parent of the branch
	 * @param firstNode if this is the first node in the branch
	 */
	private void setNewAssetPosition(
			TreePath path, 
			boolean isParent, 
			boolean firstNode) {
		
		if (isParent) {
			newParentPath = path;
			MutableTreeNode node = (MutableTreeNode) path.getLastPathComponent();
			newAssetIndex = (firstNode) ? 0 : node.getChildCount();
		} else {
			newParentPath = path.getParentPath();
			MutableTreeNode siblingNode 
				= (MutableTreeNode) path.getLastPathComponent();
			newAssetIndex = ((AssetTreeNode) 
					newParentPath.getLastPathComponent()).getIndex(siblingNode) + 1;
		}
	}
	
	/**
	 * Returns the bounds of the Icon that belongs to the 
	 * cell of the node at the specified <code>TreePath</code>
	 * 
	 * @param path the <code>TreePath</code>of the wanted cell to whom
	 * the <code>Icon</code> belongs
	 * @return the Icons bounds
	 */
	public Rectangle getNodeIconBounds(TreePath path) {
		if (path == null)
			throw new IllegalArgumentException("path is null");
		TreeCellRenderer renderer = getCellRenderer();
		
		if (!(path.getLastPathComponent() instanceof AssetTreeNode))
			return null;
		
		AssetTreeNode node = (AssetTreeNode) path.getLastPathComponent();
		
		JLabel cell = (JLabel) renderer.getTreeCellRendererComponent(
				this, 
				node,
				false,
				isExpanded(path),
				node.isLeaf(),  
				0,
				true
		);

		Rectangle bounds = getPathBounds(path);
		Icon icon = cell.getIcon();
		if (icon == null)
			throw new NullPointerException("Node for specified path has no icon");
		bounds.setSize(icon.getIconWidth(), icon.getIconHeight());
		
		return bounds;
	}
	
	/**
	 * Returns TRUE if the specified point is within the icon's bounds
	 * 
	 * @param path the path to the node
	 * @param point the point to check
	 * @return TRUE if the point is within the icon's bounds
	 */
	private boolean nodeIconContainsPoint(TreePath path, Point point) {
		Rectangle bounds = getNodeIconBounds(path);
		if (bounds == null)
			return false;
		return (bounds.contains(point));
	}
	
	/**
	 * Returns TRUE if the specified point is within the icon's bounds
	 * 
	 * @param path the path to the node
	 * @param point the point to check
	 * @return TRUE if the point is within the icon's bounds
	 */
	private boolean nodeIconContainsPoint(Point point) {
		TreePath path = getPathForLocation(point.x, point.y);
		if (path == null)
			return false;
		return nodeIconContainsPoint(path, point);
	}
	
	/**
	 * Called when a mouse click event is fired
	 */
	public void mouseClicked(MouseEvent e) {
		if (expandRequest) {
			expandRequest = false;
			return;
		}
			
		if (assetFinderMode() || createLinkMode)
			return;
		
		TreePath path = getPathForLocation(e.getX(), e.getY());
		
		if (path != null) {
			if (nodeIconContainsPoint(path, e.getPoint())) {
				createLinkMode = true;
				moverPath = path;
				mover = (AssetTreeNode) path.getLastPathComponent();
				
			}
		}
	}

	/**
	 * Called when the mouse is released
	 */
	public void mouseReleased(MouseEvent e) {
		if (assetFinderMode()) {
			super.mouseReleased(e);
			return;
		}
		
		if (expandRequest) {
			expandRequest = false;
			return;
		}
		if (!nodeIconContainsPoint(e.getPoint())) {
			if (createAssetMode){
				
				showCreateMenu(
					getNewParentNode(), 
					getNewAssetTypeCode(), 
					getNewAssetIndex(), 
					e.getX(), 
					e.getY()
				);
				exitCreateAssetMode();
			
			} else if (createLinkMode) {
				showNewLinkMenu(
						mover, getNewParentNode(), getNewAssetIndex(), e.getX(), e.getY());
				exitNewLinkMode();
			} else {
				if ((e.getModifiers() & MouseEvent.BUTTON3_MASK) 
						!= MouseEvent.BUTTON3_MASK)
					return;
				TreePath clickPath = getSelectionPath();
				if (clickPath != null) {
					AssetTreeNode node = (AssetTreeNode) clickPath.getLastPathComponent();
					if (node.getAsset().isAccessible())
						showAssetMenu(node, e.getX(), e.getY());
				}
			}
		}
	}
	
	/**
	 * Called when the tree is set to expand
	 */
	public void treeWillExpand(TreeExpansionEvent e) 
	throws ExpandVetoException {
		super.treeWillExpand(e);
		expandRequest = true;
	}
	
	/**
	 * Called when the tree is set to collapse
	 */
	public void treeWillCollapse(TreeExpansionEvent e) {
		super.treeWillCollapse(e);
		expandRequest = true;
	}
	
	/**
	 * initlialises the node asset and enters create asset mode
	 * 
	 * @param type the type of asset to create
	 */
	public void initNewAsset(AssetType type) {
		setNewAssetTypeCode(type.getTypeCode());
		createAssetMode = true;
	}

	/**
	 * Exists create new link mode
	 */
	public void exitNewLinkMode() {
		createLinkMode = false;
		clearCueImage();
	}
	
	/**
	 * Removes the cue image from the canvas
	 */
	protected void clearCueImage() {
		if (cueImageBounds != VOID_RECTANGLE)
			paintImmediately(cueImageBounds);
	}
	
	
	/**
	 * Exits create Asset Mode
	 */
	protected void exitCreateAssetMode() {
		clearCueImage();
		createAssetMode = false;
		newAssetTypeCode = null;
		newAssetIndex = 0;
	}
	
	/**
	 * Sets the cursor for the location of the mouse. If the mouse
	 * is over an icon, the ICON_OVER_CURSOR is used
	 *  
	 * @param point the current mouse point 
	 */
	protected void setCursorForLocation(Point point) {
		if (nodeIconContainsPoint(point) || createAssetMode) {
			if (getCursor() != ICON_OVER_CURSOR)
				setCursor(ICON_OVER_CURSOR);
		} else if ((!createLinkMode || !createAssetMode)) {
			if (getCursor() == ICON_OVER_CURSOR)
				setCursor(DEFAULT_CURSOR);
		}
	}
	
	/**
	 * Returns the path to where the new asset will be created under
	 * 
	 * @return the new parent path
	 */
	public TreePath getNewParentPath() {
		return newParentPath;
	}
	
	/**
	 * Returns the node where the new asset will be created under
	 * 
	 * @return the new parent node
	 */
	public AssetTreeNode getNewParentNode() {
		return (AssetTreeNode) getNewParentPath().getLastPathComponent();
	}
	
	/**
	 * Returns the index where the new asset will be created
	 * 
	 * @return the new asset index
	 */
	public int getNewAssetIndex() {
		return newAssetIndex;
	}
	
	/**
	 * Sets the type code of the new asset that will be created
	 * 
	 * @param typeCode the new asset type code
	 */
	public void setNewAssetTypeCode(String typeCode) {
		this.newAssetTypeCode = typeCode;
	}
	
	/**
	 * Returns the new asset type code
	 * 
	 * @return the new asset type code
	 */
	public String getNewAssetTypeCode() {
		return newAssetTypeCode;
	}
	
	
	public void mouseDragged(MouseEvent e) {}
	
	public void mouseMoved(MouseEvent e) {
		
		// if we are over an image we want
		// to set the cursor to a hand
		setCursorForLocation(e.getPoint());
		
		if (!createLinkMode && !createAssetMode)
			return;
			
		clearCueImage();
		drawCueLineForLocation(e.getX(), e.getY(), false);
	
	}
	
	public void assetFinderStarted(JsEvent e) {
		super.assetFinderStarted(e);
	}
	
	public void assetFinderStopped(JsEvent e) {
		super.assetFinderStopped(e);
	}
	
	public void stop() {
		super.stop();
		createLinkMode = false;
		createAssetMode = false;
	}
	
}
