/**
* +--------------------------------------------------------------------+
* | MySource 3 - MySource Matrix                                       |
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
* $Id: AssetTreeModel.java,v 1.2 2003/11/18 15:37:36 brobertson Exp $
* $Name: not supported by cvs2svn $
*/

package squiz.matrix;

import javax.swing.tree.TreeModel;
import javax.swing.tree.TreePath;
import javax.swing.event.TreeModelListener;
import javax.swing.event.TreeModelEvent;

import java.util.Collections;
import java.util.Set;
import java.util.HashSet;

/**
 * <p>Data model for the asset hierarchy for display with <code>JTree</code>. </p>
 * 
 * <p>The data model is fed from <code>AssetFactory</code>, and each node in the 
 * tree hierarchy is an <code>AssetLink</code>, <b>except</b> for the root of the 
 * tree, which represents the root folder asset (and thus is of type 
 * <code>Asset</code>). </p>
 * 
 * <p><code>$Id: AssetTreeModel.java,v 1.2 2003/11/18 15:37:36 brobertson Exp $</code></p>
 * 
 * @author	Dominic Wong <dwong@squiz.net>
 * @version $Version$
 * @see		javax.swing.tree.TreeModel
 * @see		AssetFactory
 */

public class AssetTreeModel implements TreeModel
{
	// member variables
	/** The set of TreeModelListeners */
	private Set listeners;

	/** The set of asset types to restrict ourselves to */
	private Set restrictedAssetTypes;

	/**
	 * Constructor.
	 *
	 */
	public AssetTreeModel() {
		listeners = new HashSet();
		restrictedAssetTypes = null;
	}//end constructor

	/** TreeModel interface **/

	/**
	 * Returns the root of the tree. Returns <code>null</code> only if the 
	 * tree has no nodes. 
	 *
	 * @return			the root of the tree
	 * @see				javax.swing.tree.TreeModel
	 */
	public Object getRoot() { 
		Asset root = AssetFactory.getInstance().getAsset(1); 
		return root;
	}//end getRoot()

	/**
	 * Returns the asset that this node represents. For the root node, this is 
	 * the asset itself, but for the non-root nodes, they are asset links
	 * and the minor asset is the asset that we want.
	 *
	 * @param node		the node of the tree we are interested in
	 * @return			the asset for this node
	 */
	public Asset getAssetFromNode(Object node) {
		Asset nodeAsset = null;
		if (node instanceof Asset) {
			// this must be the root node/root folder asset
			nodeAsset = (Asset)node;
		} else {
			// other wise it is a link
			AssetLink link = (AssetLink)node;
			nodeAsset = link.getMinor();
		}
		return nodeAsset;
	}//end getAssetFromNode()
	
	/**
	 * Returns the child of parent at index index in the parent's child array. 
	 * <code>parent</code> must be a node previously obtained from this data 
	 * source. This should not return <code>null</code> if <code>index</code>
	 * is a valid index for <code>parent</code> 
	 * (that is <code>index >= 0 && index < getChildCount(parent)</code>). 
	 * 
	 * @param parent	a node in the tree, obtained from this data source 
	 * @return			the child of <code>parent</code> at 
	 *					index <code>index</code>
	 * @see				javax.swing.tree.TreeModel
	 */
	public Object getChild(Object parent, int index) { 
		Asset parentAsset = getAssetFromNode(parent);
		AssetLink childLink = parentAsset.getChildLinkAt(index);

		return childLink;
	}

	/**
	 * Returns the number of children of <code>parent</code>. Returns 
	 * <code>0</code> if the node is a leaf or if it has no children. 
	 * <code>parent</code> must be a node previously obtained from this data 
	 * source. 
	 * 
	 * @param parent	a node in the tree, obtained from this data source 
	 * @return			the number of children of the node <code>parent</code>
	 * @see				javax.swing.tree.TreeModel
	 */
	public int getChildCount(Object parent) { 
		Asset parentAsset = getAssetFromNode(parent);
		return parentAsset.getChildCount();
	}//end getChildCount()

	/**
	 * Returns <code>true</code> if node is a leaf. It is possible for this 
	 * method to return <code>false</code> even if node has 
	 * no children. A directory in a filesystem, for example, may contain no 
	 * files; the node representing the directory is not a leaf, but it also 
	 * has no children. 
	 * 
	 * @param node		a node in the tree, obtained from this data source
	 * @return			<code>true</code> if <code>node</code> is a leaf
	 * @see				javax.swing.tree.TreeModel
	 */
	public boolean isLeaf(Object node)  { 
		Asset asset = getAssetFromNode(node);
		return (asset.getChildCount() == 0);
	}//end isLeaf()

	/**
	 * Messaged when the user has altered the value for the item identified by 
	 * <code>path</code> to <code>newValue</code>. 
	 * If <code>newValue</code> signifies a truly new value the model should 
	 * post a <code>treeNodesChanged</code> event. 
	 * 
	 * @param path		path to the node that the user has altered
	 * @param newValue	the new value from the TreeCellEditor
	 * 
	 */
	public void valueForPathChanged(TreePath path, Object newValue) {
		// there should be no tree cell editor - so ignore this
	}// end valueForPathChanged()
  
          
	/**
	 * Returns the index of <code>child</code> in <code>parent</code>. If 
	 * <code>parent</code> is <code>null</code> or <code>child</code> is 
	 * <code>null</code>, returns -1.
	 * 
	 * @param parent	a node in the tree, obtained from this data source 
	 *					(either <code>Asset</code> for the root node or
	 *					<code>AssetLink</code> for any nodes under it.
	 * @param child		the node we are interested in, which should be of type 
	 *					<code>AssetLink</code>
	 * @return			the index of the child in parent, or -1 if either 
	 *					<code>child</code> or <code>parent</code> are 
	 *					<code>null</code>
	 * @see				javax.swing.tree.TreeModel
	 */
	public int getIndexOfChild(Object parent, Object child) { 
		Asset parentAsset = getAssetFromNode(parent);
		return parentAsset.getChildLinkIndex((AssetLink)child);
	}//end getIndexOfChild()

	/**
	 * Adds a listener for the TreeModelEvent posted after the tree changes. 
	 * 
	 * @param l			The TreeModelListener to add
	 * @see				javax.swing.event.TreeModelListener
	 * @see				AssetTreeModel#removeTreeModelListener 
	 *					removeTreeModelListener
	 */
	public void addTreeModelListener(TreeModelListener l){
		System.err.println ("tree model listener added: of type " + l.getClass().getName());
		listeners.add(l);
	}//end addTreeModelListener()

	/**
	 * Removes a listener previously added with 
	 * <code>addTreeModelListener</code>. 
	 * 
	 * @param l			The listener to remove
	 * @see				javax.swing.tree.TreeModel
	 * @see				AssetTreeModel#addTreeModelListener 
	 *					addTreeModelListener
	 */
	public void removeTreeModelListener(TreeModelListener l) {
		System.err.println ("tree model listener removed");
		listeners.remove(l);
	}//end removeTreeModelListener()


	/** end TreeModel interface **/

	/**
	 * need to have a few functions that notify tree model listeners when assets are updated
	 */

	/**
	 * Returns whether a particular node is enabled (according to what mode 
	 * the asset tree is in). 
	 * <p>For example, it could be in asset finding mode, and restricted to 
	 * a certain set of asset types.</p>
	 * 
	 */
	public boolean isNodeEnabled(Object node) {
		Asset asset = getAssetFromNode(node);
		if (restrictedAssetTypes == null) {
			return true;
		} else {
			return (restrictedAssetTypes.contains(asset.getType()));
		}
	}//end isNodeEnabled()

	/**
	 * Sets the restricted asset types to <code>assetTypes</code>. This will
	 * affect <code>isNodeEnabled</code>'s behaviour.
	 * 
	 * @param assetTypes	the set of <code>AssetType</code>s
	 * @see #isNodeEnabled
	 */
	public void setRestrictedAssetTypes(Set assetTypes) {
		restrictedAssetTypes = Collections.unmodifiableSet(assetTypes);
	}//end setRestrictedAssetTypes()

	/**
	 * Sets the restricted asset types to <code>assetTypes</code>. This will
	 * affect <code>isNodeEnabled</code>'s behaviour.
	 * 
	 * @param assetTypes	the set of <code>AssetType</code>s
	 * @see #isNodeEnabled isNodeEnabled
	 */
	public void clearRestrictedAssetTypes() {
		restrictedAssetTypes = null;
	}//end clearRestrictedAssetTypes()


}//end class
