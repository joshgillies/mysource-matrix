
package net.squiz.matrix.ui;

import net.squiz.matrix.core.*;
import javax.swing.*;
import java.awt.event.*;
import java.awt.*;
import javax.swing.border.*;
import java.util.*;

public class SearchDialog extends MatrixDialog {

	private JButton searchButton;
	private JButton cancelButton;

	private SearchDialog() {
		JPanel contentPane = new JPanel(new BorderLayout());
		setContentPane(contentPane);
		contentPane.setBorder(new EmptyBorder(0, 12, 12, 12));
		contentPane.add(getSearchPanel());
		contentPane.add(getButtonPanel(), BorderLayout.SOUTH);
		setSize(300, 200);
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

		return splitPane;
	}

	private JPanel getSearcFormPanel() {
		JPanel searchForm = new JPanel();
		JTextField searchTerm = new JTextField();
		searchForm.add(searchTerm);

		return searchForm;
	}

	private JPanel getButtonPanel() {
		ButtonHandler buttonHandler = new ButtonHandler();

		JPanel buttonPanel = new JPanel(new FlowLayout(FlowLayout.RIGHT));
		searchButton = new JButton(Matrix.translate("asset_map_button_search"));
		cancelButton = new JButton(Matrix.translate("asset_map_button_cancel"));

		searchButton.addActionListener(buttonHandler);
		cancelButton.addActionListener(buttonHandler);

		buttonPanel.add(searchButton);
		buttonPanel.add(cancelButton);

		return buttonPanel;
	}

	//private JPanel getResultsPanel() {
	//
//	}

	private JComboBox getTypeCodeSelector() {
		String[] assetTypes = AssetManager.getTypeCodeNames();
		Arrays.sort(assetTypes);
		JComboBox box = new JComboBox(assetTypes);

		return box;
	}

	class ButtonHandler implements ActionListener {
		public void actionPerformed(ActionEvent evt) {
			Object source = evt.getSource();

			if (source == searchButton) {

			} else if (source == cancelButton) {
				dispose();
			}
		}
	}
}
