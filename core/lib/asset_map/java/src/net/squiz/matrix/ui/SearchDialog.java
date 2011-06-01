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
* $Id: SearchDialog.java,v 1.6 2006/12/05 05:26:37 bcaldwell Exp $
*
*/

package net.squiz.matrix.ui;

import net.squiz.matrix.plaf.MatrixLookAndFeel;
import net.squiz.matrix.matrixtree.*;
import net.squiz.matrix.core.*;
import javax.swing.*;
import java.awt.event.*;
import java.awt.*;
import javax.swing.border.*;
import java.util.*;

public class SearchDialog extends MatrixDialog {

	private JButton searchButton;
	private JButton cancelButton;
	private JTextField searchTerm;
	private JLabel message;
	private static Point prevScreenLocation = null;

	private SearchDialog(Point locationOnScreen, Dimension treeDimension) {
		JPanel contentPane = new JPanel(new BorderLayout());
		setContentPane(contentPane);
		contentPane.setBorder(new EmptyBorder(1, 1, 1, 1));

		contentPane.add(getTopPanel(Matrix.translate("asset_map_dialog_jump")),BorderLayout.NORTH);

		contentPane.add(getSearcFormPanel());
		contentPane.setBackground(MatrixLookAndFeel.PANEL_BORDER_COLOUR);
		enableDrag(contentPane);
	}


	public void dispose() {
		prevScreenLocation = getPrevLoc();
		super.dispose();
	}

	/**
	 * Creates a new SearchDialog if one does not exists, otherwise the
	 * existing SearchDialog is brought to the front and given focus
	 * @return a new or existing SearchDialog
	 */
	public static SearchDialog getSearchDialog(Point locationOnScreen, Dimension treeDimension) {

		SearchDialog searchDialog = null;
		if (!MatrixDialog.hasDialog(SearchDialog.class)) {
			searchDialog = new SearchDialog(locationOnScreen, treeDimension);
			MatrixDialog.putDialog(searchDialog);
		} else {
			// bring the seach dialog to the front
			searchDialog = (SearchDialog) MatrixDialog.getDialog(SearchDialog.class);
			searchDialog.toFront();
		}

		searchDialog.pack();
		searchDialog.centerDialogOnTree(locationOnScreen, treeDimension, prevScreenLocation);

		return searchDialog;
	}


	private JPanel getMessagePanel() {
		JPanel messagePanel = new JPanel();
		message = new JLabel("");
		messagePanel.add(message);

		return messagePanel;
	}

	private JPanel getSearcFormPanel() {
		// create a form panel
		JPanel searchForm = new JPanel();
		searchForm.setBackground(MatrixLookAndFeel.PANEL_COLOUR);

		// Text field settings
		searchTerm = new JTextField(5);
		searchTerm.setPreferredSize(new Dimension(150,15));
		searchTerm.setFont(MatrixTreeBus.getActiveTree().getFontInUse());
		searchTerm.setOpaque(false);
		searchTerm.addKeyListener(new KeyListener());
		searchTerm.setBorder( new LineBorder(MatrixLookAndFeel.PANEL_BORDER_COLOUR, 1) );

		// Label settings
		//JLabel label = new JLabel("Jump to ");
		//label.setFont(new Font("Arial",Font.PLAIN, 10));

		// add components to formPanel
		//searchForm.add(label);
		searchForm.add(searchTerm);

		return searchForm;
	}

	class ButtonHandler implements ActionListener {
		public void actionPerformed(ActionEvent evt) {
			Object source = evt.getSource();

			if (source == searchButton) {
				try {
					MatrixTree tree =  MatrixTreeBus.getActiveTree();
					MatrixTreeNode[] nodes = tree.getSelectionNodes();
					tree.loadChildAssets(nodes[0],"",Integer.parseInt(searchTerm.getText())-1,-1);
					dispose();
				} catch (NullPointerException ex) {
					message.setText(Matrix.translate("asset_map_error_invalid_node"));
				} catch (NumberFormatException ex) {
					message.setText(Matrix.translate("asset_map_error_invalid_number"));
				} catch (Exception ex) {
					Object[] transArgs = {
						ex.getMessage()
					};
					message.setText(Matrix.translate("asset_map_error_unknown_error", transArgs));
				}
			} else if (source == cancelButton) {
				dispose();
			}
		}
	}

	private class KeyListener extends KeyAdapter {
		public void keyPressed(KeyEvent evt) {
			if (evt.getSource() == searchTerm) {
				if (evt.getKeyCode() == evt.VK_ENTER) {
					MatrixTree tree =  MatrixTreeBus.getActiveTree();
					MatrixTreeNode[] nodes = tree.getSelectionNodes();
					if (nodes[0].getAsset().getNumKids() >= (Integer.parseInt(searchTerm.getText())-1)) {
						tree.loadChildAssets(nodes[0],"",Integer.parseInt(searchTerm.getText())-1,-1);
					}
					dispose();
				}else if(evt.getKeyCode() == evt.VK_ESCAPE) {
					dispose();
				}
			}
		}
	}
}

