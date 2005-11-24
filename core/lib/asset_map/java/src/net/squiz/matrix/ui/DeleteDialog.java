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
* $Id: DeleteDialog.java,v 1.5 2005/11/24 22:54:54 sdanis Exp $
*
*/

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
		if (nodes.length == 1) {
			Object[] transArgs = {
				new String(nodes[0].getAsset().getName())
			};
			label = new JLabel(Matrix.translate("asset_map_confirm_move_child", transArgs));
		} else {
			Object[] transArgs = {
				new Integer(nodes.length)
			};
			label = new JLabel(Matrix.translate("asset_map_confirm_move_children", transArgs));
		}
		contentPane.add( BorderLayout.CENTER, label);

		deleteBtn = new JButton(Matrix.translate("asset_map_button_delete"));
		cancelBtn = new JButton(Matrix.translate("asset_map_button_cancel"));

		//ActionListener btnListener = new ButtonActionListener();
		//deleteBtn.addActionListener(btnListener);
		deleteBtn.addActionListener(new java.awt.event.ActionListener() {
			public void actionPerformed(java.awt.event.ActionEvent evt) {
				btn_pressed(evt);
			}
		});


		deleteBtn.addKeyListener(new KeyAdapter() {
			public void keyPressed(KeyEvent evt) {
				key_pressed(evt);
			}
		});

		cancelBtn.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent evt) {
				dispose();
			}
		});

		cancelBtn.addKeyListener(new KeyAdapter() {
			public void keyPressed(KeyEvent evt) {
				dispose();
			}
		});

		//cancelBtn.addActionListener(btnListener);

		JPanel panel = new JPanel();
		panel.add(deleteBtn);
		panel.add(cancelBtn);

		contentPane.add( BorderLayout.SOUTH, panel );

		FontMetrics fm = getFontMetrics(label.getFont());
		int textWidth = fm.stringWidth(label.getText());
		setSize(textWidth + 32, 125);
	}

	private void btn_pressed(ActionEvent evt) {
		delete();
		dispose();
	}

	private void key_pressed(KeyEvent evt) {
		if(evt.getKeyCode() == evt.VK_ENTER) {
			if (evt.getSource() == deleteBtn) {
				delete();
			}
			dispose();
		} else if(evt.getKeyCode() == evt.VK_ESCAPE) {
			dispose();
		}
	}

	private void delete() {
		// there can only be on trash folder in the system.
		String[] assetids = AssetManager.getAssetsOfType("trash_folder");
		Asset trash = AssetManager.getAsset(assetids[0]);
		Iterator nodes = trash.getTreeNodes();
		MatrixTreeNode trashNode = null;
		while (nodes.hasNext()) {
			trashNode = (MatrixTreeNode) nodes.next();
		}
		MatrixTreeComm.createLink(NewLinkEvent.LINK_TYPE_MOVE, DeleteDialog.this.nodes, trashNode, 0, 0);
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

	/*class ButtonActionListener implements ActionListener {
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
				MatrixTreeComm.createLink(NewLinkEvent.LINK_TYPE_MOVE, DeleteDialog.this.nodes, trashNode, 0, 0);

			}

			//TODO: (MM): need to call super.dispose(), maybe rename dispose() to disposeDialog()
			// in MatrixDialog and invoke it so it's clear that we are doing something different from dispose
			dispose();
		}
	}//end class ButtonActionListener
	*/

}//end class DeleteDialog

