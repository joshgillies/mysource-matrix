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
* $Id: MatrixTree.java,v 1.30 2009/01/06 05:07:38 bshkara Exp $
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
import net.squiz.matrix.debug.*;

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
 * The MatrixTree class is the main tree in the Matrix asset map.
 * @author Marc McIntyre <mmcintyre@squiz.net>
 */
public class MatrixTree extends CueTree
	implements CueGestureListener, TreeWillExpandListener, Draggable, Autoscroll {

	private MenuHandler menuHandler;
	private DragHandler dragHandler;
	private DropHandler dropHandler;
	private DoubleClickHandler dcHandler;
	private SelectionTool selTool;

	private String lastTypeCodeCreated = null;

	private BufferedImage dblBuffer = null;
	private DragSource dragSource = null;
	private boolean isInAssetFinderMode = false;
	private boolean hasExpandNextNode = false;
	private boolean hasExpandPrevNode = false;
	private static final int AUTOSCROLL_MARGIN = 12;

	public static final Color ASSET_FINDER_BG_COLOR = new Color(0xE9D4F4);

	//TODO: (MM) this is a dirty hack to get the menu to show up before
	// the cue tree knows anything about it
	private String multipleMoveType = null;
	private JTree tree = this;

	//{{{ Public Methods

	/**
	 * Returns a MatrixTree with some sample data.
	 */
	public MatrixTree() {
		super();
		addCueGestureListener(this);
	}

	/**
	 * Returns a MatrixTree constructed by the tree model.
	 * @param model the tree model to construct the tree with.
	 */
	public MatrixTree(TreeModel model) {
		super(model);

		selTool = new SelectionTool(this);

		menuHandler = getMenuHandler();
		dragHandler = getDragHandler();
		dropHandler = getDropHandler();
		dcHandler   = getDoubleClickHandler();

		addCueGestureListener(this);
		addTreeWillExpandListener(this);

		addMouseListener(selTool);
		addMouseListener(menuHandler);
		addMouseListener(dcHandler);
		addMouseMotionListener(selTool);

		setUI(new MatrixTreeUI());

		dragSource = DragSource.getDefaultDragSource();
		DragGestureRecognizer dgr =
			dragSource.createDefaultDragGestureRecognizer(
			this, // DragSource
			DnDConstants.ACTION_COPY_OR_MOVE, // specifies valid actions
			dragHandler // DragGestureListener
		);

		dgr.setSourceActions(dgr.getSourceActions() & ~InputEvent.BUTTON3_MASK);
		DropTarget dropTarget = new DropTarget(this, dropHandler);

		// create a mouse motion listener to update the ViewPort when we
		// do a drag operation where the drag extends greater than the tree size
		MouseMotionListener mmListener = new MouseMotionAdapter() {
			public void mouseDragged(MouseEvent evt) {
				Rectangle r = new Rectangle(evt.getX(), evt.getY(), 6, 6);
				scrollRectToVisible(r);
			}
		};
		setAutoscrolls(true);
		addMouseMotionListener(mmListener);
		ToolTipManager.sharedInstance().registerComponent(this);
		setKeyboardActions();

	}

	/**
	 * Autoscrolls to the specified point
	 * @param pt the point to scroll to
	 */
	public void autoscroll(Point pt) {
		int row = getRowForLocation(pt.x, pt.y);
		Rectangle bounds = getBounds();

		if (pt.y + bounds.y <= AUTOSCROLL_MARGIN) {
			if (row > 0) --row;
		} else {
			if (row < getRowCount() - 1) ++row;
		}
		scrollRowToVisible(row);
	}

	/**
	 * Returns the Insets for use during autoscrolling
	 * @return the Insets for use during autoscrolling
	 */
	public Insets getAutoscrollInsets() {
		Rectangle outer = getBounds();
		Rectangle inner = getParent().getBounds();

		return new Insets(inner.y - outer.y + AUTOSCROLL_MARGIN,
			inner.x - outer.x + AUTOSCROLL_MARGIN,
			outer.height - inner.height - inner.y + outer.y + AUTOSCROLL_MARGIN,
			outer.width - inner.width - inner.x + outer.x + AUTOSCROLL_MARGIN);
	}

	/**
	 * Adds a NewLinkListener to listen for new link events
	 * @param l the NewLinkListener to add
	 */
	public void addNewLinkListener(NewLinkListener l) {
		listenerList.add(NewLinkListener.class, l);
	}

	/**
	 * Removes a NewLinkListener
	 * @param l the NewLinkListener to remove
	 */
	public void removeNewLinkListener(NewLinkListener l) {
		listenerList.remove(NewLinkListener.class, l);
	}

	/**
	 * Adds a NewAssetListener to listen for new asset events
	 * @param l the NewLinkListener to add
	 */
	public void addNewAssetListener(NewAssetListener l) {
		listenerList.add(NewAssetListener.class, l);
	}

	/**
	 * Removes a NewAssetListener
	 * @param l the NewAssetListener to remove
	 */
	public void removeNewAssetListener(NewAssetListener l) {
		listenerList.remove(NewAssetListener.class, l);
	}

	/**
	* Adds a NodeDoubleClickedListener
	* @param cl the listener
	*/
	public void addNodeDoubleClickedListener(NodeDoubleClickedListener cl) {
		listenerList.add(NodeDoubleClickedListener.class, cl);
	}

	/**
	* Removes a NodeDoubleClickedListener
	* @param cl the listener
	*/
	public void removeNodeDoubleClickedListener(NodeDoubleClickedListener cl) {
		listenerList.remove(NodeDoubleClickedListener.class, cl);
	}

	/**
	 * Returns the nodes in the current selection.
	 * @return the nodes in the current selection, or null if there are
	 * no nodes in the current selection
	 * @see #getSelectionNode()
	 */
	public MatrixTreeNode[] getSelectionNodes() {
		TreePath[] paths = getSelectionPaths();
		if (paths == null)
			return null;
		MatrixTreeNode[] nodes = new MatrixTreeNode[paths.length];
		for (int i = 0; i < paths.length; i++)
			nodes[i] = (MatrixTreeNode) paths[i].getLastPathComponent();
		return nodes;
	}

	/**
	 * Returns the first node that is currently selected in tree
	 * @return the first node that is selected in the tree, or null
	 * if there are no nodes currently selected
	 * @see #getSelectionNodes()
	 */
	public MatrixTreeNode getSelectionNode() {
		TreePath path = getSelectionPath();
		if (path == null)
			return null;
		return (MatrixTreeNode) path.getLastPathComponent();
	}

	/**
	 * Returns TRUE if there more than one node selected, FALSE otherwise
	 * @return TRUE if there more than one node selected, FALSE otherwise
	 * @see #isEmptySelection()
	 */
	public boolean isMultipleSelection() {
		TreePath[] paths = getSelectionPaths();
		return (paths != null && paths.length > 1);
	}

	/**
	 * Returns TRUE if no nodes are currently selected
	 * @return TRUE if no nodes are currently selected
	 * @see #isMultipleSelection()
	 */
	public boolean isEmptySelection() {
		return (getSelectionPath() == null);
	}

	/**
	 * Returns the TreePath for the specfied node
	 * @param node the node of the wanted TreePath
	 * @return the TreePath for the specified node
	 */
	public TreePath getPathToRoot(MatrixTreeNode node) {
		Object[] nodes = ((DefaultTreeModel) getModel()).getPathToRoot(node);
		if (nodes == null)
			return null;
		return new TreePath(nodes);
	}

	/**
	 * Returns an array of TreePaths for the specifed List of TreePaths
	 * @param paths the List of TreePaths
	 * @return the array of TreePaths
	 */
	public TreePath[] pathsToArray(java.util.List paths) {
		return (TreePath[]) paths.toArray(new TreePath[paths.size()]);
	}

	/**
	 * Starts asset finder mode
	 * @see stopAssetFinderMode()
	 */
	public void startAssetFinderMode() {
		isInAssetFinderMode = true;
		setBackground(ASSET_FINDER_BG_COLOR);

		removeMouseListener(selTool);
		removeMouseMotionListener(selTool);
	}

	/**
	 * Stops asset finder move
	 * @see startAssetFinderMode()
	 */
	public void stopAssetFinderMode() {
		isInAssetFinderMode = false;
		setBackground(Color.WHITE);

		addMouseListener(selTool);
		addMouseMotionListener(selTool);
	}

	/**
	 * Creates a selection from the currently selected nodes in the tree.
	 * Any nodes in the current selection are replaced with the currently
	 * selected nodes.
	 */
	public void createSelection() {

		MatrixTreeNode[] nodes = getSelectionNodes();
		if (nodes == null)
			return;
		Selection.setNodes(nodes);
		if (MatrixDialog.hasDialog(SelectionDialog.class)) {
			SelectionDialog selectionDialog
				= (SelectionDialog) MatrixDialog.getDialog(SelectionDialog.class);
			selectionDialog.setNodes(nodes);
		}
	}

	/**
	 * Event listener method that is triggered when an expansion event is recognized
	 * @param evt the TreeExpansionEvent
	 */
	public void treeWillExpand(TreeExpansionEvent evt) {
		TreePath path = evt.getPath();
		MatrixTreeNode treeNode = (MatrixTreeNode) path.getLastPathComponent();

		// if user does not have access to this asset then they cannot see its kids
		if (treeNode.getAsset().isAccessible()) {
			loadChildAssets(treeNode);
		}
	}

	/**
	* Returns true if parent of this node is the root node
	*
	* @param parent
	*/
	public boolean parentIsRoot(MatrixTreeNode child) {
		MatrixTreeNode parent = (MatrixTreeNode)getModel().getRoot();
		if (parent == child.getParent()) {
			return true;
		}
		return false;
	}

	/**
	* Returns true if this node is the root node
	*
	* @param parent
	*/
	public boolean nodeIsRoot(MatrixTreeNode node) {
		MatrixTreeNode parent = (MatrixTreeNode)getModel().getRoot();
		if (parent == node) {
			return true;
		}
		return false;
	}

	/**
	* Returns true if parent node has Previous button (node)
	*
	* @param parent
	*/
	public boolean hasPreviousNode(MatrixTreeNode parent) {
		if (parent.getChildCount() != 0) {
			MatrixTreeNode node = (MatrixTreeNode)parent.getChildAt(0);
			if (node instanceof ExpandingPreviousNode) {
				return true;
			}
		}
		return false;
	}

	/**
	* Returns true if parent node has Next button (node)
	*
	* @param parent
	*/
	public boolean hasNextNode(MatrixTreeNode parent) {
		if (parent.getChildCount() != 0) {
			MatrixTreeNode node = (MatrixTreeNode)parent.getChildAt(parent.getChildCount()-1);
			if (node instanceof ExpandingNextNode) {
				return true;
			}
		}
		return false;
	}

	/**
	 * Returns the tree node for a given assetid, if not found returns null
	 *
	 * @param parent
	 * @param assetid
	 * @param incChildren Set this to true if you want to search recursively
	 */
	private MatrixTreeNode findAssetUnderParent(MatrixTreeNode parent, String assetid, boolean incChildren) {
		try {
			boolean found = false;
			int childCount = 0;
			int loc = 0;

			while (!found) {
				if (loc != 0 || parent.getChildCount() == 0) {
					removeChildNodes(parent);
					AssetManager.refreshAsset(parent, "", loc, -1);
				}
				childCount = parent.getChildCount();
				if (childCount > 0) {
					for (Enumeration children = parent.children(); children.hasMoreElements();) {
						MatrixTreeNode nextNode = (MatrixTreeNode) children.nextElement();
						if (nextNode.getAsset().getId().equals(assetid)) {
							return nextNode;
						} else if (incChildren) {
							// look under this node
							nextNode = findAssetUnderParent(nextNode, assetid, true);
							if (nextNode != null) {
								return nextNode;
							}
						}
					}
				} else {
					// not found
					return null;
				}
				if (childCount >= AssetManager.getLimit()) {
					// look at the next set of assets
					loc += AssetManager.getLimit();
				} else {
					return null;
				}
			}
		} catch (IOException ex) { }

		return null;
	}


	/**
	* Works like original loadChildAssets but uses asset ids to locate nodes in the tree
	*
	* @param assetids		Ids of the assets that are in the lineage of the asset that we are searching for
	* @param sort_orders	Sort orders of the assets. This will make search quicker
	* @param selectAll		Selects all the nodes that are in the lineage
	* @param teleport		Teleport to the last selected node (i.e. searched asset)
	*/
	public void loadChildAssets(final String[] assetids, final String[] sort_orders, final boolean selectAll, final boolean teleport) {
		MatrixStatusBar.setStatus(Matrix.translate("asset_map_status_bar_requesting"));
		Runnable runner = new Runnable() {
			public void run() {

				// if we have a different root and not the actual root then this will not work
				// or should it?
				tree.setRootVisible(false);
				if (((DefaultTreeModel) tree.getModel()).getRoot() != AssetManager.getRootFolderNode()) {
					((DefaultTreeModel) tree.getModel()).setRoot(AssetManager.getRootFolderNode());
				}

				MatrixTreeNode parent = AssetManager.getRootFolderNode();
				int numAssets = assetids.length;
				int level = 0;
				int loadedNodes = 0;
				boolean found = false;
				TreePath path = null;

				try {
					// clear all other selections
					tree.clearSelection();

					int sort_order = 0;
					int totalKids = 0;

					TreePath[] paths = new TreePath[numAssets+1];
					paths[0] = path;

					while (level < numAssets) {

						found = false;
						Asset asset = parent.getAsset();
						totalKids = asset.getTotalKidsLoaded();
						sort_order = Integer.parseInt(sort_orders[level]);

						if (!sort_orders[level].equals("-1")) {
							int modifier = (int)(sort_order/AssetManager.getLimit());
							int loc = 0;
							if (parent.getChildCount() == 0 || (AssetManager.getLimit()*modifier != totalKids)) {
								for (int i=(AssetManager.getLimit()*modifier); i > 0; i--) {
									if (parent.getAsset().getNumKids() >= AssetManager.getLimit()*modifier) {
										break;
									} else {
										modifier--;
									}
								}
								if (parent.getAsset().getNumKids() >= AssetManager.getLimit()*modifier) {
									// load another set of assets
									removeChildNodes(parent);
									AssetManager.refreshAsset(parent, "", AssetManager.getLimit()*modifier, -1);
									loadedNodes += parent.getChildCount();
								}
							}

							if (parent.getChildCount() > 0) {
								loc += (sort_order%AssetManager.getLimit());

								if (loc >= parent.getChildCount()) {
									loc = parent.getChildCount()-1;
								} else if (loc < 0) {
									loc = 0;
								}

								MatrixTreeNode foundChild = null;
								foundChild = (MatrixTreeNode)parent.getChildAt(loc);
								if (foundChild.getAsset().getId().equals(assetids[level])) {
									found = true;
									parent = foundChild;
								} else {

									for (int i = 0; i < parent.getChildCount(); i++) {
										foundChild = (MatrixTreeNode)parent.getChildAt(i);
										if (foundChild.getAsset().getId().equals(assetids[level])) {
											found = true;
											parent = foundChild;
											break;
										}
									}
								}
							}
						} else {
							parent = findAssetUnderParent(parent, assetids[level], false);
							if (parent != null) {
								found = true;
							}
						}

						if (!found) {
							break;
						} else {
							path = getPathToRoot(parent);
							paths[level] = path;
							level++;
						}
					}

					// scroll to the last selected node
					if (selectAll) {
						tree.addSelectionPaths(paths);
					} else {
						tree.addSelectionPath(path);
					}

					if (teleport) {
						teleportToRoot((MatrixTreeNode)path.getLastPathComponent());
					}

					scrollPathToVisible(path);



					if (!found) {
						// asset not found
						MatrixStatusBar.setStatusAndClear(Matrix.translate("asset_map_status_bar_requesting"), 1000);
						Object[] transArgs = {
							assetids[level]
						};
						String message = Matrix.translate("asset_map_error_locate_asset", transArgs);
						GUIUtilities.error(message, Matrix.translate("asset_map_dialog_title_error"));
						return;
					}

				} catch (Exception ioe) {
					MatrixStatusBar.setStatusAndClear(Matrix.translate("asset_map_status_bar_requesting"), 1000);
					Object[] transArgs = {
						ioe.getMessage()
					};
					String message = Matrix.translate("asset_map_error_loading_children", transArgs);
					GUIUtilities.error(message, Matrix.translate("asset_map_dialog_title_error"));
					Log.log(message, MatrixTree.class, ioe);
					return;
				}

				if (loadedNodes == 1) {
					MatrixStatusBar.setStatusAndClear(Matrix.translate("asset_map_status_bar_loaded_child"), 3000);
				} else {
					Object[] transArgs = {
						new Integer(loadedNodes)
					};
					MatrixStatusBar.setStatusAndClear(Matrix.translate("asset_map_status_bar_loaded_children", transArgs), 3000);
				}
			}
		};
		SwingUtilities.invokeLater(runner);
	}

	public void loadChildAssets(final MatrixTreeNode node) {
		loadChildAssets(node, "", -1, -1);
	}

	/**
	 * Makes a request to the Matrix system for the child nodes of
	 * the specifed node. The nodes will be automagically appended
	 * to the root node, and any assets that are not currently apart
	 * of the <code>Asset</code> map will be loaded, and their
	 * appropriate nodes will be added to the <code>Asset's</code> node
	 * list. A placeholder loading node will be appended to the parent
	 * node during the loading process, and will be removed once the
	 * loading of the child nodes has completed. If the nodes have already been
	 * loaded in a previous operation, and are not loaded under the specified
	 * parent, they are propagated to the parent. If the nodes have been loaded
	 * under the specfied parent, then the branch is simply expanded.
	 *
	 * @param node The node whos children are to be loaded
	 */
	public void loadChildAssets(final MatrixTreeNode node, final String direction, final int start, final int limit) {
		if (node == null) {
			return;
		}

		if (node.getChildCount() > 0 && direction.equals("") && start < 0) {
			removeExpandPreviousNode(node);
			removeExpandNextNode(node);

			insertExpandPreviousNode(node);
			if (node.getChildCount() == 0) {
				node.getAsset().propagateChildren(node);
			}
			insertExpandNextNode(node);
		} else {
			insertLoadingNode(node);
			MatrixStatusBar.setStatus(Matrix.translate("asset_map_status_bar_requesting"));
			Runnable runner = new Runnable() {
				public void run() {
					try {
						Asset asset = node.getAsset();
						if (asset.getId().equals("1")) {
							AssetManager.refreshAsset(node, "");
							removeLoadingNode(node);
						} else {
							removeChildNodes(node);
							AssetManager.refreshAsset(node, direction, start, limit);
							insertExpandPreviousNode(node);
							insertExpandNextNode(node);
							TreePath path = getPathToRoot(node);
							if (!isExpanded(path)) {
								expandPath(path);
							}
						}

						if (node.getChildCount() == 1) {
							MatrixStatusBar.setStatusAndClear(Matrix.translate("asset_map_status_bar_loaded_child"), 1000);
						} else {
							Object[] transArgs = {
								new Integer(node.getChildCount())
							};
							MatrixStatusBar.setStatusAndClear(Matrix.translate("asset_map_status_bar_loaded_children", transArgs), 1000);
						}

					} catch (IOException ioe) {
						MatrixStatusBar.setStatusAndClear(Matrix.translate("asset_map_status_bar_requesting"), 1000);
						Object[] transArgs = {
							ioe.getMessage()
						};
						String message = Matrix.translate("asset_map_error_loading_children", transArgs);
						GUIUtilities.error(message, Matrix.translate("asset_map_dialog_title_error"));
						Log.log(message, MatrixTree.class, ioe);
					}
				}
			};
			SwingUtilities.invokeLater(runner);
		}
	}

	public void treeWillCollapse(TreeExpansionEvent evt) {}
	public void moveGestureRecognized(CueEvent evt) {}
	public void addGestureRecognized(CueEvent evt) {}
	public void multipleMoveGestureRecognized(CueEvent evt) {}
	public void multipleAddGestureRecognized(CueEvent evt) {}
	public void multipleAddGestureCompleted(CueEvent evt) {}

	/**
	 * CueListener event that is fired when the request for a move operation is
	 * completed.
	 * @param evt the CueEvent
	 * @see CueListener#requestForMoveCompleted(CueEvent)
	 */
	public void moveGestureCompleted(CueEvent evt) {
		TreePath[] sourcePaths = evt.getSourcePaths();
		MatrixTreeNode[] sourceNodes = new MatrixTreeNode[sourcePaths.length];

		for (int i = 0; i < sourcePaths.length; i++) {
			sourceNodes[i] = (MatrixTreeNode) sourcePaths[i].getLastPathComponent();
		}

		// Ensure that nodes to be moved are sorted by "sort order"

		// Bubble Sort
		if (sourceNodes.length > 1) {
			int numSorted = 1;
			while (numSorted > 0) {
				numSorted = 0;
				for (int i = 0; i < sourceNodes.length; i++) {
					if (i+1 < sourceNodes.length) {
						MatrixTreeNode firstTreeNode = sourceNodes[i];
						MatrixTreeNode nextTreeNode = sourceNodes[i+1];

						// Get the sort order of the selected items
						int firstSortOrder = firstTreeNode.getSortOrder();
						int nextSortOrder = nextTreeNode.getSortOrder();

						// Swap elements if they are the wrong way around
						if (nextSortOrder < firstSortOrder) {
							sourceNodes[i] = nextTreeNode;
							sourceNodes[i+1] = firstTreeNode;
							numSorted++;
						}
					}
				}
			}
		}

		JPopupMenu newLinkMenu = getNewLinkMenu(
			sourceNodes,
			(MatrixTreeNode) evt.getParentPath().getLastPathComponent(),
			evt.getIndex(), evt.getPrevIndex());
		newLinkMenu.show(this, evt.getX(), evt.getY());
	}

	/**
	 * CueListener event method that is fired when a request for a new node
	 * to be added is recognized.
	 * @param evt the CueEvent
	 * @see CueListener#requestForAddCompleted(CueEvent)
	 */
	public void addGestureCompleted(CueEvent evt) {
		String typeCode = (String) evt.getSourcePath().getLastPathComponent();
		JPopupMenu newAssetMenu = getNewAssetMenu(
			typeCode,
			(MatrixTreeNode) evt.getParentPath().getLastPathComponent(),
			evt.getIndex()
		);
		newAssetMenu.show(this, evt.getX(), evt.getY());
	}

	/**
	 * CueListener event method that is fired when a request for a multiple
	 * move operation has completed
	 * @param evt the CueEvent
	 * @see CueListener#requestForMoveCompleted(CueEvent)
	 */
	public void multipleMoveGestureCompleted(CueEvent evt) {

		if (multipleMoveType != null) {
			TreePath[] sourcePaths = evt.getSourcePaths();
			MatrixTreeNode[] sourceNodes = new MatrixTreeNode[sourcePaths.length];
			for (int i = 0; i < sourcePaths.length; i++) {
				sourceNodes[i] = (MatrixTreeNode) sourcePaths[i].getLastPathComponent();
			}

			MatrixTreeNode parent = (MatrixTreeNode) evt.getParentPath().getLastPathComponent();
			fireCreateLink(multipleMoveType, sourceNodes, parent, evt.getIndex(), evt.getPrevIndex());
			multipleMoveType = null;
		} else {
			moveGestureCompleted(evt);


		}
	}

	/**
	 * Teleports the specified node to the root node in the tree
	 * @param node the node that will become the root node in the tree
	 */
	public void teleportToRoot(MatrixTreeNode node) {
		loadChildAssets(node);
		setRootVisible(true);
		((DefaultTreeModel) getModel()).setRoot(node);
	}

	/**
	 * Fires an event for a create link operation to all the NewLinkListeners.
	 * @param type the type link that will be created
	 * @param source the source of the new link
	 * @param parent the parent where the source will be linked underneath
	 * @param index the index under the parent where the link will be created
	 */
	public void fireCreateLink(
		String type,
		MatrixTreeNode[] sources,
		MatrixTreeNode parent,
		int index,
		int prevIndex) {
			// Guaranteed to return a non-null array
			Object[] listeners = listenerList.getListenerList();
			NewLinkEvent evt = null;

			// Process the listeners last to first, notifying
			// those that are interested in this event
			for (int i = listeners.length - 2; i >= 0; i -= 2) {
				if (listeners[i] == NewLinkListener.class) {
					// Lazily create the event:
					String[] parentIds = getCueModeParentIds();
					setCueModeParentIds(null);
					if (evt == null)
						evt = new NewLinkEvent(this, type, sources, parent, index, prevIndex, parentIds);
					((NewLinkListener) listeners[i + 1]).
						requestForNewLink(evt);
				}
			}
	}

	public void fireCreateLink(
		String type,
		MatrixTreeNode[] sources,
		MatrixTreeNode parent,
		int index) {
			fireCreateLink(type,sources,parent,index,0);
	}

	/**
	 * Fires an event for a new asset operation to all the NewAssetListeners.
	 * @param assetType the type of asset that will be created
	 * @param parent the parent where the new asset will be created
	 * @param index the index where the new asset will be created
	 */
	 //TODO MM: this is overkill. I cant see when someone else wants to listen
	 // to new assets. When the asset is added, the model will be updated anyway
	 // you can listen to that if you want

	public void fireNewAsset(
		String typeCode,
		MatrixTreeNode parent,
		int index) {
			// Guaranteed to return a non-null array
			Object[] listeners = listenerList.getListenerList();
			NewAssetEvent evt = null;

			// Process the listeners last to first, notifying
			// those that are interested in this event
			for (int i = listeners.length - 2; i >= 0; i -= 2) {
				if (listeners[i] == NewAssetListener.class) {
					// Lazily create the event:
					if (evt == null)
						evt = new NewAssetEvent(this, typeCode, parent, index);
					((NewAssetListener) listeners[i + 1]).
						requestForNewAsset(evt);
				}
			}
			// Store last created asset type code in ivar
			lastTypeCodeCreated = typeCode;

	}

	/**
	 * Fires an event to indicate that a node has been double clicked
	 * @param clickedPath the path of the node that has been clicked
	 * @param point the point where the click occured
	 */
	public void fireNodeDoubleClicked(TreePath clickedPath, Point point) {
		Object[] listeners = listenerList.getListenerList();
		NodeDoubleClickedEvent evt = null;

		// Process the listeners last to first, notifying
		// those that are interested in this event
		for (int i = listeners.length - 2; i >= 0; i -= 2) {
			if (listeners[i] == NodeDoubleClickedListener.class) {
				// Lazily create the event:
				if (evt == null)
				evt = new NodeDoubleClickedEvent(this, clickedPath, point);
				((NodeDoubleClickedListener) listeners[i + 1]).
				nodeDoubleClicked(evt);
			}
		}
	}

	/**
	 * Overrides the paintComponent() method in CueTree to perform
	 * double buffering operations
	 * @param g the graphics set to paint to.
	 */
	public void paintComponent(Graphics g) {
		Graphics2D g2 = (Graphics2D) g;
		// this gets executed once
		if (dblBuffer == null) {
			initDoubleBufferImage();
		}
		g2.drawImage(dblBuffer, null, 0, 0);
		super.paintComponent(g);

		if (dropHandler == null)
			return;

		if (selTool.isDragging())
			selTool.paintSelectionTool(g2);
		else if (dropHandler.isDropping())
			dropHandler.paintDropImage(g2);
	}

	/**
	 * Returns the drag image for the specified TreePaths
	 * @param paths the tree paths for the wanted drag image
	 * @return the drag image for the specified TreePaths
	 * @see Draggable.getDragImage(TreePath[])
	 * @see DragExchange
	 */
	public Image getDragImage(TreePath[] paths) {
		return (paths.length == 1) ? getGhostedNode(paths[0]) : getGhostedNode(paths);
	}

	/**
	 * Returns whether the tree is in finder mode
	 * @return TRUE if in finder mode
	 * @return FALSE otherwise
	 */
	public boolean isInAssetFinderMode() {
		return isInAssetFinderMode;
	}

	//}}}

	//{{{ Protected Methods

	/**
	 * Returns the menu handler that handles menus.
	 * @return the menu handler
	 */
	protected MenuHandler getMenuHandler() {
		return new MenuHandler();
	}

	/**
	 * Returns the double click handler that handles double clicks.
	 * @return the double click handler
	 */
	protected DoubleClickHandler getDoubleClickHandler() {
		return new DoubleClickHandler();
	}

	/**
	 * Returns the drag handler that handles drag operations.
	 * @return the drag handler
	 */
	protected DragHandler getDragHandler() {
		return new DragHandler();
	}

	/**
	 * Returns the Drop handler that handles Drop operations.
	 * @return the drop handler
	 */
	protected DropHandler getDropHandler() {
		return new DropHandler();
	}

	/**
	 * Returns thr Cue Gesture Handler to handle cue lines
	 * @return the CueGestureHandler
	 */
	protected CueGestureHandler getCueGestureHandler() {
		return new MatrixCueGestureHandler();
	}

	/**
	 * Returns a drag image for the specifed paths. If there is multiple
	 * paths, the drag image will reflect the path traversal offsets in the
	 * path tree
	 * @param paths the paths for the wanted drag image
	 * @return the drag image that reflects the specfied paths
	 */
	protected Image getDragImageForPaths(TreePath[] paths) {
		if (paths == null)
			throw new IllegalArgumentException("paths is null");
		Image ghostedImage = (paths.length == 1)
			? getGhostedNode(paths[0])
			: getGhostedNode(paths);

		return ghostedImage;
	}

	/**
	 * Returns TRUE if the specified node can be moved
	 * @param object node the node to be moved
	 * @return TRUE if the specified node can be moved
	 */
	protected boolean canMoveNode(Object node) {
		if (getModel().getRoot() == node)
			return false;
		MatrixTreeNode treeNode = (MatrixTreeNode) node;
		if (treeNode.getLinkid().equals("0"))
			return false;
		if (!treeNode.getAsset().isAccessible())
			return false;
		return true;
	}

	/**
	 * Returns TRUE if the specified nodes can be moved
	 * @param nodes the nodes to check
	 * @return TRUE if the nodes can be moved
	 */
	protected boolean canMoveNodes(Object[] nodes) {
		for (int i = 0; i < nodes.length; i++) {
			if (!canMoveNode(nodes[i]))
				return false;
		}
		return true;
	}

	private int currentFontSize = 10;
	private int previousFontSize = 0;
	private int initialFontSize = 10;
	private Font nodeFont;

	public Font getFontInUse() {
		if (currentFontSize != previousFontSize) {
			nodeFont = null;
			nodeFont = new Font("nodeFont", Font.PLAIN, currentFontSize);
			// update previous font size so we create font when size changes
			previousFontSize = currentFontSize;
		}
		return nodeFont;
	}

	private void setFontSize(int size) {
		if (size > 7 && size < 18) {
			currentFontSize = size;
		}
	}

	private java.util.List getExpandedChildren(TreeNode parent) {
		java.util.List paths = new  ArrayList();
		int count = parent.getChildCount();
		for (int i = 0; i < count; i++) {
			MatrixTreeNode node = (MatrixTreeNode)parent.getChildAt(i);
			TreePath path = getPathToRoot(node);
			if (isExpanded(path)) {
				paths.add(path);
				paths.addAll(getExpandedChildren(node));
			}
		}
		return paths;
	}

	private int[] getRowsForPaths(java.util.List paths) {
		int rows[] = new int[paths.size()];
		for (int i = 0; i < paths.size(); i++) {
			int row = getRowForPath((TreePath)paths.get(i));
			rows[i] = row;
		}
		return rows;
	}

	private void expandRows(int rows[]) {
		for (int i=0; i< rows.length; i++) {
			expandRow(rows[i]);
		}
	}

	private void expandPaths(java.util.List paths, boolean scrollPathToVisible) {
		for (int i=0; i< paths.size(); i++) {
			expandPath((TreePath)paths.get(i));
			if (scrollPathToVisible) {
				scrollPathToVisible((TreePath)paths.get(i));
			}
		}
	}

	/**
	 * Sets the keyboard actions for the tree to trigger ui components
	 */
	protected void setKeyboardActions() {
		Action deleteAction = new AbstractAction() {
			public void actionPerformed(ActionEvent evt) {
				MatrixTreeNode[] nodes = getSelectionNodes();
				if (nodes == null) {
					return;
				}
				DeleteDialog deleteDialog = DeleteDialog.getDeleteDialog(nodes, getLocationOnScreen(), getSize());
				deleteDialog.show();
			}
		};

		Action searchAction = new AbstractAction() {
			public void actionPerformed(ActionEvent evt) {
				Point topLeft = new Point(getLocationOnScreen());
				SearchDialog searchDialog = SearchDialog.getSearchDialog(topLeft, getSize());
				searchDialog.show();
			}
		};

		Action increaseFontAction = new AbstractAction() {
			public void actionPerformed(ActionEvent evt) {
				setFontSize(currentFontSize+1);
				java.util.List expandedChildren = getExpandedChildren((TreeNode)getModel().getRoot());
				((DefaultTreeModel) getModel()).reload();
				expandPaths(expandedChildren, false);

			}
		};

		Action decreaseFontAction = new AbstractAction() {
			public void actionPerformed(ActionEvent evt) {
				setFontSize(currentFontSize-1);
				java.util.List expandedChildren = getExpandedChildren((TreeNode)getModel().getRoot());
				((DefaultTreeModel) getModel()).reload();
				expandPaths(expandedChildren, false);
			}
		};

		Action normalFontAction = new AbstractAction() {
			public void actionPerformed(ActionEvent evt) {
				setFontSize(initialFontSize);
				java.util.List expandedChildren = getExpandedChildren((TreeNode)getModel().getRoot());
				((DefaultTreeModel) getModel()).reload();
				expandPaths(expandedChildren, false);
			}
		};


		getInputMap().put(KeyStroke.getKeyStroke("DELETE"), "delete");
		getActionMap().put("delete", deleteAction);
		getInputMap().put(KeyStroke.getKeyStroke("shift ctrl J"), "search");
		getActionMap().put("search", searchAction);
		getInputMap().put(KeyStroke.getKeyStroke("shift ctrl EQUALS"), "increase_font_size");
		getActionMap().put("increase_font_size", increaseFontAction);
		getInputMap().put(KeyStroke.getKeyStroke("shift ctrl ADD"), "increase_font_size");
		getActionMap().put("increase_font_size", increaseFontAction);
		getInputMap().put(KeyStroke.getKeyStroke("shift ctrl MINUS"), "decrease_font_size");
		getActionMap().put("decrease_font_size", decreaseFontAction);
		getInputMap().put(KeyStroke.getKeyStroke("shift ctrl SUBTRACT"), "decrease_font_size");
		getActionMap().put("decrease_font_size", decreaseFontAction);
		getInputMap().put(KeyStroke.getKeyStroke("shift ctrl BACK_SPACE"), "normal_font_size");
		getActionMap().put("normal_font_size", normalFontAction);

	}

	public void removeKeyStroke(String key) {
		getInputMap().put(KeyStroke.getKeyStroke(key), "none");
	}


	//}}}

	//{{{ Private Methods

	/**
	 * Inserts a placeholder loading node during the loading of
	 * child assets of a particular asset.
	 * @param parentNode the parent node to add the placeholder node
	 */
	private void insertLoadingNode(MatrixTreeNode parentNode) {
		int insertIndex = 0;
		if (parentNode.getChildCount() > 0) {
			insertIndex = parentNode.getChildCount()-1;
		}
		((DefaultTreeModel) getModel()).insertNodeInto(new LoadingNode(), parentNode, insertIndex);
	}

	/**
	 * Removes the placeholder loading node from the specified parent
	 * @param parentNode the node to remove the placeholding node from
	 */
	private void removeLoadingNode(MatrixTreeNode parent) {
		// we only want to remove the loading node from the specifed parent
		// so we can't use TreeModel.removeNodeFromParent()
		int[] childIndex = new int[1];
		Object[] removedArray = new Object[1];

		LoadingNode loadingNode = null;
		for (Enumeration children = parent.children(); children.hasMoreElements();) {
			MatrixTreeNode nextNode = (MatrixTreeNode) children.nextElement();
			if (nextNode instanceof LoadingNode) {
				loadingNode = (LoadingNode) nextNode;
				break;
			}
		}
		if (loadingNode != null) {
			childIndex[0] = parent.getIndex(loadingNode);
			parent.remove(childIndex[0]);
			loadingNode = null;
			removedArray[0] = loadingNode;
			((DefaultTreeModel) getModel()).nodesWereRemoved(parent, childIndex, removedArray);
		}
	}

	/**
	 * Removes the children of a given parent node
	 *
	 * @param parent
	 * @param incNavNodes if set to true next and previous nodes will be removed
	 */
	private void removeChildNodes(MatrixTreeNode parent) {
		Object[] removedArray = new Object[parent.getChildCount()];
		int i = 0;

		for (Enumeration children = parent.children(); children.hasMoreElements();) {
			MatrixTreeNode nextNode = (MatrixTreeNode) children.nextElement();
			if (nextNode != null) {
				removeChildNodes(nextNode);
				removedArray[i] = nextNode;
				i++;
			}
		}

		if (i > 0) {
			for (int j = i; j > 0; j--) {
				MatrixTreeModelBus.removeNodeFromParent((MatrixTreeNode)removedArray[j-1]);
			}
			MatrixTreeModelBus.nodeChanged(parent);
		}
	}


	/**
	 * Inserts a node that allows users to get next set of assets
	 *
	 * @param parentNode the parent node to add the placeholder node
	 */
	public void insertExpandNextNode(MatrixTreeNode parentNode) {
		int modifier = 0;
		if (parentNode.getAsset().getTotalKidsLoaded() > 0) {
			modifier = 1;
		}

		if ((AssetManager.getLimit() <= (parentNode.getChildCount()-modifier)) && (parentNode.getAsset().getNumKids() == -1 || (parentNode.getAsset().getNumKids() > (parentNode.getAsset().getTotalKidsLoaded()+AssetManager.getLimit())))) {
			MatrixTreeModelBus.insertNodeInto((MatrixTreeNode)new ExpandingNextNode(parentNode.getAsset().getNumKids(),parentNode.getChildCount(),parentNode.getAsset().getTotalKidsLoaded()), parentNode, parentNode.getChildCount());
		}
	}

	/**
	 * Removes the placeholder expand node from the specified parent
	 * @param parentNode the node to remove the placeholding node from
	 */
	private void removeExpandNextNode(MatrixTreeNode parent) {
		if (hasNextNode(parent)) {
			MatrixTreeNode node = (MatrixTreeNode) parent.getChildAt(parent.getChildCount()-1);
			MatrixTreeModelBus.removeNodeFromParent(node);
		}
	}

	/**
	 * Inserts a node that allows users to get previous set of assets
	 *
	 * @param parentNode the parent node to add the placeholder node
	 */
	public void insertExpandPreviousNode(MatrixTreeNode parentNode) {
		if ((parentNode.getAsset().getTotalKidsLoaded() > 0)) {
			MatrixTreeModelBus.insertNodeInto((MatrixTreeNode)new ExpandingPreviousNode(parentNode.getAsset().getNumKids(),parentNode.getChildCount(),parentNode.getAsset().getTotalKidsLoaded()), parentNode, 0);
		}
	}

	/**
	 * Removes the placeholder expand node from the specified parent
	 * @param parentNode the node to remove the placeholding node from
	 */
	private void removeExpandPreviousNode(MatrixTreeNode parent) {

		if (hasPreviousNode(parent)) {
			MatrixTreeNode node = (MatrixTreeNode) parent.getChildAt(0);
			MatrixTreeModelBus.removeNodeFromParent(node);
		}
	}


	/**
	 * Adds a menu item to the menu add adds the actionlistener.
	 * @param item the item to be added to the menu
	 * @param menu the parent menu
	 * @param l the listener to assign to the menu item
	 */
	private void addMenuItem(JMenuItem item, JPopupMenu menu, ActionListener l) {
		item.addActionListener(l);
		item.setFont(getFontInUse());
		menu.add(item);
	}

	/**
	 * Returns a menu for creating new links in the matrix system.
	 * @param source the source to be new linked
	 * @param parent the parent we are new linking to
	 * @param index the index where the new link will be created
	 * @return the menu to complete the new link operation
	 */
	private JPopupMenu getNewLinkMenu(
		final MatrixTreeNode[] sources,
		final MatrixTreeNode parent,
		final int index,
		final int prevIndex) {
			JPopupMenu newLinkMenu = new JPopupMenu();

			final JMenuItem moveMenuItem    = new JMenuItem(Matrix.translate("asset_map_menu_move_here"));
			final JMenuItem newLinkMenuItem = new JMenuItem(Matrix.translate("asset_map_menu_link_here"));
			final JMenuItem cloneMenuItem   = new JMenuItem(Matrix.translate("asset_map_menu_clone_here"));
			final JMenuItem cancelMenuItem  = new JMenuItem(Matrix.translate("asset_map_menu_cancel"));

			ActionListener listener = new ActionListener() {
				public void actionPerformed(ActionEvent evt) {
					String type = NewLinkEvent.LINK_TYPE_MOVE;
					if (evt.getSource().equals(moveMenuItem)) {
						type = NewLinkEvent.LINK_TYPE_MOVE;
					} else if (evt.getSource().equals(newLinkMenuItem)) {
						type = NewLinkEvent.LINK_TYPE_NEW_LINK;
					} else if (evt.getSource().equals(cloneMenuItem)) {
						type = NewLinkEvent.LINK_TYPE_CLONE;
					} else if (evt.getSource().equals(cancelMenuItem)) {
						return;
					}
					fireCreateLink(type, sources, parent, index, prevIndex);
				}
			};

			addMenuItem(moveMenuItem, newLinkMenu, listener);
			addMenuItem(newLinkMenuItem, newLinkMenu, listener);
			addMenuItem(cloneMenuItem, newLinkMenu, listener);
			newLinkMenu.addSeparator();
			addMenuItem(cancelMenuItem, newLinkMenu, listener);

			return newLinkMenu;
	}

	/**
	 * Returns a <code>JPopupMenu</code> to complete the process of
	 * Adding a new asset to the Matrix System.
	 * @param parent the parent of the new asset
	 * @param assetType the asset type that we are creating
	 * @param index the index where the new asset will be created
	 */
	private JPopupMenu getNewAssetMenu(
		final String typeCode,
		final MatrixTreeNode parent,
		final int index) {
			JPopupMenu createMenu = new JPopupMenu();

			final JMenuItem createItem = new JMenuItem(Matrix.translate("asset_map_menu_create_here"));
			final JMenuItem cancelItem = new JMenuItem(Matrix.translate("asset_map_menu_cancel"));

			ActionListener listener = new ActionListener() {
				public void actionPerformed(ActionEvent evt) {
					if (evt.getSource().equals(createItem)) {
						fireNewAsset(typeCode, parent, index);
					}
				}
			};
			addMenuItem(createItem, createMenu, listener);
			addMenuItem(cancelItem, createMenu, listener);

			return createMenu;
	}

	/**
	 * initialises the double buffering image for offscreen painting
	 */
	private void initDoubleBufferImage() {
		int w = getWidth();
		int h = getHeight();
		dblBuffer = (BufferedImage) createImage(w, h);
		Graphics2D gc = dblBuffer.createGraphics();
		gc.setColor(getBackground());
		gc.fillRect(0, 0, w, h);
	}


	//}}}

	//{{{ Inner Classes

	/**
	 * Handles double click events that are fired from this tree
	 * @author Nathan De Vries <ndevries@squiz.net>
	 */
	protected class DoubleClickHandler extends MouseAdapter {
		private TreePath[] selPaths = null;

		public void mouseReleased(MouseEvent evt) {
			selPaths = getSelectionPaths();
		}

		public void mousePressed(MouseEvent evt) {
			// we should remove the Expending Nodes from node selections
			TreePath[] paths = getSelectionPaths();
			if (paths != null) {
				int c = 0;
				TreePath[] newPaths = new TreePath[paths.length+1];
				for (int i=0; i < paths.length; i++) {
					Object node = paths[i].getLastPathComponent();
					if (!(node instanceof ExpandingNode)) {
						newPaths[c++] = paths[i];
					}
				}
				setSelectionPaths(newPaths);
			}
		}

		public void mouseClicked(MouseEvent evt) {
			if (evt.getClickCount() == 1) {
				final TreePath treePath = getPathForLocation(evt.getX(), evt.getY());
				if (treePath == null)
					return;
				final MatrixTreeNode node = (MatrixTreeNode) treePath.getLastPathComponent();
				if (node instanceof ExpandingNode) {
					((ExpandingNode)node).switchName(evt.getX(),tree.getPathBounds(treePath).getX());
					((DefaultTreeModel) getModel()).nodeChanged(node);
					// keep the selection
					if (selPaths != null) {
						setSelectionPaths(selPaths);
						selPaths = null;
					}
				}
				return;
			}

			if (evt.getClickCount() != 2)
					return;

			final TreePath treePath = getPathForLocation(evt.getX(), evt.getY());
			if (treePath == null)
				return;

			if (getToggleClickCount() != 2) {
				final Point point = evt.getPoint();
				final MatrixTreeNode node = (MatrixTreeNode) treePath.getLastPathComponent();

				if (!node.getAsset().childrenLoaded()) {
					AssetRefreshWorker worker = new AssetRefreshWorker(node, true);
					worker.start();
				} else {
					fireNodeDoubleClicked(treePath, point);
				}
			} else {
				final MatrixTreeNode node = (MatrixTreeNode) treePath.getLastPathComponent();

				if (node instanceof ExpandingNextNode) {
					final MatrixTreeNode parent = (MatrixTreeNode)node.getParent();
					int start = ((ExpandingNextNode)node).getStartLoc(evt.getX(),tree.getPathBounds(treePath).getX());
					if (start > -1) {
						loadChildAssets(parent, "next", start, -1);
					}
					return;
				} else if (node instanceof ExpandingPreviousNode) {
					final MatrixTreeNode parent = (MatrixTreeNode)node.getParent();
					int start = ((ExpandingPreviousNode)node).getStartLoc(evt.getX(),tree.getPathBounds(treePath).getX());
					if (start > -1) {
						loadChildAssets(parent, "prev", start, -1);
					}
					return;
				}
			}
		}
	}//end class DoubleClickHandler


	/**
	 * Class that handles the menus through right clicking. If there is
	 * a multiple selection and a right click occurs, a separate method is
	 * invokes to show a menu to the context of the selection nodes. If a
	 * multiple selection is inplace, and a single node outside the selection is
	 * selected, the multiple section is lost in place for the node right clicked on.
	 * @author Marc McIntyre <mmcintyre@squiz.net>
	 */
	protected class MenuHandler extends MouseAdapter {

		private ActionListener addMenuListener;

		/**
		 * Constructs menu handler
		 * @return the menu handler
		 */
		public MenuHandler() {
			addMenuListener = MatrixMenus.getMatrixTreeAddMenuListener(MatrixTree.this);
		}

		/**
		 * Event listener method that is called when the mouse is clicked
		 * @param evt the MouseEvent
		 */
		public void mouseClicked(MouseEvent evt) {

			if (!GUIUtilities.isRightMouseButton(evt))
				return;
			// if we are in cue mode. we dont want to show any of the
			// right click menus
			if (inCueMode)
				return;

			JPopupMenu menu = null;

			// if the click occured where there was no node, get a menu
			// for void space
			if (getPathForLocation(evt.getX(), evt.getY()) == null) {
				menu = getMenuForVoidSpace();
			} else {
				TreePath[] selectedPaths
					= getSelectionPathsForLocation(evt.getX(), evt.getY());
				setSelectionPaths(selectedPaths);

				menu = (selectedPaths.length == 1)
					? getMenuForSingleSelection()
					: getMenuForMultipleSelection();
			}
			if (menu != null)
				menu.show(MatrixTree.this, evt.getX(), evt.getY());
		}

		/**
		 * Returns a popup menu when the mouse is clicked in void space outside
		 * any tree components
		 * @return the popup menu for a click in void space
		 */
		protected JPopupMenu getMenuForVoidSpace() {
			if (isInAssetFinderMode) return null;
			return MatrixMenus.getPopupAddMenu(addMenuListener);
		}

		/**
		 * Returns the selection path for a click that occured at the specifed
		 * x and y co-ordinate
		 * @param x the x co-ordinate of the click
		 * @param y the y co-ordinate of the click
		 */
		protected TreePath[] getSelectionPathsForLocation(int x, int y) {
			TreePath path = getPathForLocation(x, y);
			TreePath[] selPaths = getSelectionPaths();

			// if the path for the right click does not exist, set the
			// selection path to the closest path near the x,y co-ordinate set
			if (path == null) {
				path = getClosestPathForLocation(x, y);
			} else if (selPaths != null) {
				// check to see if the clicked node was in
				// the current selection path
				if (selPaths.length > 1) {
					boolean found = false;
					for (int i = 0; i < selPaths.length; i++) {
						if (selPaths[i].getLastPathComponent()
							== path.getLastPathComponent()) {
								found = true;
								break;
						}
					}
					// if the clicked node is in the selected nodes, then
					// keep the selection
					if (found)
						return selPaths;
				}
			}
			// if the clicked node was not in the selection, or there was
			// zero or one selected node, set the selection to the clicked node
			TreePath[] paths = new TreePath[] { path };

			return paths;
		}

		/**
		 * Returns a popup menu for a single selection
		 * @return a popup menu for a single selection
		 */
		protected JPopupMenu getMenuForSingleSelection() {

			JPopupMenu menu = null;
			final MatrixTreeNode node = getSelectionNode();

			// if the node is not accessible, we don't want the users
			// to be able bring up an menu for it
			if (!node.getAsset().isAccessible())
				return null;

			if (isInAssetFinderMode) {
				if (MatrixTreeBus.typeIsRestricted(node.getAsset().getType())) {
					menu = MatrixMenus.getUseMeMenu(node);
				} else {
					return null;
				}
			} else {
				menu = MatrixMenus.getPopupScreenMenu(node);
				menu.addSeparator();

				// if there are any ancillery items add them after the sperator
				// and before the add menu
				JMenuItem[] items = getAncillaryMenuItems();
				if (items != null) {
					for (int i = 0; i < items.length; i++) {
						items[i].setFont(getFontInUse());
						menu.add(items[i]);
					}
				}

				// when we click on a node and choose add, we want to go
				// straight into add mode in matrix with the node clicked on
				// as the parent of the new node
				ActionListener explicitAddListener = new ActionListener() {
					public void actionPerformed(ActionEvent evt) {
						fireNewAsset(
							MatrixMenus.getTypeCodeFromEvent(evt),
							node,
							-1 // let the MatrixTreeCom handle to pos
						);
					}
				};

				JMenu addMenu = MatrixMenus.getAddMenu(explicitAddListener);
				addMenu.setText(Matrix.translate("asset_map_menu_new_child"));
				addMenu.setFont(getFontInUse());
				menu.add(addMenu);
			}

			return menu;
		}

		/**
		 * Returns the popup menu for a multiple selection.
		 * @return the popup menu for a multiple selection.
		 */
		protected JPopupMenu getMenuForMultipleSelection() {
			if (isInAssetFinderMode) return null;

			JPopupMenu menu = new JPopupMenu();
			final JMenuItem moveItem = new JMenuItem(Matrix.translate("asset_map_menu_move"));
			final JMenuItem newLinkItem = new JMenuItem(Matrix.translate("asset_map_menu_link"));
			final JMenuItem cloneItem = new JMenuItem(Matrix.translate("asset_map_menu_clone"));

			ActionListener multiplelistener = new ActionListener() {
				public void actionPerformed(ActionEvent evt) {
					if (evt.getSource() == moveItem)
						multipleMoveType = NewLinkEvent.LINK_TYPE_MOVE;
					else if (evt.getSource() == newLinkItem)
						multipleMoveType = NewLinkEvent.LINK_TYPE_NEW_LINK;
					else if (evt.getSource() == cloneItem)
						multipleMoveType = NewLinkEvent.LINK_TYPE_CLONE;
					startCueMode(getSelectionPaths());
				}
			};

			moveItem.addActionListener(multiplelistener);
			newLinkItem.addActionListener(multiplelistener);
			cloneItem.addActionListener(multiplelistener);

			// fonts
			moveItem.setFont(getFontInUse());
			newLinkItem.setFont(getFontInUse());
			cloneItem.setFont(getFontInUse());

			menu.add(moveItem);
			menu.add(newLinkItem);
			menu.add(cloneItem);

			return menu;
		}

		/**
		 * Returns the ancillery items that will be displayed below the
		 * main menu items separated by JMenu.Separator
		 * @return the ancillery menu items
		 */
		protected JMenuItem[] getAncillaryMenuItems() {
			JMenuItem[] items = new JMenuItem[3];
			final JMenuItem teleportItem = new JMenuItem(Matrix.translate("asset_map_menu_teleport"));
			final JMenuItem refreshItem = new JMenuItem(Matrix.translate("asset_map_menu_refresh"));

			// Work out the title of the new previous child menu item
			final String newChildPreviousItemTitle;
			if (lastTypeCodeCreated != null) {
				Object[] transArgs = { ((AssetType) AssetManager.getAssetType(lastTypeCodeCreated)).getName() };
				newChildPreviousItemTitle = Matrix.translate("asset_map_menu_new_child_previous", transArgs);
			} else {
				newChildPreviousItemTitle = Matrix.translate("asset_map_menu_no_previous_child");
			}

			final JMenuItem newChildPreviousItem = new JMenuItem(newChildPreviousItemTitle);
			// Disable the new child previous item if assets have not previously been created yet
			if (lastTypeCodeCreated == null) {
				newChildPreviousItem.setEnabled(false);
			}

			ActionListener extrasListener = new ActionListener() {
				public void actionPerformed(ActionEvent evt) {
					if (evt.getSource().equals(teleportItem)) {
						teleportToRoot(getSelectionNode());
					} else if (evt.getSource().equals(refreshItem)) {
						String [] assetids = new String[] { getSelectionNode().getAsset().getId() };
						AssetRefreshWorker worker = new AssetRefreshWorker(assetids, true);
						worker.start();
					} else if (evt.getSource().equals(newChildPreviousItem)) {
						// Get the selected node to use as the parent and let MatrixTreeComm handle the tree position
						final MatrixTreeNode node = getSelectionNode();
						fireNewAsset(lastTypeCodeCreated, node, -1);
					}
				}
			};

			// only show the teleport menu item if there is only one selected
			// node and its a leaf, and its not the root node in the current tree
			if (!(getSelectionNode().isLeaf())
				&& (getSelectionNode() != getModel().getRoot())
				&& !(isMultipleSelection())) {
					teleportItem.addActionListener(extrasListener);
			} else {
				teleportItem.setEnabled(false);
			}

			refreshItem.addActionListener(extrasListener);
			newChildPreviousItem.addActionListener(extrasListener);
			items[0] = teleportItem;
			items[1] = refreshItem;
			items[2] = newChildPreviousItem;

			return items;
		}
	}//end class MenuHandler

	/**
	 * Class that handles drag operations that occur in the tree.
	 * @author Marc McIntyre <mmcintyre@squiz.net>
	 */
	protected class DragHandler extends DragSourceAdapter
		implements DragGestureListener {

		protected Point dragOffset = new Point(5, 5);
		protected TreePath[] dragPaths;
		private boolean dragImageSupport = false;

		/* DragGestureListener methods */

		/**
		 * Event method from DragGestureListener that is invoked when a drag
		 * operation is recognized
		 * @param dge the DragGestureEvent
		 */
		public void dragGestureRecognized(DragGestureEvent dge) {

			if (isInAssetFinderMode()) return;

			Point initPoint = dge.getDragOrigin();
			if (getPathForLocation(initPoint.x, initPoint.y) == null)
				return;

			dragPaths = getSelectionPaths();

			if (dragPaths != null) {
				BufferedImage dragImage = (BufferedImage) getDragImageForPaths(dragPaths);

				Point topLeft = new Point(getPathBounds(dragPaths).getLocation());
				Point origin = dge.getDragOrigin();
				dragOffset.setLocation(origin.getX() - topLeft.getX(), origin.getY() - topLeft.getY());

				MatrixTreeTransferable transferable = new MatrixTreeTransferable(dragPaths);
				DragImageExchange.setDragImage(dragImage, dragOffset);
				dragPaths = null;

				// only add the drag image if its supported
				if (dge.getDragSource().isDragImageSupported()) {
					dragImageSupport = true;
					dge.startDrag(
						new Cursor(Cursor.DEFAULT_CURSOR),
						dragImage,
						dragOffset,
						transferable,
						this
					);
				} else {
					dge.startDrag(
						new Cursor(Cursor.DEFAULT_CURSOR),
						transferable,
						this
					);
				}
			}
		}

		public boolean isDragImageSupported() {
			return dragImageSupport;
		}

	}//end class DragHandler

	/**
	 * The Drop Handler class handles drop operations that occur within the
	 * MatrixTree. Currently, only MatrixTreeTransferable.TREE_NODE_FLAVOUR flavours
	 * are accepted as successful drop transferables.
	 * @author Marc McIntyre <mmcintyre@squiz.net>
	 */
	protected class DropHandler implements DropTargetListener {

		private boolean isDropping = false;
		protected Point initMousePt;
		private Point lastMousePt = null;
		private BufferedImage dragImage;
		private Point mouseOffset = new Point(5,5);

		/**
		 * Returns TRUE if we are currently performing a drop operation
		 * @return TRUE if we are currently performing a drop operation
		 */
		public boolean isDropping() {
			return isDropping;
		}

		/**
		 * Event listener method that is called when the mouse is dragged
		 * into the bounds of the MatrixTree
		 * @param dtde the DropTargetDragEvent
		 */
		public void dragEnter(DropTargetDragEvent dtde) {
			dragImage = DragImageExchange.getDragImage();
			mouseOffset = DragImageExchange.getMouseOffset();
			isDropping = true;
		}

		/**
		 * Event listener method that is called when the mouse is dragged
		 * ouside the bounds of the MatrixTree
		 * @param dte the DropTargetEvent
		 */
		public void dragExit(DropTargetEvent dte) {
			if (dragHandler == null) return;

			isDropping = false;
			dragImage = null;

			if (!dragHandler.isDragImageSupported())
				repaint();
		}

		/**
		 * Event listener method that is called repeatedly when the mouse
		 * is within the bounds of the MatrixTree
		 * @param dtde the DropTargetDragEvent
		 */
		public void dragOver(DropTargetDragEvent dtde) {
			if (dragHandler == null) return;

			if (lastMousePt != null && lastMousePt.equals(dtde.getLocation()))
				return;
			if (initMousePt == null) {
				initMousePt = dtde.getLocation();
				SwingUtilities.convertPointFromScreen(initMousePt, MatrixTree.this);
			}
			lastMousePt = dtde.getLocation();
			if (!dragHandler.isDragImageSupported())
				repaint();
		}

		/**
		 * Event Listener method that is called when the mouse is released
		 * during a drop operation
		 * @param dtde the DropTargetDropEvent
		 */
		public void drop(DropTargetDropEvent dtde) {
			if (dragHandler == null) return;

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

			Iterator iterator = paths.iterator();
			while (iterator.hasNext()) {
				TreePath path = (TreePath) iterator.next();
				MatrixTreeNode node = (MatrixTreeNode) path.getLastPathComponent();
				if (!canMoveNode(node)) {
					GUIUtilities.error(Matrix.translate("asset_map_error_move_shadow_nodes"), Matrix.translate("asset_map_dialog_title_error"));
					dtde.rejectDrop();
					isDropping = false;
					if (!dragHandler.isDragImageSupported())
						repaint();
					return;
				}
			}

			DragImageExchange.completeExchange();
			isDropping = false;
			lastMousePt = null;

			if (!dragHandler.isDragImageSupported())
				repaint();
			startCueMode((TreePath[]) paths.toArray(new TreePath[paths.size()]));
			Point p = dtde.getLocation();
			TreePath path = getClosestPathForLocation((int) p.getX(),(int) p.getY());
			if (path != null)
				drawCueLine(path, (int) p.getY());
		}

		/**
		 * Paints the drag image so the image appears in the same location
		 * under the mouse where the drag operation first started.
		 * @param g2d the graphics to paint the drag image to
		 */
		protected void paintDropImage(Graphics2D g2d) {
			if (dragHandler == null) return;

			if (dragImage == null || dragHandler.isDragImageSupported())
				return;

			int x = lastMousePt.x -  mouseOffset.x;
			int y = lastMousePt.y -  mouseOffset.y;

			g2d.drawImage(dragImage, x, y, MatrixTree.this);
		}

		/**
		 * Events Listener method that is called when the drop action changes
		 * @param dtde the DropTargetDragEvent
		 */
		public void dropActionChanged(DropTargetDragEvent dtde) {}

	}//end class DropHandler

	/**
	 * The MatrixCueGestureHandler overrides CueTree.CueGestureHandler to
	 * filter out children of nodes that are already appart of the move
	 * operation, as these nodes will get moved along with their parents.
	 * @author Marc McIntyre <mmcintyre@squiz.net>
	 */
	protected class MatrixCueGestureHandler extends CueGestureHandler {

		/**
		 * Filters out nodes that are in a multiple move operation. Nodes will
		 * be removed if their parent already exists in the move operation.
		 * @param sourcePaths the source paths to filter
		 * @return the filtered paths
		 */
		protected TreePath[] filterMultipleNodes(TreePath[] sourcePaths) {
			// we want to filter out the nodes whos parents already exist in
			// the move operation, as these nodes will be moved anyway
			java.util.List realSourcePaths = new ArrayList();
			boolean chuck = false;

			for (int i = 0; i < sourcePaths.length; i++) {
				chuck = false;
				for (int j = 0; j < sourcePaths.length; j++) {
					if (i == j)
						continue;
					if (sourcePaths[j].isDescendant(sourcePaths[i]))
						chuck = true;
				}
				if (!chuck)
					realSourcePaths.add(sourcePaths[i]);
			}
			return (TreePath[]) realSourcePaths.toArray(new TreePath[realSourcePaths.size()]);
		}

		/**
		 * Returns true if the specified point will trigger a move operation
		 * when the mouse if pressed on that point
		 * @param p the point to check
		 */
		protected boolean pointTriggersMove(Point p) {
			if (isInAssetFinderMode()) {
				return false;
			} else {
				return nodeIconContainsPoint(p);
			}
		}

	}//end MatrixCueGestureHandler

	/**
	 * MatrixTreeUI modifys the BasicTreeUI so that it is more suitable to
	 * drag operations that may occur in the MatrixTree.
	 * @author Marc McIntyre <mmcintyre@squiz.net>
	 */
	class MatrixTreeUI extends CueTree.CueTreeUI {

		public MatrixTreeUI() {
			setCueLineColor(UIManager.getColor("CueLine.stroke"));
		}

		/**
		 * Constructs the MatrixMouseListener to handle mouse events
		 * @return a new instance of the MatrixMouseHandler
		 */
		protected MouseListener createMouseListener() {
			return new MatrixMouseHandler();
		}

		/**
		 * The MatrixMouseHandler class tweaks the MouseHandler class in BasicTreeUI
		 * so that it is more suitable to Drag Operations
		 * @author Marc McIntyre <mmcintyre@squiz.net>
		 */
		public class MatrixMouseHandler extends MouseHandler
			implements MouseMotionListener {

			boolean isDragging = false;

			/**
			 * Event listener method called when a drag operation occurs
			 * @param evt the MouseEvent
			 */
			public void mouseDragged(MouseEvent evt) {
				isDragging = true;
			}

			/**
			 * Event Listener method that is called when the mouse is moved
			 * @param evt the MouseEvent
			 */
			public void mouseMoved(MouseEvent evt) {}

			/**
			 * Event Listener method that is called when the mouse is pressed
			 * @param evt the MouseEvent
			 */
			public void mousePressed(MouseEvent evt) {
				// we will set this tree as the last active tree
				MatrixTreeBus.setActiveTree((MatrixTree)evt.getSource());
			}

			/**
			 * Event listener method that is called when the mouse is released
			 * @param evt the MouseEvent
			 */
			public void mouseReleased(MouseEvent evt) {
				if (isDragging) {
					isDragging = false;
					return;
				}
				int mouseX = evt.getX();
				int mouseY = evt.getY();

				// we want it so that if we click outside of an exapansion control
				// and not on a node (eg. void space) then the selection is cleared

				TreePath path = getClosestPathForLocation(tree, mouseX, mouseY);
				boolean isControl = isLocationInExpandControl(path, mouseX, mouseY);
				if ((getPathForLocation(mouseX, mouseY) == null) && !isControl && !GUIUtilities.isRightMouseButton(evt))
					clearSelection();
				else {
					// copied from java 1.5 instead of using super.mouseReleased(evt)
					// condition checking in BasicTreeUI$Handler.mouseReleased is broken in 1.6
					if ((!evt.isConsumed())) {
						handleSelection(evt);
					}
				}
			}

			// copied from java 1.5 source code
			// drag-n-drop feature added in 1.6 breaks MatrixTree expansion and selection
			void handleSelection(MouseEvent e) {
				if(tree != null && tree.isEnabled()) {
					if (isEditing(tree) && tree.getInvokesStopCellEditing() && !stopEditing(tree)) {
						return;
					}
					if (tree.isRequestFocusEnabled()) {
						tree.requestFocus();
					}

					TreePath path = getClosestPathForLocation(tree, e.getX(), e.getY());
					if(path != null) {
						Rectangle bounds = getPathBounds(tree, path);
						if(e.getY() > (bounds.y + bounds.height)) {
							return;
						}

						// Preferably checkForClickInExpandControl could take
						// the Event to do this it self!
						if(SwingUtilities.isLeftMouseButton(e))
							checkForClickInExpandControl(path, e.getX(), e.getY());

						int x = e.getX();

						// Perhaps they clicked the cell itself. If so,
						// select it.
						if (x > bounds.x) {
							if (x <= (bounds.x + bounds.width) && !startEditing(path, e)) {
								selectPathForEvent(path, e);
							}
						}
					}
				}
			}//end handleSelection

		}//end class MatrixMouseListener
	}//end class MatrixTreeUI

}//end class MatrixTree


