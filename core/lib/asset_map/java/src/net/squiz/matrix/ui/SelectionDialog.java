/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: SelectionDialog.java,v 1.4 2012/08/30 01:09:21 ewang Exp $
*
*/

package net.squiz.matrix.ui;

import net.squiz.matrix.matrixtree.*;
import net.squiz.matrix.core.*;
import javax.swing.*;
import javax.swing.tree.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.border.*;

/**
 * @author Marc McIntyre <mmcintyre@squiz.net>
 */
public class SelectionDialog extends MatrixDialog implements MatrixConstants {

	private SelectionTree selectionTree;
	private DefaultMutableTreeNode parentNode;
	private JPopupMenu removeMenu;

	/*
	 * Constructs the selection dialog
	 */
	private SelectionDialog() {
		JPanel contentPane = new JPanel(new BorderLayout());
		setContentPane(contentPane);

		Border border = new CompoundBorder(
			new EmptyBorder(12, 12, 12, 12),
			new TitledBorder(Matrix.translate("asset_map_dialog_title_current_selection"))
		);
		Border border2 = new CompoundBorder(
			border,
			new EmptyBorder(8, 8, 8, 8)
		);
		contentPane.setBorder(border2);
		contentPane.add(getSelectionPanel());

		// create a menu for removing nodes
		JMenuItem removeItem = new JMenuItem(Matrix.translate("asset_map_menu_remove"));
		removeItem.addActionListener(new removeSelectionNodesAction());
		removeMenu = new JPopupMenu();
		removeMenu.add(removeItem);

		setSize(200, 300);
	}

	/**
	 * Creates a new SelectionDialog if one does not exists, otherwise the
	 * existing SearchDialog is brought to the front and given focus
	 * @return a new or existing SelectionDialog
	 */
	public static SelectionDialog getSelectionDialog() {
		SelectionDialog selectionDialog = null;
		if (!MatrixDialog.hasDialog(SelectionDialog.class)) {
			selectionDialog = new SelectionDialog();
			MatrixDialog.putDialog(selectionDialog);
		} else {
			// bring the selection dialog to the front
			selectionDialog = (SelectionDialog) MatrixDialog.getDialog(SelectionDialog.class);
			selectionDialog.toFront();
		}
		return selectionDialog;
	}

	/*
	 * We have this method because we want to update the model every
	 * time except when we create the tree.
	 */
	private void setNodes(MatrixTreeNode[] nodes, boolean updateModel) {
		if (nodes == null)
			return;
		parentNode.removeAllChildren();
		for (int i = 0; i < nodes.length; i++) {
			parentNode.add(nodes[i]);
		}
		if (updateModel) {
			DefaultTreeModel model = (DefaultTreeModel) selectionTree.getModel();
			model.nodeStructureChanged(parentNode);
		}
	}

	/**
	 * Set the nodes for the selection tree to the specified nodes
	 * @param nodes the nodes to set the selection tree to
	 * @see removeAllNodes()
	 */
	public void setNodes(MatrixTreeNode[] nodes) {
		setNodes(nodes, true);
	}

	/**
	 * Removes all nodes from the selection tree
	 * @see setNodes(MatrixTreeNode[])
	 */
	public void removeAllNodes() {
		parentNode.removeAllChildren();
		((DefaultTreeModel) selectionTree.getModel()).nodeStructureChanged(parentNode);
	}

	/*
	 * Returns a panel with the tree and two buttons for removing nodes
	 * from the selection. The tree extends MatrixTree so the visual
	 * selection tool can be used to select nodes
	 */
	private JPanel getSelectionPanel() {
		MatrixTreeNode[] nodes = Selection.getNodes();

		if (nodes != null) {
			parentNode = new DefaultMutableTreeNode();
			setNodes(nodes, false);
			selectionTree = new SelectionTree(new DefaultTreeModel(parentNode));
			selectionTree.setRootVisible(false);
		}
		selectionTree.setCellRenderer(MatrixTreeBus.getCellRenderer());

		JScrollPane treePane = new JScrollPane(selectionTree);
		JButton removeAllBtn = new JButton(Matrix.translate("asset_map_button_remove_all"));
		JButton removeSelectionBtn = new JButton(Matrix.translate("asset_map_button_remove_selection"));

		removeAllBtn.addActionListener(new removeAllNodesAction());
		removeSelectionBtn.addActionListener(new removeSelectionNodesAction());

		GridBagLayout gb = new GridBagLayout();
		GridBagConstraints c = new GridBagConstraints();
		JPanel selectionPanel = new JPanel(gb);

		// add the selection panel so that it is maximum width and height
		c.fill = GridBagConstraints.BOTH;
		c.gridwidth = GridBagConstraints.REMAINDER;
		c.weightx = 1.0;
		c.weighty = 1.0;
		gb.setConstraints(treePane, c);
		selectionPanel.add(treePane);

		// add the buttons so they are maximum width and minimum height
		c.weightx = 0.0;
		c.weighty = 0.0;
		gb.setConstraints(removeAllBtn, c);
		selectionPanel.add(removeAllBtn);
		gb.setConstraints(removeSelectionBtn, c);
		selectionPanel.add(removeSelectionBtn);

		return selectionPanel;
	}

	// inner classes

	/**
	 * An ActionListener to remove nodes from the selection tree
	 */
	class removeSelectionNodesAction implements ActionListener {
		public void actionPerformed(ActionEvent evt) {
			TreePath[] paths = selectionTree.getSelectionPaths();
			if (paths == null)
				return;

			for (int i = 0; i < paths.length; i++) {
				MatrixTreeNode node = (MatrixTreeNode) paths[i].getLastPathComponent();
				((DefaultTreeModel) selectionTree.getModel()).removeNodeFromParent(node);
			}
			Selection.removeNodes(paths);
		}
	}//end class removeSelectionNodesAction

	/**
	 * An ActionListener to remove All nodes from the tree
	 */
	class removeAllNodesAction implements ActionListener {
		public void actionPerformed(ActionEvent evt) {
			removeAllNodes();
			Selection.removeAllNodes();
		}
	}//end class removeAllNodesAction

	/**
	 * A Tree that handles the selection and removal of tree nodes
	 * from the current selection. Nodes can be selected using the visual selection
	 * tool provided by MatrixTree and removed with a right click menu.
	 */
	class SelectionTree extends MatrixTree {

		/**
		 * Constructs a Selection tree and disables moving operations
		 */
		public SelectionTree(TreeModel model) {
			super(model);
			setMoveEnabled(false);
		}

		/**
		 * Returns the SelectionHandler to bring up the remove
		 * menu on right clicks
		 */
		protected MenuHandler getMenuHandler() {
			return new SelectionTreeMenuHandler();
		}

		/**
		 * The SelectionTreeMenuHandler handles the menu used to
		 * remove nodes from the selection tree
		 */
		protected class SelectionTreeMenuHandler extends MenuHandler {

			/**
			 * Returns the menu for single selections
			 * @return the menu for single selections
			 */
			protected JPopupMenu getMenuForSingleSelection() {
				return removeMenu;
			}

			/**
			 * Returns the menu for multiple selections
			 * @return the menu for multiple selections
			 */
			protected JPopupMenu getMenuForMultipleSelection() {
				return removeMenu;
			}

			/**
			 * Returns the menu for void space clicks
			 * @return the menu for void space clicks
			 */
			protected JPopupMenu getMenuForVoidSpace() {
				if (!isEmptySelection())
					return removeMenu;
				return null;
			}

			/**
			 * No ancillery items are used for the selection tree
			 * @return null as no items are used
			 */
			protected JMenuItem[] getAncillaryMenuItems() {
				return null;
			}

			/**
			 * No keyboard shortcuts are registered for the selection tree
			 */
			protected void setKeyBoardsActions() {}

		}
	}//end class SelectionTree
}
