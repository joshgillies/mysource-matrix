
package net.squiz.matrix.matrixtree;

import net.squiz.matrix.core.*;
import net.squiz.matrix.debug.*;
import java.util.*;
import net.squiz.matrix.inspector.*;
import javax.swing.tree.DefaultTreeModel;

/**
 * The <code>MatrixTreeModelBus</code> allows for multiple trees each with
 * their own models to share a common set of treenodes. The models can have
 * completly different root nodes.
 *
 * @author Marc McIntyre <mmcintyre@squiz.net>
 */
public class MatrixTreeModelBus {

	private static List models = new ArrayList();
	private static DefaultTreeModel[] components;

	// cannot instantiate
	private MatrixTreeModelBus() {}

	/**
	 * Adds a model to the bus system.
	 * @param model
	 */
	public static void addToBus(DefaultTreeModel model) {
		synchronized(models) {
			models.add(model);
			components = null;
		}
	}

	public static void removeFromBus(DefaultTreeModel model) {
		synchronized(models) {
			models.remove(model);
			components = null;
		}
	}

	public static DefaultTreeModel[] getBusComponents() {
		synchronized(models) {
			if (components == null) {
				components = (DefaultTreeModel[]) models.toArray(
					new DefaultTreeModel[models.size()]);
			}
			return components;
		}
	}

	public static void setRoot(MatrixTreeNode root) {
		for (int i = 0; i < components.length; i++) {
			((DefaultTreeModel) components[i]).setRoot(root);
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

			try {
				// we have to do this ourselves, as the node tree
				// is indepenant from the tree models
				parent.insert(newChild, index);
				int[] newIndexs = new int[1];
				newIndexs[0] = index;

				DefaultTreeModel[] components = getBusComponents();

				for (int i = 0; i < components.length; i++) {
					DefaultTreeModel model = components[i];
					// only fire the event if the node is below the models
					// current root
					if (((MatrixTreeNode) model.getRoot()).isNodeDescendant(parent))
						model.nodesWereInserted(parent, newIndexs);
				}
			} catch (Throwable t) {
				Log.log("Could Not insert node", MatrixTreeModelBus.class, t);
			}
	}



	public static void removeNodeFromParent(MatrixTreeNode child) {
		MatrixTreeNode parent = (MatrixTreeNode) child.getParent();
		if (parent == null)
			throw new IllegalArgumentException("node does not have a parent");
		try {

			/// TESTING

			MatrixTreeNode mParent = parent;
			MatrixTreeNode mChild = child;

			parent = getMirrorNode(parent);
			// TODO: this is a temp hack to get this working
			if (parent.getIndex(child) == -1)
				child = getMirrorNode(child);

			// END TESTING

			// we have to do this ourselves, as the node tree
			// is indepenant from the tree models
			int[] childIndex = new int[1];
			Object[] removedArray = new Object[1];
			childIndex[0] = parent.getIndex(child);

			parent.remove(childIndex[0]);

			removedArray[0] = child;

			DefaultTreeModel[] components = getBusComponents();

			for (int i = 0; i < components.length; i++) {
				DefaultTreeModel model = components[i];
				// only fire the event if the node is below the models
				// current root
				if (((MatrixTreeNode) model.getRoot()).isNodeDescendant(parent))
					model.nodesWereRemoved(parent, childIndex, removedArray);
			}
			Log.log("removing from " + parent + "(" + child + ")", MatrixTreeModelBus.class);

		} catch (Throwable t) {
			Log.log("Could not remove node" + t.getMessage(), MatrixTreeModelBus.class, t);
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

		DefaultTreeModel[] components = getBusComponents();
		for (int i = 0; i < components.length; i++) {
			DefaultTreeModel model = components[i];
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
