
package net.squiz.matrix.ui;

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

	private SearchDialog() {
		JPanel contentPane = new JPanel(new BorderLayout());
		setContentPane(contentPane);
		contentPane.setBorder(new EmptyBorder(12, 12, 12, 12));
		contentPane.add(getSearcFormPanel(), BorderLayout.NORTH);
		contentPane.add(getMessagePanel());
		contentPane.add(getButtonPanel(), BorderLayout.SOUTH);
		setSize(300, 180);
	}

	/**
	 * Creates a new SearchDialog if one does not exists, otherwise the
	 * existing SearchDialog is brought to the front and given focus
	 * @return a new or existing SearchDialog
	 */
	public static SearchDialog getSearchDialog() {

		SearchDialog searchDialog = null;
		if (!MatrixDialog.hasDialog(SearchDialog.class)) {
			searchDialog = new SearchDialog();
			MatrixDialog.putDialog(searchDialog);
		} else {
			// bring the seach dialog to the front
			searchDialog = (SearchDialog) MatrixDialog.getDialog(SearchDialog.class);
			searchDialog.toFront();
		}
		return searchDialog;
	}

	private JSplitPane getSearchPanel() {
		JSplitPane splitPane = new JSplitPane(JSplitPane.VERTICAL_SPLIT);

		splitPane.add(getSearcFormPanel(), JSplitPane.TOP);
		splitPane.add(getMessagePanel(), JSplitPane.BOTTOM);

		return splitPane;
	}

	private JPanel getMessagePanel() {
		JPanel messagePanel = new JPanel();
		message = new JLabel("");
		messagePanel.add(message);

		return messagePanel;
	}

	private JPanel getSearcFormPanel() {
		JPanel searchForm = new JPanel();
		searchTerm = new JTextField(10);
		JLabel label = new JLabel("Jump to ");
		searchForm.add(label);
		searchForm.add(searchTerm);

		return searchForm;
	}

	private JPanel getButtonPanel() {
		ButtonHandler buttonHandler = new ButtonHandler();

		JPanel buttonPanel = new JPanel(new FlowLayout(FlowLayout.RIGHT));
		searchButton = new JButton(Matrix.translate("asset_map_button_jump"));
		cancelButton = new JButton(Matrix.translate("asset_map_button_cancel"));

		searchButton.addActionListener(buttonHandler);
		cancelButton.addActionListener(buttonHandler);

		buttonPanel.add(searchButton);
		buttonPanel.add(cancelButton);

		return buttonPanel;
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
}
