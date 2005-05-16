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
	private JSplitPane pane;

	public BasicView() {
		construct();
	}

	private void construct() {
		JScrollPane scrollPane = new JScrollPane(constructTree());

		setLayout(new BorderLayout());
		add(new AssetMapMenuPanel(tree), BorderLayout.NORTH);
		add(MatrixStatusBar.createStatusBar(), BorderLayout.SOUTH);
		add(scrollPane);
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
}
