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
* $Id: AssetTree.java,v 1.2 2004/06/29 01:23:17 mmcintyre Exp $
* $Name: not supported by cvs2svn $
*/

package net.squiz.matrix.assetmap;

import java.util.*;
import javax.swing.*;
import javax.swing.event.*;
import javax.swing.tree.*;
import java.awt.event.*;
import java.awt.*;
import java.io.IOException;
import com.sun.java.swing.plaf.windows.*;


/**
 * The <code>AssetTree</code> class extends the <code>JTree</code> class
 * to provide a simple visual display of the Matrix <code>AssetTree</code>
 * structure. The <code>AssetTree</code> class also provides a mechanism
 * for delegating a particular <code>Asset</code> back to the Matrix System, 
 * (Via Javascript)
 *
 * @author Marc McIntyre <mmcintyre@squiz.net>
 * @see AssetTreeModelnet.squiz.matrix.assetmap.AssetTreeModel
 * @see MySource net.squiz.matrix.assetmap.MySource
 */
public class AssetTree extends JTree 
	implements TreeWillExpandListener, MouseListener, AssetFinderListener {
	
	/** if TRUE, we are in asset finder mode */
	private boolean inAssetFinderMode = false;
	
	/** The font to use in the use me menu */
	public static final Font MENU_FONT = new Font("menu_font", Font.PLAIN, 9);
	
	/** The current selected node */
	private MutableTreeNode selectedNode;
	
	/** The background color that is shown when in asset finder mode */
	public static final Color ASSET_FINDER_BG_COLOUR = new Color(0xE9D4F4);
	
	/**
	 * A set of type codes that the assetfinder mode 
	 * is restricted to.
	 */
	private Set restrictedTypes = new HashSet();
	
	/** if TRUE all statuses will be painted */
	private boolean paintAllStatuses = false;
	
	/**
	 * Constructs an <code>AssetTree</code> with the specified
	 * model
	 * 
	 * @param model the <code>TreeModel</code>
	 */
	public AssetTree(TreeModel model) {
		super(model);
	}
	
	/**
	 * Initialises the <code>AssetTree</code>
	 * Initialising the asset tree will perform the following tasks:
	 * <br>
	 * <ul>
	 *   <li>Sets The cell renderer to an <code>AssetTreeCellRenderer</code></li>
	 *   <li>Register the tooltip manager to display tooltips within the 
	 * <code>AssetTree</code></li>
	 *   <li>Registers a mouse listener for this component</li>
	 *   <li>Resisters a JSEvent listener to listen for requests to go into 
	 * Asset Finder Mode</li>
	 *   <li>Registers a <code>TreeWillExpandListener</code> to listen for 
	 * expand requests</li>
	 *   <li>Sets the <code>showRootHandles</code> JTree property to TRUE</li>
	 *   <li>Set the root <code>TreeNode</code> to be hidden</li>
	 * </ul>
	 */
	public void initialise() {
		UIManager.put("Tree.expandedIcon", new WindowsTreeUI.ExpandedIcon());
		UIManager.put("Tree.collapsedIcon", new WindowsTreeUI.CollapsedIcon());
		putClientProperty("JTree.lineStyle", "Angled");
		updateUI();
		
		setCellRenderer(new AssetTreeCellRenderer());
		ToolTipManager.sharedInstance().registerComponent(this);
		setRootVisible(false);
		setShowsRootHandles(true);
		addTreeWillExpandListener(this);
		addMouseListener(this);
		JsEventManager.sharedInstance().addJsListener("asset_finder", this);
	
	}
	
	/**
	 * Returns the path to the root node for the specified tree node
	 * 
	 * @param node the tree node of the wanted tree path
	 * @return the tree path to the root node
	 */
	public Object[] getPathToRoot(TreeNode node) {
		return ((DefaultTreeModel) getModel()).getPathToRoot(node);
	}
	
	/**
	 * Returns a comma seperated list of asset ids to the root node for the
	 * given tree node
	 * 
	 * @param node the node of the wanted asset path
	 * @return the command seperated asset path
	 */
	public String getAssetPath(MutableTreeNode node) {
		Object[] path = getPathToRoot(node);
		StringBuffer assetPath = new StringBuffer();
		for (int i = 0; i < path.length; i++) {
			assetPath.append(",").append(((AssetTreeNode) path[i]).getAsset().getId());
		}
		return assetPath.toString();
	}
	
	/**
	 * Returns a comma seperated list of linkids to the root node for the
	 * given tree node 
	 * 
	 * @param node the tree node of the wanted link path
	 * @return the link path for the specifed tree node
	 */
	public String getLinkPath(MutableTreeNode node) {
		Object[] path = getPathToRoot(node);
		StringBuffer linkPath = new StringBuffer();
		for (int i = 0; i < path.length; i++) {
			linkPath.append(",").append(((AssetTreeNode) path[i]).getLinkId());
		}
		return linkPath.toString();
	}
	
	/**
	 * Collapses all the paths below the root node. 
	 * If the Tree is currently in teleportation, the paths will be collapsed
	 * below the node that has been teleported to the root position.
	 */
	public void collapseAllPaths() {
		((DefaultTreeModel) getModel()).reload();
	}
	
	
	/**
	 * Returns the node that exists at the location. If no node exists within
	 * the given co-ordinates, null is returned
	 * 
	 * @param x the x co-ordinate
	 * @param y the y co-ordinate
	 * @return the node within the given co-ordinate
	 */
	public MutableTreeNode getNodeForLocation(int x, int y) {
		TreePath path = getPathForLocation(x, y);
		if (path == null)
			return null;
		return (MutableTreeNode) path.getLastPathComponent();
	}
	
	/**
	 * Returns the closest node at the given co-ordinate
	 * 
	 * @param x the x co-ordinate
	 * @param y the y co-ordinate
	 * @return the node closest to the given co-ordinate
	 */
	public MutableTreeNode getClosestNodeForLocation(int x, int y) {
		TreePath path = getClosestPathForLocation(x, y);
		if (path == null)
			return null;
		return (MutableTreeNode) path.getLastPathComponent();
	}
	
	/**
	 * Returns <code>True</code> if the tree is currently 
	 * in asset finder mode
	 *
	 * @return the current state of the tree
	 */
	public boolean assetFinderMode() {
		return inAssetFinderMode;
	}
	
	/**
	 * Sets the asset finder mode to the specified 
	 * <code>boolean</code>
	 *
	 * @param mode the mode to set
	 */
	public void setAssetFinderMode(boolean mode) {
		inAssetFinderMode = mode;
		
		if (mode == false) {
			clearRestrictedTypes();
			repaint();
		}
	}
	
	public boolean noTypesRestricted() {
		return restrictedTypes.isEmpty();
	}
	
	/**
	 * Returns a <code>Set</code> of type codes of
	 * the types that can be selected during asset finder mode.
	 * 
	 * @return the set of <code>AssetTypes</code> that can be selected
	 * during asset finder mode.
	 */
	public Set getRestrictedTypes() {
		return restrictedTypes;
	}
	
	/**
	 * Retruns TRUE if the specifed <code>AssetType</code> is currently in
	 * the set of restricted types. ie, it can be selected
	 * 
	 * @param type the type to check if 
	 * it is currently in the restricted <code>AssetTypes</code> 
	 * @return TRUE if the type is in the restricted set
	 */
	public boolean typeIsRestricted(AssetType type) {
		if (restrictedTypes.isEmpty())
			return true;
		return restrictedTypes.contains(type);
	}
	
	/**
	 * Sets the set of <code>AssetTypes</code> that can be
	 * selected during asset finder mode
	 * 
	 * @param restrictedTypes a set of type codes that define
	 * the <code>AssetTypes</code> that can be selected during
	 * asset finder mode.
	 */
	public void setRestrictedTypes(Set restrictedTypes) {
		this.restrictedTypes = restrictedTypes;
	}
	
	public void clearRestrictedTypes() {
		restrictedTypes.clear();
	}

	/**
	 * If the current assets under the node that was triggered to expand are not
	 * currently loaded, a request is made to the Matrix system for the children
	 * of that node.
	 * 
	 * @param e the <code>TreeExpansionEvent</code> that was triggered
	 * @throws ExpandVetoException if the children of the expaned node could not
	 * be loaded successfully
	 */
	public void treeWillExpand(TreeExpansionEvent e) throws ExpandVetoException {
		
		TreePath path = e.getPath();
		AssetTreeNode treeNode = (AssetTreeNode) path.getLastPathComponent();
		
		if (treeNode.getAsset().childrenLoaded()) {
			if (treeNode.getChildCount() == 0)
				AssetManager.INSTANCE.propagateChildren(treeNode);
		} else {
			try {
				AssetManager.INSTANCE.loadChildAssets(treeNode);
			} catch (IOException ioe) {
				String message = "Error while attempting to update asset "
					+ treeNode.getAsset().getId() + " : \n" + ioe.getMessage() + ": ";
				throwVisibleError("Error", message);
				ioe.printStackTrace();
				throw new ExpandVetoException(e);
			}
		}
	}
	
	/**
	 * Displays a visible error to the user in a <code>JOptionPane</code>.
	 * 
	 * @param title the title of the message that will be displayed
	 * in the title bar
	 * @param message the message to be displayed
	 */
	public void throwVisibleError(String title, String message) {
		JOptionPane.showMessageDialog(this, message, title, JOptionPane.ERROR_MESSAGE);
	}

	/**
	* Abstract Method
	*
	* @param e the tree expansion event
	*/
	public void treeWillCollapse(TreeExpansionEvent e) {}

	/**
	* Abstract Method
	*
	* @param e the mouse event
	*/
	public void mouseClicked(MouseEvent e) {}
	
	/**
	* Abstract Method
	*
	* @param e the mouse event
	*/
	public void mouseEntered(MouseEvent e) {}

	/**
	* Abstract Method
	*
	* @param e the mouse event
	*/
	public void mouseExited(MouseEvent e) {}
	
	/**
	 * Event Listener Method that is called when the mouse
	 * is depressed.
	 * 
	 * @param e The Mouse Event
	 */
	public void mousePressed(MouseEvent e) {
		
		TreePath clickPath = getPathForLocation(e.getX(), e.getY());
		
		if (clickPath != null) {
			selectedNode = (MutableTreeNode) clickPath.getLastPathComponent();
			setSelectionPath(clickPath);
		}
	}
	
	/**
	 * Shows the asset action menu when the right mouse button is released on 
	 * the tree for the currently selected path. 
	 * 
	 * @param e the mouse event
	 * @see MouseListener java.awt.event.MouseListener
	 */
	public void mouseReleased(MouseEvent e) {
		
		if (!(inAssetFinderMode))
			return;
		if ((e.getModifiers() & MouseEvent.BUTTON3_MASK) != MouseEvent.BUTTON3_MASK)
			return;
		TreePath path = getPathForLocation(e.getX(), e.getY());
		if (path == null)
			return;

		AssetTreeNode node = (AssetTreeNode) path.getLastPathComponent();
		if (!typeIsRestricted(node.getAsset().getType()))
			return;
		
		showUseMeMenu(node, e.getX(), e.getY());
	}
	
	/**
	 * Returns the selected that was delegated
	 * by mouse click
	 * 
	 * @return the selected Node
	 */
	public MutableTreeNode getLastSelectedNode() {
		return selectedNode;
	}
	
	/**
	 * An Event Listener methof that is called when
	 * An asset finder request os triggered from the Matrix System
	 * 
	 * @param e the js event
	 * @see #setAssetFinderMode
	 * @see #assetFinderMode
	 * @see #assetFinderStopped
	 * @see #getRestrictedTypes
	 * @see #setRestrictedTypes
	 */
	public void assetFinderStarted(JsEvent e) {
	
		startAssetFinderMode();
		
		String assetTypes = (String) e.getParams().get("type_codes");

		Set types = new HashSet();
		if (assetTypes != null) {
			StringTokenizer st = new StringTokenizer(assetTypes, "|");
			while (st.hasMoreTokens())
				types.add(AssetManager.INSTANCE.getAssetType((String) st.nextToken()));
		}

		if (!(types.isEmpty()))
			setRestrictedTypes(types);
		repaint();
	}

	/**
	 * Gets called when the asset finder is canceled
	 *
	 * @param e the javascript event
	 *
	 * @see <code>JsEventManager</code>
	 * @see <code>AssetFinderListener</code>
	 */
	public void assetFinderStopped(JsEvent e) {
		stopAssetFinderMode();
	}
	
	/**
	 * Starts asset finder mode where assets can be chosen for a particular action
	 */
	public void startAssetFinderMode() {
		setAssetFinderMode(true);
		setBackground(ASSET_FINDER_BG_COLOUR);
	}
	
	/**
	 * Stops asset finder mode
	 */
	public void stopAssetFinderMode() {
		setBackground(Color.white);
		setAssetFinderMode(false);
	}

	/**
	 * Shows the use me menu for the asset finder
	 * 
	 * @param node		The node that was selected to show the use me menu
	 * @param x			The x-coordinate in the <code>AssetTree</code>'s 
	 *	coordinate-space
	 * @param y			The y-coordinate in the <code>AssetTree</code>'s 
	 * 	coordinate-space
	 */
	private void showUseMeMenu(AssetTreeNode node, int x, int y) {
		
		Asset asset = node.getAsset();

		JPopupMenu menu = new JPopupMenu(asset.getName());
		menu.setFont(MENU_FONT);
		AssetType type = asset.getType();
		
		Action action = new AbstractAction() {
			public void actionPerformed(ActionEvent e) {
				
				// pack an array with vars to be passed back to the javascript
				String [] args = new String[4];
				args[0] = (String) getValue("assetid");
				args[1] = (String) getValue("label");
				args[2] = (String) getValue("url");
				args[3] = (String) getValue("linkid");
				// call the javascript method with the event type
				JsEventManager.getInstance().javaToJsCall("asset_finder", args);
				// reset the state of the asset finder
				
				stopAssetFinderMode();
			}
		};
		action.putValue(Action.NAME, "Use Me");
		action.putValue("assetid", asset.getId());
		action.putValue("url", node.getPreviewURL());
		action.putValue("label", asset.getName());
		action.putValue("linkid", node.getLinkId());

		JMenuItem item = new JMenuItem(action);
		menu.add(item);
		menu.show(this, x, y);

	}
	
	/**
	 * Stops the Asset Tree
	 */
	public void stop() {}
	
	/**
	 * Sets the paintAllStatuses property
	 * @param paintAllStatuses the boolean to set the paintAllStatuses property
	 * to
	 */
	public void setPaintAllStatuses(boolean paintAllStatuses) {
		this.paintAllStatuses = paintAllStatuses;
	}
	
	/**
	 * returns the <code>paintAllStatuses</code> property
	 * @return TRUE if the paintAllStatus properties is TRUE
	 */
	public boolean paintAllStatuses() {
		return paintAllStatuses;
	}

}
