
package net.squiz.matrix.ui;

import net.squiz.matrix.matrixtree.*;
import net.squiz.matrix.core.*;

import java.util.*;
import java.io.IOException;
import net.squiz.matrix.debug.*;
import javax.swing.*;
import javax.swing.border.*;

import java.awt.BorderLayout;
import java.awt.event.*;
import java.awt.FontMetrics;

/**
 * The DeleteDialog class is the delete confirmation popup when a a node(s) is
 * selected for deletion.
 *
 * @author Nathan de Vries <ndvries@squiz.net>
 */
public class DeleteDialog 	extends 	MatrixDialog
							implements 	MatrixConstants {

	private JButton deleteBtn;
	private JButton cancelBtn;
	private MatrixTreeNode[] nodes;

	private DeleteDialog(MatrixTreeNode[] nodes) {
		this.nodes = nodes;
		JPanel contentPane = new JPanel(new BorderLayout());
		setContentPane(contentPane);

		Border border = new EmptyBorder(8, 8, 8, 8);
		contentPane.setBorder(border);

		JLabel label;
		// MM: what if nodes.length == 0 ? or nodes == null ?
		if (nodes.length == 1)
			label = new JLabel("Are you sure you want move the asset \"" +
								nodes[0].getAsset().getName() +
								"\" to the Trash?");
		else
			label = new JLabel("Are you sure you want move " +
								nodes.length +
								" assets to the Trash?");
		contentPane.add( BorderLayout.CENTER, label);

		deleteBtn = new JButton( "Delete..." );
		cancelBtn = new JButton( "Cancel" );

		ActionListener btnListener = new ButtonActionListener();
		deleteBtn.addActionListener(btnListener);
		cancelBtn.addActionListener(btnListener);

		JPanel panel = new JPanel();
		panel.add(deleteBtn);
		panel.add(cancelBtn);

		contentPane.add( BorderLayout.SOUTH, panel );

		FontMetrics fm = getFontMetrics(label.getFont());
		int textWidth = fm.stringWidth(label.getText());
		setSize(textWidth + 32, 125);
	}


	/**
	 * Creates a new DeleteDialog if one does not exists, otherwise the
	 * existing DeleteDialog is brought to the front and given focus
	 *
	 * @return a new or existing DeleteDialog
	 */
	public static DeleteDialog getDeleteDialog(MatrixTreeNode[] nodes) {
		DeleteDialog deleteDialog = null;
		if (!MatrixDialog.hasDialog(DeleteDialog.class)) {
			deleteDialog = new DeleteDialog(nodes);
			MatrixDialog.putDialog(deleteDialog);
		} else {
			// bring the selection dialog to the front
			deleteDialog = (DeleteDialog) MatrixDialog.getDialog(DeleteDialog.class);
			deleteDialog.toFront();
		}
		return deleteDialog;
	}

	class ButtonActionListener implements ActionListener {
		public void actionPerformed(ActionEvent evt) {
			Object source = evt.getSource();

			if (source == deleteBtn) {
				// there can only be on trash folder in the system.
				String[] assetids = AssetManager.getAssetsOfType("trash_folder");
				Asset trash = AssetManager.getAsset(assetids[0]);
				Iterator nodes = trash.getTreeNodes();
				MatrixTreeNode trashNode = null;
				while (nodes.hasNext()) {
					trashNode = (MatrixTreeNode) nodes.next();
				}
				MatrixTreeComm.createLink(NewLinkEvent.LINK_TYPE_MOVE, DeleteDialog.this.nodes, trashNode, 0);
			
			}

			// MM: need to call super.dispose(), maybe rename dispose() to disposeDialog()
			// in MatrixDialog and invoke it so it's clear that we are doing something different from dispose
			dispose();
		}
	}//end class ButtonActionListener
}//end class DeleteDialog
