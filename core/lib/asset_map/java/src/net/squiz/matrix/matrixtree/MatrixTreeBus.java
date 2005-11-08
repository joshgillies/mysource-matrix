

package net.squiz.matrix.matrixtree;

import java.util.*;
import java.awt.Color;
import javax.swing.tree.*;
import javax.swing.event.*;
import net.squiz.matrix.core.*;

/**
 * Handles the creation and messageing of <code>MatrixTree's</code>
 * in the system.
 * All trees that are created should use the <code>createTree</code> Method.
 *
 * @author Marc McIntyre <mmcintyre@squiz.net>
 */
public class MatrixTreeBus {

	private static List trees = new ArrayList();
	private static MatrixTree lastExpandedTree;
	private static MatrixTree lastCollapsedTree;
	private static TreeExpansionHandler teHandler = new TreeExpansionHandler();
	private static MatrixTreeCellRenderer cellRenderer = new MatrixTreeCellRenderer();
	private static MatrixTreeComm comm = new MatrixTreeComm();
	private static String[] restrictedTypes = new String[0];

	// cannot instantiate
	private MatrixTreeBus() {}

	public static MatrixTree getLastExpandedTree() {
		return lastExpandedTree;
	}

	public static MatrixTree getLastCollapsedTree() {
		return lastCollapsedTree;
	}

	public static MatrixTreeCellRenderer getCellRenderer() {
		return cellRenderer;
	}

	private static class TreeExpansionHandler implements TreeWillExpandListener {
		public void treeWillExpand(TreeExpansionEvent evt) {
			lastExpandedTree = (MatrixTree) evt.getSource();
		}

		public void treeWillCollapse(TreeExpansionEvent evt) {
			lastCollapsedTree = (MatrixTree) evt.getSource();
		}
	}

	public static void startAssetFinderMode(String[] assetTypes) {
		restrictedTypes = assetTypes;
		Iterator iterator = trees.iterator();
		while (iterator.hasNext()) {
			MatrixTree tree = (MatrixTree) iterator.next();
			tree.startAssetFinderMode();
			tree.repaint();
		}
	}

	public static void stopAssetFinderMode() {
		restrictedTypes = new String[0];
		Iterator iterator = trees.iterator();
		while (iterator.hasNext()) {
			MatrixTree tree = (MatrixTree) iterator.next();
			tree.stopAssetFinderMode();
			tree.repaint();
		}
	}

	/**
	 * Constructs a tree with the specified root node and returns it.
	 */
	public static MatrixTree createTree(MatrixTreeNode root) {
		DefaultTreeModel model = new DefaultTreeModel(root);
		MatrixTree tree = new MatrixTree(model);
		trees.add(tree);

		MatrixTreeModelBus.addToBus(model);
		tree.setCellRenderer(new MatrixTreeCellRenderer());
		tree.addTreeWillExpandListener(teHandler);
		tree.addNewLinkListener(comm);
		tree.addNewAssetListener(comm);
		tree.setRootVisible(false);
		tree.setShowsRootHandles(true);
		// MM: need to add a listener for the tree to fire events to
		// there only really needs to be one place listening to events, but
		// we could always add another manually to the tree

		return tree;
	}

	/**
	 * Constructs a simplified FinderTree with the specified root node and returns it.
	 */
	public static FinderTree createFinderTree(MatrixTreeNode root) {
		DefaultTreeModel model = new DefaultTreeModel(root);
		FinderTree tree = new FinderTree(model);
		trees.add(tree);

		MatrixTreeModelBus.addToBus(model);

		tree.setCellRenderer(new MatrixTreeCellRenderer());
		tree.setRootVisible(false);
		tree.setShowsRootHandles(true);
		
		//disable move, right click menu and delete key
		tree.setMoveEnabled(false);
		tree.removeKeyStroke("DELETE");

		return tree;
	}

	public static boolean typeIsRestricted(AssetType type) {
		if (restrictedTypes.length == 0)
			return true;
		for (int i = 0; i < restrictedTypes.length; i++) {
			if (type.isAncestor(AssetManager.getAssetType(restrictedTypes[i])))
				return true;
		}
		return false;
	}

	public static void destroyTree(MatrixTree tree) {
		MatrixTreeModelBus.removeFromBus((DefaultTreeModel) tree.getModel());
		trees.remove(tree);
		tree = null;
	}

	public static Iterator getTrees() {
		return trees.iterator();
	}
}
