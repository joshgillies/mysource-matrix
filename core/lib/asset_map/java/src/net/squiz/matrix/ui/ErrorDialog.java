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
* $Id: ErrorDialog.java,v 1.4 2012/08/30 01:09:21 ewang Exp $
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
 * The ErrorDialog class is the delete confirmation popup when a a node(s) is
 * selected for deletion.
 *
 * @author Nathan de Vries <ndvries@squiz.net>
 */
public class ErrorDialog 	extends 	MatrixDialog
							implements 	MatrixConstants, KeyListener {

	private static Point prevScreenLocation = null;

	private ErrorDialog(String message, String title) {
		JPanel contentPane = new JPanel(new BorderLayout());
		setContentPane(contentPane);

		Border border = new EmptyBorder(1, 1, 1, 1);
		contentPane.setBorder(border);

		JPanel midPanel = new JPanel();
		midPanel.setBackground(MatrixLookAndFeel.PANEL_COLOUR);

		JLabel label = new JLabel(message);
		label.setFont(MatrixTreeBus.getActiveTree().getFontInUse());
		midPanel.add(label);

		contentPane.add(getTopPanel(title), BorderLayout.NORTH);
		contentPane.add(midPanel, BorderLayout.CENTER);


		JPanel bottomPanel = new JPanel();
		JLabel okButton = new JLabel();
		okButton.setIcon(GUIUtilities.getAssetMapIcon("ok.png"));
		closeOnClick(okButton, "ok");


	/*	// mouse events
		okButton.addMouseListener(new MouseAdapter(){
			public void mouseClicked(MouseEvent e){
				setCursor(DEFAULT_CURSOR);
				dispose();
			}

			public void mouseExited(MouseEvent e) {
				setCursor(DEFAULT_CURSOR);
				okButton.setIcon(GUIUtilities.getAssetMapIcon("ok.png"));
			}

			public void mouseEntered(MouseEvent e) {
				setCursor(HAND_CURSOR);
				okButton.setIcon(GUIUtilities.getAssetMapIcon("ok_on.png"));
			}
		});*/

		okButton.setOpaque(false);
		bottomPanel.add(okButton);
		bottomPanel.setBackground(MatrixLookAndFeel.PANEL_COLOUR);

		contentPane.add(bottomPanel, BorderLayout.SOUTH);


		contentPane.setBackground(MatrixLookAndFeel.PANEL_BORDER_COLOUR);
		enableDrag(contentPane);

		FontMetrics fm = getFontMetrics(label.getFont());
		int textWidth = fm.stringWidth(label.getText());
		setSize(textWidth + 32, 125);
		addKeyListener(this);

	}

	public void keyTyped(KeyEvent evt) {
	}

	public void keyPressed(KeyEvent evt) {
		if(evt.getKeyCode() == evt.VK_ENTER) {
			dispose();
		} else if(evt.getKeyCode() == evt.VK_ESCAPE) {
			dispose();
		}
	}

	public void keyReleased(KeyEvent evt) {
	}

	public void dispose() {
		prevScreenLocation = getPrevLoc();
		super.dispose();
	}

	/**
	 * Creates a new ErrorDialog if one does not exists, otherwise the
	 * existing ErrorDialog is brought to the front and given focus
	 *
	 * @return a new or existing ErrorDialog
	 */
	public static ErrorDialog getErrorDialog(String message, String title, Point locationOnScreen, Dimension treeDimension) {
		ErrorDialog ErrorDialog = null;

		if (MatrixDialog.hasDialog(ErrorDialog.class)) {
			ErrorDialog = (ErrorDialog) MatrixDialog.getDialog(ErrorDialog.class);
			ErrorDialog.dispose();
		}

		ErrorDialog = new ErrorDialog(message, title);
		MatrixDialog.putDialog(ErrorDialog);

		ErrorDialog.pack();
		ErrorDialog.centerDialogOnTree(locationOnScreen, treeDimension, prevScreenLocation);

		return ErrorDialog;
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
				MatrixTreeComm.createLink(NewLinkEvent.LINK_TYPE_MOVE, ErrorDialog.this.nodes, trashNode, 0, 0);

			}

			//TODO: (MM): need to call super.dispose(), maybe rename dispose() to disposeDialog()
			// in MatrixDialog and invoke it so it's clear that we are doing something different from dispose
			dispose();
		}
	}//end class ButtonActionListener
	*/

}//end class ErrorDialog
