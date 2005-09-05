package net.squiz.matrix.assetmap;

import javax.swing.*;
import net.squiz.matrix.matrixtree.*;
import java.awt.*;
import java.awt.event.*;
import net.squiz.matrix.core.*;
import net.squiz.matrix.ui.*;
import javax.swing.plaf.*;
import net.squiz.matrix.plaf.*;

public class BasicView extends JPanel implements View {

	protected MatrixTree tree;
	private JSplitPane splitPane;

	public BasicView() {
		construct();
	}

	private void construct() {
		splitPane = new JSplitPane(JSplitPane.VERTICAL_SPLIT);
		splitPane.setUI(new MatrixSplitPaneUI());

		JScrollPane scrollPane = new JScrollPane(constructTree());
		scrollPane.setBorder(BorderFactory.createEmptyBorder());

		StatusKey statusKey = new StatusKey();

		splitPane.setTopComponent(scrollPane);
		splitPane.setBottomComponent(statusKey);
		splitPane.setOneTouchExpandable(true);
		splitPane.setDividerLocation(Integer.MAX_VALUE);

		setLayout(new BorderLayout());
		JPanel tabUnderlay = new JPanel();
		tabUnderlay.setBackground(MatrixLookAndFeel.PANEL_COLOUR);
		add(tabUnderlay, BorderLayout.WEST);
		add(new AssetMapMenuPanel(tree), BorderLayout.NORTH);
		add(MatrixStatusBar.createStatusBar(), BorderLayout.SOUTH);
		add(splitPane);

	}

	protected MatrixTree constructTree() {
		tree = MatrixTreeBus.createTree(new LoadingNode());
		return tree;
	}

	public MatrixTree getTree() {
		return tree;
	}

	public String getName() {
		return getAccessibleContext().getAccessibleName();
	}

	public void setName(String name) {
		getAccessibleContext().setAccessibleName(name);
	}

	public JComponent getViewComponent() {
		return this;
	}

	public JSplitPane getSplitPane() {
		return splitPane;
	}
}
