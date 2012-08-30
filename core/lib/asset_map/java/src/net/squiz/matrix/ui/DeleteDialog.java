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
* $Id: DeleteDialog.java,v 1.11 2012/08/30 01:09:21 ewang Exp $
*
*/

package net.squiz.matrix.ui;

import net.squiz.matrix.matrixtree.*;
import net.squiz.matrix.core.*;
import net.squiz.matrix.plaf.MatrixLookAndFeel;

import java.util.*;
import java.io.IOException;
import net.squiz.matrix.debug.*;
import javax.swing.*;
import javax.swing.border.*;

import java.awt.BorderLayout;
import java.awt.event.*;
import java.awt.FontMetrics;
import java.awt.*;

/**
 * The DeleteDialog class is the delete confirmation popup when a a node(s) is
 * selected for deletion.
 *
 * @author Nathan de Vries <ndvries@squiz.net>
 */
public class DeleteDialog 	extends 	MatrixDialog
							implements 	MatrixConstants, KeyListener {

	private JButton deleteBtn;
	private JButton cancelBtn;
	private MatrixTreeNode[] nodes;
	private static Point prevScreenLocation = null;

	private DeleteDialog(MatrixTreeNode[] nodes) {
		this.nodes = nodes;
		JPanel contentPane = new JPanel(new BorderLayout());
		setContentPane(contentPane);

		Border border = new EmptyBorder(1, 1, 1, 1);
		contentPane.setBorder(border);

		JPanel midPanel = new JPanel();
		midPanel.setBackground(MatrixLookAndFeel.PANEL_COLOUR);

		JLabel label;
		// MM: what if nodes.length == 0 ? or nodes == null ?
		if (nodes.length == 1) {
			Object[] transArgs = {
				new String(nodes[0].getName())
			};
			label = new JLabel(Matrix.translate("asset_map_confirm_move_child", transArgs));
		} else {
			Object[] transArgs = {
				new Integer(nodes.length)
			};
			label = new JLabel(Matrix.translate("asset_map_confirm_move_children", transArgs));
		}

		label.setFont(MatrixTreeBus.getActiveTree().getFontInUse());
		midPanel.add(label);

		contentPane.add(getTopPanel(Matrix.translate("asset_map_dialog_delete")), BorderLayout.NORTH);
		contentPane.add(midPanel, BorderLayout.CENTER);
		enableDrag(contentPane);

		JPanel bottomPanel = new JPanel();
		final JLabel deleteButton = new JLabel();
		deleteButton.setIcon(GUIUtilities.getAssetMapIcon("ok.png"));

		// mouse events
		deleteButton.addMouseListener(new MouseAdapter(){
			public void mouseClicked(MouseEvent e){
				setCursor(new Cursor(Cursor.DEFAULT_CURSOR));
				delete();
				dispose();
			}

			public void mouseExited(MouseEvent e) {
				deleteButton.setIcon(GUIUtilities.getAssetMapIcon("ok.png"));
				setCursor(new Cursor(Cursor.DEFAULT_CURSOR));
			}

			public void mouseEntered(MouseEvent e) {
				deleteButton.setIcon(GUIUtilities.getAssetMapIcon("ok_on.png"));
				setCursor(new Cursor(Cursor.HAND_CURSOR));
			}
		});

		deleteButton.setOpaque(false);
		bottomPanel.add(deleteButton);

		JLabel cancelButton = new JLabel();
		cancelButton.setIcon(GUIUtilities.getAssetMapIcon("cancel.png"));

		closeOnClick(cancelButton, "cancel");
		bottomPanel.add(cancelButton);

		bottomPanel.setBackground(MatrixLookAndFeel.PANEL_COLOUR);

		contentPane.add(bottomPanel, BorderLayout.SOUTH);


		contentPane.setBackground(MatrixLookAndFeel.PANEL_BORDER_COLOUR);

		FontMetrics fm = getFontMetrics(label.getFont());
		int textWidth = fm.stringWidth(label.getText());
		setSize(textWidth + 32, 125);
		addKeyListener(this);

	}

	public void dispose() {
		prevScreenLocation = getPrevLoc();
		super.dispose();
	}


	private void btn_pressed(ActionEvent evt) {
		delete();
		dispose();
	}

	public void keyTyped(KeyEvent evt) {
	}

	public void keyPressed(KeyEvent evt) {
		if(evt.getKeyCode() == evt.VK_ENTER || evt.getKeyCode() == evt.VK_SPACE) {
			delete();
			dispose();
		} else if(evt.getKeyCode() == evt.VK_ESCAPE) {
			dispose();
		}
	}

	public void keyReleased(KeyEvent evt) {
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
		MatrixTreeComm.createLink(NewLinkEvent.LINK_TYPE_MOVE, DeleteDialog.this.nodes, trashNode, 0, 0, null);
	}

	/**
	 * Creates a new DeleteDialog if one does not exists, otherwise the
	 * existing DeleteDialog is brought to the front and given focus
	 *
	 * @return a new or existing DeleteDialog
	 */
	public static DeleteDialog getDeleteDialog(MatrixTreeNode[] nodes, Point locationOnScreen, Dimension treeDimension) {
		DeleteDialog deleteDialog = null;
		if (!MatrixDialog.hasDialog(DeleteDialog.class)) {
			deleteDialog = new DeleteDialog(nodes);
			MatrixDialog.putDialog(deleteDialog);
		} else {
			// bring the selection dialog to the front
			deleteDialog = (DeleteDialog) MatrixDialog.getDialog(DeleteDialog.class);
			deleteDialog.toFront();
		}

		deleteDialog.pack();
		deleteDialog.centerDialogOnTree(locationOnScreen, treeDimension, prevScreenLocation);

		return deleteDialog;
	}

}//end class DeleteDialog
