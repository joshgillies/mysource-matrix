
package net.squiz.matrix.matrixtree;

import net.squiz.matrix.core.*;
import java.util.*;
import net.squiz.matrix.inspector.*;
import javax.swing.tree.DefaultTreeModel;

/**
 * The <code>MatrixTreeBus</code> allows for multiple trees each with
 * their own models to share a common set of treenodes. The models can have
 * completly different root nodes.
 *
 * The collection class used to store the models in synchronized, and therefore
 * any model events fired before the adding or removing of the model should
 * ensure model integrity.
 *
 * @author Marc McIntyre <mmcintyre@squiz.net>
 */
public class MatrixTreeModelBus {

	private static List models = new Vector();
	
	// cannot instantiate
	private MatrixTreeModelBus() {}

	/**
	 * Adds a model to the bus system.
	 * @param model
	 */
	public static void addToBus(DefaultTreeModel model) {
		if (!models.contains(model))
			models.add(model);
	}

	public static void removeFromBus(DefaultTreeModel model) {
		models.remove(model);
	}
	
	public static Iterator getModels() {
		return models.iterator();
	}
	
	public static void setRoot(MatrixTreeNode root) {
		Iterator modelIterator = models.iterator();
		while (modelIterator.hasNext()) {
			DefaultTreeModel model = (DefaultTreeModel) modelIterator.next();
			model.setRoot(root);
		}
	}
	

	public static void moveNode(
		MatrixTreeNode child,
		MatrixTreeNode newParent,
		int index) {
			// if the node hasn't moved, just return
			if ((child.getParent() == newParent)
				&& newParent.getIndex(child) == index)
					return;

			removeNodeFromParent(child);
			insertNodeInto(child, newParent, index);
	}

	public static void insertNodeInto(
		MatrixTreeNode newChild,
		MatrixTreeNode parent,
		int index) {
			// we have to do this ourselves, as the node tree 
			// is indepenant from the tree models
			parent.insert(newChild, index);
			int[] newIndexs = new int[1];
			newIndexs[0] = index;

			Iterator modelIterator = models.iterator();
			while (modelIterator.hasNext()) {
				DefaultTreeModel model = (DefaultTreeModel) modelIterator.next();
				// only fire the event if the node is below the models
				// current root
				if (((MatrixTreeNode) model.getRoot()).isNodeDescendant(parent))
					model.nodesWereInserted(parent, newIndexs);
			}
	}

	

	public static void removeNodeFromParent(MatrixTreeNode child) {
		MatrixTreeNode parent = (MatrixTreeNode) child.getParent();
		if (parent == null)
			throw new IllegalArgumentException("node does not have a parent");
		
		/// TESTING

		parent = getMirrorNode(parent);
		child = getMirrorNode(child);
		
		// END TESTING
		
		// we have to do this ourselves, as the node tree 
		// is indepenant from the tree models
		int[] childIndex = new int[1];
		Object[] removedArray = new Object[1];
		childIndex[0] = parent.getIndex(child);
		parent.remove(childIndex[0]);
		
		removedArray[0] = child;
		
		Iterator modelIterator = models.iterator();
		while (modelIterator.hasNext()) {
			DefaultTreeModel model = (DefaultTreeModel) modelIterator.next();
			// only fire the event if the node is below the models
			// current root
			if (((MatrixTreeNode) model.getRoot()).isNodeDescendant(parent))
				model.nodesWereRemoved(parent, childIndex, removedArray);
		}
	}
	
	private static MatrixTreeNode getMirrorNode(MatrixTreeNode parent) {
		Asset parentAsset = AssetManager.getAsset(parent.getAsset().getId());
		Iterator iterator = parentAsset.getTreeNodes();
		MatrixTreeNode mirrorParent = null;
		
		while (iterator.hasNext()) {
			MatrixTreeNode nextNode = (MatrixTreeNode) iterator.next();
			if (nextNode == parent)
				return parent;
			if (nextNode.getLinkid().equals(parent.getLinkid())) {
				mirrorParent = nextNode;
				break;
			}
		}
		return mirrorParent;
	}

	public static void nodeChanged(MatrixTreeNode node) {
		Iterator modelIterator = models.iterator();
		while (modelIterator.hasNext()) {
			DefaultTreeModel model = (DefaultTreeModel) modelIterator.next();
			// only fire the event if the node is below the models
			// current root
			if (((MatrixTreeNode) model.getRoot()).isNodeDescendant(node))
				model.nodeChanged(node);
		}
	}

	/**
	 * Invoke this method after you've changed how a set of noded are
	 * to be represented in the tree.
	 *
	 * This method is perferred if you have a multiple set of nodes
	 * that have been updated. If nodeChanged is called multiple times,
	 * the TreeUI will cause the tree to revalidate and repaint for every call.
	 *
	 */
/*	public static void nodesChanged() {
		// MM: i'm unsure why the DefaultTreeModel cannot update a set of
		// structure independant nodes without causing the tree to repaint for
		// each node, so we will have to do a little more work here
		Iterator modelIterator = models.iterator();
		while (modelIterator.hasNext()) {
			DefaultTreeModel model = (DefaultTreeModel) modelIterator.next();
			treeModeListeners[] listeners = model.getTreeModelListeners();
			for (int i = 0; i < listeners.length; i++) {
				//fuck'n
			}
		}
	}*/
}
